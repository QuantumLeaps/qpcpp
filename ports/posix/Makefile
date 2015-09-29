##############################################################################
# Product: Makefile for QP/C++ port to POSIX, GNU toolset
# Last Updated for Version: 5.5.0
# Date of the Last Update:  2015-09-24
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) Quantum Leaps, LLC. All rights reserved.
#
# This program is open source software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Alternatively, this program may be distributed and modified under the
# terms of Quantum Leaps commercial licenses, which expressly supersede
# the GNU General Public License and are specifically designed for
# licensees interested in retaining the proprietary status of their code.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Contact information:
# http://www.state-machine.com
# mailto:info@state-machine.com
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
#

#-----------------------------------------------------------------------------
# project name
#
PROJECT     := qp

#-----------------------------------------------------------------------------
# project directories
#

# location of the QP/C++ framework
QPCPP := ../..

# QP port used in this project
QP_PORT_DIR := .


# list of all source directories used by this project
VPATH = \
	$(QPCPP)/source \
	$(QP_PORT_DIR)

# list of all include directories needed by this project
INCLUDES  = \
	-I$(QPCPP)/include \
	-I$(QPCPP)/source \
	-I$(QP_PORT_DIR)

#-----------------------------------------------------------------------------
# files
#

# C++ source files
CPP_SRCS := \
	qep_hsm.cpp \
	qep_msm.cpp \
	qf_act.cpp \
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
	qs_64bit.cpp

# defines
DEFINES  :=

#-----------------------------------------------------------------------------
# GNU toolset
#
CPP   := g++
LIB   := ar


##############################################################################
# Typically, you should not need to change anything below this line

MKDIR := mkdir
RM    := rm

#-----------------------------------------------------------------------------
# build options for various configurations
#

LIBFLAGS := rs

ifeq (rel, $(CONF))  # Release configuration .................................

BIN_DIR := rel

CPPFLAGS = -c -O2 -fno-rtti -fno-exceptions -DNDEBUG \
	-ffunction-sections -fdata-sections	$(INCLUDES) $(DEFINES) -Wall -pthread

else ifeq (spy, $(CONF))  # Spy configuration ................................

BIN_DIR := spy

CPPFLAGS = -c -g -O -fno-rtti -fno-exceptions -DQ_SPY \
	-ffunction-sections -fdata-sections	$(INCLUDES) $(DEFINES) -Wall -pthread

# add the QS sources...
CPP_SRCS += $(CPP_QS_SRCS)

else   # default Debug configuration .........................................

BIN_DIR := dbg

CPPFLAGS = -c -g -O -fno-rtti -fno-exceptions \
	-ffunction-sections -fdata-sections	$(INCLUDES) $(DEFINES) -Wall -pthread

endif


TARGET_LIB   := $(BIN_DIR)/lib$(PROJECT).a
CPP_OBJS     := $(patsubst %.cpp, %.o, $(notdir $(CPP_SRCS)))
CPP_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(CPP_OBJS))
CPP_DEPS_EXT := $(patsubst %.o, %.d, $(CPP_OBJS_EXT))

# create $(BIN_DIR) if it does not exist
ifeq ("$(wildcard $(BIN_DIR))","")
$(shell $(MKDIR) $(BIN_DIR))
endif

#-----------------------------------------------------------------------------
# rules
#

all: $(TARGET_LIB)
	-$(RM) $(BIN_DIR)/*.o $(BIN_DIR)/*.d

$(TARGET_LIB) : $(ASM_OBJS_EXT) $(C_OBJS_EXT) $(CPP_OBJS_EXT)
	$(LIB) $(LIBFLAGS) $@ $^

$(BIN_DIR)/%.d : %.cpp
	$(CPP) -MM -MT $(@:.d=.o) $(CPPFLAGS) $< > $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

# include dependency files only if our goal depends on their existence
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),show)
-include $(CPP_DEPS_EXT)
endif
endif

#-----------------------------------------------------------------------------
# the clean target
#
.PHONY : clean
clean:
	-$(RM) $(BIN_DIR)/*.o $(BIN_DIR)/*.d $(TARGET_LIB)
	
#-----------------------------------------------------------------------------
# the show target for debugging
#
show:
	@echo PROJECT = $(PROJECT)
	@echo CONF = $(CONF)
	@echo TARGET_LIB = $(TARGET_LIB)
	@echo CPP_SRCS = $(CPP_SRCS)
	@echo CPP_OBJS_EXT = $(CPP_OBJS_EXT)
	@echo CPP_DEPS_EXT = $(CPP_DEPS_EXT)
