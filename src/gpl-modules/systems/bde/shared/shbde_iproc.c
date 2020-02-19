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

#include <shbde_iproc.h>
#include <shbde_mdio.h>
#include <shbde_pci.h>

/* PAXB register offsets within PCI BAR0 window */
#define BAR0_PAXB_ENDIANESS                     0x2030
#define BAR0_PAXB_PCIE_EP_AXI_CONFIG            0x2104
#define BAR0_PAXB_CONFIG_IND_ADDR               0x2120
#define BAR0_PAXB_CONFIG_IND_DATA               0x2124

#define PAXB_0_CMICD_TO_PCIE_INTR_EN            0x2380

#define BAR0_PAXB_IMAP0_0                       (0x2c00)
#define BAR0_PAXB_IMAP0_1                       (0x2c04)
#define BAR0_PAXB_IMAP0_2                       (0x2c08)
#define BAR0_PAXB_IMAP0_7                       (0x2c1c)

#define BAR0_PAXB_OARR_FUNC0_MSI_PAGE           0x2d34
#define BAR0_PAXB_OARR_2                        0x2d60
#define BAR0_PAXB_OARR_2_UPPER                  0x2d64
#define BAR0_DMU_PCU_PCIE_SLAVE_RESET_MODE      0x7024

#define PAXB_0_FUNC0_IMAP1_3                    0x2d88

/* Force byte pointer for offset adjustments */
#define ROFFS(_ptr, _offset) ((unsigned char*)(_ptr) + (_offset))

#define PAXB_CONFIG_IND_ADDRr_PROTOCOL_LAYERf_SHFT      11
#define PAXB_CONFIG_IND_ADDRr_PROTOCOL_LAYERf_MASK      0x3
#define PAXB_CONFIG_IND_ADDRr_ADDRESSf_SHFT             0
#define PAXB_CONFIG_IND_ADDRr_ADDRESSf_MASK             0x7ff
#define PAXB_0_FUNC0_IMAP1_3_ADDR_SHIFT                 20

