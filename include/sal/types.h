/***************************************************************************
 * 
 * This software is governed by the Broadcom Switch APIs license.
 * This license is set out in https://raw.githubusercontent.com/Broadcom-Network-Switching-Software/OpenNSA/master/Legal/LICENSE file.
 * 
 * Copyright 2005-2019 Broadcom Inc. All rights reserved.
 * 
 ***************************************************************************
 *
 * File:     types.h
 * Purpose:     SAL type definitions
 */

#ifdef SDK_SOURCE_CHECK
#include <sdk_source_check.h>
#endif

#ifndef _SAL_TYPES_H
#define _SAL_TYPES_H

#include <sal/compiler.h>

/*
 * Define platform-independent types
 */

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE            0
#endif

#ifndef NULL
#define NULL            0
#endif

#ifndef DONT_CARE
#define DONT_CARE        0
#endif

#define VOL            volatile

/*
 *  We need to skip these defs if we are compiling for the uKernel.
 */
#ifndef BCM_UKERNEL

/*
 * Define unsigned and signed integers with guaranteed sizes.
 * Adjust if your compiler uses different sizes for short or int.
 */

typedef unsigned char        uint8;        /* 8-bit quantity  */
typedef unsigned short        uint16;        /* 16-bit quantity */
typedef unsigned int        uint32;        /* 32-bit quantity */
typedef COMPILER_UINT64        uint64;        /* 64-bit quantity */

typedef signed char        int8;        /* 8-bit quantity  */
typedef signed short        int16;        /* 16-bit quantity */
typedef signed int        int32;        /* 32-bit quantity */
typedef COMPILER_INT64          int64;          /* 64-bit quantity */

#endif

#define SAL_UINT8_NOF_BITS  (8)
#define SAL_UINT16_NOF_BITS (16)
#define SAL_UINT32_NOF_BITS (32)
#define SAL_UINT64_NOF_BITS (64)

#define SAL_UINT8_MAX_BIT    (SAL_UINT8_NOF_BITS  - 1)
#define SAL_UINT16_MAX_BIT   (SAL_UINT16_NOF_BITS - 1)
#define SAL_UINT32_MAX_BIT   (SAL_UINT32_NOF_BITS - 1)
#define SAL_UINT64_MAX_BIT   (SAL_UINT64_NOF_BITS - 1)






#define SAL_BIT(x)  (1 << (x))
#define SAL_RBIT(x) (~(1<<(x)))

/*******************************************
 * SET ALL BITS, BEGINNING AT BIT X, TO 1  *
 */
#define SAL_FROM_BIT(x)     (SAL_RBIT(x) + 1)

/*******************************************
 * SET ALL BITS UP TO BIT X (NOT INCLUDING *
 * BIT X) TO 1.                            *
 */
#define SAL_UPTO_BIT(x)     (~(SAL_FROM_BIT(x)))

/**************************************************************
 * MACRO TO GET THE OFFSET OF AN ELEMENT IN A                 *
 * GIVEN STRUCTURE TYPE.                                      *
 * 'X' IS THE TYPE OF THE STRUCTURE.                          *
 * 'Y' IS THE THE ELEMENT (IN THE STRUCTURE) WHOSE            *
 *     OFFSET WE NEED.                                        *
 * THE OFFSET RETURNS AS 'unsigned long'.                     *
 */
#define SAL_OFFSETOF(x,y) ((unsigned long)(&(((x *)0)->y)))

/**************************************************************
 * MACRO TO GET THE SIZE OF AN ELEMENT IN A                   *
 * GIVEN STRUCTURE TYPE.                                      *
 * 'X' IS THE TYPE OF THE STRUCTURE.                          *
 * 'Y' IS THE  ELEMENT (IN THE STRUCTURE) WHOSE               *
 *     SIZE (IN BYTES) WE NEED.                               *
 * THE SIZE RETURNS AS 'unsigned long'.                       *
 */
#define SAL_SIZEOF(x,y) ((unsigned long)(sizeof(((x *)0)->y)))

#define BITS2BYTES(x)        (((x) + 7) / 8)
#define BITS2WORDS(x)        (((x) + 31) / 32)

#define BYTES2BITS(x)        ((x) * 8)
#define BYTES2WORDS(x)        (((x) + 3) / 4)

#define WORDS2BITS(x)        ((x) * 32)
#define WORDS2BYTES(x)        ((x) * 4)

#ifndef COUNTOF
#define COUNTOF(ary)        ((int) (sizeof (ary) / sizeof ((ary)[0])))
#endif

