powerpc-eabi-gcc.exe -nostartfiles -nodefaultlibs -Wl,-Ttext,0x80001800 -o GCcodehandlerDebug.elf GCcodehandlerDebug.s
powerpc-eabi-strip.exe --strip-debug --strip-all --discard-all -o GCcodehandlerDebugs.elf -F elf32-powerpc GCcodehandlerDebug.elf
powerpc-eabi-objcopy.exe -I elf32-powerpc -O binary GCcodehandlerDebugs.elf GCcodehandlerDebug.bin
del *.elf
pause 