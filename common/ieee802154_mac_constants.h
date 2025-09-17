#ifndef IEEE802154_MAC_CONSTANTS_H
#define IEEE802154_MAC_CONSTANTS_H

#ifdef namespace_E002F9B9
#error "namespace E002F9B9" is already used 
#endif //namespace_E002F9B9
#define namespace_E002F9B9

#include <stdint.h>

// how are variable named:
// E002F9B9_MAC__<octet name>__<bit name>_<value>

#pragma region MAC sublayer constants //IEEE pg 377 8.4.2
    #define E002F9B9_aBaseSlotDuration              60
    #define E002F9B9_aGtsDescPersistenceTime        4
    #define E002F9B9_aMaxLostBeacons                4
    #define E002F9B9_aMaxSifsFrameSize              18
    #define E002F9B9_aMinCapLength                  440
    #define E002F9B9_aNumSuperframeSlots            16
    #define E002F9B9_aBaseSuperframeDuration        E002F9B9_aBaseSlotDuration * E002F9B9_aNumSuperframeSlots
    // #define E002F9B9_aUnitBackoffPeriod          aTurnaroundTime + aCcaTime.
    #define E002F9B9_aRccnBaseSlotDuration          60
#pragma endregion MAC sublayer constants



#pragma region Frame Control Field //IEEE pg 162
//---------------------------------------------------------------

    // Frame type (bits 0-2)
    #define E002F9B9_MAC__FCF__FRAME_TYPE_BEACON        0b000
    #define E002F9B9_MAC__FCF__FRAME_TYPE_DATA          0b001
    #define E002F9B9_MAC__FCF__FRAME_TYPE_ACK           0b010
    #define E002F9B9_MAC__FCF__FRAME_TYPE_MAC_CMD       0b011
    #define E002F9B9_MAC__FCF__FRAME_TYPE_RESERVED      0b100
    #define E002F9B9_MAC__FCF__FRAME_TYPE_MULTIPURPOSE  0b101
    #define E002F9B9_MAC__FCF__FRAME_TYPE_FRAGMENT      0b110
    #define E002F9B9_MAC__FCF__FRAME_TYPE_EXTENDED      0b111

    // Security Enabled (bit 3)
    #define E002F9B9_MAC__FCF__SECURITY_ENABLED_1    (1 << 3)
    #define E002F9B9_MAC__FCF__SECURITY_ENABLED_0    0b0

    // Frame Pending (bit 4)
    #define E002F9B9_MAC__FCF__FRAME_PENDING_1       (1 << 4)
    #define E002F9B9_MAC__FCF__FRAME_PENDING_0       0b0

    // Acknowledgment Request (bit 5)
    #define E002F9B9_MAC__FCF__ACK_REQUEST_1         (1 << 5)
    #define E002F9B9_MAC__FCF__ACK_REQUEST_0         0b0

    // PAN ID Compression (bit 6)
    #define E002F9B9_MAC__FCF__PAN_ID_COMPRESSION_1  (1 << 6)
    #define E002F9B9_MAC__FCF__PAN_ID_COMPRESSION_0  0b0

    // Bit 7 is reserved

    // Sequence Number Suppression (bit 8)
    #define E002F9B9_MAC__FCF__SEQUENCE_SUPPRESS_1   (1 << 8)
    #define E002F9B9_MAC__FCF__SEQUENCE_SUPPRESS_0   0b0

    // Information Elements Present (bit 9)
    #define E002F9B9_MAC__FCF__IE_PRESENT_1          (1 << 9)
    #define E002F9B9_MAC__FCF__IE_PRESENT_0          0b0

    // Destination Addressing Modes (bits 10-11)
    #define E002F9B9_MAC__FCF__DEST_ADDR_MODE_NONE_00      (0b00 << 10)
    #define E002F9B9_MAC__FCF__DEST_ADDR_MODE_SHORT_10     (0b10 << 10)
    #define E002F9B9_MAC__FCF__DEST_ADDR_MODE_EXTENDED_11  (0b11 << 10)

    // Frame version (bits 12-13)
    #define E002F9B9_MAC__FCF__FRAME_VERSION_2003_00     (0b00 << 12)
    #define E002F9B9_MAC__FCF__FRAME_VERSION_2006_01     (0b01 << 12)
    #define E002F9B9_MAC__FCF__FRAME_VERSION_2020_10     (0b10 << 12)
    #define E002F9B9_MAC__FCF__FRAME_VERSION_RESERVED_11 (0b11 << 12)

    // Source Addressing Modes (bits 14-15)
    #define E002F9B9_MAC__FCF__SRC_ADDR_MODE_NONE_00       (0b00 << 14)
    #define E002F9B9_MAC__FCF__SRC_ADDR_MODE_SHORT_01      (0b10 << 14)
    #define E002F9B9_MAC__FCF__SRC_ADDR_MODE_EXTENDED_11   (0b11 << 14)
