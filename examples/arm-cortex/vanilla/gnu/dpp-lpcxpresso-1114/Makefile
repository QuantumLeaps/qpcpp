##############################################################################
# Product: Makefile for QP/C++ app, LPCXpresso-1114, Vanilla, GNU/CodeRed
# Last Updated for Version: 4.2.02
# Date of the Last Update:  Sep 07, 2011
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) 2002-2011 Quantum Leaps, LLC. All rights reserved.
#
# This software may be distributed and modified under the terms of the GNU
# General Public License version 2 (GPL) as published by the Free Software
# Foundation and appearing in the file GPL.TXT included in the packaging of
# this file. Please note that GPL Section 2[b] requires that all works based
# on this software must also be made publicly available under the terms of
# the GPL ("Copyleft").
#
# Alternatively, this software may be distributed and modified under the
# terms of Quantum Leaps commercial licenses, which expressly supersede
# the GPL and are specifically designed for licensees interested in
# retaining the proprietary status of their code.
#
# Contact information:
# Quantum Leaps Web site:  http://www.quantum-leaps.com
# e-mail:                  info@quantum-leaps.com
##############################################################################
# examples of invoking this Makefile:
# building configurations: Debug (default), Release, and Spy
# make
# make CONF=rel
# make CONF=spy
#
# cleaning configurations: Debug (default), Release, and Spy
# make clean
# make CONF=rel clean
# make CONF=spy clean

#-----------------------------------------------------------------------------
# NOTE: the Makefile expects that the QPCPP environment variable is defined
# and points to the QP/C++ installation directory
#
ifndef QPCPP
$(error The QPCPP environment variable must be defined)
endif

#-----------------------------------------------------------------------------
# tools
#
ifeq ($(GNU_ARM),)
GNU_ARM = C:/tools/CodeRed/lpcxpresso/Tools
endif

CC    := $(GNU_ARM)/bin/arm-none-eabi-gcc
CPP   := $(GNU_ARM)/bin/arm-none-eabi-g++
AS    := $(GNU_ARM)/bin/arm-none-eabi-as
LINK  := $(GNU_ARM)/bin/arm-none-eabi-g++
BIN   := $(GNU_ARM)/bin/arm-none-eabi-objcopy
RM    := rm -rf
MKDIR := mkdir

#-----------------------------------------------------------------------------
# directories
#
QP_PORT_DIR := "$(QPCPP)"/ports/arm-cortex/vanilla/gnu
CMSIS_DIR   := cmsis
APP_DIR     := .
LIB_DIR     :=

# source directories
VPATH = $(APP_DIR) \
	$(CMSIS_DIR) \
	lpc11xx_lib/src

# Output file basename
OUTPUT    := dpp

# include directories
INCLUDES  = -I"$(QPCPP)"/include	\
	-I$(QP_PORT_DIR) \
	-I. \
	-I$(CMSIS_DIR) \
	-Ilpc11xx_lib/inc

# defines
DEFINES = -D__REDLIB__ \
	-D__USE_CMSIS=CMSISv1p30_LPC11xx

#-----------------------------------------------------------------------------
# files
#

# assembler source files
ASM_SRCS :=

# C source files
C_SRCS := startup_LPC11.c \
	$(wildcard *.c) \
	core_cm0.c \
	system_LPC11xx.c \
	clkconfig.c \
	gpio.c \
	timer16.c \
	timer32.c \
	uart.c

# C++ source files
CPP_SRCS := $(wildcard *.cpp)

LD_SCRIPT := lpc1114.ld

#-----------------------------------------------------------------------------
# build options for various configurations
#

ARM_CORE = cortex-m0

ifeq (rel, $(CONF))       # Release configuration ............................

BIN_DIR := rel
LIBS    := -lqf_$(ARM_CORE)_cr -lqep_$(ARM_CORE)_cr

ASFLAGS = -mcpu=$(ARM_CORE)

CFLAGS = -mcpu=$(ARM_CORE) -mthumb -Wall \
	-Os $(INCLUDES) $(DEFINES) -DNDEBUG

CPPFLAGS = -mcpu=$(ARM_CORE) -mthumb \
	-Wall -fno-rtti -fno-exceptions \
	-Os $(INCLUDES) $(DEFINES) -DNDEBUG

LINKFLAGS = -T$(LD_SCRIPT) -mcpu=$(ARM_CORE) -mthumb -nostdlib -Xlinker \
	-Map=$(BIN_DIR)/$(OUTPUT).map --gc-sections \
	-L$(QP_PORT_DIR)/$(BIN_DIR)

