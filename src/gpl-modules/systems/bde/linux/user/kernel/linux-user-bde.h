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
 */

#ifndef __LINUX_USER_BDE_H__
#define __LINUX_USER_BDE_H__

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux-bde.h>
#ifndef __KERNEL__
#include <stdint.h>
#endif

#if defined(SAL_BDE_32BIT_USER_64BIT_KERNEL) || defined(PTRS_ARE_64BITS)
typedef uint64_t bde_kernel_addr_t;
#else
typedef uint32_t bde_kernel_addr_t;
#endif

/* Ioctl control structure */
typedef struct  {
    unsigned int dev;   /* Device ID */
    unsigned int rc;    /* Operation Return Code */
    unsigned int d0;    /* Operation specific data */
    unsigned int d1;
    unsigned int d2;
    unsigned int d3;    
    bde_kernel_addr_t p0;
    union {
        unsigned int dw[2];
        unsigned char buf[64];
    } dx;
} lubde_ioctl_t;


/* LUBDE ioctls */
#define LUBDE_MAGIC 'L'

#define LUBDE_VERSION             _IO(LUBDE_MAGIC, 0)
#define LUBDE_GET_NUM_DEVICES     _IO(LUBDE_MAGIC, 1)
#define LUBDE_GET_DEVICE          _IO(LUBDE_MAGIC, 2)
#define LUBDE_PCI_CONFIG_PUT32    _IO(LUBDE_MAGIC, 3)
#define LUBDE_PCI_CONFIG_GET32    _IO(LUBDE_MAGIC, 4)
#define LUBDE_GET_DMA_INFO        _IO(LUBDE_MAGIC, 5)
#define LUBDE_ENABLE_INTERRUPTS   _IO(LUBDE_MAGIC, 6)
#define LUBDE_DISABLE_INTERRUPTS  _IO(LUBDE_MAGIC, 7)
#define LUBDE_USLEEP              _IO(LUBDE_MAGIC, 8)
#define LUBDE_WAIT_FOR_INTERRUPT  _IO(LUBDE_MAGIC, 9)
#define LUBDE_SEM_OP              _IO(LUBDE_MAGIC, 10)
#define LUBDE_UDELAY              _IO(LUBDE_MAGIC, 11)
#define LUBDE_GET_DEVICE_TYPE     _IO(LUBDE_MAGIC, 12)
#define LUBDE_SPI_READ_REG        _IO(LUBDE_MAGIC, 13)
#define LUBDE_SPI_WRITE_REG       _IO(LUBDE_MAGIC, 14)
#define LUBDE_READ_REG_16BIT_BUS  _IO(LUBDE_MAGIC, 19)
#define LUBDE_WRITE_REG_16BIT_BUS _IO(LUBDE_MAGIC, 20)
#define LUBDE_GET_BUS_FEATURES    _IO(LUBDE_MAGIC, 21)
#define LUBDE_WRITE_IRQ_MASK      _IO(LUBDE_MAGIC, 22)
#define LUBDE_CPU_WRITE_REG       _IO(LUBDE_MAGIC, 23)
#define LUBDE_CPU_READ_REG        _IO(LUBDE_MAGIC, 24)
#define LUBDE_CPU_PCI_REGISTER    _IO(LUBDE_MAGIC, 25)
#define LUBDE_DEV_RESOURCE        _IO(LUBDE_MAGIC, 26)
#define LUBDE_IPROC_READ_REG      _IO(LUBDE_MAGIC, 27)
#define LUBDE_IPROC_WRITE_REG     _IO(LUBDE_MAGIC, 28)
#define LUBDE_ATTACH_INSTANCE     _IO(LUBDE_MAGIC, 29)
#define LUBDE_GET_DEVICE_STATE    _IO(LUBDE_MAGIC, 30)
#define LUBDE_REPROBE             _IO(LUBDE_MAGIC, 31)

#define LUBDE_SEM_OP_CREATE       1
#define LUBDE_SEM_OP_DESTROY      2
#define LUBDE_SEM_OP_TAKE         3
#define LUBDE_SEM_OP_GIVE         4

#define LUBDE_SUCCESS 0
#define LUBDE_FAIL ((unsigned int)-1)


/* 
 * Version history
 * 1: add LUBDE_GET_DEVICE_STATE to support PCI hot plug 
 * 2: add LUBDE_REPROBE to support reprobe available devices
 */
#define KBDE_VERSION    2


/* This is the signal that will be used
 * when an interrupt occurs
 */

#ifndef __KERNEL__
#include <signal.h>
#endif

#define LUBDE_INTERRUPT_SIGNAL    SIGUSR1
#define LUBDE_ETHER_INTERRUPT_SIGNAL    SIGUSR2

#endif /* __LUBDE_H__ */
