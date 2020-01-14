#
# This software is governed by the Broadcom Switch APIs license.
# This license is set out in https://raw.githubusercontent.com/Broadcom-Network-Switching-Software/OpenNSA/master/Legal/LICENSE file.
#
# Copyright 2007-2020 Broadcom Inc. All rights reserved.
#

ifeq ($(SDK),)
$(error Environment variable "$SDK" must be set)
endif

DRV_LIBS_NOSHELL = -lopennsa
DRV_LIBS_SHELL = -Wl,--start-group -lopennsa -Wl,--end-group

all: opennsa_no_shell opennsa_diag_shell

opennsa_no_shell: opennsa_no_shell.c version.c config_init_defaults.c $(SDK)/lib/x86-64/libopennsa.a
	gcc -I$(SDK)/include -I$(SDK)/src/sdk/gpl-modules/systems/bde/linux/include -I$(SDK)/src/sdk/gpl-modules/include -DSYS_BE_PIO=0 -DSYS_BE_PACKET=0 -DSYS_BE_OTHER=0 -DLE_HOST=1 -DNO_SAL_APPL -o $@ $^ -L$(SDK)/lib/x86-64 $(DRV_LIBS_NOSHELL) -lrt -pthread -lm

opennsa_diag_shell: opennsa_diag_shell.c version.c config_init_defaults.c $(SDK)/lib/x86-64/libopennsa.a
	gcc -I$(SDK)/include -I$(SDK)/src/sdk/gpl-modules/systems/bde/linux/include -I$(SDK)/src/sdk/gpl-modules/include -DSYS_BE_PIO=0 -DSYS_BE_PACKET=0 -DSYS_BE_OTHER=0 -DLE_HOST=1 -DNO_SAL_APPL -o $@ $^ -L$(SDK)/lib/x86-64 $(DRV_LIBS_SHELL) -lrt -pthread -lm
