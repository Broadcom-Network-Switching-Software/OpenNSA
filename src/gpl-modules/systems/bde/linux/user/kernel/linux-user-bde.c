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
 * Linux User BDE Helper Module
 */
#include <gmodule.h>
#include <mpool.h>
#include <linux-bde.h>

#include <sal/core/thread.h>
#include <sal/core/sync.h>
#include <soc/devids.h>

#include "linux-user-bde.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
#include <linux/uaccess.h>
#endif


MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("User BDE Helper Module");
MODULE_LICENSE("GPL");

/* CMIC/CMICe defines */
#define CMIC_IRQ_STAT                   0x00000144
#define CMIC_IRQ_MASK                   0x00000148
#define CMIC_IRQ_MASK_1                 0x0000006C
#define CMIC_IRQ_MASK_2                 0x00000070

/* CMICm defines */
#define CMIC_CMCx_IRQ_STAT0_OFFSET(x)                    (0x31400 + (0x1000 * x))
#define CMIC_CMCx_IRQ_STAT1_OFFSET(x)                    (0x31404 + (0x1000 * x))
#define CMIC_CMCx_IRQ_STAT2_OFFSET(x)                    (0x31408 + (0x1000 * x))
#define CMIC_CMCx_IRQ_STAT3_OFFSET(x)                    (0x3140c + (0x1000 * x))
#define CMIC_CMCx_IRQ_STAT4_OFFSET(x)                    (0x31410 + (0x1000 * x))

#define CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(x)               (0x31414 + (0x1000 * x))
#define CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(x)               (0x31418 + (0x1000 * x))
#define CMIC_CMCx_PCIE_IRQ_MASK2_OFFSET(x)               (0x3141c + (0x1000 * x))
#define CMIC_CMCx_PCIE_IRQ_MASK3_OFFSET(x)               (0x31420 + (0x1000 * x))
#define CMIC_CMCx_PCIE_IRQ_MASK4_OFFSET(x)               (0x31424 + (0x1000 * x))

/* CMICd defines */
#define CMIC_CMCx_IRQ_STAT5_OFFSET(x)                    (0x314b0 + (0x1000 * x))
#define CMIC_CMCx_IRQ_STAT6_OFFSET(x)                    (0x314b4 + (0x1000 * x))
#define CMIC_CMCx_PCIE_IRQ_MASK5_OFFSET(x)               (0x314b8 + (0x1000 * x))
#define CMIC_CMCx_PCIE_IRQ_MASK6_OFFSET(x)               (0x314bc + (0x1000 * x))
#define CMIC_CMCx_UC0_IRQ_MASK5_OFFSET(x)                (0x314c0 + (0x1000 * x))
#define CMIC_CMCx_UC0_IRQ_MASK6_OFFSET(x)                (0x314c4 + (0x1000 * x))

#define CMIC_CMCx_UC0_IRQ_MASK0_OFFSET(x)                (0x31428 + (0x1000 * x))
#define CMIC_CMCx_UC0_IRQ_MASK1_OFFSET(x)                (0x3142c + (0x1000 * x))
#define CMIC_CMCx_UC0_IRQ_MASK2_OFFSET(x)                (0x31430 + (0x1000 * x))
#define CMIC_CMCx_UC0_IRQ_MASK3_OFFSET(x)                (0x31434 + (0x1000 * x))
#define CMIC_CMCx_UC0_IRQ_MASK4_OFFSET(x)                (0x31438 + (0x1000 * x))

/* CMICX defines */
#define INTC_INTR_REG_NUM               (8)


#define PAXB_0_PAXB_IC_INTRCLR_0       (0x180123a0)
#define PAXB_0_PAXB_IC_INTRCLR_1       (0x180123a4)

#define PAXB_0_PAXB_IC_INTRCLR_MODE_0  (0x180123a8)
#define PAXB_0_PAXB_IC_INTRCLR_MODE_1  (0x180123ac)

#define HX5_PAXB_0_PAXB_IC_INTRCLR_0   (0x102303a0)
#define HX5_PAXB_0_PAXB_IC_INTRCLR_1   (0x102303a4)

#define HX5_PAXB_0_PAXB_IC_INTRCLR_MODE_0   (0x102303a8)
#define HX5_PAXB_0_PAXB_IC_INTRCLR_MODE_1   (0x102303ac)

#define INTC_INTR_ENABLE_REG0          (0x180130f0)
#define INTC_INTR_STATUS_REG0          (0x18013190)
#define INTC_INTR_RAW_STATUS_REG0      (0x18013140)

#define INTC_INTR_ENABLE_BASE           (INTC_INTR_ENABLE_REG0)
#define INTC_INTR_STATUS_BASE           (INTC_INTR_STATUS_REG0)
#define INTC_INTR_RAW_STATUS_BASE       (INTC_INTR_RAW_STATUS_REG0)

#define HX5_INTC_INTR_ENABLE_REG0          (0x102310f0)
#define HX5_INTC_INTR_STATUS_REG0          (0x10231190)
#define HX5_INTC_INTR_RAW_STATUS_REG0      (0x10231140)

#define HX5_INTC_INTR_ENABLE_BASE          (HX5_INTC_INTR_ENABLE_REG0)
#define HX5_INTC_INTR_STATUS_BASE          (HX5_INTC_INTR_STATUS_REG0)
#define HX5_INTC_INTR_RAW_STATUS_BASE      (HX5_INTC_INTR_RAW_STATUS_REG0)

#define IOREMAP(addr, size)                ioremap_nocache(addr, size)

