#
# Main makefile
#

CROSS		= powerpc-eabi-

export CC		:=	$(CROSS)gcc-4.4.3
export CXX		:=	$(CROSS)g++
export AR		:=	$(CROSS)ar
export OBJCOPY	:=	$(CROSS)objcopy
export AS		:=	$(CROSS)as
export LD		:=	$(CXX)
CFLAGS_LOCAL=
DOLTOOL=doltool
SOURCE=source
INCLUDE=include

# Configuration
CFLAGS =	-Wall $(CFLAGS_LOCAL) -I$(INCLUDE) -I../include \
			-mcpu=750 -meabi -mhard-float \
			-Os -fno-exceptions -nostdinc \
			-Werror-implicit-function-declaration -Wimplicit-int

AFLAGS	=	-Iinclude
LDFLAGS =	-mcpu=750 -meabi -mhard-float -nostdlib

# Files
#DVD_MODS = dvd forward jumptable cache memcpy
DVD_MODS = dvd forward jumptable cache

ifeq ($(HIGH_PLUGIN),yes)
	ifeq ($(GEKKO_DEBUG),yes)
		TYPE=debug
	else
		TYPE=high
	endif
	CFLAGS += -DHIGH_PLUGIN
	AFLAGS += -DHIGH_PLUGIN
	CFLAGS += -DACTION_REPLAY
	AFLAGS += -DACTION_REPLAY
	ifeq ($(DONTTELL),yes)
		CFLAGS += -DDONTTELL
		DONTTELL_MODS = patchhook
	endif
else
	TYPE=low
	CFLAGS += -DLOW_PLUGIN
	AFLAGS += -DLOW_PLUGIN
endif

ifeq ($(GEKKO_DEBUG),yes)
	CFLAGS += -DGEKKO_DEBUG
	AFLAGS += -DGEKKO_DEBUG
	ifeq ($(GEKKO_CHANNEL),0)
		CFLAGS += -DGEKKO_CHANNEL=0
		GECKO_MODS += usb_port0
	else
		CFLAGS += -DGEKKO_CHANNEL=1
		GECKO_MODS += usb_port1
	endif

	ifeq ($(SHOW_DVD_DETAILS),yes)
		CFLAGS += -DSHOW_DVD_DETAILS
	endif
	ifeq ($(SHOW_MEMSETS),yes)
		CFLAGS += -DSHOW_MEMSETS
	endif
	ifeq ($(SHOW_FILENAMES),yes)
		CFLAGS += -DSHOW_FILENAMES
	endif
	ifeq ($(REPLACE_DVD_CALLBACK),yes)
		CFLAGS += -DREPLACE_DVD_CALLBACK
		AFLAGS += -DREPLACE_DVD_CALLBACK
	endif

	GECKO_MODS += debugprintf snprintf strnlen stream
	EXCEPTION_MODS = exception
endif

DVD_MODS += pluginprotector
ifeq ($(RELOAD_SUPPORT),yes)
	CFLAGS += -DRELOAD_SUPPORT
	AFLAGS += -DRELOAD_SUPPORT
	RELOAD_MODS = dvdreadpatch writebranch codecmp
endif

BUILD=build-$(TYPE)

OBJS = $(addsuffix .o,$(addprefix $(BUILD)/,$(DVD_MODS) $(GECKO_MODS) $(RELOAD_MODS) $(DONTTELL_MODS) $(EXCEPTION_MODS) $(ULTRA_MODS))) 

# Build rules
all:
	@echo    
	@echo Building lowplugin...
	@echo   
	make MIOS_PLUGIN=no HIGH_PLUGIN=no RELOAD_SUPPORT=yes lowplugin.bin
	install -m 644 lowplugin.bin ../gcbackuplauncher/data/

	@echo    
	@echo Building highplugin...
	@echo   
	make MIOS_PLUGIN=no HIGH_PLUGIN=yes RELOAD_SUPPORT=yes highplugin.bin
	install -m 644 highplugin.bin ../gcbackuplauncher/data/

	@echo    
	@echo Building debuglugin...
	@echo   
	make MIOS_PLUGIN=no HIGH_PLUGIN=yes RELOAD_SUPPORT=yes GEKKO_DEBUG=yes SHOW_FILENAMES=yes debugplugin.bin
	#make MIOS_PLUGIN=no HIGH_PLUGIN=yes RELOAD_SUPPORT=yes GEKKO_DEBUG=yes SHOW_FILENAMES=yes GEKKO_CHANNEL=0 debugplugin.bin
	install -m 644 debugplugin.bin ../gcbackuplauncher/data/

clean:
	rm -rf build-high
	rm -rf build-low
	rm -rf build-debug
	rm -f highplugin.elf highplugin.bin
	rm -f lowplugin.elf lowplugin.bin
	rm -f debugplugin.elf debugplugin.bin
	
realclean:
	find -name '*.o' | xargs -r rm
	find -name '*.a' | xargs -r rm
	find -name '*~' | xargs -r rm
	find -name '*.bin' | xargs -r rm
	find -name '*.dol' | xargs -r rm
	find -name '*.elf' | xargs -r rm
	find -name '*.map' | xargs -r rm
	find -name '*.tmp' | xargs -r rm


$(BUILD)/%.o: $(SOURCE)/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS)  -c -o $@ $^

$(BUILD)/%.o: $(SOURCE)/%.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CFLAGS) -c -o $@ $^

$(BUILD)/%.o: $(SOURCE)/%.s
	@mkdir -p $(BUILD)
	$(CC) $(AFLAGS) -c -o $@ $^

$(BUILD)/%.o: $(SOURCE)/%.S
	@mkdir -p $(BUILD)
	$(CC) $(AFLAGS) -c -o $@ $^

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

highplugin.elf: $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-n -o $@ -Wl,-Tlinker-high.lds $(OBJS) -lm

lowplugin.elf: $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-n -o $@ -Wl,-Tlinker-low.lds $(OBJS) -lm

debugplugin.elf: $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-n -o $@ -Wl,-Tlinker-debug.lds $(OBJS) -lm

