About this Example==================This directory contains the simple "Blinky" QP-RTX example application
for the EK-TM4C123GXL board, a.k.a. TivaC LauchPad (ARM Cortex-M4F).

Limitations of the Blinky Demo
------------------------------
The "Blinky" demo is specifically kept small and simple to demonstrate
the basic bare-bones application quickly.

However, to achieve this simplicity, the demo intentionally omits 
many features of QP, such as: multiple active objects, dynamic events
(event pools), publish-subscribe event delivery, or QS software tracing.

Please refer to more complete examples (such as the "Dining Philosophers
Problem" (DPP) for demonstration of the aforementioned features.


Support Code for EK-TM4C123GXL Board 
====================================
The directory qpc\3rd_party\ek-tm4c123gxl contains the CMSIS-compliant
device code for the TM4C123GH6PM MCU (ARM Cortex-M4F). Please see the
README.txt file in this folder for more details.


Adjusting Stack and Heap Sizes
==============================
The stack and heap sizes are determined in this project by the 
command-line options for the ARM assembler (see the Asm tab in
the "Options for Target" dialog box in uVision). Specifically,
you should define symbols: Stack_Size=xxx Heap_Size=yyy, where
xxx represents a numerical value of stack size and yyy the
numerical value of the heap size.

***
NOTE:
C++ programs seem not to tolerate heap size of 0. Therefore it is
recommended to set the Heap_Size symbol to a minimal value of 16.  
***   