#define HX5_IHOST_GICD_ISENABLERN_0        (0x10781100)
#define HX5_IHOST_GICD_ISENABLERN_1        (0x10781104)
#define HX5_IHOST_GICD_ICENABLERN_1        (0x10781184)
#define HX5_IHOST_GICD_ICENABLERN_8        (0x107811a0)
/* Offset between ISENABLERN_1 and ICENABLERN_1 in 4-bytes */
#define HX5_IHOST_IRQ_MASK_OFFSET          0x20
#define HX5_IHOST_INTR_MAP_NUM             (HX5_IHOST_GICD_ICENABLERN_8 - HX5_IHOST_GICD_ISENABLERN_0)
#define HX5_IHOST_INTR_STATUS_MAP_NUM      (INTC_INTR_REG_NUM * (sizeof(uint32)))
#define IRQ_BIT(intr)                      (intr % (sizeof(uint32)*8))
#define IRQ_MASK_INDEX(intr)               (intr / (sizeof(uint32)*8))
#define HX5_CHIP_INTR_LOW_PRIORITY         119
#define INTR_LOW_PRIORITY_BITPOS           (1 << IRQ_BIT(HX5_CHIP_INTR_LOW_PRIORITY))
#define INTC_LOW_PRIORITY_INTR_REG_IND     IRQ_MASK_INDEX(HX5_CHIP_INTR_LOW_PRIORITY)
#define INTC_PDMA_INTR_REG_IND             4

#define READ_INTC_INTR(d, reg, v) \
        (v = user_bde->iproc_read(d, reg))
#define WRITE_INTC_INTR(d, reg, v) \
        (user_bde->iproc_write(d, reg, v))

#define IHOST_READ_INTR(d, reg, v) \
        (v = readl((reg)))
#define IHOST_WRITE_INTR(d, reg, v) \
        (writel((v), (reg)))

/* Allow override of default CMICm CMC */
#ifndef BDE_CMICM_PCIE_CMC
#define BDE_CMICM_PCIE_CMC              0
#endif

/* Allow override of default CMICm CMC */
#ifndef BDE_CMICD_PCIE_CMC
#define BDE_CMICD_PCIE_CMC              0
#endif

/* Defines used to distinguish CMICe from CMICm */
#define CMICE_DEV_REV_ID                (0x178 / sizeof(uint32))

static uint32 *ihost_intr_status_base = NULL;
static uint32 *ihost_intr_enable_base = NULL;

static ibde_t *user_bde = NULL;

typedef void (*isr_f)(void *);

typedef struct _intr_regs_s {
    uint32 intc_intr_status_base;
    uint32 intc_intr_enable_base;
    uint32 intc_intr_clear_0;
    uint32 intc_intr_clear_1;
    uint32 intc_intr_clear_mode_0;
    uint32 intc_intr_clear_mode_1;
} _intr_regs_t;

typedef struct bde_ctrl_s {
    uint32 dev_type;
    int irq;
    int enabled;
    int devid;
    isr_f isr;
    uint32 *ba;
    int inst;   /* associate to _bde_inst_resource[] */
    _intr_regs_t intr_regs;
} bde_ctrl_t;

#define VALID_DEVICE(_n) (_n < LINUX_BDE_MAX_DEVICES)

static bde_ctrl_t _devices[LINUX_BDE_MAX_DEVICES];

static wait_queue_head_t _ether_interrupt_wq;
static atomic_t _ether_interrupt_has_taken_place = ATOMIC_INIT(0);

/*
 * Multiple instance resource data structure.
 * To keep the DMA resource per instance.
 * And track the DMA pool usage.
 */
static int _bde_multi_inst = 0;

typedef struct {
    unsigned int    inst_id;
    unsigned int    dma_offset;
    unsigned int    dma_size;
    wait_queue_head_t intr_wq;
    atomic_t intr;
} bde_inst_resource_t;

static bde_inst_resource_t _bde_inst_resource[LINUX_BDE_MAX_DEVICES];

typedef struct {
    phys_addr_t  cpu_pbase; /* CPU physical base address of the DMA pool */
    phys_addr_t  dma_pbase; /* Bus base address of the DMA pool */
    uint32  total_size; /* Total size of the pool in MB */
    uint32  offset; /* Current offset of the pool in MB */
}_dma_pool_t;

static _dma_pool_t _dma_pool;

#define ONE_MB      (1024 * 1024)

#ifdef KEYSTONE
/*
 * Enforce PCIE transaction ordering. Commit the write transaction.
 */

#define SSOC_WRITEL(val, addr)                  \
            do {                                \
                writel((val), (addr));          \
                __asm__ __volatile__("sync");   \
            } while(0)

#else

#define SSOC_WRITEL(val, addr) \
            writel((val), (addr))

#endif
/*
 * Function: _interrupt
 *
 * Purpose:
 *    Interrupt Handler.
 *    Mask all interrupts on device and wake up interrupt
 *    thread. It is assumed that the interrupt thread unmasks
 *    interrupts again when interrupt handling is complete.
 * Parameters:
 *    ctrl - BDE control structure for this device.
 * Returns:
 *    Nothing
 */
static void 
_cmic_interrupt(bde_ctrl_t *ctrl)
{
    int d;
    uint32_t mask = 0, stat, imask = 0, fmask = 0;
    bde_inst_resource_t *res;

    d = (((uint8 *)ctrl - (uint8 *)_devices) / sizeof (bde_ctrl_t));
    res = &_bde_inst_resource[ctrl->inst];

    /* Check for secondary interrupt handler */
    if (lkbde_irq_mask_get(d, &mask, &fmask) < 0) {
        fmask = 0;
    }

    if (fmask != 0) {
        imask = mask & ~fmask;
        /* Check for pending user mode interrupts */
        stat = user_bde->read(d, CMIC_IRQ_STAT);
        if ((stat & imask) == 0) {
            /* All handled in kernel mode */
            lkbde_irq_mask_set(d, CMIC_IRQ_MASK, imask, 0);
            return;
        }
    }

    lkbde_irq_mask_set(d, CMIC_IRQ_MASK, 0, 0);

    atomic_set(&res->intr, 1);

#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&res->intr_wq);
#else
    wake_up_interruptible(&res->intr_wq);
#endif
}

