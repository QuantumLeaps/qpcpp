Known Issues
============
The CCS reports a fake Semantic Error for the following items:

Function 'QF_INT_DISABLE' could not be resolved  bsp.cpp  /blinky-qk line 130
Function 'QF_INT_ENABLE_ALL' could not be resolved  bsp.cpp /blinky-qk line 118
Function 'QF_INT_ENABLE' could not be resolved  bsp.cpp  /blinky-qk line 133
Invalid arguments main.cpp  /blinky-qk  line 48 Semantic Error

These "Semantic Errors" seem to be coming from the Indexer. 

NOTE: The final image is built correctly in spite of these "Semantic Errors"

QL 08/31/2016



