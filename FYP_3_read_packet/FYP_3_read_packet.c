#pragma region includes
#include <deca_device_api.h>
#include <deca_regs.h>
#include <deca_spi.h>
#include <port.h>
#include <shared_defines.h>

//zephyr includes
#include <zephyr/kernel.h>

#define APP_NAME "FYP_3_read_packet"

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(FYP_3_read_packet);
//-----------------
#include "../common/E002F9B9_ieee802154_mac_constants.h"
#include "../common/E002F9B9_ieee802154_mac_helper.h"
#include "../common/E002F9B9_input.h"
#include "../common/E002F9B9_device_init.h"
#pragma endregion includes

/*
ASSUMPTIONS
for FCF
    standard beacon format, not enhanced IEEE pg169
    security enabled = false
    frame pending = false
    AR = false

assume single PAN
using short addresses for this implmentation, up to 65565 devices supported
there is no higher layer than the MC layer currently so beacon payload will be empty
assume address of source/beacon/coordinator are 0
assume broadcast address is 0
Omit superframe specification and GTS in this example for simplicity, also because is not manditory in version 0b10
beacon frame taken as ALOHA, always clear, no need CCA

Current version:
reads packets at 2 second intervals
*/

#pragma region global variables
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

int tx_sleep_period = 2; /* Sleep period after successful rx */

/* holds copy of status register */
uint32_t status_reg = 0;
uint32_t status_regh = 0; /* holds the high 32 bits of SYS_STATUS_HI */

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and 
 * power of the spectrum at the current temperature. 
 * These values can be calibrated prior to taking reference measurements.*/
extern dwt_txconfig_t txconfig_options;

static uint8_t rx_buffer[FRAME_LEN_MAX];

#pragma endregion global variables


int app_main(void)
{
    LOG_INF(APP_NAME);
    E002F9B9_device_init();
    LOG_INF("Ready to Receive");

    while (TRUE)
    {
        //clear rx buffer
        memset(rx_buffer, 0, sizeof(rx_buffer));

        //set state to listen
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        uint16_t frame_len;

        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR )))
        { /* spin */ };

        if (status_reg & SYS_STATUS_ALL_RX_ERR) {
            if (status_reg & SYS_STATUS_RXPHE_BIT_MASK)  LOG_ERR("receive error: RXPHE");  // Phy. Header Error
            if (status_reg & SYS_STATUS_RXFCE_BIT_MASK)  LOG_ERR("receive error: RXFCE");  // Rcvd Frame & CRC Error
            if (status_reg & SYS_STATUS_RXFSL_BIT_MASK)  LOG_ERR("receive error: RXFSL");  // Frame Sync Loss
            if (status_reg & SYS_STATUS_RXSTO_BIT_MASK)  LOG_ERR("receive error: RXSTO");  // Rcv Timeout
            if (status_reg & SYS_STATUS_ARFE_BIT_MASK)   LOG_ERR("receive error: ARFE");   // Rcv Frame Error
            if (status_reg & SYS_STATUS_CIAERR_BIT_MASK) LOG_ERR("receive error: CIAERR"); //
        }

        if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) {

            //A frame has been received, copy it to our local buffer.
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_BIT_MASK;
            if (frame_len <= FRAME_LEN_MAX) {
                dwt_readrxdata(rx_buffer, frame_len-FCS_LEN, 0); //No need to read the FCS/CRC.
            }

            //Clear good RX frame event in the DW IC status register.
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

            E002F9B9_packet_t pkt = {0};
            E002F9B9_initialise_packet(&pkt, rx_buffer, frame_len);
            E002F9B9_print_rx_packet(&pkt);
            k_sleep(K_SECONDS(tx_sleep_period));
        }
        else {
            //Clear RX error events in the DW IC status register.
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        }
    }
}