static void 
_cmicx_interrupt(bde_ctrl_t *ctrl)
{
    int d, ind;
    uint32 stat, iena, mask, fmask;
    bde_inst_resource_t *res;

    d = (((uint8 *)ctrl - (uint8 *)_devices) / sizeof (bde_ctrl_t));

    if (ctrl->dev_type & BDE_PCI_DEV_TYPE) {
        /* Clear MSI interrupts immediately to prevent spurious interrupts */
        WRITE_INTC_INTR(d, ctrl->intr_regs.intc_intr_clear_0, 0xFFFFFFFF);
        WRITE_INTC_INTR(d, ctrl->intr_regs.intc_intr_clear_1, 0xFFFFFFFF);
    }

    res = &_bde_inst_resource[ctrl->inst];

    lkbde_irq_mask_get(d, &mask, &fmask);

    if (fmask) {
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            IHOST_READ_INTR(d, ihost_intr_status_base + INTC_PDMA_INTR_REG_IND, stat);
            IHOST_READ_INTR(d, ihost_intr_enable_base + INTC_PDMA_INTR_REG_IND, iena);
        } else {
            READ_INTC_INTR(d, ctrl->intr_regs.intc_intr_status_base + 4 * INTC_PDMA_INTR_REG_IND, stat);
            READ_INTC_INTR(d, ctrl->intr_regs.intc_intr_enable_base + 4 * INTC_PDMA_INTR_REG_IND, iena);
        }
        if (stat & iena) {
            if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
                IHOST_WRITE_INTR(d, ihost_intr_enable_base + INTC_PDMA_INTR_REG_IND +
                    HX5_IHOST_IRQ_MASK_OFFSET, ~0);
            } else {
                WRITE_INTC_INTR(d, ctrl->intr_regs.intc_intr_enable_base + 4 * INTC_PDMA_INTR_REG_IND, 0);
            }

            for (ind = 0; ind < INTC_INTR_REG_NUM; ind++) {
                if (ind == INTC_PDMA_INTR_REG_IND) {
                    continue;
                }
                if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
                    if (ind < INTC_LOW_PRIORITY_INTR_REG_IND) {
                        continue;
                    }
                    IHOST_READ_INTR(d, ihost_intr_status_base + ind, stat);
                    IHOST_READ_INTR(d, ihost_intr_enable_base + ind, iena);
                    if (ind == INTC_LOW_PRIORITY_INTR_REG_IND) {
                        stat &= INTR_LOW_PRIORITY_BITPOS;
                    }
                } else {
                    READ_INTC_INTR(d, ctrl->intr_regs.intc_intr_status_base + 4 * ind, stat);
                    READ_INTC_INTR(d, ctrl->intr_regs.intc_intr_enable_base + 4 * ind, iena);
                }
                if (stat & iena) {
                    break;
                }
            }

            if (ind >= INTC_INTR_REG_NUM) {
                return;
            }
        }
    }

    /* Disable all interrupts.. Re-enable unserviced interrupts later
     * So as to avoid getting new interrupts until the user level driver
     * enumerates the interrupts to be serviced
     */
    for (ind = 0; ind < INTC_INTR_REG_NUM; ind++) {
        if (fmask && ind == INTC_PDMA_INTR_REG_IND) {
            continue;
        }
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            if (ind < INTC_LOW_PRIORITY_INTR_REG_IND) {
                continue;
            }
            if (ind == INTC_LOW_PRIORITY_INTR_REG_IND) {
                IHOST_WRITE_INTR(d, ihost_intr_enable_base + INTC_LOW_PRIORITY_INTR_REG_IND +
                    HX5_IHOST_IRQ_MASK_OFFSET, INTR_LOW_PRIORITY_BITPOS);
            } else {
                IHOST_WRITE_INTR(d, ihost_intr_enable_base + ind +
                    HX5_IHOST_IRQ_MASK_OFFSET, ~0);
            }
        } else {
            WRITE_INTC_INTR(d, ctrl->intr_regs.intc_intr_enable_base + 4*ind, 0);
        }
    }

    /* Notify */
    atomic_set(&res->intr, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&res->intr_wq);
#else
    wake_up_interruptible(&res->intr_wq);
#endif
}

static void
_cmicm_interrupt(bde_ctrl_t *ctrl)
{
    int d;
    int cmc = BDE_CMICM_PCIE_CMC;
    uint32 stat, mask = 0, fmask = 0, imask = 0;
    bde_inst_resource_t *res;

    d = (((uint8 *)ctrl - (uint8 *)_devices) / sizeof (bde_ctrl_t));
    res = &_bde_inst_resource[ctrl->inst];

    lkbde_irq_mask_get(d, &mask, &fmask);

    while (fmask) {
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT0_OFFSET(cmc));
        imask = mask & ~fmask;
        if (stat & imask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT1_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK1_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT2_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK2_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK2_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT3_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK3_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK3_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT4_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK4_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK4_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        return;
    }

    if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
        lkbde_irq_mask_set(d, CMIC_CMCx_UC0_IRQ_MASK0_OFFSET(cmc), 0, 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK1_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK2_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK3_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK4_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK0_OFFSET(1), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK0_OFFSET(2), 0);
    }
    else {
        lkbde_irq_mask_set(d, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(cmc), 0, 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK2_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK3_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK4_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(1), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(2), 0);
    }
    atomic_set(&res->intr, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&res->intr_wq);
#else
    wake_up_interruptible(&res->intr_wq);
#endif
}

/* some device has cmc0 only */
static void
_cmicd_cmc0_interrupt(bde_ctrl_t *ctrl)
{
    int d;
    int cmc = 0;
    uint32 stat, mask = 0, fmask = 0, imask = 0;
    bde_inst_resource_t *res;

    d = (((uint8 *)ctrl - (uint8 *)_devices) / sizeof (bde_ctrl_t));
    res = &_bde_inst_resource[ctrl->inst];
    lkbde_irq_mask_get(d, &mask, &fmask);

    while (fmask) {
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT0_OFFSET(cmc));
        imask = mask & ~fmask;
        if (stat & imask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT1_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK1_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT2_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK2_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK2_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT3_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK3_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK3_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT4_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK4_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK4_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT5_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK5_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK5_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT6_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK6_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK6_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        return;
    }

    if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
        lkbde_irq_mask_set(d, CMIC_CMCx_UC0_IRQ_MASK0_OFFSET(cmc), 0, 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK1_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK2_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK3_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK4_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK5_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK6_OFFSET(cmc), 0);
    } else {
        lkbde_irq_mask_set(d, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(cmc), 0, 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK2_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK3_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK4_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK5_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK6_OFFSET(cmc), 0);
    }
    atomic_set(&res->intr, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&res->intr_wq);
