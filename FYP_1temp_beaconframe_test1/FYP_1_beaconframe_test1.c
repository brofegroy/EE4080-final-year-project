#include <deca_device_api.h>
#include <deca_regs.h>
#include <deca_spi.h>
#include <port.h>
#include <shared_defines.h>

//zephyr includes
#if __has_include(<zephyr/kernel.h>)
// @suppress "Cannot open source file"
#include <zephyr/kernel.h>
#endif

#define APP_NAME "FYP_1_beaconframe_test1"

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(FYP_1_beaconframe_test1);
//-----------------
#include "../common/E002F9B9_ieee802154_mac_constants.h"

/*
ASSUMPTIONS
standard beacon format, not enhanced IEEE pg169
security enabled = false
frame pending = false
AR = false
no IE for now as no additional information needs to be carried at lin layer for now
assume single PAN
using short addresses for this implmentation, up to 65565 devices supported
there is no hiher layer than the MC layer currently so beacon payload will be empty
assume address of source/beacon/coordinator are 0
assume broadcast address is 0
Omit superframe specification and GTS in this example for simplicity, also because is not manditory in version 0b10
beacon frame taken as ALOHA, always clear, no need CCA

Current version:
assumes no Reduced function device (RFD) will request GTS in this version, 
only generating the base beacon frame.
*/

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    9,               /* Channel number. */
    DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard 8 symbol SFD, 
                      *   1 to use non-standard 8 symbol, 
                      *   2 for non-standard 16 symbol SFD and 
                      *   3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,      /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    DWT_PHRRATE_STD, /* PHY header rate. */
    (129 + 8 - 8),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). 
                      *  Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,/* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0,      /* PDOA mode off */
};

/* Index to access to sequence number of the blink frame in the tx_msg array. */
#define BLINK_FRAME_SN_IDX 1

/* Inter-frame delay period, in milliseconds.
 * this example will try to transmit a frame every 100 ms*/
#define TX_DELAY_MS 2

// no cca
// /* Initial backoff period when failed to transmit a frame due to preamble detection. */
// #define INITIAL_BACKOFF_PERIOD 400 /* This constant would normally smaller (e.g. 1ms),
//                                     * however here it is set to 400 ms so that
//                                     * user can see (on Zephyr RTT console) the report 
//                                     * that the CCA detects a preamble on the air occasionally,
//                                     * and is doing a TX back-off.
//                                     */

int tx_sleep_period; /* Sleep period until the next TX attempt */

/* holds copy of status register */
uint32_t status_reg = 0;
uint32_t status_regh = 0; /* holds the high 32 bits of SYS_STATUS_HI */

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and 
 * power of the spectrum at the current temperature. 
 * These values can be calibrated prior to taking reference measurements. 
* See NOTE 3 below. */
extern dwt_txconfig_t txconfig_options;


