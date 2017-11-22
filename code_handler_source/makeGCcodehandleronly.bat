powerpc-eabi-gcc.exe -nostartfiles -nodefaultlibs -Wl,-Ttext,0x80001800 -o GCcodehandleronly.elf GCcodehandleronly.s
powerpc-eabi-strip.exe --strip-debug --strip-all --discard-all -o GCcodehandleronlys.elf -F elf32-powerpc GCcodehandleronly.elf
powerpc-eabi-objcopy.exe -I elf32-powerpc -O binary GCcodehandleronlys.elf GCcodehandleronly.bin
del *.elf
pause 