#else
    wake_up_interruptible(&res->intr_wq);
#endif
}

static void
_cmicd_interrupt(bde_ctrl_t *ctrl)
{
    int d;
    int cmc = BDE_CMICD_PCIE_CMC;
    uint32 stat, mask = 0, fmask = 0, imask = 0;
    bde_inst_resource_t *res;

    d = (((uint8 *)ctrl - (uint8 *)_devices) / sizeof (bde_ctrl_t));
    res = &_bde_inst_resource[ctrl->inst];
    lkbde_irq_mask_get(d, &mask, &fmask);

    while (fmask) {
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT0_OFFSET(cmc));
        imask = mask & ~fmask;
        if (stat & imask) {
            break;
        }
        /** Check if there are interrupts other than PacketIO interrupts on CMC1 */
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT0_OFFSET(1));
        imask = mask & ~fmask;
        if (stat & imask) {
            break;
        }
        /** Check if there are interrupts other than PacketIO interrupts on CMC2 */
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT0_OFFSET(2));
        imask = mask & ~fmask;
        if (stat & imask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT1_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK1_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT2_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK2_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK2_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT3_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK3_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK3_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT4_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK4_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK4_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT5_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK5_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK5_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        stat = user_bde->read(d, CMIC_CMCx_IRQ_STAT6_OFFSET(cmc));
        if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
            mask = user_bde->read(d, CMIC_CMCx_UC0_IRQ_MASK6_OFFSET(cmc));
        } else {
            mask = user_bde->read(d, CMIC_CMCx_PCIE_IRQ_MASK6_OFFSET(cmc));
        }
        if (stat & mask) {
            break;
        }
        return;
    }

    if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
        lkbde_irq_mask_set(d, CMIC_CMCx_UC0_IRQ_MASK0_OFFSET(cmc), 0, 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK1_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK2_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK3_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK4_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK5_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK6_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK0_OFFSET(1), 0);
        user_bde->write(d, CMIC_CMCx_UC0_IRQ_MASK0_OFFSET(2), 0);
    } else {
        lkbde_irq_mask_set(d, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(cmc), 0, 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK2_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK3_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK4_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK5_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK6_OFFSET(cmc), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(1), 0);
        user_bde->write(d, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(2), 0);
    }
    atomic_set(&res->intr, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&res->intr_wq);
#else
    wake_up_interruptible(&res->intr_wq);
#endif
}

static void 
_bcm88750_interrupt(bde_ctrl_t *ctrl)
{
    int d;
    bde_inst_resource_t *res;

    d = (((uint8 *)ctrl - (uint8 *)_devices) / sizeof (bde_ctrl_t));
    res = &_bde_inst_resource[ctrl->inst];
    lkbde_irq_mask_set(d, CMIC_IRQ_MASK, 0, 0);

    lkbde_irq_mask_set(d, CMIC_IRQ_MASK_1, 0, 0); 
    lkbde_irq_mask_set(d, CMIC_IRQ_MASK_2, 0, 0);
    atomic_set(&res->intr, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&res->intr_wq);
#else
    wake_up_interruptible(&res->intr_wq);
#endif
}

/* The actual interrupt handler of ethernet devices */
static void 
_ether_interrupt(bde_ctrl_t *ctrl)
{
    SSOC_WRITEL(0, ctrl->ba + 0x024/4);

    atomic_set(&_ether_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_ether_interrupt_wq);
#else
    wake_up_interruptible(&_ether_interrupt_wq);
#endif
}


static struct _intr_mode_s {
    isr_f isr;
    const char *name;
} _intr_mode[] = {
    { (isr_f)_cmic_interrupt,       "CMIC/CMICe" },
    { (isr_f)_cmicm_interrupt,      "CMICm" },
    { (isr_f)_cmicd_interrupt,      "CMICd" },
    { (isr_f)_cmicd_cmc0_interrupt, "CMICd CMC0" },
    { (isr_f)_bcm88750_interrupt,   "BCM88750" },
    { (isr_f)_cmicx_interrupt,      "CMICx" },
    { NULL, NULL }
};

static const char *
_intr_mode_str(void *isr)
{
    int imx;

    imx = 0;
    while (_intr_mode[imx].isr != NULL) {
        if (isr == _intr_mode[imx].isr) {
            return _intr_mode[imx].name;
        }
        imx++;
    }
    return NULL;
}

static void
_intr_regs_init(bde_ctrl_t *ctrl, int hx5_intr)
{
    if (hx5_intr) {
        ctrl->intr_regs.intc_intr_status_base = HX5_INTC_INTR_STATUS_BASE;
        ctrl->intr_regs.intc_intr_enable_base = HX5_INTC_INTR_ENABLE_BASE;
        ctrl->intr_regs.intc_intr_clear_0 = HX5_PAXB_0_PAXB_IC_INTRCLR_0;
        ctrl->intr_regs.intc_intr_clear_1 = HX5_PAXB_0_PAXB_IC_INTRCLR_1;
        ctrl->intr_regs.intc_intr_clear_mode_0 = HX5_PAXB_0_PAXB_IC_INTRCLR_MODE_0;
        ctrl->intr_regs.intc_intr_clear_mode_1 = HX5_PAXB_0_PAXB_IC_INTRCLR_MODE_1;
    } else {
        ctrl->intr_regs.intc_intr_status_base = INTC_INTR_STATUS_BASE;
        ctrl->intr_regs.intc_intr_enable_base = INTC_INTR_ENABLE_BASE;
        ctrl->intr_regs.intc_intr_clear_0 = PAXB_0_PAXB_IC_INTRCLR_0;
        ctrl->intr_regs.intc_intr_clear_1 = PAXB_0_PAXB_IC_INTRCLR_1;
        ctrl->intr_regs.intc_intr_clear_mode_0 = PAXB_0_PAXB_IC_INTRCLR_MODE_0;
        ctrl->intr_regs.intc_intr_clear_mode_1 = PAXB_0_PAXB_IC_INTRCLR_MODE_1;
    }
}

