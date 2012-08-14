@echo off
:: ==========================================================================
:: Product: QP/C++ library buld script, H8, Vanilla port, Renesas H8 compiler
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
:: the Renesas H8 toolset...
::
set RENESAS_H8_DIR=C:\tools\Renesas\Hew\Tools\Renesas\H8\6_2_1

::
:: Adjust the Target MCU to your project
:: 
set TARGET_MCU=300HN

set PATH=%RENESAS_H8_DIR%\bin;%PATH%
set CC=ch38.exe
set ASM=asm38.exe
set LIB=optlnk.exe
set H8_INCDIR=%RENESAS_H8_DIR%\include

set QP_INCDIR=..\..\..\..\include
set QP_PRTDIR=.

if "%1"=="" (
    echo default selected
    set BINDIR=%QP_PRTDIR%\dbg
    set CCFLAGS=-debug -nolist -chgincpath -cpu=%TARGET_MCU%
)
if "%1"=="rel" (
    echo rel selected
    set BINDIR=%QP_PRTDIR%\rel
    set CCFLAGS=-nolist -chgincpath -cpu=%TARGET_MCU% -define=NDEBUG -speed=register,switch,shift,struct,expression,loop=2,inline 
)
if "%1"=="spy" (
    echo spy selected
    set BINDIR=%QP_PRTDIR%\spy
    set CCFLAGS=-debug -nolist -chgincpath -cpu=%TARGET_MCU% -define=Q_SPY
)

set LIBDIR=%BINDIR%
set LIBFLAGS=-form=library -noprelink
mkdir %BINDIR%

:: QEP ----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qep\source
set CCINC=-include=%H8_INCDIR%,%QP_PRTDIR%,%QP_INCDIR%,%SRCDIR%

%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qep.obj      %SRCDIR%\qep.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qfsm_ini.obj %SRCDIR%\qfsm_ini.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qfsm_dis.obj %SRCDIR%\qfsm_dis.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qhsm_ini.obj %SRCDIR%\qhsm_ini.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qhsm_dis.obj %SRCDIR%\qhsm_dis.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qhsm_top.obj %SRCDIR%\qhsm_top.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qhsm_in.obj  %SRCDIR%\qhsm_in.cpp

%LIB% %LIBFLAGS% -output=%LIBDIR%\qep_%TARGET_MCU%.lib %BINDIR%\qep.obj %BINDIR%\qfsm_ini.obj %BINDIR%\qfsm_dis.obj %BINDIR%\qhsm_ini.obj %BINDIR%\qhsm_dis.obj %BINDIR%\qhsm_top.obj %BINDIR%\qhsm_in.obj

:: QF -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qf\source
set CCINC=-include=%H8_INCDIR%,%QP_PRTDIR%,%QP_INCDIR%,%SRCDIR%

%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qa_defer.obj %SRCDIR%\qa_defer.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qa_fifo.obj  %SRCDIR%\qa_fifo.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qa_lifo.obj  %SRCDIR%\qa_lifo.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qa_get_.obj  %SRCDIR%\qa_get_.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qa_sub.obj   %SRCDIR%\qa_sub.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qa_usub.obj  %SRCDIR%\qa_usub.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qa_usuba.obj %SRCDIR%\qa_usuba.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qeq_fifo.obj %SRCDIR%\qeq_fifo.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qeq_get.obj  %SRCDIR%\qeq_get.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qeq_init.obj %SRCDIR%\qeq_init.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qeq_lifo.obj %SRCDIR%\qeq_lifo.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_act.obj   %SRCDIR%\qf_act.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_gc.obj    %SRCDIR%\qf_gc.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_log2.obj  %SRCDIR%\qf_log2.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_new.obj   %SRCDIR%\qf_new.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_pool.obj  %SRCDIR%\qf_pool.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_psini.obj %SRCDIR%\qf_psini.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_pspub.obj %SRCDIR%\qf_pspub.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_pwr2.obj  %SRCDIR%\qf_pwr2.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qf_tick.obj  %SRCDIR%\qf_tick.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qmp_get.obj  %SRCDIR%\qmp_get.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qmp_init.obj %SRCDIR%\qmp_init.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qmp_put.obj  %SRCDIR%\qmp_put.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qte_ctor.obj %SRCDIR%\qte_ctor.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qte_arm.obj  %SRCDIR%\qte_arm.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qte_darm.obj %SRCDIR%\qte_darm.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qte_rarm.obj %SRCDIR%\qte_rarm.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qvanilla.obj %SRCDIR%\qvanilla.cpp

%LIB% %LIBFLAGS% -output=%LIBDIR%\qf_%TARGET_MCU%.lib %BINDIR%\qa_defer.obj %BINDIR%\qa_fifo.obj %BINDIR%\qa_lifo.obj %BINDIR%\qa_get_.obj %BINDIR%\qa_sub.obj %BINDIR%\qa_usub.obj %BINDIR%\qa_usuba.obj %BINDIR%\qeq_fifo.obj %BINDIR%\qeq_get.obj %BINDIR%\qeq_init.obj %BINDIR%\qeq_lifo.obj %BINDIR%\qf_act.obj %BINDIR%\qf_gc.obj %BINDIR%\qf_log2.obj %BINDIR%\qf_new.obj %BINDIR%\qf_pool.obj %BINDIR%\qf_psini.obj %BINDIR%\qf_pspub.obj %BINDIR%\qf_pwr2.obj %BINDIR%\qf_tick.obj %BINDIR%\qmp_get.obj %BINDIR%\qmp_init.obj %BINDIR%\qmp_put.obj %BINDIR%\qte_ctor.obj %BINDIR%\qte_arm.obj %BINDIR%\qte_darm.obj %BINDIR%\qte_rarm.obj %BINDIR%\qvanilla.obj

:: QS -----------------------------------------------------------------------
if not "%1"=="spy" goto clean

set SRCDIR=..\..\..\..\qs\source
set CCINC=-include=%H8_INCDIR%,%QP_PRTDIR%,%QP_INCDIR%,%SRCDIR%

%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qs.obj      %SRCDIR%\qs.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qs_.obj     %SRCDIR%\qs_.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qs_blk.obj  %SRCDIR%\qs_blk.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qs_byte.obj %SRCDIR%\qs_byte.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qs_f32.obj  %SRCDIR%\qs_f32.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qs_f64.obj  %SRCDIR%\qs_f64.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qs_mem.obj  %SRCDIR%\qs_mem.cpp
%CC% %CCFLAGS% %CCINC% -object=%BINDIR%\qs_str.obj  %SRCDIR%\qs_str.cpp

%LIB% %LIBFLAGS% -output=%LIBDIR%\qs_%TARGET_MCU%.lib %BINDIR%\qs.obj %BINDIR%\qs_.obj %BINDIR%\qs_blk.obj %BINDIR%\qs_byte.obj %BINDIR%\qs_f32.obj %BINDIR%\qs_f64.obj %BINDIR%\qs_mem.obj %BINDIR%\qs_str.obj

:: --------------------------------------------------------------------------
:clean

erase %BINDIR%\*.obj


endlocal