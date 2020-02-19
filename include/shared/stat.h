/* 
 * 
 * This software is governed by the Broadcom Switch APIs license.
 * This license is set out in https://raw.githubusercontent.com/Broadcom-Network-Switching-Software/OpenNSA/master/Legal/LICENSE file.
 * 
 * Copyright 2007-2020 Broadcom Inc. All rights reserved.
 *
 * File:        stat.h
 * Purpose:     Defines common stat parameters.
 */

#ifndef   _SHR_STAT_H_
#define   _SHR_STAT_H_

#define _SHR_STAT_COUNTER_STAT_MACRO_INDICATION_POSITION (31)
#define _SHR_STAT_COUNTER_STAT_IS_BYTE_POSITION (16)
#define _SHR_STAT_COUNTER_STAT_OFFSET_POSITION (0)
#define _SHR_STAT_COUNTER_STAT_MACRO_INDICATION_MASK (0x1)
#define _SHR_STAT_COUNTER_STAT_IS_BYTE_MASK (0x1)
#define _SHR_STAT_COUNTER_STAT_OFFSET_MASK (0xFF)


#define _SHR_STAT_COUNTER_STAT_SET(is_bytes, offset) \
    ((1 << _SHR_STAT_COUNTER_STAT_MACRO_INDICATION_POSITION) | \
     ((is_bytes & _SHR_STAT_COUNTER_STAT_IS_BYTE_MASK) << _SHR_STAT_COUNTER_STAT_IS_BYTE_POSITION) | \
     ((offset & _SHR_STAT_COUNTER_STAT_OFFSET_MASK) << _SHR_STAT_COUNTER_STAT_OFFSET_POSITION))

#define _SHR_STAT_COUNTER_STAT_OFFSET_GET(stat) \
    ((stat >> _SHR_STAT_COUNTER_STAT_OFFSET_POSITION) & _SHR_STAT_COUNTER_STAT_OFFSET_MASK)
#define _SHR_STAT_COUNTER_STAT_IS_BYTES_GET(stat) \
    ((stat >> _SHR_STAT_COUNTER_STAT_IS_BYTE_POSITION) & _SHR_STAT_COUNTER_STAT_IS_BYTE_MASK)

#define _SHR_STAT_COUNTER_STAT_IS_VALID(stat) \
    ((stat >> _SHR_STAT_COUNTER_STAT_MACRO_INDICATION_POSITION) & _SHR_STAT_COUNTER_STAT_MACRO_INDICATION_MASK) ? TRUE : FALSE

#endif /* _SHR_STAT_H_ */