static void
_devices_init(int d)
{
    bde_ctrl_t *ctrl;
    uint32 ver;
    uint16 device_id_mask = 0xFFF0;
    uint16 device_id;
    uint32 state = 0;

    (void)lkbde_dev_state_get(d, &state);
    if (state == BDE_DEV_STATE_REMOVED) {
        return;
    }

    ctrl = &_devices[d];
    /* Initialize our control info */
    ctrl->dev_type = user_bde->get_dev_type(d);
    ctrl->devid = user_bde->get_dev(d)->device;
    ctrl->inst = 0;

    if (BDE_DEV_MEM_MAPPED(ctrl->dev_type)) {
        ctrl->enabled = 0;
        ctrl->ba = lkbde_get_dev_virt(d);
    }
    if (ctrl->dev_type & BDE_SWITCH_DEV_TYPE) {
        switch (user_bde->get_dev(d)->device) {
        case BCM88750_DEVICE_ID:
        case BCM88753_DEVICE_ID:
        case BCM88754_DEVICE_ID:
        case BCM88755_DEVICE_ID:
        case BCM88752_DEVICE_ID:
            ctrl->isr = (isr_f)_bcm88750_interrupt;
            break;
        case BCM53540_DEVICE_ID:
        case BCM53547_DEVICE_ID:
        case BCM53548_DEVICE_ID:
        case BCM53549_DEVICE_ID:
            ctrl->isr = (isr_f)_cmicd_cmc0_interrupt;
            break;
        case BCM88670_DEVICE_ID:
        case BCM88671_DEVICE_ID:
        case BCM88671M_DEVICE_ID:
        case BCM88672_DEVICE_ID:
        case BCM88673_DEVICE_ID:
        case BCM88674_DEVICE_ID:
        case BCM88675_DEVICE_ID:
        case BCM88675M_DEVICE_ID:
        case BCM88676_DEVICE_ID:
        case BCM88676M_DEVICE_ID:
        case BCM88677_DEVICE_ID:
        case BCM88678_DEVICE_ID:
        case BCM88679_DEVICE_ID:
        case BCM88370_DEVICE_ID:
        case BCM88371_DEVICE_ID:
        case BCM88371M_DEVICE_ID:
        case BCM88375_DEVICE_ID:
        case BCM88376_DEVICE_ID:
        case BCM88376M_DEVICE_ID:
        case BCM88377_DEVICE_ID:
        case BCM88378_DEVICE_ID:
        case BCM88379_DEVICE_ID:
        case BCM88681_DEVICE_ID:
        case BCM88682_DEVICE_ID:
        case BCM88683_DEVICE_ID:
        case BCM88684_DEVICE_ID:
        case BCM88685_DEVICE_ID:
        case BCM88687_DEVICE_ID:
        case BCM88380_DEVICE_ID:
        case BCM88381_DEVICE_ID:
        case BCM88680_DEVICE_ID:
        case BCM88770_DEVICE_ID:
        case BCM88773_DEVICE_ID:
        case BCM88774_DEVICE_ID:
        case BCM88775_DEVICE_ID:
        case BCM88776_DEVICE_ID:
        case BCM88777_DEVICE_ID:
        case BCM88470_DEVICE_ID:
        case BCM88470P_DEVICE_ID:
        case BCM88471_DEVICE_ID:
        case BCM88473_DEVICE_ID:
        case BCM88474_DEVICE_ID:
        case BCM88474H_DEVICE_ID:
        case BCM88476_DEVICE_ID:
        case BCM88477_DEVICE_ID:
        case BCM88270_DEVICE_ID:
        case BCM88272_DEVICE_ID:
        case BCM88273_DEVICE_ID:
        case BCM88274_DEVICE_ID:
        case BCM88278_DEVICE_ID:
        case BCM88279_DEVICE_ID:
        case BCM8206_DEVICE_ID:
        case BCM88950_DEVICE_ID:
        case BCM88953_DEVICE_ID:
        case BCM88954_DEVICE_ID:
        case BCM88955_DEVICE_ID:
        case BCM88956_DEVICE_ID:
        case BCM88772_DEVICE_ID:
        case BCM88952_DEVICE_ID:
            ctrl->isr = (isr_f)_cmicd_interrupt;
            break;
        case BCM56370_DEVICE_ID:
        case BCM56371_DEVICE_ID:
        case BCM56372_DEVICE_ID:
        case BCM56374_DEVICE_ID:
        case BCM56375_DEVICE_ID:
        case BCM56376_DEVICE_ID:
        case BCM56377_DEVICE_ID:
        case BCM56577_DEVICE_ID:
        case BCM56578_DEVICE_ID:
        case BCM56579_DEVICE_ID:
        case BCM56273_DEVICE_ID:
        case BCM56274_DEVICE_ID:
        case BCM56275_DEVICE_ID:
        case BCM56276_DEVICE_ID:
        case BCM56277_DEVICE_ID:
        case BCM56278_DEVICE_ID:
            ctrl->isr = (isr_f)_cmicx_interrupt;
            if (ctrl->dev_type & BDE_AXI_DEV_TYPE) {
                if (!ihost_intr_enable_base) {
                    ihost_intr_enable_base = (uint32_t *)IOREMAP(HX5_IHOST_GICD_ISENABLERN_1,
                                                                 HX5_IHOST_INTR_MAP_NUM);
                }
                if (!ihost_intr_status_base) {
                    ihost_intr_status_base = (uint32_t *)IOREMAP(HX5_INTC_INTR_RAW_STATUS_REG0,
                                                                 HX5_IHOST_INTR_STATUS_MAP_NUM);
                }
            }
            _intr_regs_init(ctrl, 1);
            break;
        default:
            /* Get CMIC version */
            if (user_bde->get_cmic_ver(d, &ver) != 0) {
                ver = -1;
            }
            device_id = ctrl->devid & device_id_mask;
            /* TH/TH+/TH2 should use cmicd interrupt handler */
            if (BCM56960_DEVICE_ID == device_id ||
                BCM56930_DEVICE_ID == device_id ||
                BCM56970_DEVICE_ID == device_id) {
                ctrl->isr = (isr_f)_cmicd_interrupt;
            }
            /* check if version is CMICX */
            else if (ver == 0x04) {
                ctrl->isr = (isr_f)_cmicx_interrupt;
                _intr_regs_init(ctrl, 0);
            } else {
                ctrl->isr = (isr_f)_cmic_interrupt;
                if ((ctrl->dev_type & BDE_256K_REG_SPACE) &&
#ifdef BCM_PETRA_SUPPORT 
                    ctrl->devid != 0x1234 &&
#endif
                    readl(ctrl->ba + CMICE_DEV_REV_ID) == 0) {
                    ctrl->isr = (isr_f)_cmicm_interrupt;
                }
            }
            break;
        }

        /* configure interrupts for DNX devices using iproc >=14 */
#if defined(BCM_DNXF_SUPPORT) || defined(BCM_DNX_SUPPORT)
        switch (user_bde->get_dev(d)->device & DNXC_DEVID_FAMILY_MASK) {
#ifdef BCM_DNX_SUPPORT
          case JERICHO_2_DEVICE_ID:
          case J2C_DEVICE_ID:
          case J2C_2ND_DEVICE_ID:
          case Q2A_DEVICE_ID:
          case J2P_DEVICE_ID:
#endif
#ifdef BCM_DNXF_SUPPORT
          case  BCM88790_DEVICE_ID:
#endif
            ctrl->isr = (isr_f)_cmicx_interrupt;
            _intr_regs_init(ctrl, 0);
            break;
        }
#endif /* defined(BCM_DNXF_SUPPORT) || defined(BCM_DNX_SUPPORT) */

        if (_intr_mode_str(ctrl->isr) == NULL) {
            gprintk("Warning: Unknown interrupt mode\n");
        }
    }
}
/*
 * Function: _init
 *
 * Purpose:
 *    Module initialization.
 *    Attaches to kernel BDE.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_init(void)
{
    int i;
    phys_addr_t cpu_pbase, dma_pbase;
    ssize_t dmasize;
    bde_inst_resource_t *res;

    /* Connect to the kernel bde */
    if ((linux_bde_create(NULL, &user_bde) < 0) || user_bde == NULL) {
        return -ENODEV;
    }

    init_waitqueue_head(&_ether_interrupt_wq);

    lkbde_get_dma_info(&cpu_pbase, &dma_pbase, &dmasize);

    memset(&_dma_pool, 0, sizeof(_dma_pool));
    _dma_pool.cpu_pbase = cpu_pbase;
    _dma_pool.dma_pbase = dma_pbase;
    _dma_pool.total_size = dmasize / ONE_MB;

    memset(_devices, 0, sizeof(_devices));

    /* Use _bde_inst_resource[0] as the default resource */
    memset(_bde_inst_resource, 0, sizeof(_bde_inst_resource));
    res = &_bde_inst_resource[0];
    res->dma_offset = 0;
    res->dma_size = _dma_pool.total_size;
    init_waitqueue_head(&res->intr_wq);
    atomic_set(&res->intr, 0);

    ihost_intr_enable_base = NULL;
    ihost_intr_status_base = NULL;

    for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
        res->inst_id |= (1 << i);
        _devices_init(i);
    }

    return 0;
}

