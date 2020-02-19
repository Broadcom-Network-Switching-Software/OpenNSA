/*
 * Copyright 2017 Broadcom
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 (GPLv2) along with this source code.
 */
/*
 * 
 * This software is governed by the Broadcom Switch APIs license.
 * This license is set out in https://raw.githubusercontent.com/Broadcom-Network-Switching-Software/OpenNSA/master/Legal/LICENSE file.
 * 
 * Copyright 2007-2020 Broadcom Inc. All rights reserved.
 *
 * File: 	types.h
 * Purpose: 	SAL type definitions
 */

#ifndef _SAL_TYPES_H
#define _SAL_TYPES_H

/*
 * Define platform-independent types
 */

#ifndef TRUE
#define TRUE			1
#endif

#ifndef FALSE
#define FALSE			0
#endif

#ifndef NULL
#define NULL			0
#endif

#ifndef DONT_CARE
#define DONT_CARE		0
#endif

#define VOL			volatile

/*
 * 64-bit type
 */

#ifdef LONGS_ARE_64BITS

#define COMPILER_64BIT
#define	COMPILER_UINT64			unsigned long
#define u64_H(v)			(((uint32 *) &(v))[u64_MSW])
#define u64_L(v)			(((uint32 *) &(v))[u64_LSW])

#else /* !LONGS_ARE_64BITS */

#define COMPILER_64BIT
#define	COMPILER_UINT64			unsigned long long
#define u64_H(v)			(((uint32 *) &(v))[u64_MSW])
#define u64_L(v)			(((uint32 *) &(v))[u64_LSW])

#endif /* LONGS_ARE_64BITS */

/*
 * Define unsigned and signed integers with guaranteed sizes.
 * Adjust if your compiler uses different sizes for short or int.
 */

typedef unsigned char		uint8;		/* 8-bit quantity  */
typedef unsigned short		uint16;		/* 16-bit quantity */
typedef unsigned int		uint32;		/* 32-bit quantity */
typedef COMPILER_UINT64		uint64;		/* 64-bit quantity */

typedef signed char		int8;		/* 8-bit quantity  */
typedef signed short		int16;		/* 16-bit quantity */
typedef signed int		int32;		/* 32-bit quantity */

#define BITS2BYTES(x)		(((x) + 7) / 8)
#define BITS2WORDS(x)		(((x) + 31) / 32)

#define BYTES2BITS(x)		((x) * 8)
#define BYTES2WORDS(x)		(((x) + 3) / 4)

#define WORDS2BITS(x)		((x) * 32)
#define WORDS2BYTES(x)		((x) * 4)

#define COUNTOF(ary)		((int) (sizeof (ary) / sizeof ((ary)[0])))

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

#define INT_TO_PTR(x)		((void *)((sal_vaddr_t)(x)))

#define PTR_TO_UINTPTR(x)       ((sal_vaddr_t)(x))
#define UINTPTR_TO_PTR(x)       ((void *)(x))

typedef union
{
	uint8			u8;
	uint16			u16;
	uint32			u32;
	uint64			u64;
	sal_paddr_t		paddr;
	sal_vaddr_t		vaddr;
	void			*ptr;
}				any_t;

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

#endif	/* !_SAL_TYPES_H */
