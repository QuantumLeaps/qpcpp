##############################################################################
# QP/C++ Real-Time Embedded Framework (RTEF)
#
#                    Q u a n t u m  L e a P s
#                    ------------------------
#                    Modern Embedded Software
#
# Copyright (C) 2005 Quantum Leaps, LLC, <state-machine.com>.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
#
# This software is dual-licensed under the terms of the open source GNU
# General Public License version 3 (or any later version), or alternatively,
# under the terms of one of the closed source Quantum Leaps commercial
# licenses.
#
# The terms of the open source GNU General Public License version 3
# can be found at: <www.gnu.org/licenses/gpl-3.0>
#
# The terms of the closed source Quantum Leaps commercial licenses
# can be found at: <www.state-machine.com/licensing>
#
# Redistributions in source code must retain this top-level comment block.
# Plagiarizing this software to sidestep the license obligations is illegal.
#
# Contact information:
# <www.state-machine.com>
# <info@state-machine.com>
##############################################################################
# examples of invoking this Makefile:
# building configurations: Debug (default), Release and Spy
# make
# make CONF=rel
# make CONF=spy
#
# cleaning configurations: Debug (default), Release, and Spy
# make clean
# make CONF=rel clean
# make CONF=spy clean
#
# NOTE:
# To use this Makefile on Windows, you will need the GNU make utility, which
# is included in the QTools collection for Windows, see:
#    https://github.com/QuantumLeaps/qtools
#

#-----------------------------------------------------------------------------
# project name:
#
PROJECT := qp

#-----------------------------------------------------------------------------
# project directories:
#

# location of the QP/C++ framework
QPCPP := ../..

# QP port used in this project
QP_PORT_DIR := .

# list of all source directories used by this project
VPATH = \
	$(QPCPP)/src/qf \
	$(QPCPP)/src/qs \
	$(QP_PORT_DIR)

# list of all include directories needed by this project
INCLUDES  = \
	-I$(QPCPP)/include \
	-I$(QPCPP)/src \
	-I$(QP_PORT_DIR)

#-----------------------------------------------------------------------------
# files
#

# C source files
C_SRCS := \
	qwin_gui.c

# C++ source files
CPP_SRCS := \
	qep_hsm.cpp \
	qep_msm.cpp \
	qf_actq.cpp \
	qf_defer.cpp \
	qf_dyn.cpp \
	qf_mem.cpp \
	qf_ps.cpp \
	qf_qact.cpp \
	qf_qeq.cpp \
	qf_qmact.cpp \
	qf_time.cpp \
	qf_port.cpp

# C++ QS source files
CPP_QS_SRCS := \
	qs.cpp \
	qs_rx.cpp \
	qs_fp.cpp \
	qs_64bit.cpp \
	qs_port.cpp

# defines:
DEFINES  := -DQWIN_GUI -DQP_API_VERSION=9999

#-----------------------------------------------------------------------------
# MinGW toolset (NOTE: assumed to be on your PATH)
#
# NOTE:
# MinGW toolset is included in the Qtools collection for Windows, see:
#    https://github.com/QuantumLeaps/qtools

CC    := gcc
CPP   := g++
LIB   := ar


##############################################################################
# Typically, you should not need to change anything below this line

# basic utilities (included in Qtools for Windows), see:
#    https://github.com/QuantumLeaps/qtools

MKDIR := mkdir
RM    := rm

#-----------------------------------------------------------------------------
# build options for various configurations
#

LIBFLAGS := rs

ifeq (rel, $(CONF))  # Release configuration .................................

BIN_DIR := rel

# gcc options:
CFLAGS  := -c -O3 -fno-pie -std=c99 -pedantic -Wall -Wextra -W \
	$(INCLUDES) $(DEFINES) -DNDEBUG

CPPFLAGS := -c -O3 -fno-pie -std=c++11 -pedantic -Wall -Wextra \
	-fno-rtti -fno-exceptions \
	$(INCLUDES) $(DEFINES) -DNDEBUG

else ifeq (spy, $(CONF))  # Spy configuration ................................

BIN_DIR := spy

# add the QS sources...
C_SRCS += $(C_QS_SRCS)

# gcc options:
CFLAGS  := -c -g -O -fno-pie -std=c99 -pedantic -Wall -Wextra -W \
	$(INCLUDES) $(DEFINES) -DQ_SPY

CPPFLAGS := -c -g -O -fno-pie -std=c++11 -pedantic -Wall -Wextra \
	-fno-rtti -fno-exceptions \
	$(INCLUDES) $(DEFINES) -DQ_SPY

else # default Debug configuration .........................................

BIN_DIR := dbg

# gcc options:
CFLAGS  := -c -g -O -fno-pie -std=c99 -pedantic -Wall -Wextra -W \
	$(INCLUDES) $(DEFINES)

CPPFLAGS := -c -g -O -fno-pie -std=c++11 -pedantic -Wall -Wextra \
	-fno-rtti -fno-exceptions \
	$(INCLUDES) $(DEFINES)

endif

#-----------------------------------------------------------------------------
C_OBJS       := $(patsubst %.c,%.o,  $(notdir $(C_SRCS)))
CPP_OBJS     := $(patsubst %.cpp,%.o,$(notdir $(CPP_SRCS)))

TARGET_LIB   := $(BIN_DIR)/lib$(PROJECT).a
C_OBJS_EXT   := $(addprefix $(BIN_DIR)/, $(C_OBJS))
C_DEPS_EXT   := $(patsubst %.o,%.d, $(C_OBJS_EXT))
CPP_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(CPP_OBJS))
CPP_DEPS_EXT := $(patsubst %.o,%.d, $(CPP_OBJS_EXT))

# create $(BIN_DIR) if it does not exist
ifeq ("$(wildcard $(BIN_DIR))","")
$(shell $(MKDIR) $(BIN_DIR))
endif

#-----------------------------------------------------------------------------
# rules
#

all: $(TARGET_LIB)
	-$(RM) $(BIN_DIR)/*.o

$(TARGET_LIB) : $(ASM_OBJS_EXT) $(C_OBJS_EXT) $(CPP_OBJS_EXT)
	$(LIB) $(LIBFLAGS) $@ $^

$(BIN_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

#-----------------------------------------------------------------------------
# the clean target
#
.PHONY : clean
clean:
	-$(RM) $(BIN_DIR)/*.o $(TARGET_LIB)

#-----------------------------------------------------------------------------
# the show target for debugging
#
show:
	@echo PROJECT = $(PROJECT)
	@echo CONF = $(CONF)
	@echo TARGET_LIB = $(TARGET_LIB)
	@echo C_SRCS = $(C_SRCS)
	@echo CPP_SRCS = $(CPP_SRCS)
	@echo C_OBJS_EXT = $(C_OBJS_EXT)
	@echo C_DEPS_EXT = $(C_DEPS_EXT)
	@echo CPP_OBJS_EXT = $(CPP_OBJS_EXT)
	@echo CPP_DEPS_EXT = $(CPP_DEPS_EXT)
