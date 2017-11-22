GC_VERSION=0.2
GC_DATE=`date +%Y%m%d%H%M`

ifeq ($(GEKKO_DEBUG),yes)
RELEASE=gcbackup$(shell echo $(GC_VERSION) | tr ' ' '_')_dbg
else
RELEASE=gcbackup$(shell echo $(GC_VERSION) | tr ' ' '_')
endif

all:
	make -C gcplugin DONTTELL=yes
	make -C gcbackuplauncher DONTTELL=yes
	
clean:
	make -C gcplugin clean
	make -C gcbackuplauncher clean
	-rm gcbackuplauncher/data/debugplugin.bin gcbackuplauncher/data/highplugin.bin gcbackuplauncher/data/lowplugin.bin
