@echo off
:: ===========================================================================
:: Product: QP/C++ buld script for ARM Cortex-M0, Vanilla port, Code Sourcery
:: Last updated for version 5.3.0
:: Last updated on  2014-04-13
::
::                    Q u a n t u m     L e a P s
::                    ---------------------------
::                    innovating embedded systems
::
:: Copyright (C) Quantum Leaps, www.state-machine.com.
::
:: This program is open source software: you can redistribute it and/or
:: modify it under the terms of the GNU General Public License as published
:: by the Free Software Foundation, either version 3 of the License, or
:: (at your option) any later version.
::
:: Alternatively, this program may be distributed and modified under the
:: terms of Quantum Leaps commercial licenses, which expressly supersede
:: the GNU General Public License and are specifically designed for
:: licensees interested in retaining the proprietary status of their code.
::
:: This program is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
:: GNU General Public License for more details.
::
:: You should have received a copy of the GNU General Public License
:: along with this program. If not, see <http://www.gnu.org/licenses/>.
::
:: Contact information:
:: Web:   www.state-machine.com
:: Email: info@state-machine.com
:: ===========================================================================
setlocal

:: adjust the following path to the location where you've installed
:: the GNU_ARM toolset...
::
if "%GNU_ARM%"=="" set GNU_ARM="C:\tools\CodeSourcery"

set CC=%GNU_ARM%\bin\arm-none-eabi-g++
set ASM=%GNU_ARM%\bin\arm-none-eabi-as
set LIB=%GNU_ARM%\bin\arm-none-eabi-ar

set QP_INCDIR=..\..\..\..\include
set QP_PRTDIR=.

set ARM_CORE=cortex-m0
set ARM_FPU=

if "%1"=="" (
    echo default selected
    set BINDIR=%QP_PRTDIR%\dbg
    set CCOPT=-O
    set CCFLAGS=-g -c -mcpu=%ARM_CORE% -DARM_ARCH_V6M -mthumb -Wall -fno-rtti -fno-exceptions
    set ASMFLAGS=-g -mcpu=%ARM_CORE% -DARM_ARCH_V6M
)
if "%1"=="rel" (
    echo rel selected
    set BINDIR=%QP_PRTDIR%\rel
    set CCOPT=-Os
    set CCFLAGS=-c -mcpu=%ARM_CORE% -DARM_ARCH_V6M -mthumb -Wall -fno-rtti -fno-exceptions -DNDEBUG
    set ASMFLAGS=-mcpu=%ARM_CORE% -DARM_ARCH_V6M
)
if "%1"=="spy" (
    echo spy selected
    set BINDIR=%QP_PRTDIR%\spy
    set CCOPT=-O
    set CCOPT_QS=-Os
    set CCFLAGS=-g -c -mcpu=%ARM_CORE% -DARM_ARCH_V6M -mthumb -Wall -fno-rtti -fno-exceptions -DQ_SPY
    set ASMFLAGS=-g -mcpu=%ARM_CORE% -DARM_ARCH_V6M
)

mkdir %BINDIR%
set LIBDIR=%BINDIR%
set LIBFLAGS=rs
erase %LIBDIR%\libqp_%ARM_CORE%_cs.a

:: QEP ----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qep\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qep.cpp      -o%BINDIR%\qep.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qmsm_ini.cpp -o%BINDIR%\qmsm_ini.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qmsm_dis.cpp -o%BINDIR%\qmsm_dis.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qmsm_in.cpp  -o%BINDIR%\qmsm_in.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qfsm_ini.cpp -o%BINDIR%\qfsm_ini.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qfsm_dis.cpp -o%BINDIR%\qfsm_dis.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qhsm_ini.cpp -o%BINDIR%\qhsm_ini.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qhsm_dis.cpp -o%BINDIR%\qhsm_dis.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qhsm_top.cpp -o%BINDIR%\qhsm_top.o
%CC% %CCFLAGS% %CCOPT% %CCINC% %SRCDIR%\qhsm_in.cpp  -o%BINDIR%\qhsm_in.o

%LIB% %LIBFLAGS% %LIBDIR%\libqp_%ARM_CORE%_cs.a %BINDIR%\qep.o %BINDIR%\qmsm_ini.o %BINDIR%\qmsm_dis.o %BINDIR%\qmsm_in.o %BINDIR%\qfsm_ini.o %BINDIR%\qfsm_dis.o %BINDIR%\qhsm_ini.o %BINDIR%\qhsm_dis.o %BINDIR%\qhsm_top.o %BINDIR%\qhsm_in.o
@echo off
erase %BINDIR%\*.o

