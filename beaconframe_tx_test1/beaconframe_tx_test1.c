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
#include <zephyr/sys/printk.h>

#define APP_NAME "beaconframe_test1"

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(APP_NAME);
//-----------------
#include "../common/ieee802154_mac_constants.h"

//Physical layer
/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    5,               /* Channel number. */
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

/*
ASSUMPTIONS
standard beacon format, not enhanced IEEE pg169
security enabled = false
frame pending = false
AR = false
assume single PAN
*/

//MAC layer
//structure of a message
typedef struct {
    uint16_t fcf;
    uint8_t seq_num;
    uint16_t dest_pan;
    uint16_t dest_addr;
    uint8_t src_addr[8];
} mac_header_t;
uint16_t beacon_fcf = 
    E002F9B9_MAC__FCF__FRAME_TYPE_BEACON       | // beacon frame always 0
    E002F9B9_MAC__FCF__SECURITY_ENABLED_0      | // assume no security
    E002F9B9_MAC__FCF__FRAME_PENDING_0         | // beacon has pending data from recepient
    E002F9B9_MAC__FCF__ACK_REQUEST_0           | // beacon frames not require ack
    E002F9B9_MAC__FCF__PAN_ID_COMPRESSION_1    | //  
    E002F9B9_MAC__FCF__SEQUENCE_SUPPRESS_0     | // 
    E002F9B9_MAC__FCF__IE_PRESENT_0            | // 
    E002F9B9_MAC__FCF__DEST_ADDR_MODE_SHORT_10 | // 
    E002F9B9_MAC__FCF__FRAME_VERSION_2020_10   | // 
    E002F9B9_MAC__FCF__SRC_ADDR_MODE_SHORT_01;   // 
/* The frame sent in this example is an 802.15.4 standard blink. 
 * It is a 12-byte frame composed of the following fields:
 *     - byte 0: frame type (0x00 for a beacon). -page 162 of IEEE802
 *     - byte 1: sequence number, incremented for each new frame.
 *     - byte 2 -> 9: device ID, see NOTE 2 below.
 *     - byte 10/11: frame check-sum, automatically set/added by DW3000.  */
static uint8_t tx_msg[] = {beacon_fcf, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E'};

/* Index to access to sequence number of the blink frame in the tx_msg array. */
#define BLINK_FRAME_SN_IDX 1

/* The real length that is going to be transmitted */
#define FRAME_LENGTH    (sizeof(tx_msg)+FCS_LEN) 

/* Inter-frame delay period, in milliseconds.
 * this example will try to transmit a frame every 100 ms*/
#define TX_DELAY_MS 100

/* Initial backoff period when failed to transmit a frame due to preamble detection. */
#define INITIAL_BACKOFF_PERIOD 400 /* This constant would normally smaller (e.g. 1ms),
                                    * however here it is set to 400 ms so that
                                    * user can see (on Zephyr RTT console) the report 
                                    * that the CCA detects a preamble on the air occasionally,
                                    * and is doing a TX back-off.
                                    */

int tx_sleep_period; /* Sleep period until the next TX attempt */

/* Next backoff in the event of busy channel detection by this pseudo CCA algorithm */
int next_backoff_interval = INITIAL_BACKOFF_PERIOD;

/* holds copy of status register */
uint32_t status_reg = 0;
uint32_t status_regh = 0; /* holds the high 32 bits of SYS_STATUS_HI */

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and 
 * power of the spectrum at the current temperature. 
 * These values can be calibrated prior to taking reference measurements. 
 * See NOTE 3 below. */
extern dwt_txconfig_t txconfig_options;