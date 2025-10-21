#include "E002F9B9_ieee802154_mac_helper.h"
#include "E002F9B9_ieee802154_mac_constants.h"
#include <stdio.h>
#include <stdlib.h>

#pragma region "packet processing helper"

// //comment this struct out, this is only for reference
// typedef struct E002F9B9_packet {
//     bool isPacketDropped;
//     uint8_t frame_control_field[2];
//     size_t frame_control_field_len;
//     uint8_t seq_num;
//     size_t seq_num_len;
//     uint8_t src_addr[8]; // max IEEE 802.15.4 addr length
//     size_t src_addr_len;
//     uint8_t dst_addr[8];
//     size_t dst_addr_len;
//     uint8_t aux_sec_header_field[14];
//     size_t aux_sec_header_field_len;
//     uint8_t superframe_specification_field[2];
//     size_t superframe_specification_field_len;
//     uint8_t custom_gts_specification_field[1];
//     size_t custom_gts_specification_field_len;
//     uint8_t custom_gts_list_field[28];
//     size_t custom_gts_list_field_len;    
//     uint8_t frame_payload[127];
//     size_t frame_payload_len;
// } E002F9B9_packet_t;

#pragma region initialise packet
    bool E002F9B9_initialise_beacon_packet(E002F9B9_packet_t *pkt, const uint8_t *data, size_t data_len)
    {
        uint16_t packet_byte_currently_read = 0;
        size_t required_data_len = 2/*fcf*/ + 1/*seq num*/ +0/*src&addr len added later*/ + 2/*superframe spec*/ + 1/*GTS specification*/;
        if (data_len<required_data_len){
            pkt->isPacketDropped = true;
            return;
        }

        //assign fcf
        pkt->frame_control_field[0] = data[0];
        pkt->frame_control_field[1] = data[1];
        pkt->frame_control_field_len = 2;

        //assign seq num
        pkt->seq_num = data[2];
        pkt->seq_num_len = 1;
        packet_byte_currently_read = 2 + 1; // FCF + Seq Num

        //check and update pkt->src_addr_len and pkt->dst_addr_len
        #pragma region "Address Length Checks"
        uint16_t src_type = (pkt->frame_control_field[1]<<8)&E002F9B9_MAC__FCF__SRC_ADDR_MODE_BITMASK; //only second half needed
        uint16_t dst_type = (pkt->frame_control_field[1]<<8)&E002F9B9_MAC__FCF__DST_ADDR_MODE_BITMASK; //only second half needed
        switch (src_type) {
            case E002F9B9_MAC__FCF__SRC_ADDR_MODE_SHORT_01: // short address 2 bytes
                required_data_len += 2;
                pkt->src_addr_len = 2;
                break;  
            case E002F9B9_MAC__FCF__SRC_ADDR_MODE_EXTENDED_11: // extended address
                required_data_len += 8;
                pkt->src_addr_len = 8;
                break;
            case E002F9B9_MAC__FCF__SRC_ADDR_MODE_NONE_00: //no address
                pkt->src_addr_len = 0;
                break;
            default:   //invalid
                pkt->isPacketDropped = true;
                return;
        }
        switch (dst_type) {
            case E002F9B9_MAC__FCF__DST_ADDR_MODE_SHORT_10: // short address 2 bytes
                required_data_len += 2;
                pkt->dst_addr_len = 2;
                break;  
            case E002F9B9_MAC__FCF__DST_ADDR_MODE_EXTENDED_11: // extended address
                required_data_len += 8;
                pkt->dst_addr_len = 8;
                break;
            case E002F9B9_MAC__FCF__DST_ADDR_MODE_NONE_00: //no address
                break;
            default:   //invalid
                pkt->isPacketDropped = true;
                return;
        }
        if (data_len<required_data_len){
            pkt->isPacketDropped = true;
            return;
        }
        #pragma endregion "Address Length Checks"
        
        //assign src addr
        memcpy(pkt->src_addr, data + packet_byte_currently_read, pkt->src_addr_len);
        packet_byte_currently_read += pkt->src_addr_len;

        //assign dst addr
        memcpy(pkt->dst_addr, data + packet_byte_currently_read, pkt->dst_addr_len);
        packet_byte_currently_read += pkt->dst_addr_len;

        //TODO maybe, security header always doesnt exist for now

        //superframe spec
        pkt->superframe_specification_field[0] = data[packet_byte_currently_read];
        pkt->superframe_specification_field[1] = data[packet_byte_currently_read+1];
        pkt->superframe_specification_field_len = 2; //unnecessary, superframe spec is always 2 length
        packet_byte_currently_read += 2;

        //check GTS field length and update pkt->custom_gts_list_field_len
        uint8_t gts_descriptor_count =  data[packet_byte_currently_read]&0b111;
        required_data_len += 4*gts_descriptor_count;
        if (data_len<required_data_len){
            pkt->isPacketDropped = true;
            return;
        }
        pkt->custom_gts_specification_field_len = 1; //redundant, always 1
        pkt->custom_gts_list_field_len = gts_descriptor_count*4;
        
        //assign GTS and GTS list
        pkt->custom_gts_specification_field[0] = data[packet_byte_currently_read];
        packet_byte_currently_read += 1;
        memcpy(pkt->custom_gts_list_field, data + packet_byte_currently_read, pkt->custom_gts_list_field_len);
        packet_byte_currently_read += pkt->custom_gts_list_field_len;

        //assign payload
        pkt->frame_payload_len = data_len - required_data_len;
        memcpy(pkt->frame_payload, data + packet_byte_currently_read, pkt->frame_payload_len);
        packet_byte_currently_read += pkt->frame_payload_len; //last entry, redundant
    }

    bool E002F9B9_initialise_packet(E002F9B9_packet_t *pkt, const uint8_t *data, size_t data_len)
    {
        if (!pkt || !data){
            pkt->isPacketDropped = true;
            return false;
        }

        uint8_t frame_type = data[0] & 0b00000111;

        switch (frame_type)
        {
            case E002F9B9_MAC__FCF__FRAME_TYPE_BEACON:
                E002F9B9_initialise_beacon_packet(pkt, data, data_len);
                break;
            //TODO
            // case E002F9B9_MAC__FCF__FRAME_TYPE_DATA:
            //     break;

            // case E002F9B9_MAC__FCF__FRAME_TYPE_ACK:
            //     break;

            // case E002F9B9_MAC__FCF__FRAME_TYPE_MAC_CMD:
            //     break;
            
            default:
                pkt->isPacketDropped = true;
                return false;
        }

        return !pkt->isPacketDropped;
    }
