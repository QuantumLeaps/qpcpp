#*************************************************
# 
# Connect to J-Link and debug application in flash on Cortex-M3
# 
# Download to flash is performed.

 
# Connect to the J-Link gdb server
target remote localhost:2331
monitor speed 1000
monitor flash device = STM32F107VC
monitor flash download = 1

# Set gdb server to little endian
monitor endian little

# Set JTAG speed to 30 kHz
monitor speed 30

# Reset the chip to get to a known state.
monitor reset
monitor sleep 10

# Set JTAG speed in khz
monitor speed 12000

load

monitor sleep 100

# Reset the chip to get to a known state.
monitor reset
monitor sleep 10

break main
continue
