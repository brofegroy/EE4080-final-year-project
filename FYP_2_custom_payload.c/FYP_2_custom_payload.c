#include <deca_device_api.h>
#include <deca_regs.h>
#include <deca_spi.h>
#include <port.h>
#include <shared_defines.h>

//zephyr includes
#include <zephyr/kernel.h>

#define APP_NAME "FYP_2_custom_payload"

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(FYP_2_custom_payload);
//-----------------
#include "../common/E002F9B9_ieee802154_mac_constants.h"
#include "../common/E002F9B9_input.h"
#include "../common/E002F9B9_device_init.h"

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
    DWT_PDOA_M0      /* PDOA mode off */
};

/* Index to access to sequence number of the blink frame in the tx_msg array. */
#define BLINK_FRAME_SN_IDX 1

/* Inter-frame delay period, in milliseconds.
 * this example will try to transmit a frame every 100 ms*/
#define TX_DELAY_MS 1000


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
    LOG_INF(APP_NAME);
    E002F9B9_device_init();

    dwt_setpreambledetecttimeout(3);

    uint8_t beacon_seq_num_field = 0;

    const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    char line[CONFIG_CONSOLE_INPUT_MAX_LINE_LEN];

    /* Loop forever sending BEACON frames periodically. */
    while(1)
    {
        // Get a line from user
        getline_uart(uart_dev, line, sizeof(line));
        size_t line_len = strlen(line);

        // Echo back
        printk("You typed: %s\n", line);

        #pragma region cca
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

        uint8_t tx_msg_no_payload[] = {
            (uint8_t)(beacon_frame_control_field & 0xFF),                        // FCF LSB
            (uint8_t)((beacon_frame_control_field >> 8) & 0xFF),                 // FCF MSB
            beacon_seq_num_field,                                                // Sequence number
            (uint8_t)(beacon_addressing_field_source_address & 0xFF),            // Src addr LSB
            (uint8_t)((beacon_addressing_field_source_address >> 8) & 0xFF),     // Src addr MSB
            (uint8_t)(beacon_addressing_field_destination_address & 0xFF),       // Dest addr LSB
            (uint8_t)((beacon_addressing_field_destination_address >> 8) & 0xFF) // Dest addr MSB
        };

        uint8_t tx_msg[sizeof(tx_msg_no_payload) + line_len];
        memcpy(tx_msg, tx_msg_no_payload, sizeof(tx_msg_no_payload));
        memcpy(&tx_msg[sizeof(tx_msg_no_payload)], line, line_len);

        int frame_length = sizeof(tx_msg_no_payload) + line_len + FCS_LEN;



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
        #pragma endregion cca
    }
}