OpenNSA
=======
OpenNSA Broadcom Switch Software 

Version
=======
6.5.17

Overview
========

Broadcom Open Network Switch APIs (OpenNSA) is a library of network switch APIs that
is openly available for programming Broadcom network switch silicon based
platforms. These APIs enable development of networking application
software based on Broadcom network switch architecture based platforms.

Migration from OpenNsl
======================
This software package is replacing the OpenNSL (https://github.com/Broadcom-Switch/OpenNSL) software package.  OpenNSL users are encouraged to migrate this release by replacing all OpenNSL API calls with bcm_xxx API calls in their application and use the new opensoc APIs for soc and SDK layer initialization.

Legal
=====

The SDK APIs, sample application and binary library are covered by the following license see Legal/LICENSE file.
SDK uses a few open source components, see Legal/EXTLICENSE file for license terms.
The software in the gpl-module and dcb-gpl-modules directories are covered by GPLv2 license.

Supported devices
=================

This release supports the following devices:
* BCM56850 "Trident2"
* BCM56960 "Tomahawk"
* BCM56970 "Tomahawk2"
* BCM56980 "Tomahawk3"
* BCM88470 "Qumran AX"

Build configuration
======================

The included libopennsa.a library was built for x86 without debug symbols.

Built-in demo utility
======================

The package includes a sample utility that can be used for the following purposes:
1. To verify that an application can be linked successfully with the provided library
1. To verify that the application can successfully probe and attach a supported Broadcom device
1. To verify that the application can successfully initialize the device

The demo application can be found in $SDK/src/diag and can be compiled by executing "make" in that directory.

When executed on a system that has a supported Broadcom switch device on a PCIe bus, the utility should produce the following output:

    Broadcom_SVK_Linux# cd opennsa/src/diag
    Broadcom_SVK_Linux# ./opennsa_diag
    DMA pool size: 33554432
    BDE unit 0 (PCI), Dev 0xb980, Rev 0x11, Chip BCM56980_B0, Driver BCM56980_B0
    SUCCESS: soc_attach( 0 ), result=0
    SUCCESS: soc_init( 0 ), result=0
    SUCCESS: soc_misc_init( 0 ), result=0
    SUCCESS: soc_mmu_init( 0 ), result=0
    SUCCESS: bcm_init( 0 ), result=0
    Unit=0 initialization completed.
    Broadcom_SVK_Linux# 

Note that the utility is built with the assumption that the device can be initialized with the default configuration and may not work across all SKUs or device variants.  It is intended to be used primarily as a build verification utility and a sample smoke test to demonstrate the compilation, linking and device initialization process with OpenNSA.

When integrating the OpenNSA library into your network operating system you should use the example demo program for how to make the soc/opensoc.h API calls which are necessary for SDK initialization and startup.