/*
 * Function: _cleanup
 *
 * Purpose:
 *    Module cleanup function.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_cleanup(void)
{
    int i;

    if (user_bde) {
        for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
            if (_devices[i].enabled &&
                BDE_DEV_MEM_MAPPED(_devices[i].dev_type)) {
                user_bde->interrupt_disconnect(i);
            }
            lkbde_dev_instid_set(i, 0);
        }
        linux_bde_destroy(user_bde);
        user_bde = NULL;
    }

    if (ihost_intr_enable_base) {
        iounmap(ihost_intr_enable_base);
        ihost_intr_enable_base = NULL;
    }
    if (ihost_intr_status_base) {
        iounmap(ihost_intr_status_base);
        ihost_intr_status_base = NULL;
    }

    return 0;
}

/*
 * Function: _pprint
 *
 * Purpose:
 *    Print proc filesystem information.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_pprint(void)
{
    int idx;
    const char *name;
    bde_inst_resource_t *res;
    uint32 state, instid;

    pprintf("Broadcom Device Enumerator (%s)\n", LINUX_USER_BDE_NAME);
    for (idx = 0; idx < user_bde->num_devices(BDE_ALL_DEVICES); idx++) {
        name = _intr_mode_str(_devices[idx].isr);
        if (name == NULL) {
            name = "unknown";
        }
        pprintf("\t%d: Interrupt mode  %s ",idx, name);
        (void)lkbde_dev_state_get(idx, &state);
        if (state == BDE_DEV_STATE_REMOVED) {
            pprintf(" Device REMOVED ! \n");
        } else {
            (void)lkbde_dev_instid_get(idx, &instid);
            if (instid) {
                pprintf("Inst id 0x%x\n",instid);
            } else {
                pprintf("\n");
            }
        }
    }
    pprintf("Instance resource \n");

    for (idx = 0; idx < user_bde->num_devices(BDE_ALL_DEVICES); idx++) {
        res = &_bde_inst_resource[idx];
        if (res->inst_id) {
            pprintf("\tDev mask 0x%x : "
                    "DMA offset %d size %d MB\n",
                    res->inst_id,
                    res->dma_offset,
                    res->dma_size);
        }
    }

    return 0;
}

/* 
 * Allocate the DMA resource from DMA pool
 * Parameter :
 * dma_size (IN): allocate dma_size in MB
 * dma_offset (OUT): dma offset in MB
 */
static int
_dma_resource_alloc(unsigned int dma_size, unsigned int *dma_offset)
{
    uint32 left;

    left = _dma_pool.total_size - _dma_pool.offset;
    if (dma_size > left) {
        gprintk("ERROR: Run out the dma resource!\n");
        return -1;
    }
    *dma_offset = _dma_pool.offset;
    _dma_pool.offset += dma_size;
    return 0;
}

