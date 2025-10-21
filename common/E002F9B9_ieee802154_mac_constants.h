#ifndef E002F9B9_IEEE802154_MAC_CONSTANTS_H
#define E002F9B9_IEEE802154_MAC_CONSTANTS_H

#include <stdint.h>
#include <assert.h>

// how are variable named:
// E002F9B9_MAC__<octet name>__<bit name>_<value>

///

#pragma region THROW_ERROR
    #define THROW_ERROR 0/0
#pragma endregion THROW_ERROR

#pragma region MAC Symbol periods
    #define E002F9B9_MAC_TPSYM_LONG        993.59   // HRP UWB, channel 9, preamble 127 (long)
    #define E002F9B9_MAC_TPSYM_LONG_PRF4   3974.36  // HRP UWB, channel 9, preamble 127, low PRF
    #define E002F9B9_MAC_TPSYM_SHORT       1017.63  // HRP UWB, channel 9, preamble 31 (short)
#pragma endregion MAC Symbol periods



#pragma region PHY sublayer constants //IEEE pg 460 
    #define E002F9B9_aTurnaroundTime                12
    #define E002F9B9_CcaTime                        8
#pragma endregion PHY sublayer constants

#pragma region MAC sublayer constants //IEEE pg 377 8.4.2
    /// note to self: these use symbol rate, associated with the preamble, not data rate.
    #define E002F9B9_aBaseSlotDuration              60
    #define E002F9B9_aGtsDescPersistenceTime        4
    #define E002F9B9_aMaxLostBeacons                4
    #define E002F9B9_aMaxSifsFrameSize              18
    #define E002F9B9_aMinCapLength                  440
    #define E002F9B9_aNumSuperframeSlots            16
    #define E002F9B9_aBaseSuperframeDuration        E002F9B9_aBaseSlotDuration * E002F9B9_aNumSuperframeSlots
    #define E002F9B9_aUnitBackoffPeriod             E002F9B9_aTurnaroundTime /*12 symbols*/ + E002F9B9_CcaTime /*8 symbols*/
    #define E002F9B9_aRccnBaseSlotDuration          60
#pragma endregion MAC sublayer constants

