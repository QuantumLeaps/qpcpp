The port to ARM-LLVM toolchain is identical to ARM-CLANG
(see ports/arm-cm/armclang/ sub-directory), except it uses
a different interrupt disabling policy ("save/restore interrupt
status" instead of "unconditionally disabe/enable interrupts").