static int
_dma_resource_get(int inst_id, phys_addr_t *cpu_pbase, phys_addr_t *dma_pbase, ssize_t* size)
{
    int i;
    unsigned int dma_size = 0, dma_offset = 0;
    bde_inst_resource_t *res;

    for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
        res = &_bde_inst_resource[i];
        if (res->inst_id == inst_id) {
            dma_size = res->dma_size;
            dma_offset = res->dma_offset;
            break;
        }
    }

    *cpu_pbase = _dma_pool.cpu_pbase + dma_offset * ONE_MB;
    *dma_pbase = _dma_pool.dma_pbase + dma_offset * ONE_MB;
    *size = dma_size * ONE_MB;

    return 0;
}

static int
_instance_validate(unsigned int inst_id, unsigned int dmasize)
{
    int i;
    bde_inst_resource_t *res;

    for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
        res = &_bde_inst_resource[i];
        if (res->inst_id == inst_id) {
            if (res->dma_size != dmasize) {
                if(_devices[i].inst == 0){
                    /* Skip _instance_validate (not init yet) */
                    return LUBDE_SUCCESS;
                }
                gprintk("ERROR: dma_size mismatch\n");
                return LUBDE_FAIL;
            }
            return (1);
        }
    }
    return LUBDE_SUCCESS;
}

static int
_device_reprobe(void)
{
    int i;

    for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
        if (_devices[i].devid == 0) {
            _devices_init(i);
        }
    }
    return 0;
}

static int
_instance_attach(unsigned int inst_id, unsigned int dma_size)
{
    unsigned int dma_offset;
    int i, exist;
    bde_inst_resource_t *res;
    int inst_idx = -1;
    uint32 instid;

    /* Reprobe the system for hot-plugged device */
    _device_reprobe();

    /* Validate the resource with inst_id */
    exist = _instance_validate(inst_id, dma_size);
    if (exist < 0) {
        return LUBDE_FAIL;
    }
    if (exist > 0) {
        return LUBDE_SUCCESS;
    }
    if (_dma_resource_alloc(dma_size, &dma_offset) < 0) {
        return LUBDE_FAIL;
    }
    for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
        res = &_bde_inst_resource[i];
        if ((_bde_multi_inst == 0) || (res->inst_id == 0)) {
            res->inst_id = inst_id;
            res->dma_offset = dma_offset;
            res->dma_size = dma_size;
            _bde_multi_inst++;
            inst_idx = i;
            init_waitqueue_head(&res->intr_wq);
            atomic_set(&res->intr, 0);
            break;
        }
    }

    for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
        if (inst_id & (1 << i)) {
            _devices[i].inst = inst_idx;
            /* Pass the instid to the kernel BDE */
            if (lkbde_dev_instid_get(i, &instid) == 0) {
                if (!instid) {
                    lkbde_dev_instid_set(i, inst_id);
                }
            }
        }
    }

    return LUBDE_SUCCESS;
}

/*
 * Function: _ioctl
 *
 * Purpose:
 *    Handle IOCTL commands from user mode.
 * Parameters:
 *    cmd - IOCTL cmd
 *    arg - IOCTL parameters
 * Returns:
 *    0 on success, <0 on error
 */
