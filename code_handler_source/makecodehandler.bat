powerpc-eabi-gcc.exe -nostartfiles -nodefaultlibs -Wl,-Ttext,0x80001800 -o codehandler.elf codehandler.s
powerpc-eabi-strip.exe --strip-debug --strip-all --discard-all -o codehandlers.elf -F elf32-powerpc codehandler.elf
powerpc-eabi-objcopy.exe -I elf32-powerpc -O binary codehandlers.elf codehandler.bin
del *.elf
pause 