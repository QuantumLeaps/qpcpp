This example demonstrates how to use the uVision IDE together with
the MDK-ARM toolchain.


uVision Project File
--------------------
The MDK-ARM uVision project file provided with this example
uses relative paths to the QP/C++ framework location (includes,
port, and libraries. These relative paths must be modified
when the project is moved to different relative location.

***
NOTE: This example requires installing the following Software Pack
in the Keil uVision: Keil::STM32L0xx_DFP.
***


Support Code for the STM32 NUCLEO-L053 Board 
============================================
The directory qpc\3rd_party\nucleo-l053r8 contains the CMSIS-compliant
device code for the STM32L053xx MCU (ARM Cortex-M0+). Please see the
README.txt file in this folder for more details.

Startup Code
------------
The startup file startup_stm32l053xx.s provides a template
of the recommended startup for QP projects. The file needs
to be customized to adjust the stack/heap sizes and to implement
the product-specific error/assertion handling policy
in the assert_failed() function.


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

  