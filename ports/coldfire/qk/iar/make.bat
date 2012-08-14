@echo off
:: ==========================================================================
:: Product: QP/C++ buld script for ColdFire, QK port, IAR EWCF 1.2
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
set IAR_CF=c:\tools\IAR\ColdFire_KS_1.20

set PATH=%IAR_CF%\cf\bin;%IAR_CF%\common\bin;%PATH%

set CC=icccf
set ASM=acf
set LIB=xar

set QP_INCDIR=..\..\..\..\include
set QP_PRTDIR=.

if "%1"=="" (
    echo default selected
    set BINDIR=%QP_PRTDIR%\dbg
    set CCFLAGS=-I %IAR_CF%\cf\INC\ -D DEBUG --debug --core v2 --isa isa_a+ --code_model far --eec++ --dlib_config %IAR_CF%\cf\LIB\dlcfaffn.h --diag_suppress Pa050 -Ol
    set ASMFLAGS=-r --isa isa_a+ --mac mac -I%IAR_CF%\cf\INC\ 

)
if "%1"=="rel" (
    echo rel selected
    set BINDIR=%QP_PRTDIR%\rel
    set CCFLAGS=-I %IAR_CF%\cf\INC\ -D NDEBUG --core v2 --isa isa_a+ --code_model far --eec++ --dlib_config %IAR_CF%\cf\LIB\dlcfaffn.h --diag_suppress Pa050 -Oh
    set ASMFLAGS=-r --isa isa_a+ --mac mac -I%IAR_CF%\cf\INC\ 
)
if "%1"=="spy" (
    echo spy selected
    set BINDIR=%QP_PRTDIR%\spy
    set CCFLAGS=-I %IAR_CF%\cf\INC\ -D Q_SPY --debug --core v2 --isa isa_a+ --code_model far --eec++ --dlib_config %IAR_CF%\cf\LIB\dlcfaffn.h --diag_suppress Pa050 -Ol
    set ASMFLAGS=-r --isa isa_a+ --mac mac -I%IAR_CF%\cf\INC\ 
)

set LIBDIR=%BINDIR%
set LIBFLAGS=
mkdir %BINDIR%

:: QEP ----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qep\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qep.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qfsm_ini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qfsm_dis.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qhsm_ini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qhsm_dis.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qhsm_top.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qhsm_in.cpp

%LIB% %LIBFLAGS% %LIBDIR%\libqep.lib %BINDIR%\qep.r68 %BINDIR%\qfsm_ini.r68 %BINDIR%\qfsm_dis.r68 %BINDIR%\qhsm_ini.r68 %BINDIR%\qhsm_dis.r68 %BINDIR%\qhsm_top.r68 %BINDIR%\qhsm_in.r68
@echo off

:: QF -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qf\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qa_defer.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qa_fifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qa_lifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qa_get_.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qa_sub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qa_usub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qa_usuba.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qeq_fifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qeq_get.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qeq_init.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qeq_lifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_act.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_gc.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_log2.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_new.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_pool.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_psini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_pspub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_pwr2.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qf_tick.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qmp_get.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qmp_init.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qmp_put.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qte_ctor.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qte_arm.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qte_darm.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qte_rarm.cpp

%LIB% %LIBFLAGS% %LIBDIR%\libqf.lib %BINDIR%\qa_defer.r68 %BINDIR%\qa_fifo.r68 %BINDIR%\qa_lifo.r68 %BINDIR%\qa_get_.r68 %BINDIR%\qa_sub.r68 %BINDIR%\qa_usub.r68 %BINDIR%\qa_usuba.r68 %BINDIR%\qeq_fifo.r68 %BINDIR%\qeq_get.r68 %BINDIR%\qeq_init.r68 %BINDIR%\qeq_lifo.r68 %BINDIR%\qf_act.r68 %BINDIR%\qf_gc.r68 %BINDIR%\qf_log2.r68 %BINDIR%\qf_new.r68 %BINDIR%\qf_pool.r68 %BINDIR%\qf_psini.r68 %BINDIR%\qf_pspub.r68 %BINDIR%\qf_pwr2.r68 %BINDIR%\qf_tick.r68 %BINDIR%\qmp_get.r68 %BINDIR%\qmp_init.r68 %BINDIR%\qmp_put.r68 %BINDIR%\qte_ctor.r68 %BINDIR%\qte_arm.r68 %BINDIR%\qte_darm.r68 %BINDIR%\qte_rarm.r68 
@echo off

:: QK -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qk\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qk.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qk_sched.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qk_mutex.cpp

%LIB% %LIBFLAGS% %LIBDIR%\libqk.lib %BINDIR%\qk.r68 %BINDIR%\qk_sched.r68 %BINDIR%\qk_mutex.r68
@echo off

if not "%1"=="spy" goto clean

:: QS -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qs\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qs.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qs_.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qs_blk.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qs_byte.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qs_f32.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qs_f64.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qs_mem.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR%\ %SRCDIR%\qs_str.cpp

%LIB% %LIBFLAGS% %LIBDIR%\libqs.lib %BINDIR%\qs.r68 %BINDIR%\qs_.r68 %BINDIR%\qs_blk.r68 %BINDIR%\qs_byte.r68 %BINDIR%\qs_f32.r68 %BINDIR%\qs_f64.r68 %BINDIR%\qs_mem.r68 %BINDIR%\qs_str.r68
@echo off

:clean
@echo off
erase %BINDIR%\*.r68
rename %BINDIR%\*.lib *.r68

endlocal