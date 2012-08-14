#*************************************************
# 
# Connect to ST-LINK and debug application
# 
 
# Connect to the J-Link gdb server
target remote localhost:61234

# Reset the chip to get to a known state
monitor reset

# Download to flash
load

# Reset the chip to get to a known state
monitor reset

break main
continue
