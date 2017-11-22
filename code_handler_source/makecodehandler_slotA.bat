powerpc-eabi-gcc.exe -nostartfiles -nodefaultlibs -Wl,-Ttext,0x80001800 -o codehandlerslota.elf codehandlerslota.s
powerpc-eabi-strip.exe --strip-debug --strip-all --discard-all -o codehandlerslotas.elf -F elf32-powerpc codehandlerslota.elf
powerpc-eabi-objcopy.exe -I elf32-powerpc -O binary codehandlerslotas.elf codehandlerslota.bin
del *.elf
pause 