int app_main(void)
{
    /* Display application name. */
    LOG_INF(APP_NAME);

    /* Configure SPI rate, DW3000 supports up to 38 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

    /* Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC,
     * or could wait for SPIRDY event) */
    Sleep(2); 

    /* Need to make sure DW IC is in IDLE_RC before proceeding */
    while (!dwt_checkidlerc()) { /* spin */ };

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        LOG_ERR("INIT FAILED");
        while (1) { /* spin */ };
    }

    // txconfig_options.power = 0x00fefefe;   // was 0x00fe0000
    // printk("txconfig_option.power = %x \n", txconfig_options.power);
    // printk("txconfig_option.delay = %x \n", txconfig_options.PGdly);
    // printk("txconfig_option.count = %x \n", txconfig_options.PGcount);

    /* Enabling LEDs here for debug so that for each TX the D1 LED will flash
     * on DW3000 red eval-shield boards. */
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK) ;

    /* Configure DW IC. See NOTE 7 below. */
    /* If the dwt_configure returns DWT_ERROR either the PLL or RX calibration
     * has failed the host should reset the device */
    if (dwt_configure(&config)) 
    {
        LOG_ERR("CONFIG FAILED");
        while (1) { /* spin */ };
    }

    /* Configure the TX spectrum parameters (power, PG delay and PG count) */
    dwt_configuretxrf(&txconfig_options);

    /* Configure preamble timeout to 3 PACs; if no preamble detected in this 
     * time we assume channel is clear. See NOTE 4*/
    dwt_setpreambledetecttimeout(3);

    /* The frame sent in this example is an 802.15.4 standard blink. 
    * It is a 12-byte frame composed of the following fields:
    *     - byte 0: frame type (0x00 for a beacon). -page 162 of IEEE802
    *     - byte 1: sequence number, incremented for each new frame.
    *     - byte 10/11: frame check-sum, automatically set/added by DW3000.  */
    // static uint8_t tx_msg[] = {beacon_fcf, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E'};

    //Forming the initial Sequence number of the frame
    uint8_t beacon_seq_num_field = 0;

    /* Loop forever sending BEACON frames periodically. */
    while(1)
    {
        ///Forming the frame to be sent

        //Forming the frame control field of the frame
        uint16_t beacon_frame_control_field = 
            E002F9B9_MAC__FCF__FRAME_TYPE_BEACON       | // beacon frame always 0
            E002F9B9_MAC__FCF__SECURITY_ENABLED_0      | // assume no security
            E002F9B9_MAC__FCF__FRAME_PENDING_0         | // beacon has pending data from recepient
            E002F9B9_MAC__FCF__ACK_REQUEST_0           | // beacon frames not require ack
            E002F9B9_MAC__FCF__PAN_ID_COMPRESSION_1    | // assumed single PAN for this implementation, so no need pan ID
            E002F9B9_MAC__FCF__SEQUENCE_SUPPRESS_0     | // reccomended to have sequence number for duplicate detection/missing frames
            E002F9B9_MAC__FCF__IE_PRESENT_0            | // no need for IE in this implementation
            E002F9B9_MAC__FCF__DEST_ADDR_MODE_SHORT_10 | // beacon frames not require destination
            E002F9B9_MAC__FCF__FRAME_VERSION_2020_10   | // self explanatory
            E002F9B9_MAC__FCF__SRC_ADDR_MODE_SHORT_01;   // can accomodate 65535 devices

        //Addressing Field 
        uint16_t beacon_addressing_field_source_address      = 0;      //assume address of beacon is 0
        uint16_t beacon_addressing_field_destination_address = 0xFFFF; //assume address of broadcast is 0xFFFF

        //Security field omitted

        int frame_length = 
            sizeof(beacon_frame_control_field) +
            sizeof(beacon_seq_num_field) +
            sizeof(beacon_addressing_field_source_address) +
            sizeof(beacon_addressing_field_destination_address) +
            //no security header
            //no superframe specification
            //no GTS info
            //no pending address field
            //no IE field
            //no beacon payload
            FCS_LEN;

        uint8_t tx_msg[] = {
            (uint8_t)(beacon_frame_control_field & 0xFF),                        // FCF LSB
            (uint8_t)((beacon_frame_control_field >> 8) & 0xFF),                 // FCF MSB
            beacon_seq_num_field,                                                // Sequence number
            (uint8_t)(beacon_addressing_field_source_address & 0xFF),            // Src addr LSB
            (uint8_t)((beacon_addressing_field_source_address >> 8) & 0xFF),     // Src addr MSB
            (uint8_t)(beacon_addressing_field_destination_address & 0xFF),       // Dest addr LSB
            (uint8_t)((beacon_addressing_field_destination_address >> 8) & 0xFF), // Dest addr MSB
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',//
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',//
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',//
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',//

        };

        if (SYS_STATUS_TXFRS_BIT_MASK)
        {
            /* Write frame data to DW3000 and prepare transmission..*/
            dwt_writetxdata(frame_length - FCS_LEN, tx_msg, 0); /* Zero offset in TX buffer. */
            dwt_writetxfctrl(frame_length, 0, 0); /* Zero offset in TX buffer, no ranging. */
            /* Start transmission. */ 
            dwt_starttx(DWT_START_TX_IMMEDIATE);

            tx_sleep_period = TX_DELAY_MS; /* sent a frame - set interframe period */
            // next_backoff_interval = INITIAL_BACKOFF_PERIOD; /* set initial backoff period */

            /* Increment the blink frame sequence number (modulo 256). */
            beacon_seq_num_field++; // increase seq number
            tx_msg[2]++;

            /* Reflect frame number */
            LOG_INF("frame: %d", (int) tx_msg[2]);
            LOG_HEXDUMP_INF(tx_msg, sizeof(tx_msg), "TX frame");
        }

        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
        { /* spin */ };

        /* Clear TX frame sent event. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

        /* Execute a delay between transmissions. */
        Sleep(tx_sleep_period);
    }
}