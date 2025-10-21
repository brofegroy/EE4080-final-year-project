#ifndef E002F9B9_IEEE802154_MAC_HELPER_H
#define E002F9B9_IEEE802154_MAC_HELPER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//semi-opaque header file

//custom implementation
//assuming:
//    single PAN network
//    custom GTS implementation
//soft assuming:
//    this struct is zero initialised
typedef struct E002F9B9_packet {
    bool isPacketDropped;
    uint8_t frame_control_field[2];
    size_t frame_control_field_len;
    uint8_t seq_num;
    size_t seq_num_len;
    uint8_t src_addr[8]; // max IEEE 802.15.4 addr length
    size_t src_addr_len;
    uint8_t dst_addr[8];
    size_t dst_addr_len;
    uint8_t aux_sec_header_field[14];
    size_t aux_sec_header_field_len;
    uint8_t superframe_specification_field[2];
    size_t superframe_specification_field_len;
    uint8_t custom_gts_specification_field[1];
    size_t custom_gts_specification_field_len;
    uint8_t custom_gts_list_field[28];
    size_t custom_gts_list_field_len;    
    uint8_t frame_payload[127];
    size_t frame_payload_len;
} E002F9B9_packet_t;

bool E002F9B9_initialise_packet(E002F9B9_packet_t *pkt, const uint8_t *data, size_t data_len);

void E002F9B9_print_rx_packet(const E002F9B9_packet_t *pkt);

#endif //E002F9B9_IEEE802154_MAC_HELPER_H