#pragma region MAC fields
    #pragma region Frame Control Field //IEEE pg 162
    //---------------------------------------------------------------

        // Frame type (bits 0-2)
        #define E002F9B9_MAC__FCF__FRAME_TYPE_BITSHIFT      0
        #define E002F9B9_MAC__FCF__FRAME_TYPE_BEACON        0b000
        #define E002F9B9_MAC__FCF__FRAME_TYPE_DATA          0b001
        #define E002F9B9_MAC__FCF__FRAME_TYPE_ACK           0b010
        #define E002F9B9_MAC__FCF__FRAME_TYPE_MAC_CMD       0b011
        #define E002F9B9_MAC__FCF__FRAME_TYPE_RESERVED      0b100
        #define E002F9B9_MAC__FCF__FRAME_TYPE_MULTIPURPOSE  0b101
        #define E002F9B9_MAC__FCF__FRAME_TYPE_FRAGMENT      0b110
        #define E002F9B9_MAC__FCF__FRAME_TYPE_EXTENDED      0b111

        // Security Enabled (bit 3)
        #define E002F9B9_MAC__FCF__SECURITY_ENABLED_BITSHIFT    3
        #define E002F9B9_MAC__FCF__SECURITY_ENABLED_1           (1 << E002F9B9_MAC__FCF__SECURITY_ENABLED_BITSHIFT)
        #define E002F9B9_MAC__FCF__SECURITY_ENABLED_0           0b0

        // Frame Pending (bit 4)
        #define E002F9B9_MAC__FCF__FRAME_PENDING_BITSHIFT       4
        #define E002F9B9_MAC__FCF__FRAME_PENDING_1              (1 << E002F9B9_MAC__FCF__FRAME_PENDING_BITSHIFT)
        #define E002F9B9_MAC__FCF__FRAME_PENDING_0              0b0

        // Acknowledgment Request (bit 5)
        #define E002F9B9_MAC__FCF__ACK_REQUEST_BITSHIFT         5
        #define E002F9B9_MAC__FCF__ACK_REQUEST_1                (1 << E002F9B9_MAC__FCF__ACK_REQUEST_BITSHIFT)
        #define E002F9B9_MAC__FCF__ACK_REQUEST_0                0b0

        // PAN ID Compression (bit 6)
        #define E002F9B9_MAC__FCF__PAN_ID_COMPRESSION_BITSHIFT  6
        #define E002F9B9_MAC__FCF__PAN_ID_COMPRESSION_1         (1 << E002F9B9_MAC__FCF__PAN_ID_COMPRESSION_BITSHIFT)
        #define E002F9B9_MAC__FCF__PAN_ID_COMPRESSION_0         0b0

        // Bit 7 is reserved

        // Sequence Number Suppression (bit 8)
        #define E002F9B9_MAC__FCF__SEQUENCE_SUPPRESS_BITSHIFT   8
        #define E002F9B9_MAC__FCF__SEQUENCE_SUPPRESS_1          (1 << E002F9B9_MAC__FCF__SEQUENCE_SUPPRESS_BITSHIFT)
        #define E002F9B9_MAC__FCF__SEQUENCE_SUPPRESS_0          0b0

        // Information Elements Present (bit 9)
        #define E002F9B9_MAC__FCF__IE_PRESENT_BITSHIFT 9 
        #define E002F9B9_MAC__FCF__IE_PRESENT_1            (1 << E002F9B9_MAC__FCF__IE_PRESENT_BITSHIFT)
        #define E002F9B9_MAC__FCF__IE_PRESENT_0            0b0

        // Destination Addressing Modes (bits 10-11)
        #define E002F9B9_MAC__FCF__DST_ADDR_MODE_BITSHIFT     10
        #define E002F9B9_MAC__FCF__DST_ADDR_MODE_BITMASK      (0b11 << E002F9B9_MAC__FCF__DST_ADDR_MODE_BITSHIFT)
        #define E002F9B9_MAC__FCF__DST_ADDR_MODE_NONE_00      (0b00 << E002F9B9_MAC__FCF__DST_ADDR_MODE_BITSHIFT)
        #define E002F9B9_MAC__FCF__DST_ADDR_MODE_SHORT_10     (0b10 << E002F9B9_MAC__FCF__DST_ADDR_MODE_BITSHIFT)
        #define E002F9B9_MAC__FCF__DST_ADDR_MODE_EXTENDED_11  (0b11 << E002F9B9_MAC__FCF__DST_ADDR_MODE_BITSHIFT)

        // Frame version (bits 12-13)
        #define E002F9B9_MAC__FCF__FRAME_VERSION_BITSHIFT     12
        #define E002F9B9_MAC__FCF__FRAME_VERSION_BITMASK      (0b11 << E002F9B9_MAC__FCF__FRAME_VERSION_BITSHIFT)
        #define E002F9B9_MAC__FCF__FRAME_VERSION_2003_00      (0b00 << E002F9B9_MAC__FCF__FRAME_VERSION_BITSHIFT)
        #define E002F9B9_MAC__FCF__FRAME_VERSION_2006_01      (0b01 << E002F9B9_MAC__FCF__FRAME_VERSION_BITSHIFT)
        #define E002F9B9_MAC__FCF__FRAME_VERSION_2020_10      (0b10 << E002F9B9_MAC__FCF__FRAME_VERSION_BITSHIFT)
        #define E002F9B9_MAC__FCF__FRAME_VERSION_RESERVED_11  (0b11 << E002F9B9_MAC__FCF__FRAME_VERSION_BITSHIFT)

        // Source Addressing Modes (bits 14-15)
        #define E002F9B9_MAC__FCF__SRC_ADDR_MODE_BITSHIFT      14
        #define E002F9B9_MAC__FCF__SRC_ADDR_MODE_BITMASK       (0b11 << E002F9B9_MAC__FCF__SRC_ADDR_MODE_BITSHIFT)
        #define E002F9B9_MAC__FCF__SRC_ADDR_MODE_NONE_00       (0b00 << E002F9B9_MAC__FCF__SRC_ADDR_MODE_BITSHIFT)
        #define E002F9B9_MAC__FCF__SRC_ADDR_MODE_SHORT_01      (0b10 << E002F9B9_MAC__FCF__SRC_ADDR_MODE_BITSHIFT)
        #define E002F9B9_MAC__FCF__SRC_ADDR_MODE_EXTENDED_11   (0b11 << E002F9B9_MAC__FCF__SRC_ADDR_MODE_BITSHIFT)
    //----------------------------------------------------------------------------------
    #pragma endregion Frame Control Field

    #pragma region Beacon Superframe Specification Field //IEEE pg 171
    //--------------------------------------------------------
        /// Beacon Superframe Specifiation Field, Beacon Order (bits 0-3)
        #define E002F9B9_MAC__BSSF__BEACON_ORDER_BITSHIFT 0
        #define E002F9B9_MAC__BSSF__BEACON_ORDER(n) (((n) >= 0 && (n) <= 15) ? \
            (n << E002F9B9_MAC__BSSF__BEACON_ORDER_BITSHIFT) : THROW_ERROR)

        /// Beacon Superframe Specifiation Field, Superframe Order (bits 4-7)
        #define E002F9B9_MAC__BSSF__SUPERFRAME_ORDER_BITSHIFT 4
        #define E002F9B9_MAC__BSSF__SUPERFRAME_ORDER(n) (((n) >= 0 && (n) <= 15) ? \
            (n << E002F9B9_MAC__BSSF__SUPERFRAME_ORDER_BITSHIFT) : THROW_ERROR)

        /// Beacon Superframe Specifiation Field, Final CAP Slot (bits 8-11)
        #define E002F9B9_MAC__BSSF__FINAL_CAP_SLOT_BITSHIFT 8
        #define E002F9B9_MAC__BSSF__FINAL_CAP_SLOT(n) (((n) >= 0 && (n) <= 15) ? \
            (n << E002F9B9_MAC__BSSF__FINAL_CAP_SLOT_BITSHIFT) : THROW_ERROR)

        // Battery Life Extension (bit 12)
        #define E002F9B9_MAC__BSSF__BATTERY_LIFE_EXTENSION_BITSHIFT 12
        #define E002F9B9_MAC__BSSF__BATTERY_LIFE_EXTENSION_1        (1 << E002F9B9_MAC__BSSF__BATTERY_LIFE_EXTENSION_BITSHIFT)
        #define E002F9B9_MAC__BSSF__BATTERY_LIFE_EXTENSION_0        0

        // Bit 13 is Reserved

        // PAN Coordinator (bit 14)
        #define E002F9B9_MAC__BSSF__IS_PAN_COORDINATOR_BITSHIFT     14
        #define E002F9B9_MAC__BSSF__IS_PAN_COORDINATOR_1            (1 << E002F9B9_MAC__BSSF__IS_PAN_COORDINATOR_BITSHIFT)
        #define E002F9B9_MAC__BSSF__IS_PAN_COORDINATOR_0            0

        // Association Permit (bit 15)
        #define E002F9B9_MAC__BSSF__ASSOCIATION_PERMIT_BITSHIFT     15
        #define E002F9B9_MAC__BSSF__ASSOCIATION_PERMIT_1            (1 << E002F9B9_MAC__BSSF__ASSOCIATION_PERMIT_BITSHIFT)
        #define E002F9B9_MAC__BSSF__ASSOCIATION_PERMIT_0            0
    //--------------------------------------------------------
    #pragma endregion Beacon Superframe Specification

    #pragma region Beacon Pending Address Specification Field //IEEE pg 172
    //--------------------------------------------------------
        /// Beacon Pending Address Specification Field,
        /// Number of Short Addresses Pending (bits 0-2)
        #define E002F9B9_MAC__BPASF__SHORT_ADDRESSES_PENDING_COUNT_BITSHIFT 0 
        #define E002F9B9_MAC__BPASF__SHORT_ADDRESSES_PENDING_COUNT(n) (((n) >= 0 && (n) <= 7) ? \
            ((n) << E002F9B9_MAC__BPASF__SHORT_ADDRESSES_PENDING_COUNT_BITSHIFT) : THROW_ERROR)

        // Bit 3 is Reserved

        /// Beacon Pending Address Specification Field,
        /// Number of Long Addresses Pending (bits 4-6)
        #define E002F9B9_MAC__BPASF__LONG_ADDRESSES_PENDING_COUNT_BITSHIFT 4
        #define E002F9B9_MAC__BPASF__LONG_ADDRESSES_PENDING_COUNT(n) (((n) >= 0 && (n) <= 7) ? \
            ((n) << E002F9B9_MAC__BPASF__LONG_ADDRESSES_PENDING_COUNT_BITSHIFT) : THROW_ERROR)

        // Bit 7 is Reserved

    //--------------------------------------------------------
    #pragma endregion Beacon Pending Address Specification Field

    #pragma region Custom GTS Specification Field //custom implementation, but has similarities with IEEE
    //--------------------------------------------------------
        /// Custom GTS Specification Field
        /// GTS Descriptor Count (bits 0-2)
        #define E002F9B9_CUSTOM_GTS__DESCRIPTOR_COUNT_BITSHIFT 0
        #define E002F9B9_CUSTOM_GTS__DESCRIPTOR_COUNT(n) (((n) >= 0 && (n) <= 7) ? \
            ((n) << E002F9B9_CUSTOM_GTS__DESCRIPTOR_COUNT_BITSHIFT) : THROW_ERROR)

        //Bit 3-6 is Reserved

        /// GTS Permit (bit 7)
        #define E002F9B9_CUSTOM_GTS__PERMIT_BITSHIFT 7
        #define E002F9B9_CUSTOM_GTS__PERMIT_ACCEPT_1 (0b1 << E002F9B9_CUSTOM_GTS__PERMIT_BITSHIFT)
        #define E002F9B9_CUSTOM_GTS__PERMIT_DENY_0   0b0
    //--------------------------------------------------------
    #pragma endregion Custom GTS Specification Field
    
#pragma endregion MAC fields

#endif //E002F9B9_IEEE802154_MAC_CONSTANTS_H