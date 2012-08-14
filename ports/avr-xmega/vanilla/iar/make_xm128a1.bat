@echo off
:: ==========================================================================
:: Product: QP/C++ buld script for ARM-xmega, Vanilla port, IAR compiler
:: Last Updated for Version: 4.4.00
:: Date of the Last Update:  Apr 19, 2012
::
::                    Q u a n t u m     L e a P s
::                    ---------------------------
::                    innovating embedded systems
::
:: Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
::
:: This program is open source software: you can redistribute it and/or
:: modify it under the terms of the GNU General Public License as published
:: by the Free Software Foundation, either version 2 of the License, or
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
:: along with this program. If not, see <http:::www.gnu.org/licenses/>.
::
:: Contact information:
:: Quantum Leaps Web sites: http://www.quantum-leaps.com
::                          http://www.state-machine.com
:: e-mail:                  info@quantum-leaps.com
:: ==========================================================================
setlocal

:: adjust the following path to the location where you've installed
:: the IAR toolset...
::
if "%IAR_AVR%"=="" set IAR_AVR=C:\tools\IAR\AVR_5.30

::
:: Adjust the Target MCU to your project
:: 
set TARGET_MCU=xm128a1

:: Typically, you don't need to modify this file past this line -------------

set PATH=%IAR_AVR%\avr\bin;%IAR_AVR%\common\bin;%PATH%

set CC=iccavr
set ASM=aavr
set LIB=xar

set QP_INCDIR=..\..\..\..\include
set QP_PRTDIR=.

if "%1"=="" (
    @echo default selected
    set BINDIR=dbg
    set CCFLAGS=-I%IAR_AVR%\avr\inc -I%IAR_AVR%\avr\inc\dlib --cpu=%TARGET_MCU% -ms -y --initializers_in_flash -e --ec++ -z9 --cross_call_passes=3 --do_cross_call -DENABLE_BIT_DEFINITIONS --dlib_config %IAR_AVR%\avr\lib\dlib\dlAVR-6s-xmega-n.h --debug --diag_suppress Pa050
    set ASMFLAGS=-I%IAR_AVR%\avr\inc -u_enhancedCore -D__HAS_ENHANCED_CORE__=1 -D__HAS_MUL__=1 -D__XMEGA_CORE__=1 --xmega -D__MEMORY_MODEL__=2 -D__ATxmega128A1__=1 -D__HAS_ELPM__=1
)
if "%1"=="rel" (
    @echo rel selected
    set BINDIR=rel
    set CCFLAGS=-I%IAR_AVR%\avr\inc -I%IAR_AVR%\avr\inc\dlib --cpu=%TARGET_MCU% -ms -y --initializers_in_flash -e --ec++ -z9 --cross_call_passes=3 --do_cross_call -DENABLE_BIT_DEFINITIONS --dlib_config %IAR_AVR%\avr\lib\dlib\dlAVR-6s-xmega-n.h --diag_suppress Pa050 -DNDEBUG
    set ASMFLAGS=-I%IAR_AVR%\avr\inc -u_enhancedCore -D__HAS_ENHANCED_CORE__=1 -D__HAS_MUL__=1 -D__XMEGA_CORE__=1 --xmega -D__MEMORY_MODEL__=2 -D__ATxmega128A1__=1 -D__HAS_ELPM__=1
)
if "%1"=="spy" (
    @echo spy selected
    set BINDIR=spy
    set CCFLAGS=-I%IAR_AVR%\avr\inc -I%IAR_AVR%\avr\inc\dlib --cpu=%TARGET_MCU% -ms -y --initializers_in_flash -e --ec++ -z9 --cross_call_passes=3 --do_cross_call -DENABLE_BIT_DEFINITIONS --dlib_config %IAR_AVR%\avr\lib\dlib\dlAVR-6s-xmega-n.h --debug --diag_suppress Pa050 -DQ_SPY
    set ASMFLAGS=-I%IAR_AVR%\avr\inc -u_enhancedCore -D__HAS_ENHANCED_CORE__=1 -D__HAS_MUL__=1 -D__XMEGA_CORE__=1 --xmega -D__MEMORY_MODEL__=2 -D__ATxmega128A1__=1 -D__HAS_ELPM__=1
)

set LIBDIR=%BINDIR%
set LIBFLAGS=
mkdir %BINDIR%

:: QEP ----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qep\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo ON
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qep.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qfsm_ini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qfsm_dis.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qhsm_ini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qhsm_dis.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qhsm_top.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qhsm_in.cpp 

%LIB% %LIBFLAGS% %LIBDIR%\qep_%TARGET_MCU%.lib %BINDIR%\qep.r90 %BINDIR%\qfsm_ini.r90 %BINDIR%\qfsm_dis.r90 %BINDIR%\qhsm_ini.r90 %BINDIR%\qhsm_dis.r90 %BINDIR%\qhsm_top.r90 %BINDIR%\qhsm_in.r90
@echo OFF

:: QF -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qf\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo ON
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_defer.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_fifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_lifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_get_.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_sub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_usub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_usuba.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qeq_fifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qeq_get.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qeq_init.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qeq_lifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_act.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_gc.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_log2.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_new.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_pool.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_psini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_pspub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_pwr2.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_tick.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qmp_get.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qmp_init.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qmp_put.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_ctor.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_arm.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_darm.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_rarm.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qvanilla.cpp

%LIB% %LIBFLAGS% %LIBDIR%\qf_%TARGET_MCU%.lib %BINDIR%\qa_defer.r90 %BINDIR%\qa_fifo.r90 %BINDIR%\qa_lifo.r90 %BINDIR%\qa_get_.r90 %BINDIR%\qa_sub.r90 %BINDIR%\qa_usub.r90 %BINDIR%\qa_usuba.r90 %BINDIR%\qeq_fifo.r90 %BINDIR%\qeq_get.r90 %BINDIR%\qeq_init.r90 %BINDIR%\qeq_lifo.r90 %BINDIR%\qf_act.r90 %BINDIR%\qf_gc.r90 %BINDIR%\qf_log2.r90 %BINDIR%\qf_new.r90 %BINDIR%\qf_pool.r90 %BINDIR%\qf_psini.r90 %BINDIR%\qf_pspub.r90 %BINDIR%\qf_pwr2.r90 %BINDIR%\qf_tick.r90 %BINDIR%\qmp_get.r90 %BINDIR%\qmp_init.r90 %BINDIR%\qmp_put.r90 %BINDIR%\qte_ctor.r90 %BINDIR%\qte_arm.r90 %BINDIR%\qte_darm.r90 %BINDIR%\qte_rarm.r90 %BINDIR%\qvanilla.r90
@echo OFF

:: QS -----------------------------------------------------------------------
if not "%1"=="spy" goto clean

set SRCDIR=..\..\..\..\qs\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo ON
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_blk.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_byte.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_f32.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_f64.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_mem.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_str.cpp

%LIB% %LIBFLAGS% %LIBDIR%\qs_%TARGET_MCU%.lib %BINDIR%\qs.r90 %BINDIR%\qs_.r90 %BINDIR%\qs_blk.r90 %BINDIR%\qs_byte.r90 %BINDIR%\qs_f32.r90 %BINDIR%\qs_f64.r90 %BINDIR%\qs_mem.r90 %BINDIR%\qs_str.r90
@echo OFF
:: --------------------------------------------------------------------------

:clean
@@echo off

erase %BINDIR%\*.r90
rename %BINDIR%\*.lib *.r90

endlocal