#ifdef PTRS_ARE_64BITS
typedef uint64    sal_vaddr_t;        /* Virtual address (Host address) */
typedef uint64    sal_paddr_t;        /* Physical address (PCI address) */
#define PTR_TO_INT(x)        ((uint32)(((sal_vaddr_t)(x))&0xFFFFFFFF))
#define PTR_HI_TO_INT(x)        ((uint32)((((sal_vaddr_t)(x))>>32)&0xFFFFFFFF))

#else
typedef uint32    sal_vaddr_t;        /* Virtual address (Host address) */
/* Physical address (PCI address) */
#ifdef PHYS_ADDRS_ARE_64BITS
typedef uint64    sal_paddr_t;
#else
typedef uint32    sal_paddr_t;
#endif
#define PTR_TO_INT(x)        ((uint32)(x))
#define PTR_HI_TO_INT(x)        (0)
#endif

#define INT_TO_PTR(x)        ((void *)((sal_vaddr_t)(x)))

#define PTR_TO_UINTPTR(x)       ((sal_vaddr_t)(x))
#define UINTPTR_TO_PTR(x)       ((void *)(x))

typedef union
{
    uint8            u8;
    uint16            u16;
    uint32            u32;
    uint64            u64;
    sal_paddr_t        paddr;
    sal_vaddr_t        vaddr;
    void            *ptr;
}                any_t;

#define SAL_IP6_ADDRLEN       (16)      /* IPv6 address length in bytes.      */

typedef uint8   sal_mac_addr_t[6];      /* MAC address */
typedef uint32  sal_ip_addr_t;          /* IP Address */
typedef uint8   sal_ip6_addr_t[SAL_IP6_ADDRLEN];    /* IPv6 Address */

/* sal_mac_addr_t mac;  Just generate a list of the macs for display */
#define SAL_MAC_ADDR_LIST(mac) \
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]

/* sal_mac_addr_t m1, m2;  True if equal */
#define SAL_MAC_ADDR_EQUAL(m1, m2) (!sal_memcmp(m1, m2, sizeof(sal_mac_addr_t)))

/* sal_mac_addr_t m1, m2;  like memcmp, returns -1, 0, or 1 */
#define SAL_MAC_ADDR_CMP(m1, m2) sal_memcmp(m1, m2, sizeof(sal_mac_addr_t))

#define SAL_MACADDR_STR_LEN     18              /* Formatted MAC address */
#define SAL_IPADDR_STR_LEN      16              /* Formatted IP address */


/* Adjust justification for uint32 writes to fields */
/* dst is an array name of type uint32 [] */
#define SAL_MAC_ADDR_TO_UINT32(mac, dst) do {\
        (dst)[0] = (((uint32)(mac)[2]) << 24 | \
                  ((uint32)(mac)[3]) << 16 | \
                  ((uint32)(mac)[4]) << 8 | \
                  ((uint32)(mac)[5])); \
        (dst)[1] = (((uint32)(mac)[0]) << 8 | \
                  ((uint32)(mac)[1])); \
    } while (0)

/* Adjust justification for uint32 writes to fields */
/* src is an array name of type uint32 [] */
#define SAL_MAC_ADDR_FROM_UINT32(mac, src) do {\
        (mac)[0] = (uint8) ((src)[1] >> 8 & 0xff); \
        (mac)[1] = (uint8) ((src)[1] & 0xff); \
        (mac)[2] = (uint8) ((src)[0] >> 24); \
        (mac)[3] = (uint8) ((src)[0] >> 16 & 0xff); \
        (mac)[4] = (uint8) ((src)[0] >> 8 & 0xff); \
        (mac)[5] = (uint8) ((src)[0] & 0xff); \
    } while (0)


/* dst is a uint64 */
#define SAL_MAC_ADDR_TO_UINT64(mac, dst) do {        \
        uint32 _val[2];                              \
        SAL_MAC_ADDR_TO_UINT32(mac, _val);           \
        COMPILER_64_SET(dst, _val[1], _val[0]);      \
    } while (0)

/* src is a uint64 */
#define SAL_MAC_ADDR_FROM_UINT64(mac, src) do {      \
        uint32 _val[2];                              \
        COMPILER_64_TO_32_LO(_val[0], src);          \
        COMPILER_64_TO_32_HI(_val[1], src);          \
        SAL_MAC_ADDR_FROM_UINT32(mac, _val);         \
    } while (0)


