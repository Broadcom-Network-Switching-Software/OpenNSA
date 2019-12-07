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
 * File:        soc/shared/dcbformats/type24.h
 * Purpose:     Define dma control block (DCB) format for a type24 DCB
 *              used by the 56440 (Katana)
 *
 *              This file is shared between the SDK and the embedded applications.
 */

#ifndef _SOC_SHARED_DCBFORMATS_TYPE24_H
#define _SOC_SHARED_DCBFORMATS_TYPE24_H

/*
 * DMA Control Block - Type 24
 * Used on 56440 devices
 * 16 words
 */
typedef struct {
        uint32  addr;                   /* T24.0: physical address */
                                        /* T24.1: Control 0 */
#ifdef  LE_HOST
        uint32  c_count:16,             /* Requested byte count */
                c_chain:1,              /* Chaining */
                c_sg:1,                 /* Scatter Gather */
                c_reload:1,             /* Reload */
                c_hg:1,                 /* Higig (TX) */
                c_stat:1,               /* Update stats (TX) */
                c_pause:1,              /* Pause packet (TX) */
                c_purge:1,              /* Purge packet (TX) */
                :9;                     /* Don't care */
#else
        uint32  :9,                     /* Don't care */
                c_purge:1,              /* Purge packet (TX) */
                c_pause:1,              /* Pause packet (TX) */
                c_stat:1,               /* Update stats (TX) */
                c_hg:1,                 /* Higig (TX) */
                c_reload:1,             /* Reload */
                c_sg:1,                 /* Scatter Gather */
                c_chain:1,              /* Chaining */
                c_count:16;             /* Requested byte count */
#endif  /* LE_HOST */
        uint32  reason_hi;              /* T24.2: CPU opcode (hi bits) */
        uint32  reason;                 /* T24.3: CPU opcode (low bits) */
#ifdef  LE_HOST
    union {                             /* T24.4 */
        struct {
            uint32  queue_num:16,       /* Queue number for CPU packets */
                    :4,                 /* Reserved */
                    outer_vid:12;       /* Outer VLAN ID */
        } overlay1;
        struct {
            uint32  cpu_cos:6,          /* CPU COS */
                    :26;                /* Reserved */
        } overlay2;
    } word4;

                                        /* T24.5 */
        uint32  srcport:8,              /* Source port number */
                pkt_len:14,             /* Packet length */
                match_rule:8,           /* Matched FP rule */
                hgi:2;                  /* Higig Interface Format Indicator */
#else
    union {                             /* T24.4 */
        struct {
            uint32  outer_vid:12,       /* Outer VLAN ID */
                    :4,                 /* Reserved */
                    queue_num:16;       /* Queue number for CPU packets */
        } overlay1;
        struct {
            uint32  :26,                /* Reserved */
                    cpu_cos:6;          /* CPU COS */
        } overlay2;
    } word4;

                                        /* T24.5 */
        uint32  hgi:2,                  /* Higig Interface Format Indicator */
                match_rule:8,           /* Matched FP rule */
                pkt_len:14,             /* Packet length */
                srcport:8;              /* Source port number */
#endif

        uint32  mh0;                    /* T24.6: Module Header word 0 */
        uint32  mh1;                    /* T24.7: Module Header word 1 */
        uint32  mh2;                    /* T24.8: Module Header word 2 */
        uint32  mh3;                    /* T24.9: Module Header word 3 */
#ifdef  LE_HOST
                                        /* T24.10 */
        uint32  inner_pri:3,            /* Inner priority */
                :1,                     /* Reserved */
                regen_crc:1,            /* Packet modified and needs new CRC */
                repl_nhi:19,            /* Replication or Next Hop index */
                vfi_valid:1,            /* Validates VFI field */
                :7;                     /* Reserved */

    union {                             /* T24.11 */
        struct {
            uint32  dscp:6,             /* New DSCP (+ oam pkt type:3) */
                    chg_tos:1,          /* DSCP has been changed by HW */
                    vntag_action:2,     /* VN tag action */
                    outer_cfi:1,        /* Outer Canoncial Format Indicator */
                    outer_pri:3,        /* Outer priority */
                    decap_tunnel_type:5,/* Tunnel type that was decapsulated */
                    inner_vid:12,       /* Inner VLAN ID */
                    inner_cfi:1,        /* Inner Canoncial Format Indicator */
                    :1;                 /* Reserved */
        } overlay1;
        struct {
            uint32  special_pkt_type:3, /* Special packet type (OAM, BFD) */
                    :29;                /* Reserved */
        } overlay2;
    } word11;

                                        /* T24.12 */
        uint32  timestamp;              /* Timestamp (+ VFI:14) */

                                        /* T24.13 */
        uint32  itag_status:2,          /* Ingress Inner tag status */
                otag_action:2,          /* Ingress Outer tag action */
                itag_action:2,          /* Ingress Inner tag action */
                service_tag:1,          /* SD tag present */
                switch_pkt:1,           /* Switched packet */
                all_switch_drop:1,      /* All switched copies dropped */
                hg_type:1,              /* 0: Higig+, 1: Higig2 */
                src_hg:1,               /* Source is Higig */
                l3routed:1,             /* Any IP routed packet */
                l3only:1,               /* L3 only IPMC packet */
                replicated:1,           /* Replicated copy */
                hg2_ext_hdr:1,          /* Extended Higig2 header valid */
                oam_pkt:1,              /* OAM Packet indicator */
                do_not_change_ttl:1,    /* Do not change TTL */
                bpdu:1,                 /* BPDU Packet */
                timestamp_type:2,       /* Timestamp type */
                loopback_pkt_type:4,    /* Loopback packet type */
                mtp_index:5,            /* MTP index */
                ecn:2,                  /* New ECN value */
                chg_ecn:1;              /* ECN changed */

    union {                             /* T24.14 */
        struct {
            uint32  timestamp_hi;       /* Timestamp upper bits */
        } overlay1;
        struct {
            uint32  eh_seg_sel:3,       /* HG2 EH seg sel */
                    eh_tag_type:2,      /* HG2 EH tag type */
                    eh_tm:1,            /* HG2 EH tm */
                    eh_queue_tag: 16,   /* HG2 EH queue tag */                
                    eh_tag_reserved:10; /* Reserved */
        } overlay2;
        struct {
            uint32  rx_bfd_session_index:11,    /* Rx BFD pkt session ID */
                    rx_bfd_start_offset_type:2, /* Rx BFD pkt st offset type */
                    rx_bfd_start_offset:8,      /* Rx BFD pkt start offset */
                   :11;                        /* Reserved */
        } overlay3;
    } word14;

                                         /* T24.15: DMA Status 0 */
        uint32  count:16,                /* Transferred byte count */
                end:1,                   /* End bit (RX) */
                start:1,                 /* Start bit (RX) */
                error:1,                 /* Cell Error (RX) */
                pktdata_read_ecc_error:1,/* 2 bit ECC error while reading packet
                                            data from RX data Buffers.*/
                status_read_ecc_error:1, /* 2 bit ECC error while reading Status
                                            from RX Status Buffers.*/
                :10,                     /* Reserved */
                done:1;                  /* Descriptor Done */
#else
                                        /* T24.10 */
        uint32  :7,                     /* Reserved */
                vfi_valid:1,            /* Validates VFI field */
                repl_nhi:19,            /* Replication or Next Hop index */
                regen_crc:1,            /* Packet modified and needs new CRC */
                :1,                     /* Reserved */
                inner_pri:3;            /* Inner priority */


    union {                             /* T24.11 */
        struct {
            uint32  :1,                 /* Reserved */
                    inner_cfi:1,        /* Inner Canoncial Format Indicator */
                    inner_vid:12,       /* Inner VLAN ID */
                    decap_tunnel_type:5,/* Tunnel type that was decapsulated */
                    outer_pri:3,        /* Outer priority */
                    outer_cfi:1,        /* Outer Canoncial Format Indicator */
                    vntag_action:2,     /* VN tag action */
                    chg_tos:1,          /* DSCP has been changed by HW */
                    dscp:6;             /* New DSCP (+ oam pkt type:3) */
        } overlay1;
        struct {
            uint32  :29,                /* Reserved */
                    special_pkt_type:3; /* Special packet type (OAM, BFD) */
        } overlay2;
    } word11;

                                        /* T24.12 */
        uint32  timestamp;              /* Timestamp (+ VFI:14) */

                                        /* T24.13 */
        uint32  chg_ecn:1,              /* ECN changed */
                ecn:2,                  /* New ECN value */
                mtp_index:5,            /* MTP index */
                loopback_pkt_type:4,    /* Loopback packet type */
                timestamp_type:2,       /* Timestamp type */
                bpdu:1,                 /* BPDU Packet */
                do_not_change_ttl:1,    /* Do not change TTL */
                oam_pkt:1,              /* OAM Packet indicator */
                hg2_ext_hdr:1,          /* Extended Higig2 header valid */
                replicated:1,           /* Replicated copy */
                l3only:1,               /* L3 only IPMC packet */
                l3routed:1,             /* Any IP routed packet */
                src_hg:1,               /* Source is Higig */
                hg_type:1,              /* 0: Higig+, 1: Higig2 */
                all_switch_drop:1,      /* All switched copies dropped */
                switch_pkt:1,           /* Switched packet */
                service_tag:1,          /* SD tag present */
                itag_action:2,          /* Ingress Inner tag action */
                otag_action:2,          /* Ingress Outer tag action */
                itag_status:2;          /* Ingress Inner tag status */

    union {                             /* T24.14 */
        struct {
            uint32  timestamp_hi;       /* Timestamp upper bits */
        } overlay1;
        struct {
            uint32  eh_tag_reserved:10, /* Reserved */
                    eh_queue_tag: 16,   /* HG2 EH queue tag */
                    eh_tm:1,            /* HG2 EH tm */
                    eh_tag_type:2,      /* HG2 EH tag type */
                    eh_seg_sel:3;       /* HG2 EH seg sel */
        } overlay2;
        struct {
            uint32  :11,                        /* Reserved */
                    rx_bfd_start_offset:8,      /* Rx BFD pkt start offset */
                    rx_bfd_start_offset_type:2, /* Rx BFD pkt st offset type */
                    rx_bfd_session_index:11;    /* Rx BFD pkt session ID */
        } overlay3;
    } word14;

                                         /* T24.15: DMA Status 0 */
        uint32  done:1,                  /* Descriptor Done */
                :10,                     /* Reserved */
                status_read_ecc_error:1, /* 2 bit ECC error while reading Status
                                            from RX Status Buffers. */
                pktdata_read_ecc_error:1,/* 2 bit ECC error while reading packet
                                            data from RX data Buffers.*/
                error:1,                 /* Cell Error (RX) */
                start:1,                 /* Start bit (RX) */
                end:1,                   /* End bit (RX) */
                count:16;                /* Transferred byte count */
#endif
} dcb24_t;
#endif