//----------------------------------------------------------------------------------
#pragma endregion Frame Control Field

#pragma region Beacon Superframe Specification Field //IEEE pg 171
//--------------------------------------------------------
    /// Beacon Superframe Specifiation Field, Beacon Order (bits 0-3)
    #define E002F9B9_MAC__BSSF__BEACON_ORDER(n) \
    (__builtin_constant_p(n) ? \
        (static_assert((n) >= 0 && (n) <= 15, "Beacon Order must be 0-15"), \
            (n << 0)) : \
        (assert((n) >= 0 && (n) <= 15), \
            (n << 0)))

    /// Beacon Superframe Specifiation Field, Superframe Order (bits 4-7)
    #define E002F9B9_MAC__BSSF__SUPERFRAME_ORDER(n) \
    (__builtin_constant_p(n) ? \
        (static_assert((n) >= 0 && (n) <= 15, "Superframe Order must be 0-15"), \
            (n << 4)) : \
        (assert((n) >= 0 && (n) <= 15), \
            (n << 4)))

    /// Beacon Superframe Specifiation Field, Final CAP Slot (bits 8-11)
    #define E002F9B9_MAC__BSSF__FINAL_CAP_SLOT(n) \
    (__builtin_constant_p(n) ? \
        (static_assert((n) >= 0 && (n) <= 15, "Final CAP Slot must be 0-15"), \
            (n << 8)) : \
        (assert((n) >= 0 && (n) <= 15), \
            (n << 8)))

    // Battery Life Extension (bit 12)
    #define E002F9B9_MAC__BSSF__BATTERY_LIFE_EXTENSION_1 (1 << 12)
    #define E002F9B9_MAC__BSSF__BATTERY_LIFE_EXTENSION_0 0

    // Bit 13 is Reserved

    // PAN Coordinator (bit 14)
    #define E002F9B9_MAC__BSSF__IS_PAN_COORDINATOR_1 (1 << 14)
    #define E002F9B9_MAC__BSSF__IS_PAN_COORDINATOR_0 0

    // Association Permit (bit 15)
    #define E002F9B9_MAC__BSSF__ASSOCIATION_PERMIT_1 (1 << 15)
    #define E002F9B9_MAC__BSSF__ASSOCIATION_PERMIT_0 0
//--------------------------------------------------------
#pragma endregion Beacon Superframe Specification

#pragma region Beacon Pending Address Specification Field //IEEE pg 172
//--------------------------------------------------------
    /// Beacon Pending Address Specification Field,
    /// Number of Short Addresses Pending (bits 0-2)
    #define E002F9B9_MAC__BPASF__SHORT_ADDRESSES_PENDING_COUNT(n) \
        (__builtin_constant_p(n) ? \
            (static_assert((n) >= 0 && (n) <= 7, "Number of Short Addresses must be in range(0-7)"), \
                (n << 0)) : \
            (assert((n) >= 0 && (n) <= 7), \
                (n << 0)))

    // Bit 3 is Reserved

    /// Beacon Pending Address Specification Field,
    /// Number of Long Addresses Pending (bits 4-6)
    #define E002F9B9_MAC__BPASF__LONG_ADDRESSES_PENDING_COUNT(n) \
    (__builtin_constant_p(n) ? \
        (static_assert((n) >= 0 && (n) <= 7, "Number of Long Addresses must be in range(0-7)"), \
            (n << 4)) : \
        (assert((n) >= 0 && (n) <= 7), \
            (n << 4)))

    // Bit 7 is Reserved

//--------------------------------------------------------
#pragma endregion Beacon Pending Address Specification Field

#endif //IEEE802154_MAC_CONSTANTS_H