/* Adjust IP6 justification for uint32 field accesses */
/* 
 * These macros are used on IP6 "half addresses", being
 * either the "upper" 64 bits or the "lower" 64 bits of
 * an IPv6 address.
 */

/* dst is an array name of type uint32 [] */
#define SAL_IP6_ADDR_HALF_TO_UINT32(ip6, dst) do {\
        (dst)[1] = (((uint32)(ip6)[0]) << 24 | \
                   ((uint32)(ip6)[1]) << 16 | \
                   ((uint32)(ip6)[2]) << 8 | \
                   ((uint32)(ip6)[3])); \
        (dst)[0] = (((uint32)(ip6)[4]) << 24 | \
                   ((uint32)(ip6)[5]) << 16 | \
                   ((uint32)(ip6)[6]) << 8 | \
                   ((uint32)(ip6)[7])); \
    } while (0)

/* src is an array name of type uint32 [] */
#define SAL_IP6_ADDR_HALF_FROM_UINT32(ip6, src) do {\
        (ip6)[0] = (uint8) ((src)[1] >> 24); \
        (ip6)[1] = (uint8) ((src)[1] >> 16 & 0xff); \
        (ip6)[2] = (uint8) ((src)[1] >> 8 & 0xff); \
        (ip6)[3] = (uint8) ((src)[1] & 0xff); \
        (ip6)[4] = (uint8) ((src)[0] >> 24); \
        (ip6)[5] = (uint8) ((src)[0] >> 16 & 0xff); \
        (ip6)[6] = (uint8) ((src)[0] >> 8 & 0xff); \
        (ip6)[7] = (uint8) ((src)[0] & 0xff); \
    } while (0)

/* 
 * These macros are used on full 128-bit IP6 addresses.
 */

/* dst is an array name of type uint32 [] */
#define SAL_IP6_ADDR_TO_UINT32(ip6, dst) do {\
        SAL_IP6_ADDR_HALF_TO_UINT32(&((ip6)[8]), (dst)); \
        SAL_IP6_ADDR_HALF_TO_UINT32((ip6), &((dst)[2])); \
    } while (0)

/* src is an array name of type uint32 [] */
#define SAL_IP6_ADDR_FROM_UINT32(ip6, src) do {\
        SAL_IP6_ADDR_HALF_FROM_UINT32(&((ip6)[8]), (src)); \
        SAL_IP6_ADDR_HALF_FROM_UINT32((ip6), &((src)[2])); \
    } while (0)


/* Device bus types */
#define SAL_PCI_DEV_TYPE        0x00001 /* PCI device */
#define SAL_SPI_DEV_TYPE        0x00002 /* SPI device */
#define SAL_EB_DEV_TYPE         0x00004 /* EB device */
#define SAL_ICS_DEV_TYPE        0x00008 /* ICS device */
#define SAL_MII_DEV_TYPE        0x00010 /* MII device */
#define SAL_RCPU_DEV_TYPE       0x00020 /* RCPU device */
#define SAL_I2C_DEV_TYPE        0x00040 /* I2C device */
#define SAL_AXI_DEV_TYPE        0x00080 /* AXI device */
#define SAL_EMMI_DEV_TYPE       0x10000 /* EMMI device */
#define SAL_DEV_BUS_TYPE_MASK   0xf00ff /* Odd for historical reasons */

/* Device types */
#define SAL_SWITCH_DEV_TYPE     0x00100 /* Switch device */
#define SAL_ETHER_DEV_TYPE      0x00200 /* Ethernet device */
#define SAL_CPU_DEV_TYPE        0x00400 /* CPU device */
#define SAL_DEV_TYPE_MASK       0x00f00

/* Access types */
#define SAL_DEV_BUS_RD_16BIT    0x01000 /* 16 bit reads on bus */
#define SAL_DEV_BUS_WR_16BIT    0x02000 /* 16 bit writes on bus */
#define SAL_DEV_BUS_ALT         0x04000 /* Alternate access */
#define SAL_DEV_BUS_MSI         0x08000 /* Message-signaled interrupts */
#define SAL_DEV_FLAG_MASK       0x0f000

/* BDE reserved mask (cannot be used by SAL) */
#define SAL_DEV_BDE_MASK        0xff000000

/* Backward compatibility */
#define SAL_ET_DEV_TYPE         SAL_MII_DEV_TYPE

/* Special access addresses */
#define SAL_DEV_OP_EMMI_INIT    0x0fff1000

#endif    /* !_SAL_TYPES_H */
