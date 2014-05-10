@echo off
:: ==========================================================================
:: Product: DPP/C++ example, uC/OS-II/80x86 port, Open Watcom compiler
:: Last updated for version 5.3.1
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

:: If you have defined the WATCOM environment variable, the following line has
:: no effect and your definition is used. However, if the varible WATCOM is
:: not defined, the following default is assuemed: 
if "%WATCOM%"=="" set WATCOM=c:\tools\WATCOM

path %WATCOM%\binw
set INCLUDE=%WATCOM%\H
set CC=wpp.exe
set AS=wasm.exe
set LD=wlink.exe

if not "%1"=="rel" goto spy
    echo rel selected
    set BINDIR=rel
    set CCFLAGS=-d0 -ml -3 -fpi87 -ot -dNDEBUG
    set LDFLAGS=@link_rel.rsp
goto compile
:spy
if not "%1"=="spy" goto dbg
    echo spy selected
    set BINDIR=spy
    set CCFLAGS=-d2 -ml -3 -fpi87 -dQ_SPY
    set LDFLAGS=@link_spy.rsp
goto compile
:dbg
    echo default selected
    set BINDIR=dbg
    set CCFLAGS=-d2 -ml -3 -fpi87
    set LDFLAGS=@link_dbg.rsp

:: compile -------------------------------------------------------------------
:compile
mkdir %BINDIR%
set CCINC=@inc_qp.rsp

:: DPP ----------------------------------------------------------------------
:dpp
set SRCDIR=.

@echo on
%CC% %CCFLAGS% %CCINC% -fo=%BINDIR%\main.obj   %SRCDIR%\main.cpp
%CC% %CCFLAGS% %CCINC% -fo=%BINDIR%\bsp.obj    %SRCDIR%\bsp.cpp
%CC% %CCFLAGS% %CCINC% -fo=%BINDIR%\philo.obj  %SRCDIR%\philo.cpp
%CC% %CCFLAGS% %CCINC% -fo=%BINDIR%\table.obj  %SRCDIR%\table.cpp
%CC% %CCFLAGS% %CCINC% -fo=%BINDIR%\video.obj  %SRCDIR%\video.cpp

:: link ----------------------------------------------------------------------
@echo on
%LD% %LDFLAGS%
@echo off
