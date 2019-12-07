#
#  Copyright 2017 Broadcom
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License, version 2, as
#  published by the Free Software Foundation (the "GPL").
#  
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License version 2 (GPLv2) for more details.
#  
#  You should have received a copy of the GNU General Public License
#  version 2 (GPLv2) along with this source code.
#
# Makefile for DCB Library
#

# General Compilation Flags
CC = gcc
CFLAGS = -c -Wall -Werror
ARFLAGS = cru

# DCB Directory
ifndef DCBDIR
DCBDIR = ..
endif

# Build Directory
ifndef BLDDIR
BLDDIR = $(DCBDIR)/build
endif

# Include Directories
INCDIR = -I$(DCBDIR)/include

# Sources
SOURCES = \
	$(DCBDIR)/src/soc/common/dcb.c \
	$(DCBDIR)/src/appl/dcb/dcb_handler.c

# Objects
OBJECTS=$(subst .c,.o,$(subst $(DCBDIR),$(BLDDIR),$(SOURCES)))
OBJECTSDIR = $(dir $(OBJECTS))

# Library
LIB = $(BLDDIR)/libdcb.a


# Compilation Flags
CFLAGS += $(INCDIR)

DFLAGS =
DFLAGS += -DBCM_53400_A0    # GREYHOUND
DFLAGS += -DBCM_53570_A0    # GREYHOUND2
DFLAGS += -DBCM_56150_A0    # HURRICANE2
DFLAGS += -DBCM_56160_A0    # HURRICANE3
DFLAGS += -DBCM_56340_A0    # HELIX4
DFLAGS += -DBCM_56450_A0    # KATANA2
DFLAGS += -DBCM_56560_A0    # APACHE
DFLAGS += -DBCM_56850_A0    # TRIDENT2
DFLAGS += -DBCM_56860_A0    # TRIDENT2+
DFLAGS += -DBCM_56870_A0    # TRIDENT3
DFLAGS += -DBCM_56770_A0    # MAVERICK2
DFLAGS += -DBCM_56370_A0    # HELIX5
DFLAGS += -DBCM_56960_A0    # TOMAHAWK

#
# Compiler Related Options
#

# Define if pointers are 64 bits on your compiler;
# this is typically true ONLY for 64-bit processors.
# NOTE: This support works ONLY under conditions where the
# upper 32 bits for ALL pointers are ZERO.
#CFGFLAGS += -DPTRS_ARE_64BITS

# Disable inlining of functions
#CFGFLAGS += -DCOMPILER_OVERRIDE_NO_INLINE

# Disable use of const
#CFGFLAGS += -DCOMPILER_OVERRIDE_NO_CONST

# Disable use of static functions
#CFGFLAGS += -DCOMPILER_OVERRIDE_NO_STATIC


#
# Rules
#
all: build_dir $(LIB)

$(LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

define compile_rules
$(1): $(subst .o,.c,$(subst $(BLDDIR),$(DCBDIR),$(1)))
	$(CC) $(CFLAGS) $(DFLAGS) $$< -o $$@
endef

$(foreach object,$(OBJECTS),$(eval $(call compile_rules,$(object))))

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(LIB)

.PHONY: build_dir
build_dir:
	mkdir -p $(BLDDIR)
	mkdir -p $(OBJECTSDIR)

.PHONY: print_vars
print_vars:
	echo $(SOURCES)
	echo $(OBJECTS)

#
#
# Example
#
test: all $(DCBDIR)/src/appl/test/dcb_test.c
	$(CC) -Wall -I$(DCBDIR)/include \
	$(DCBDIR)/src/appl/test/dcb_test.c $(LIB) \
	-o  $(BLDDIR)/test.exe