:: QF -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qf\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qa_defer.cpp -o%BINDIR%\qa_defer.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qa_fifo.cpp  -o%BINDIR%\qa_fifo.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qa_lifo.cpp  -o%BINDIR%\qa_lifo.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qa_get_.cpp  -o%BINDIR%\qa_get_.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qa_sub.cpp   -o%BINDIR%\qa_sub.o  
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qa_usub.cpp  -o%BINDIR%\qa_usub.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qa_usuba.cpp -o%BINDIR%\qa_usuba.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qeq_fifo.cpp -o%BINDIR%\qeq_fifo.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qeq_get.cpp  -o%BINDIR%\qeq_get.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qeq_init.cpp -o%BINDIR%\qeq_init.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qeq_lifo.cpp -o%BINDIR%\qeq_lifo.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_act.cpp   -o%BINDIR%\qf_act.o  
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_gc.cpp    -o%BINDIR%\qf_gc.o      
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_log2.cpp  -o%BINDIR%\qf_log2.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_new.cpp   -o%BINDIR%\qf_new.o  
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_pool.cpp  -o%BINDIR%\qf_pool.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_psini.cpp -o%BINDIR%\qf_psini.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_pspub.cpp -o%BINDIR%\qf_pspub.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_pwr2.cpp  -o%BINDIR%\qf_pwr2.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qf_tick.cpp  -o%BINDIR%\qf_tick.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qmp_get.cpp  -o%BINDIR%\qmp_get.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qmp_init.cpp -o%BINDIR%\qmp_init.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qmp_put.cpp  -o%BINDIR%\qmp_put.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qte_ctor.cpp -o%BINDIR%\qte_ctor.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qte_arm.cpp  -o%BINDIR%\qte_arm.o 
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qte_darm.cpp -o%BINDIR%\qte_darm.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qte_rarm.cpp -o%BINDIR%\qte_rarm.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qte_ctr.cpp  -o%BINDIR%\qte_ctr.o
%CC% %CCFLAGS% %CCINC% %CCOPT% %SRCDIR%\qvanilla.cpp -o%BINDIR%\qvanilla.o

%LIB% %LIBFLAGS% %LIBDIR%\libqp_%ARM_CORE%_cs.a %BINDIR%\qa_defer.o %BINDIR%\qa_fifo.o %BINDIR%\qa_lifo.o %BINDIR%\qa_get_.o %BINDIR%\qa_sub.o %BINDIR%\qa_usub.o %BINDIR%\qa_usuba.o %BINDIR%\qeq_fifo.o %BINDIR%\qeq_get.o %BINDIR%\qeq_init.o %BINDIR%\qeq_lifo.o %BINDIR%\qf_act.o %BINDIR%\qf_gc.o %BINDIR%\qf_log2.o %BINDIR%\qf_new.o %BINDIR%\qf_pool.o %BINDIR%\qf_psini.o %BINDIR%\qf_pspub.o %BINDIR%\qf_pwr2.o %BINDIR%\qf_tick.o %BINDIR%\qmp_get.o %BINDIR%\qmp_init.o %BINDIR%\qmp_put.o %BINDIR%\qte_ctor.o %BINDIR%\qte_arm.o %BINDIR%\qte_darm.o %BINDIR%\qte_rarm.o %BINDIR%\qte_ctr.o %BINDIR%\qvanilla.o
@echo off
erase %BINDIR%\*.o

:: QS -----------------------------------------------------------------------
if not "%1"=="spy" goto clean

set SRCDIR=..\..\..\..\qs\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs.cpp      -o%BINDIR%\qs.o     
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs_.cpp     -o%BINDIR%\qs_.o     
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs_blk.cpp  -o%BINDIR%\qs_blk.o 
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs_byte.cpp -o%BINDIR%\qs_byte.o
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs_dict.cpp -o%BINDIR%\qs_dict.o
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs_f32.cpp  -o%BINDIR%\qs_f32.o 
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs_f64.cpp  -o%BINDIR%\qs_f64.o 
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs_mem.cpp  -o%BINDIR%\qs_mem.o 
%CC% %CCFLAGS% %CCINC% %CCOPT_QS% %SRCDIR%\qs_str.cpp  -o%BINDIR%\qs_str.o 

%LIB% %LIBFLAGS% %LIBDIR%\libqp_%ARM_CORE%_cs.a %BINDIR%\qs.o %BINDIR%\qs_.o %BINDIR%\qs_blk.o %BINDIR%\qs_byte.o %BINDIR%\qs_dict.o %BINDIR%\qs_f32.o %BINDIR%\qs_f64.o %BINDIR%\qs_mem.o %BINDIR%\qs_str.o
@echo off
erase %BINDIR%\*.o

:: --------------------------------------------------------------------------

:clean

endlocal
