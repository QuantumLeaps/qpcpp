##############################################################################
# Product: Makefile for QP-lwIP, STM3210C-EVAL board, QK, GNU/Sourcery G++
# Last Updated for Version: 4.1.06
# Date of the Last Update:  Jan 07, 2011
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

#-----------------------------------------------------------------------------
# tools
#
ifeq ($(GNU_ARM),)
GNU_ARM = C:/tools/CodeSourcery/bin
endif

CC    := $(GNU_ARM)/arm-none-eabi-gcc
CPP   := $(GNU_ARM)/arm-none-eabi-g++
AS    := $(GNU_ARM)/arm-none-eabi-as
LINK  := $(GNU_ARM)/arm-none-eabi-g++                       # for C++ programs
BIN   := $(GNU_ARM)/arm-none-eabi-objcopy
RM    := rm -rf
MKDIR := mkdir

#-----------------------------------------------------------------------------
# directories
#

QP_DIR      := ../../../../..
QP_PORT_DIR := $(QP_DIR)/ports/arm-cortex/qk/gnu
CMSIS_DIR   := cmsis
APP_DIR     := .
LIB_DIR     :=

# source directories
VPATH = . \
	$(CMSIS_DIR) \
	STM3210C-EVAL \
	STM32F10x/src \
	lwip_port/netif \
	webserver

# Output file basename
OUTPUT    := lwip-qk

# include directories
INCLUDES  = -I$(QP_DIR)/include	\
	-I$(QP_PORT_DIR) \
	-I$(APP_DIR) \
	-I$(CMSIS_DIR) \
	-ISTM3210C-EVAL \
	-ISTM32F10x/inc \
	-Iwebserver \
	-Ilwip_port \
	-I../../../../../../lwip-1.3.2/src/include \
	-I../../../../../../lwip-1.3.2/src/include/ipv4 \
	-I../../../../../../lwip-1.3.2/src

# defines
DEFINES = -DSTM32F10X_CL \
	-DUSE_STDPERIPH_DRIVER \
	-DUSE_STM3210C_EVAL

#-----------------------------------------------------------------------------
# files
#

# assembler source files
ASM_SRCS :=

# C source files
C_SRCS := startup_stm32f10x_cl.c \
	$(wildcard *.c) \
	core_cm3.c \
	system_stm32f10x.c \
	stm32_eval.c \
	stm3210c_eval_lcd.c \
	stm3210c_eval_ioe.c \
	misc.c \
	stm32f10x_adc.c \
	stm32f10x_bkp.c \
	stm32f10x_can.c \
	stm32f10x_crc.c \
	stm32f10x_dac.c \
	stm32f10x_dbgmcu.c \
	stm32f10x_dma.c \
	stm32f10x_exti.c \
	stm32f10x_flash.c \
	stm32f10x_fsmc.c \
	stm32f10x_gpio.c \
	stm32f10x_i2c.c \
	stm32f10x_iwdg.c \
	stm32f10x_pwr.c \
	stm32f10x_rcc.c \
	stm32f10x_rtc.c \
	stm32f10x_sdio.c \
	stm32f10x_spi.c \
	stm32f10x_tim.c \
	stm32f10x_usart.c \
	stm32f10x_wwdg.c \
	httpd.c \
	fs.c

# C++ source files
CPP_SRCS :=	$(wildcard *.cpp) \
	eth_driver.cpp


LD_SCRIPT := stm32f10x.ld

#-----------------------------------------------------------------------------
# build options for various configurations
#

ARM_CORE = cortex-m3

ifeq (rel, $(CONF))       # Release configuration ............................

BIN_DIR := rel
LIBS    := -lqk_$(ARM_CORE) -lqf_$(ARM_CORE) -lqep_$(ARM_CORE)

ASFLAGS = -mcpu=$(ARM_CORE)

CFLAGS = -mcpu=$(ARM_CORE) -mthumb -Wall \
	-Os $(INCLUDES) $(DEFINES) -DNDEBUG

CPPFLAGS = -mcpu=$(ARM_CORE) -mthumb \
	-Wall -fno-rtti -fno-exceptions \
	-Os $(INCLUDES) $(DEFINES) -DNDEBUG

LINKFLAGS = -T$(LD_SCRIPT) -mcpu=$(ARM_CORE) -mthumb -nostdlib -Xlinker \
	-Map=$(BIN_DIR)/$(OUTPUT).map --gc-sections \
	-L$(QP_PORT_DIR)/$(BIN_DIR)

else ifeq (spy, $(CONF))  # Spy configuration ................................

BIN_DIR := spy
LIBS    := -lqk_$(ARM_CORE) -lqf_$(ARM_CORE) -lqep_$(ARM_CORE) -lqs_$(ARM_CORE)

ASFLAGS = -g -mcpu=$(ARM_CORE)

CFLAGS = -mcpu=$(ARM_CORE) -mthumb -Wall \
	-g -O $(INCLUDES) $(DEFINES) -DQ_SPY

CPPFLAGS = -mcpu=$(ARM_CORE) -mthumb \
	-Wall -fno-rtti -fno-exceptions \
	-g -O $(INCLUDES) $(DEFINES) -DQ_SPY

LINKFLAGS = -T$(LD_SCRIPT) -mcpu=$(ARM_CORE) -mthumb -nostdlib -Xlinker \
	-Map=$(BIN_DIR)/$(OUTPUT).map --gc-sections \
	-L$(QP_PORT_DIR)/$(BIN_DIR)

else                     # default Debug configuration .......................

BIN_DIR := dbg
LIBS    := -lqk_$(ARM_CORE) -lqf_$(ARM_CORE) -lqep_$(ARM_CORE)

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

ASM_OBJS     := $(patsubst %.s,%.o,$(ASM_SRCS))
C_OBJS       := $(patsubst %.c,%.o,$(C_SRCS))
CPP_OBJS     := $(patsubst %.cpp,%.o,$(CPP_SRCS))

TARGET_BIN   := $(BIN_DIR)/$(OUTPUT).hex
TARGET_ELF   := $(BIN_DIR)/$(OUTPUT).elf
ASM_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(ASM_OBJS))
C_OBJS_EXT   := $(addprefix $(BIN_DIR)/, $(C_OBJS))
C_DEPS_EXT   := $(patsubst %.o, %.d, $(C_OBJS_EXT))
CPP_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(CPP_OBJS))
CPP_DEPS_EXT := $(patsubst %.o, %.d, $(CPP_OBJS_EXT))


#-----------------------------------------------------------------------------
# rules
#

all: $(BIN_DIR) $(TARGET_BIN)

$(BIN_DIR):
	@echo
	mkdir -p $@

$(TARGET_BIN): $(TARGET_ELF)
	$(BIN) -O ihex $< $@

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
	$(BIN_DIR)/*.hex \
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
