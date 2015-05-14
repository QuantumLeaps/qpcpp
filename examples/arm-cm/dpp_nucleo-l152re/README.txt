About this Example
==================
This directory contains the "Dining Philosophers Problem" (DPP) example
running on the STM32 NUCLEO-L152RE board (ARM Cortex-M3). The following
versions of the example are provided:

dpp_nucleo-l152re/
 |
 +-qk/      - preemptive QK kernel
 | +-arm/   - ARM-KEIL toolset
 | +-gnu/   - GNU-ARM toolset
 | +-iar/   - IAR-ARM toolset
 |
 +-qv/      - cooperative QV kernel
 | +-arm/   - ARM-KEIL toolset
 | +-gnu/   - GNU-ARM toolset
 | +-iar/   - IAR-ARM toolset

***
NOTE: The sub-directory "gnu" contains the Makefile for a generic GNU-ARM
toolset (e.g., see http://gnutoolchains.com/arm-eabi/). Thus, this
project provides a way of building mbed applications locally with
free and unrestricted tools.
***
***
NOTE: To build the code on Windows, you need to download and install
the GNU make utility. The Qtools collection from Quantum Leaps contains
GNU make and other UNIX-style utilites for Windows (native Windows,
without the need to install CygWin).
***

Downloading the Code to STM32 NUCLEO-L152RE Board
=================================================
After building the code with any of the supported toosets, you have
two options of loading the code to the NUCLEO board.

First, you can use the on-board ST-Link debugger to download the code
from the toolset IDE (e.g., uVision or IAR EW).

Alternatively, you can simply copy the binary image to the NUCLEO
folder for execution.

***
NOTE:
The whole build process and loading the image to the mbed board can
be executed directly from the QM modeling (by means of external tools).
The provided QM model (dpp.qm) comes pre-configured with tools
setup to build (via make) and copy the code to the mbed board. 
***


Support Code for STM32 NUCLEO-L152RE Board
------------------------------------------
The directory qpc\3rd_party\nucleo-l152re contains the CMSIS-
compliant device code for the STM32L0xx MCUs (ARM Cortex-M3). Please see
the README file in this folder for more details.


QS Software Tracing Instrumentation
===================================
This example provides the "Spy" build configuration, which outputs the QS
(Quantum Spy) software tracing data through UART2, which is connected to
the virtual COM port of the ST-Link V2 USB debugger.

The output is generated at 115200 baud rate.

Here is an example invocation of the QSPY host application to receive
the QS data from NUCLEO board:

qspy -cCOM20

The actual COM port number might be different on your Windows machine.
Please check the Device Manager to find the COM port number.