else

ifeq (spy, $(CONF))  # Spy configuration ................................

BIN_DIR := spy
LIBS    := -lqf_$(ARM_CORE)_cr -lqep_$(ARM_CORE)_cr -lqs_$(ARM_CORE)_cr

ASFLAGS = -g -mcpu=$(ARM_CORE)

CFLAGS = -mcpu=$(ARM_CORE) -mthumb -Wall \
	-g -O $(INCLUDES) $(DEFINES) -DQ_SPY

CPPFLAGS = -mcpu=$(ARM_CORE) -mthumb -Wall \
	-fno-rtti -fno-exceptions \
	-g -O $(INCLUDES) $(DEFINES) -DQ_SPY

LINKFLAGS = -T$(LD_SCRIPT) -mcpu=$(ARM_CORE) -mthumb -nostdlib -Xlinker \
	-Map=$(BIN_DIR)/$(OUTPUT).map --gc-sections \
	-L$(QP_PORT_DIR)/$(BIN_DIR)

else                # default Debug configuration .......................

BIN_DIR := dbg
LIBS    := -lqf_$(ARM_CORE)_cr -lqep_$(ARM_CORE)_cr

ASFLAGS = -g -mcpu=$(ARM_CORE)

CFLAGS = -mcpu=$(ARM_CORE) -mthumb -Wall \
	-g -O $(INCLUDES) $(DEFINES)

CPPFLAGS = -mcpu=$(ARM_CORE) -mthumb \
	-Wall -fno-rtti -fno-exceptions \
	-g -O $(INCLUDES) $(DEFINES)

LINKFLAGS = -T$(LD_SCRIPT) -mcpu=$(ARM_CORE) -mthumb -nostdlib -Xlinker \
	-Map=$(BIN_DIR)/$(OUTPUT).map --gc-sections \
	-L$(QP_PORT_DIR)/$(BIN_DIR)

endif

endif


ASM_OBJS     := $(patsubst %.s,%.o,$(ASM_SRCS))
C_OBJS       := $(patsubst %.c,%.o,$(C_SRCS))
CPP_OBJS     := $(patsubst %.cpp,%.o,$(CPP_SRCS))

TARGET_BIN   := $(BIN_DIR)/$(OUTPUT).bin
TARGET_ELF   := $(BIN_DIR)/$(OUTPUT).elf
ASM_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(ASM_OBJS))
C_OBJS_EXT   := $(addprefix $(BIN_DIR)/, $(C_OBJS))
C_DEPS_EXT   := $(patsubst %.o, %.d, $(C_OBJS_EXT))
CPP_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(CPP_OBJS))
CPP_DEPS_EXT := $(patsubst %.o, %.d, $(CPP_OBJS_EXT))


#-----------------------------------------------------------------------------
# rules
#

all: $(BIN_DIR) $(TARGET_ELF)
#all: $(BIN_DIR) $(TARGET_BIN)

$(BIN_DIR):
	@echo
	mkdir -p $@

$(TARGET_BIN): $(TARGET_ELF)
	$(BIN) -O binary $< $@

$(TARGET_ELF) : $(ASM_OBJS_EXT) $(C_OBJS_EXT) $(CPP_OBJS_EXT)
	$(LINK) $(LINKFLAGS) -o $@ $^ $(LIBS)

$(BIN_DIR)/%.d : %.c
	$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $< > $@

$(BIN_DIR)/%.d : %.cpp
	$(CPP) -MM -MT $(@:.d=.o) $(CPPFLAGS) $< > $@

$(BIN_DIR)/%.o : %.s
	$(AS) $(ASFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

-include $(C_DEPS_EXT) $(CPP_DEPS_EXT)


.PHONY : clean
clean:
	-$(RM) $(BIN_DIR)/*.o \
	$(BIN_DIR)/*.d \
	$(BIN_DIR)/*.bin \
	$(BIN_DIR)/*.elf \
	$(BIN_DIR)/*.map
	
show:
	@echo CONF = $(CONF)
	@echo ASM_SRCS = $(ASM_SRCS)
	@echo C_SRCS = $(C_SRCS)
	@echo CPP_SRCS = $(CPP_SRCS)
	@echo ASM_OBJS_EXT = $(ASM_OBJS_EXT)
	@echo C_OBJS_EXT = $(C_OBJS_EXT)
	@echo C_DEPS_EXT = $(C_DEPS_EXT)
	@echo CPP_DEPS_EXT = $(CPP_DEPS_EXT)
	@echo TARGET_ELF = $(TARGET_ELF)