#pragma endregion initialise packet

//for debugging only
void E002F9B9_print_rx_packet(const E002F9B9_packet_t *pkt) {
    // Print Frame Control Field
    printk("FCF  = ");
    for (size_t i = 0; i < pkt->frame_control_field_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->frame_control_field[i] >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");

    // Print Sequence Number
    printk("seqnum = ");
    for (size_t i = 0; i < pkt->seq_num_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->seq_num >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");

    // Print Source Address
    printk("src_addr = ");
    for (size_t i = 0; i < pkt->src_addr_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->src_addr[i] >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");

    // Print Destination Address
    printk("dst_addr = ");
    for (size_t i = 0; i < pkt->dst_addr_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->dst_addr[i] >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");

    // Print Auxiliary Security Header
    printk("aux_sec_header_field = ");
    for (size_t i = 0; i < pkt->aux_sec_header_field_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->aux_sec_header_field[i] >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");

    // Print Superframe Specification Field
    printk("superframe_specification_field = ");
    for (size_t i = 0; i < pkt->superframe_specification_field_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->superframe_specification_field[i] >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");

    // Print Custom GTS Specification Field
    printk("custom_gts_specification_field = ");
    for (size_t i = 0; i < pkt->custom_gts_specification_field_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->custom_gts_specification_field[i] >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");

    // Print Custom GTS List Field
    printk("custom_gts_list_field = ");
    for (size_t i = 0; i < pkt->custom_gts_list_field_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->custom_gts_list_field[i] >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");

    // Print Frame Payload
    printk("frame_payload = ");
    for (size_t i = 0; i < pkt->frame_payload_len; i++) {
        for (int b = 7; b >= 0; b--) {
            printk("%d", (pkt->frame_payload[i] >> b) & 1);
        }
        printk(" ");
    }
    printk("\n");
}

#pragma region "packet processing helper"