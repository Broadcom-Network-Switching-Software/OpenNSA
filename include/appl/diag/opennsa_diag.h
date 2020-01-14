/*
 * This software is governed by the Broadcom Switch APIs license.
 * This license is set out in https://raw.githubusercontent.com/Broadcom-Network-Switching-Software/OpenNSA/master/Legal/Legal/LICENSE file.
 *
 * Copyright 2007-2020 Broadcom Inc. All rights reserved.
 *
 * This header file defines important constants and structures for
 * use with Broadcom Diagnostics Shell which is built upon the SOC
 * driver and SAL (System Abstraction Layer)
 */

#ifndef _DIAGSHELL_H
#define _DIAGSHELL_H

/*
 * Shell routines: used for diagnostics shell.
 */

extern void    diag_init(void);
extern void    diag_rc_set(int unit, const char *fname);
extern void    diag_rc_get(int unit, const char **fname);
extern int     diag_rc_load(int unit);
extern void    diag_shell(void);

extern int     system_init(int unit);
extern int     system_shutdown(int unit, int cleanup);

#endif /* !_DIAGSHELL_H */
