##############################################################################
# Product: QDK/C++_ARM-GNU_AT91SAM7S-EK	QK kernel, GNU toolset
# Last Updated for Version: 4.1.00
# Date of the Last Update:  Jan 10, 2010
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) 2002-2010 Quantum Leaps, LLC. All rights reserved.
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
GNU_ARM = C:/tools/devkitPro/devkitARM/bin
endif

CC    := $(GNU_ARM)/arm-eabi-g++
ASM   := $(GNU_ARM)/arm-eabi-as
LINK  := $(GNU_ARM)/arm-eabi-g++                            # for C++ programs
BIN   := $(GNU_ARM)/arm-eabi-objcopy
RM    := rm -rf
MKDIR := mkdir

QP_INCDIR = ../../../../../include
QP_PRTDIR = ../../../../../ports/arm/qk/gnu
QP_LIBDIR = $(QP_PRTDIR)/$(BINDIR)
BLDDIR    = .

CCINC   = -I$(QP_INCDIR) -I$(QP_PRTDIR) -I$(BLDDIR)
QP_DEP  = $(QP_INCDIR)/qassert.h \
	$(QP_INCDIR)/qep.h \
	$(QP_PRTDIR)/qep_port.h \
	$(QP_INCDIR)/qf.h \
	$(QP_PRTDIR)/qf_port.h \
	$(QP_INCDIR)/qk.h \
	$(QP_PRTDIR)/qk_port.h

APP_DEP = $(BLDDIR)/bsp.h \
	$(BLDDIR)/dpp.h
	
APP_NAME = dpp-qk
ARM_CORE = arm7tdmi


ifeq (rel, $(CONF))       # Release configuration ............................

BINDIR   = rel

CCFLAGS = -c -mcpu=$(ARM_CORE) -mthumb-interwork -Os \
	-fno-rtti -fno-exceptions \
	-mlong-calls -ffunction-sections -Wall -DNDBEBUG -o$@

ASMFLAGS = -mcpu=$(ARM_CORE) -mthumb-interwork -o$@

LINKFLAGS = -T .\$(APP_NAME).ld -o $(BINDIR)\$(APP_NAME).elf \
	-Wl,-Map,$(BINDIR)\$(APP_NAME).map,--cref,--gc-sections

else ifeq (spy, $(CONF))  # Spy configuration ................................

BINDIR   = spy

QS_LIB = $(QP_LIBDIR)/libqs_$(ARM_CORE).a
QP_DEP := $(QP_DEP) \
	$(QP_INCDIR)/qs.h \
	$(QP_PRTDIR)/qs_port.h

CCFLAGS = -g -c -mcpu=$(ARM_CORE) -mthumb-interwork -O \
	-fno-rtti -fno-exceptions \
	-mlong-calls -ffunction-sections -Wall -DQ_SPY -o$@

ASMFLAGS = -g -mcpu=$(ARM_CORE) -mthumb-interwork -o$@

LINKFLAGS = -T .\$(APP_NAME).ld -o $(BINDIR)\$(APP_NAME).elf \
	-Wl,-Map,$(BINDIR)\$(APP_NAME).map,--cref,--gc-sections

else                     # default Debug configuration .......................

BINDIR   = dbg

CCFLAGS = -g -c -mcpu=$(ARM_CORE) -mthumb-interwork -O \
	-fno-rtti -fno-exceptions \
	-mlong-calls -ffunction-sections -Wall -o$@

ASMFLAGS = -g -mcpu=$(ARM_CORE) -mthumb-interwork -o$@

LINKFLAGS = -T .\$(APP_NAME).ld -o $(BINDIR)\$(APP_NAME).elf \
	-Wl,-Map,$(BINDIR)\$(APP_NAME).map,--cref,--gc-sections

endif

#.............................................................................
.PHONY: all

all: $(BINDIR)/$(APP_NAME).elf

$(BINDIR)/$(APP_NAME).elf : \
	./$(APP_NAME).ld \
	$(BINDIR)/startup.o \
	$(BINDIR)/low_level_init.o \
	$(BINDIR)/main.o \
	$(BINDIR)/bsp.o \
	$(BINDIR)/isr.o \
	$(BINDIR)/philo.o \
	$(BINDIR)/table.o \
	$(BINDIR)/mini_cpp.o \
	$(BINDIR)/no_heap.o	\
	$(QP_LIBDIR)/libqk_$(ARM_CORE).a \
	$(QP_LIBDIR)/libqf_$(ARM_CORE).a \
	$(QP_LIBDIR)/libqep_$(ARM_CORE).a \
	$(QS_LIB)
	$(LINK) \
	$(BINDIR)/startup.o \
	$(BINDIR)/low_level_init.o \
	$(BINDIR)/main.o \
	$(BINDIR)/bsp.o \
	$(BINDIR)/isr.o \
	$(BINDIR)/philo.o \
	$(BINDIR)/table.o \
	$(BINDIR)/mini_cpp.o \
	$(BINDIR)/no_heap.o	\
	$(QP_LIBDIR)/libqk_$(ARM_CORE).a \
	$(QP_LIBDIR)/libqf_$(ARM_CORE).a \
	$(QP_LIBDIR)/libqep_$(ARM_CORE).a \
	$(QS_LIB) \
	$(LINKFLAGS)

$(BINDIR)/startup.o: $(BLDDIR)/startup.s $(APP_DEP)
	$(ASM) $(ASMFLAGS) $<

$(BINDIR)/low_level_init.o: $(BLDDIR)/low_level_init.cpp $(APP_DEP)
	$(CC) -marm   $(CCFLAGS) $(CCINC) $<

$(BINDIR)/main.o: $(BLDDIR)/main.cpp $(APP_DEP) $(QP_DEP)
	$(CC) -mthumb $(CCFLAGS) $(CCINC) $<

$(BINDIR)/bsp.o: $(BLDDIR)/bsp.cpp $(APP_DEP) $(QP_DEP)
	$(CC) -marm   $(CCFLAGS) $(CCINC) $<

$(BINDIR)/isr.o: $(BLDDIR)/isr.cpp $(APP_DEP) $(QP_DEP)
	$(CC) -marm   $(CCFLAGS) $(CCINC) $<

$(BINDIR)/philo.o: $(BLDDIR)/philo.cpp $(APP_DEP) $(QP_DEP)
	$(CC) -mthumb $(CCFLAGS) $(CCINC) $<

$(BINDIR)/table.o: $(BLDDIR)/table.cpp $(APP_DEP) $(QP_DEP)
	$(CC) -mthumb $(CCFLAGS) $(CCINC) $<

$(BINDIR)/mini_cpp.o: $(BLDDIR)/mini_cpp.cpp
	$(CC) -mthumb $(CCFLAGS) $(CCINC) $<

$(BINDIR)/no_heap.o: $(BLDDIR)/no_heap.cpp
	$(CC) -mthumb $(CCFLAGS) $(CCINC) $<


.PHONY : clean
clean:
	-$(RM) $(BINDIR)/*.o \
	$(BINDIR)/*.elf \
	$(BINDIR)/*.map