/* Register value set/get by field */
#define REG_FIELD_SET(_r, _f, _r_val, _f_val) \
        _r_val = ((_r_val) & ~(_r##_##_f##_MASK << _r##_##_f##_SHFT)) | \
                 (((_f_val) & _r##_##_f##_MASK) << _r##_##_f##_SHFT)
#define REG_FIELD_GET(_r, _f, _r_val) \
        (((_r_val) >> _r##_##_f##_SHFT) & _r##_##_f##_MASK)

/* PCIe capabilities definition */
#ifndef PCI_EXP_LNKSTA
#define PCI_EXP_LNKSTA                          0x12
#endif
/* Current Link Speed 5.0GT/s */
#ifndef PCI_EXP_LNKSTA_CLS_5_0GB
#define PCI_EXP_LNKSTA_CLS_5_0GB                2
#endif
#ifndef PCI_EXP_LNKSTA2
#define PCI_EXP_LNKSTA2                         0x32
#endif
/* Current Deemphasis Level -3.5 dB */
#ifndef PCI_EXP_LNKSTA2_CDL_3_5DB
#define PCI_EXP_LNKSTA2_CDL_3_5DB               0x1
#endif

static unsigned int
iproc32_read(shbde_hal_t *shbde, void *addr)
{
    if (!shbde || !shbde->io32_read) {
        return 0;
    }
    return shbde->io32_read(addr);
}

static void
iproc32_write(shbde_hal_t *shbde, void *addr, unsigned int data)
{
    if (!shbde || !shbde->io32_write) {
        return;
    }
    shbde->io32_write(addr, data);
}

static void
wait_usec(shbde_hal_t *shbde, int usec)
{
    if (shbde && shbde->usleep) {
        shbde->usleep(usec);
    } else {
        int idx;
        volatile int count;
        for (idx = 0; idx < usec; idx++) {
            for (count = 0; count < 100; count++);
        }
    }
}

/*
 * Function:
 *      shbde_iproc_config_init
 * Purpose:
 *      Initialize iProc configuration parameters
 * Parameters:
 *      icfg - pointer to empty iProc configuration structure
 * Returns:
 *      -1 if error, otherwise 0
 */
int
shbde_iproc_config_init(shbde_iproc_config_t *icfg,
                        unsigned int dev_id, unsigned int dev_rev)
{
    if (!icfg) {
        return -1;
    }

    /* Save device ID and revision */
    icfg->dev_id = dev_id;
    icfg->dev_rev = dev_rev;

    /* Check device families first */
    switch (icfg->dev_id & 0xfff0) {
    case 0x8400: /* Greyhound Lite */
    case 0x8410: /* Greyhound */
    case 0x8420: /* Bloodhound */
    case 0x8450: /* Elkhound */
    case 0xb060: /* Ranger2(Greyhound) */
    case 0x8360: /* Greyhound 53365 & 53369 */
    case 0xb260: /* saber2 */
    case 0xb460: /* saber2+ */
    case 0xb170: /* Hurricane3-MG */
    case 0x8570: /* Greyhound2 */
    case 0xb070: /* Greyhound2(emulation) */
    case 0x8580: /* Greyhound2(emulation) */
    case 0xb230: /* Dagger2 */
        icfg->iproc_ver = 7;
        icfg->dma_hi_bits = 0x2;
        break;
    case 0xb560: /* Apache */
    case 0xb670: /* MO */
    case 0xb760: /* Maverick */
        icfg->iproc_ver = 0xB;
        break;
    case 0xb160: /* Hurricane3 */
    case 0x8440: /* Buckhound2 */
    case 0x8430: /* Foxhound2 */
    case 0x8540: /* Wolfhound2 */
        icfg->iproc_ver = 10;
        icfg->dma_hi_bits = 0x2;
        break;
    default:
        break;
    }

    /* Check for exceptions */
    switch (icfg->dev_id) {
       case 0xb069:
       case 0xb068:
          icfg->iproc_ver = 0xB;       /*Ranger2+  Apache Family */
          icfg->dma_hi_bits = 0;
          break;
    case 0xb168: /* Ranger3+ */
    case 0xb169:
        icfg->iproc_ver = 0;
        icfg->dma_hi_bits = 0;
        break;
    default:
        break;
    }
    /* Check for PCIe PHY address that needs PCIe preemphasis and
     * assign the MDIO base address
     */
    switch (icfg->dev_id & 0xfff0) {
    case 0xb150: /* Hurricane2 */
    case 0x8340: /* Wolfhound */
    case 0x8330: /* Foxhound */
    case 0x8390: /* Dearhound */
        icfg->mdio_base_addr = 0x18032000;
        icfg->pcie_phy_addr = 0x2;
        break;
    case 0xb340: /* Helilx4 */
    case 0xb540: /* FireScout */
    case 0xb040: /* Spiral, Ranger */
        icfg->mdio_base_addr = 0x18032000;
        icfg->pcie_phy_addr = 0x5;
        icfg->adjust_pcie_preemphasis = 1;
        break;
    case 0xa450: /* Katana2 */
    case 0xb240:
    case 0xb450:
        icfg->mdio_base_addr = 0x18032000;
        icfg->pcie_phy_addr = 0x5;
        icfg->adjust_pcie_preemphasis = 1;
        break;
    default:
        break;
    }

    /* Check for exceptions */
    switch (icfg->dev_id) {
    default:
        break;
    }

    return 0;
}

/*
 * Function:
 *      shbde_iproc_paxb_init
 * Purpose:
 *      Initialize iProc PCI-AXI bridge for CMIC access
 * Parameters:
 *      shbde - pointer to initialized hardware abstraction module
 *      iproc_regs - memory mapped iProc registers in PCI BAR
 *      icfg - iProc configuration parameters
 * Returns:
 *      -1 if error, otherwise 0
 */
int
shbde_iproc_paxb_init(shbde_hal_t *shbde, void *iproc_regs,
                      shbde_iproc_config_t *icfg)
{
    void *reg;
    unsigned int data;
    int pci_num;

    if (!iproc_regs || !icfg) {
        return -1;
    }

    /*
     * The following code attempts to auto-detect the correct
     * iProc PCI endianess configuration by reading a well-known
     * register (the endianess configuration register itself).
     * Note that the PCI endianess may be different for different
     * big endian host processors.
     */
    reg = ROFFS(iproc_regs, BAR0_PAXB_ENDIANESS);
    /* Select big endian */
    iproc32_write(shbde, reg, 0x01010101);
    /* Check if endianess register itself is correct endian */
    if (iproc32_read(shbde, reg) != 1) {
        /* If not, then assume little endian */
        iproc32_write(shbde, reg, 0x0);
    }

    /* Select which PCI core to use */
    pci_num = 0;
    reg = ROFFS(iproc_regs, BAR0_PAXB_IMAP0_2);
    data = iproc32_read(shbde, reg);
    if (data  & 0x1000) {
        /* PAXB_1 is mapped to sub-window 2 */
        pci_num = 1;
    }

    /* Default DMA mapping if uninitialized */
    if (icfg->dma_hi_bits == 0) {
        icfg->dma_hi_bits = 0x1;
        if (pci_num == 1) {
            icfg->dma_hi_bits = 0x2;
        }
    }

    /* Enable iProc DMA to external host memory */
    reg = ROFFS(iproc_regs, BAR0_PAXB_PCIE_EP_AXI_CONFIG);
    iproc32_write(shbde, reg, 0x0);
    if(icfg->cmic_ver < 4) { /* Non-CMICX */
        reg = ROFFS(iproc_regs, BAR0_PAXB_OARR_2);
        iproc32_write(shbde, reg, 0x1);
        reg = ROFFS(iproc_regs, BAR0_PAXB_OARR_2_UPPER);
        iproc32_write(shbde, reg, icfg->dma_hi_bits);
        /* Configure MSI interrupt page */
        if (icfg->use_msi) {
            reg = ROFFS(iproc_regs, BAR0_PAXB_OARR_FUNC0_MSI_PAGE);
            data = iproc32_read(shbde, reg);
            iproc32_write(shbde, reg, data | 0x1);
        }
    }

    /* Configure MSIX interrupt page, only need for iproc ver == 0x10 */
    if ((icfg->use_msi == 2) && (icfg->iproc_ver == 0x10)) {
        unsigned int mask = (0x1 << PAXB_0_FUNC0_IMAP1_3_ADDR_SHIFT) - 1;
        reg = ROFFS(iproc_regs, PAXB_0_FUNC0_IMAP1_3);
        data = iproc32_read(shbde, reg);
        data &= mask;
        data |= 0x410 << PAXB_0_FUNC0_IMAP1_3_ADDR_SHIFT;
        iproc32_write(shbde, reg, data);
    }

    /* Disable INTx interrupt if MSI/MSIX is selected */
    reg = ROFFS(iproc_regs, PAXB_0_CMICD_TO_PCIE_INTR_EN);
    data = iproc32_read(shbde, reg);
    if (icfg->use_msi) {
        data &= ~0x1;
    } else {
        data |= 0x1;
    }
    iproc32_write(shbde, reg, data);

    return pci_num;
}

/*
 * Function:
 *      shbde_iproc_pci_read
 * Purpose:
 *      Read iProc register through PCI BAR 0
 * Parameters:
 *      shbde - pointer to initialized hardware abstraction module
 *      iproc_regs - memory mapped iProc registers in PCI BAR
 *      addr - iProc register address in AXI memory space
 * Returns:
 *      Register value
 */
unsigned int
shbde_iproc_pci_read(shbde_hal_t *shbde, void *iproc_regs,
                     unsigned int addr)
{
    unsigned int subwin_base;
    void *reg;
    shbde_iproc_config_t *icfg = &shbde->icfg;

    if (!iproc_regs) {
        return -1;
    }

    /* Sub-window size is 0x1000 (4K) */
    subwin_base = (addr & ~0xfff);

    if((icfg->cmic_ver >= 4) &&
       ((subwin_base == 0x10230000) || (subwin_base == 0x18012000))) {
        /* Route the PAXB register through IMAP0_2 */
        reg = ROFFS(iproc_regs, 0x2000 + (addr & 0xfff));
    } else if((icfg->cmic_ver >= 4) &&
       ((subwin_base == 0x10231000) || (subwin_base == 0x18013000))) {
        /* Route the INTC block access through IMAP0_6 */
        reg = ROFFS(iproc_regs, 0x6000 + (addr & 0xfff));
    } else {
    /* Update base address for sub-window 7 */
        subwin_base |= 1; /* Valid bit */
        reg = ROFFS(iproc_regs, BAR0_PAXB_IMAP0_7);
        iproc32_write(shbde, reg, subwin_base);
        /* Read it to make sure the write actually goes through */
        subwin_base = iproc32_read(shbde, reg);

        /* Read register through sub-window 7 */
        reg = ROFFS(iproc_regs, 0x7000 + (addr & 0xfff));
    }

    return iproc32_read(shbde, reg);
}

/*
 * Function:
 *      shbde_iproc_pci_write
 * Purpose:
 *      Write iProc register through PCI BAR 0
 * Parameters:
 *      shbde - pointer to initialized hardware abstraction module
 *      iproc_regs - memory mapped iProc registers in PCI BAR
 *      addr - iProc register address in AXI memory space
 *      data - data to write to iProc register
 * Returns:
 *      Register value
 */
void
shbde_iproc_pci_write(shbde_hal_t *shbde, void *iproc_regs,
                      unsigned int addr, unsigned int data)
{
    unsigned int subwin_base;
    void *reg;
    shbde_iproc_config_t *icfg = &shbde->icfg;

    if (!iproc_regs) {
        return;
    }

    /* Sub-window size is 0x1000 (4K) */
    subwin_base = (addr & ~0xfff);

    if((icfg->cmic_ver >= 4) &&
       ((subwin_base == 0x10230000) || (subwin_base == 0x18012000))) {
        /* Route the PAXB register through IMAP0_2 */
        reg = ROFFS(iproc_regs, 0x2000 + (addr & 0xfff));
    } else if((icfg->cmic_ver >= 4) &&
       ((subwin_base == 0x10231000) || (subwin_base == 0x18013000))) {
        /* Route the INTC block access through IMAP0_6 */
        reg = ROFFS(iproc_regs, 0x6000 + (addr & 0xfff));
    } else {
    /* Update base address for sub-window 7 */
        subwin_base |= 1; /* Valid bit */
        reg = ROFFS(iproc_regs, BAR0_PAXB_IMAP0_7);
        iproc32_write(shbde, reg, subwin_base);
        /* Read it to make sure the write actually goes through */
        subwin_base = iproc32_read(shbde, reg);

        /* Read register through sub-window 7 */
        reg = ROFFS(iproc_regs, 0x7000 + (addr & 0xfff));
    }

    iproc32_write(shbde, reg, data);
}

int
shbde_iproc_pcie_preemphasis_set(shbde_hal_t *shbde, void *iproc_regs,
                                 shbde_iproc_config_t *icfg, void *pci_dev)
{
    shbde_mdio_ctrl_t mdio_ctrl, *smc = &mdio_ctrl;
    unsigned int phy_addr, data;
    void *reg;
    unsigned int pcie_cap_base;
    unsigned short link_stat, link_stat2;

    if (!icfg) {
        return -1;
    }

    /* PHY address for PCIe link */
    phy_addr = icfg->pcie_phy_addr;
    if (phy_addr == 0 || icfg->mdio_base_addr == 0) {
        return 0;
    }

    /* Initialize MDIO control */
    smc->shbde = shbde;
    smc->regs = iproc_regs;
    smc->base_addr = icfg->mdio_base_addr;
    smc->io32_read = shbde_iproc_pci_read;
    smc->io32_write = shbde_iproc_pci_write;
    shbde_iproc_mdio_init(smc);

    /* PCIe SerDes Gen1/Gen2 CDR Track Bandwidth Adjustment
     * for Better Jitter Tolerance
     */
    shbde_iproc_mdio_write(smc, phy_addr, 0x1f, 0x8630);
    shbde_iproc_mdio_write(smc, phy_addr, 0x13, 0x190);
    shbde_iproc_mdio_write(smc, phy_addr, 0x19, 0x191);

    if (!icfg->adjust_pcie_preemphasis) {
        return 0;
    }

    /* Check to see if the PCIe SerDes deemphasis needs to be changed
     * based on the advertisement from the root complex
     */
    /* Find PCIe capability base */
    if (!shbde || !shbde->pcic16_read || !pci_dev) {
        return -1;
    }
    pcie_cap_base = shbde_pci_pcie_cap(shbde, pci_dev);
    if (pcie_cap_base) {
        link_stat = shbde->pcic16_read(pci_dev,
                                       pcie_cap_base + PCI_EXP_LNKSTA);
        link_stat2 = shbde->pcic16_read(pci_dev,
                                        pcie_cap_base + PCI_EXP_LNKSTA2);
        if (((link_stat & 0xf) == PCI_EXP_LNKSTA_CLS_5_0GB) &&
            (link_stat2 & PCI_EXP_LNKSTA2_CDL_3_5DB)) {
            /* Device is operating at Gen2 speeds and RC requested -3.5dB */
            /* Change the transmitter setting */
            shbde_iproc_mdio_write(smc, phy_addr, 0x1f, 0x8610);
            shbde_iproc_mdio_read(smc, phy_addr, 0x17, &data);
            data &= ~0xf00;
            data |= 0x700;
            shbde_iproc_mdio_write(smc, phy_addr, 0x17, data);

            /* Force the PCIe link to retrain */
            data = 0;
            REG_FIELD_SET(PAXB_CONFIG_IND_ADDRr, PROTOCOL_LAYERf, data, 0x2);
            REG_FIELD_SET(PAXB_CONFIG_IND_ADDRr, ADDRESSf, data, 0x4);
            reg = ROFFS(iproc_regs, BAR0_PAXB_CONFIG_IND_ADDR);
            iproc32_write(shbde, reg, data);

            reg = ROFFS(iproc_regs, BAR0_PAXB_CONFIG_IND_DATA);
            data = iproc32_read(shbde, reg);
            data &= ~0x4000;
            iproc32_write(shbde, reg, data);
            data |= 0x4000;
            iproc32_write(shbde, reg, data);
            data &= ~0x4000;
            iproc32_write(shbde, reg, data);

            /* Wait a short while for the retraining to complete */
            wait_usec(shbde, 1000);
        }
    }

    return 0;
}

