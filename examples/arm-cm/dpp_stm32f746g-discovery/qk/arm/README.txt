About this Example
==================
This example demonstrates how to use the ARM-KEIL IDE to build
a QP application.


ARM-KEIL Project File
---------------------
The ARM-KEIL uVision project file provided with this example uses
relative paths to the QP/C framework location (includes, port). These
relative paths must be modified when the project is moved to different
relative location.


Stack Size and Heap Size
------------------------
In this project, the size of the C stack and heap are determined in
the assembly options, because both the stack and the heap are
allocated in the assembly startup code (see the next section).


Startup Code
============
The startup code for the STM32F746NG MCU used in this project is
located in the "3rd_party" folder in the following location:

3rd_party\stm32f7-discovery\arm\startup_stm32f746xx.s

The file startup_stm32f746xx.s provides a template of the recommended
startup for QP applications and should be easily customizable for other
ARM Cortex-M microcontrollers.

The startup file typically does not need to be modified or adapted for
applications. It provides only weak definitions of all exception and
interrupt handlers, as well as the assert_failed() function.  

The weak function assert_failed() defined in this file might be re-defined
in the application to customize it for the application-specific error-
handling policy.

***
NOTE: The function assert_failed() typically should NOT use the stack,
because stack might be corrupted by the time this function is called.
Also, assert_failed() is intended to handle catastrophic errors and
should NOT return.
***