static int 
_ioctl(unsigned int cmd, unsigned long arg)
{
    lubde_ioctl_t io;
    phys_addr_t cpu_pbase, dma_pbase;
    ssize_t size;
    const ibde_dev_t *bde_dev;
    int inst_id;
    bde_inst_resource_t *res;
    uint32_t *mapaddr;

    if (copy_from_user(&io, (void *)arg, sizeof(io))) {
        return -EFAULT;
    }
  
    io.rc = LUBDE_SUCCESS;
  
    switch(cmd) {
    case LUBDE_VERSION:
        io.d0 = KBDE_VERSION;
        break;
    case LUBDE_GET_NUM_DEVICES:
        io.d0 = user_bde->num_devices(io.dev);
        break;
    case LUBDE_GET_DEVICE:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        bde_dev = user_bde->get_dev(io.dev);
        if (bde_dev) {
            io.d0 = bde_dev->device;
            io.d1 = bde_dev->rev;
            if (BDE_DEV_MEM_MAPPED(_devices[io.dev].dev_type)) {
                /* Get physical address to map */
                io.d2 = lkbde_get_dev_phys(io.dev);
                io.d3 = lkbde_get_dev_phys_hi(io.dev);
            }
        } else {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_GET_DEVICE_TYPE:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        io.d0 = _devices[io.dev].dev_type;
        break;
    case LUBDE_GET_BUS_FEATURES:
        user_bde->pci_bus_features(io.dev, (int *) &io.d0, (int *) &io.d1,
                                   (int *) &io.d2);
        break;
    case LUBDE_PCI_CONFIG_PUT32:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        if (_devices[io.dev].dev_type & BDE_PCI_DEV_TYPE) {
            user_bde->pci_conf_write(io.dev, io.d0, io.d1);
        } else {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_PCI_CONFIG_GET32:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        if (_devices[io.dev].dev_type & BDE_PCI_DEV_TYPE) {
            io.d0 = user_bde->pci_conf_read(io.dev, io.d0);
        } else {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_GET_DMA_INFO:
        inst_id = io.dev;
        if (_bde_multi_inst){
            _dma_resource_get(inst_id, &cpu_pbase, &dma_pbase, &size);
        } else {
            lkbde_get_dma_info(&cpu_pbase, &dma_pbase, &size);
        }
        io.d0 = dma_pbase;
        io.d1 = size;
        /* Optionally enable DMA mmap via /dev/linux-kernel-bde */
        io.d2 = USE_LINUX_BDE_MMAP;
        /* Get physical address for mmap */
        io.dx.dw[0] = cpu_pbase;
#ifdef PHYS_ADDRS_ARE_64BITS
        io.dx.dw[1] = cpu_pbase >> 32;
#else
        io.dx.dw[1] = 0;
#endif
        break;
    case LUBDE_ENABLE_INTERRUPTS:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        if (_devices[io.dev].dev_type & BDE_SWITCH_DEV_TYPE) {
            if (_devices[io.dev].isr && !_devices[io.dev].enabled) {
                bde_ctrl_t *ctrl;
                ctrl = &_devices[io.dev];
                if ((ctrl->isr == (isr_f)_cmicx_interrupt)  &&
                    (ctrl->dev_type & BDE_PCI_DEV_TYPE)) {
                    /* Set MSI mode to SW clear vs auto clear */
                    WRITE_INTC_INTR(io.dev, ctrl->intr_regs.intc_intr_clear_mode_0, 0x0);
                    WRITE_INTC_INTR(io.dev, ctrl->intr_regs.intc_intr_clear_mode_1, 0x0);
                }
                user_bde->interrupt_connect(io.dev,
                                            _devices[io.dev].isr,
                                            _devices+io.dev);
                _devices[io.dev].enabled = 1;
            }
        } else {
            /* Process ethernet device interrupt */
            
            if (!_devices[io.dev].enabled) {
                user_bde->interrupt_connect(io.dev,
                                            (void(*)(void *))_ether_interrupt, 
                                            _devices+io.dev);
                _devices[io.dev].enabled = 1;
            }
        }
        break;
    case LUBDE_DISABLE_INTERRUPTS:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        if (_devices[io.dev].enabled) {
            user_bde->interrupt_disconnect(io.dev);
            _devices[io.dev].enabled = 0;
        }
        break;
    case LUBDE_WAIT_FOR_INTERRUPT:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        if (_devices[io.dev].dev_type & BDE_SWITCH_DEV_TYPE) {
            res = &_bde_inst_resource[_devices[io.dev].inst];
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
            wait_event_timeout(res->intr_wq, 
                               atomic_read(&res->intr) != 0, 100);

#else
            wait_event_interruptible(res->intr_wq,
                                     atomic_read(&res->intr) != 0);
#endif
            /* 
             * Even if we get multiple interrupts, we 
             * only run the interrupt handler once.
             */
            atomic_set(&res->intr, 0);
        } else {
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
            wait_event_timeout(_ether_interrupt_wq,     
                               atomic_read(&_ether_interrupt_has_taken_place) != 0, 100);
#else
            wait_event_interruptible(_ether_interrupt_wq,     
                                     atomic_read(&_ether_interrupt_has_taken_place) != 0);
#endif
            /* 
             * Even if we get multiple interrupts, we 
             * only run the interrupt handler once.
             */
            atomic_set(&_ether_interrupt_has_taken_place, 0);
        }
        break;
    case LUBDE_USLEEP:
    case LUBDE_UDELAY:
    case LUBDE_SEM_OP:
        return -EINVAL;
    case LUBDE_WRITE_IRQ_MASK:
        io.rc = lkbde_irq_mask_set(io.dev, io.d0, io.d1, 0);
        break;
    case LUBDE_SPI_READ_REG:
        if (user_bde->spi_read(io.dev, io.d0, io.dx.buf, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        } 
        break;
    case LUBDE_SPI_WRITE_REG:
        if (user_bde->spi_write(io.dev, io.d0, io.dx.buf, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_READ_REG_16BIT_BUS:
        io.d1 = user_bde->read(io.dev, io.d0);
        break;
    case LUBDE_WRITE_REG_16BIT_BUS:
        io.rc = user_bde->write(io.dev, io.d0, io.d1);
        break;
#ifdef BCM_SAND_SUPPORT
    case LUBDE_CPU_WRITE_REG:
    {
        if (lkbde_cpu_write(io.dev, io.d0, (uint32*)io.dx.buf) == -1) {
            io.rc = LUBDE_FAIL;
        }
        break;
    }
    case LUBDE_CPU_READ_REG:
    {
        if (lkbde_cpu_read(io.dev, io.d0, (uint32*)io.dx.buf) == -1) {
            io.rc = LUBDE_FAIL;
        }
        break;
    }
    case LUBDE_CPU_PCI_REGISTER:
    {
        if (lkbde_cpu_pci_register(io.dev) == -1) {
            io.rc = LUBDE_FAIL;
        }
        break;
    }
#endif
    case LUBDE_DEV_RESOURCE:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        bde_dev = user_bde->get_dev(io.dev);
        if (bde_dev) {
            if (BDE_DEV_MEM_MAPPED(_devices[io.dev].dev_type)) {
                /* Get physical address to map */
                io.rc = lkbde_get_dev_resource(io.dev, io.d0,
                                               &io.d1, &io.d2, &io.d3);
            }
        } else {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_IPROC_READ_REG:
        if (!VALID_DEVICE(io.dev)) {
            return -EINVAL;
        }
        if (_devices[io.dev].dev_type & BDE_AXI_DEV_TYPE) {
            mapaddr = IOREMAP(io.d0, sizeof(uint32_t));
            if (mapaddr == NULL) {
                io.rc = LUBDE_FAIL;
                return -1;
            }
            io.d1 = readl(mapaddr);
            iounmap(mapaddr);
        } else {
            io.d1 = user_bde->iproc_read(io.dev, io.d0);
            if (io.d1 == -1) {
                io.rc = LUBDE_FAIL;
            }
        }
        break;
    case LUBDE_IPROC_WRITE_REG:
        if (user_bde->iproc_write(io.dev, io.d0, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_ATTACH_INSTANCE:
        io.rc = _instance_attach(io.d0, io.d1);
        break;
    case LUBDE_GET_DEVICE_STATE:
        io.rc = lkbde_dev_state_get(io.dev, &io.d0);
        break;
    case LUBDE_REPROBE:
        io.rc = _device_reprobe();
        break;
    default:
        gprintk("Error: Invalid ioctl (%08x)\n", cmd);
        io.rc = LUBDE_FAIL;
        break;
    }

    if (copy_to_user((void *)arg, &io, sizeof(io))) {
        return -EFAULT;
    }

    return 0;
}

/* Workaround for broken Busybox/PPC insmod */
static char _modname[] = LINUX_USER_BDE_NAME;

static gmodule_t _gmodule = 
{
    name: LINUX_USER_BDE_NAME, 
    major: LINUX_USER_BDE_MAJOR, 
    init: _init, 
    cleanup: _cleanup, 
    pprint: _pprint, 
    ioctl: _ioctl,
}; 

gmodule_t*
gmodule_get(void)
{
    _gmodule.name = _modname;
    return &_gmodule;
}
