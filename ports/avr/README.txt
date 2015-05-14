About The QP-AVR Port
=====================
This directory contains the QP/C++ ports to the 8-bit AVRmega processor
family. The following ports are provided:

avr
|
+-qk      - port to the preemptive QK kernel
| +-gnu   - port with the GNU-AVR toolset
| +-iar   - port with the IAR-AVR toolset
|
+-qv      - port to the cooperative QV kernel
| +-gnu   - port with the GNU-AVR toolset
| +-iar   - port with the IAR-AVR toolset

***
NOTE: This port pertains only to 8-bit AVRmega and NOT to AVR-Xmega or
to AVR32. These latter CPU architectures require different ports.
***
