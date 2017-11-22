powerpc-eabi-gcc.exe -nostartfiles -nodefaultlibs -Wl,-Ttext,0x80001800 -o codehandleronly.elf codehandleronly.s
powerpc-eabi-strip.exe --strip-debug --strip-all --discard-all -o codehandlersonly.elf -F elf32-powerpc codehandleronly.elf
powerpc-eabi-objcopy.exe -I elf32-powerpc -O binary codehandlersonly.elf codehandleronly.bin
del *.elf
pause 