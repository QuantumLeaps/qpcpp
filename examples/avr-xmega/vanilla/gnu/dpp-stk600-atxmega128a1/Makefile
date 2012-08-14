##############################################################################
# Product: DPP example, AVR-xmega, Vanilla Kernel
# Last Updated for Version: 4.1.01
# Date of the Last Update:  Dec 10, 2009
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) 2002-2009 Quantum Leaps, LLC. All rights reserved.
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
# general utilities
RM = rm

# GNU-AVR tools directory
GNU_AVR = C:\tools\GNU\WinAVR

# Adjust the Target MCU to your project
TARGET_MCU = atxmega128a1

# Adjust the Application Name to your project
APP_NAME = dpp

# The gcc compiler and linker
CC      = $(GNU_AVR)\bin\avr-g++
LINK    = $(GNU_AVR)\bin\avr-g++
OBJCOPY = $(GNU_AVR)\bin\avr-objcopy
OBJDUMP = $(GNU_AVR)\bin\avr-objdump

QP_INCDIR = ..\..\..\..\..\include
QP_PRTDIR = ..\..\..\..\..\ports\avr-xmega\vanilla\gnu
QP_LIBDIR = $(QP_PRTDIR)\$(BINDIR)
BLDDIR    = .
LINKFLAGS =
QS_LIB    =


CCINC    := -I$(QP_INCDIR) -I$(QP_PRTDIR) -I$(BLDDIR)
QP_DEP   := $(QP_INCDIR)\qassert.h \
	$(QP_INCDIR)\qep.h \
	$(QP_PRTDIR)\qep_port.h	\
	$(QP_INCDIR)\qf.h \
	$(QP_PRTDIR)\qf_port.h

APP_DEP  := $(BLDDIR)\dpp.h \
	$(BLDDIR)\bsp.h

# dbg (default target) .......................................................

BINDIR   = dbg

# g++ options
CCFLAGS  = -c -gdwarf-2 -fsigned-char -fshort-enums -fno-threadsafe-statics \
	-Os -mmcu=$(TARGET_MCU) -Wall -o$@

LINKFLAGS = -mmcu=$(TARGET_MCU)


# release ....................................................................
ifeq ($(MAKECMDGOALS), rel)

BINDIR   = rel

# g++ options
CCFLAGS  = -c -DNDEBUG -fsigned-char -fshort-enums -fno-threadsafe-statics \
	-Os -mmcu=$(TARGET_MCU) -Wall -o$@

LINKFLAGS =	-mmcu=$(TARGET_MCU)

endif
ifeq ($(MAKECMDGOALS), rel_clean)

BINDIR   = rel

endif

# spy ........................................................................
ifeq ($(MAKECMDGOALS), spy)

BINDIR   = spy

# g++ options
CCFLAGS  = -c -gdwarf-2 -fsigned-char -fshort-enums -fno-threadsafe-statics \
	-Os -mmcu=$(TARGET_MCU) -Wall -o$@ -DQ_SPY

LINKFLAGS =	-mmcu=$(TARGET_MCU)
QS_LIB    =	-lqs_$(TARGET_MCU)

QP_DEP  := $(QP_DEP) \
	$(QP_INCDIR)\qs.h \
	$(QP_PRTDIR)\qs_port.h

spy: $(BINDIR)\$(APP_NAME).elf

endif
ifeq ($(MAKECMDGOALS), spy_clean)

BINDIR   = spy

endif

#.............................................................................

all: $(BINDIR)\$(APP_NAME).hex

$(BINDIR)\$(APP_NAME).hex : $(BINDIR)\$(APP_NAME).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(BINDIR)\$(APP_NAME).elf : \
	$(BINDIR)\main.o \
	$(BINDIR)\bsp.o \
	$(BINDIR)\philo.o \
	$(BINDIR)\table.o \
	$(QP_LIBDIR)\libqf_$(TARGET_MCU).a \
	$(QP_LIBDIR)\libqep_$(TARGET_MCU).a $(QS_LIB)
	$(LINK) $(LINKFLAGS) \
	-o $(BINDIR)\$(APP_NAME).elf \
	-Wl,-Map,$(BINDIR)\$(APP_NAME).map,--cref \
	$(BINDIR)\main.o \
	$(BINDIR)\bsp.o \
	$(BINDIR)\philo.o \
	$(BINDIR)\table.o \
	$(QP_LIBDIR)\libqf_$(TARGET_MCU).a \
	$(QP_LIBDIR)\libqep_$(TARGET_MCU).a $(QS_LIB)

$(BINDIR)\main.o: $(BLDDIR)\main.cpp $(APP_DEP) $(QP_DEP)
	$(CC) $(CCFLAGS) $(CCINC) $<

$(BINDIR)\bsp.o: $(BLDDIR)\bsp.cpp $(APP_DEP) $(QP_DEP)
	$(CC) $(CCFLAGS) $(CCINC) $<

$(BINDIR)\philo.o: $(BLDDIR)\philo.cpp $(APP_DEP) $(QP_DEP)
	$(CC) $(CCFLAGS) $(CCINC) $<

$(BINDIR)\table.o: $(BLDDIR)\table.cpp $(APP_DEP) $(QP_DEP)
	$(CC) $(CCFLAGS) $(CCINC) $<


# build configuration targets...
dbg: all

rel: all

spy: all


# clean targets...

.PHONY: clean rel_clean spy_clean

rel_clean: clean

dbg_clean: clean

spy_clean: clean

clean:
	-$(RM) $(BINDIR)/*.o
	-$(RM) $(BINDIR)/*.elf
	-$(RM) $(BINDIR)/*.hex
	-$(RM) $(BINDIR)/*.map
