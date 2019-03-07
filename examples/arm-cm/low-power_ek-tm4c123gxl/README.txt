About this Example
==================
This directory contains the simple "Low-Power" QP example application for
the EK-TM4C123GXL board (TivaC LauchPad) with the coopertative QV kernel,
the preemptive QK kernel, and the dual-mode QXK kernel. This directory
contains portable code that should compile with any C compiler for
ARM Cortex-M.

The sub-directories contain code and project files, which are specific
to the particular ARM toolchains, such as ARM (MDK-ARM), GNU-ARM, and
IAR EWARM.

Please refer to the README files in the sub-directories for specific
instructions how to use and customize the example to your needs.


Support Code for EK-TM4C123GXL Board 
====================================
The directory qpc\3rd_party\ek-tm4c123gxl contains the CMSIS-compliant
device code for the TM4C123GH6PM MCU. Please see the README.txt file in
this folder for more details.


QS Software Tracing Instrumentation
-----------------------------------
This example does NOT provide the "Spy" build configuration.
