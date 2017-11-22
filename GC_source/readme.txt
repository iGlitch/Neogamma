GC Backup Launcher for Wii by WiiGator
######################################

Package name: GCBackup

Rules
#####
1. The software is free of charge.
2. If you redistribute this package, you must include the source code.
3. It is forbidden to pack the files with tools, where the source code is not
available (e.g. winrar, winzip or winace).

Disclaimer
##########
This package include modifications of other programs, and is unsupported and
not condoned by the original authors of it. The backup loader modification is
solely the work of WiiGator.

What it does
############

You can start GameCube backups on Wii. There is no hardware change required.
This is only software. There is only DVD-R directly supported. DVD+R need to
be patched to booktype DVD-ROM.

Compatibilty
############

Some games are not working. Games with audio streaming are working. You can't
hear the audio stream (but the game sound). Some games can't be started, because
the audio is stopped to fast and the game goes back to the previous menu (e.g. Ikaruga).
Memory saves are now stored with the correct disc id.
Some games like Zelda Collection can't be started using Backup Launcher 0.3 Gamma.
You need to use Reloader or rungcbackup.

How to get this run
###################

CHECK THE TGZ FILE IN THE DIRECTORY, WHERE YOU FOUND THIS FILE.

How it works
############

MIOS is patched to allow starting of homebrew. It is also patched to leave
DVD-R support enabled. The rungcbackup copies the gcbackuplauncher to memory
and changes to GameCube mode. gcbackuplauncher patches the DVD read functions,
so reading is possible.
There is now a 2 stage loader included for MIOS. The MIOS plugin is located at
0x80f80000 and the low game plugin is located at 0x80001800 (button B). The high
game plugin is located at the end of the memory (button A). This is for easier
debugging, because the MIOS plugin and the high game plugin can be much larger.
The high game plugin is only used when AR is started.

System requirements:
####################

IOS249 with backup support need to be installed (e.g. my cIOS r7 from
backuplauncher 0.3 gamma).

Instructions:
#############

1. Install MIOS by running miospatcher.
2. Put the GameCube backup into the tray.
3. Run rungcbackup
4. Press A on the GameCube controller until game starts.

Note 1: Changing to GameCube mode is sometimes not working. Turn off Wii and
	on again and retry, if screen looks strange.
Note 2: Starting with patched MIOS enables MIOS to load the game. Some games
	are not working with this method. Other games will only work with this
	method.
Note 3: DVD speed is still limited to 3x.
Note 4: There are sometimes problems with the DVD read speed. So there is
	sometimes a very short delay in a few games. The game play is only barely
	influenced.
Note 5: If patching of DVD read function fails, the game will still be started.
	If the game accesses the DVD, there will be an error.
Note 6: On read error there will be 30 read retries. I don't know if this
	works, because I don't have such read problems with my DVD-R brand.
Note 7: If you eject the disc, the next inserted disc can be detected, but it
	is not always working.
Note 8: -
Note 9: Multigame discs can be created with "GCOS MultiGame Creator V4F.exe".
Note 10: It supports DVD9 offsets.
Note 11: Dual Layer DVDs should work. I don't have tested it.
Note 12: When you have both discs of a game burned within one multigame ISO
	image, you can switch the disc by ejecting and reinserting the disc. But
	you need first to select the second disc with button Z on the startup
	screen (green GC logo). This is only working if you started the game with
	the B button.
Note 13: The video mode is not patched if you press B.

Details:
########

dolloader - not changed
dvdtest - test program for reading DVDs in GameCube mode.
rundvdtest - A program for fast starting of dvdtest program.
gcplugin - Plugin for games and MIOS. DVD functions are replaced by the functions included.
gcbackuplauncher - GameCube mode loader.
rungcbackup - Switches to IOS249 and start gcbackuplauncher which is linked into the DOL.
miospatcher - Enable backup access in MIOS and include homebrew support. This downloads files
	from the internet, it doesn't use a WAD file. You can include the gcbackuplauncher into MIOS,
	if you enable Wii Backup Launcher support.

Note: To get debug messages on the serial console, you need USB Gecko and set "export GEKKO_DEBUG=yes" when compiling.
Note: To get debug messages for low plugin on the serial console, you need also to set "export GAME_DEBUG=yes" when compiling.
Note: To change slot of USB Gecko set "export GEKKO_CHANNEL=0".
Note: To compile MIOS version of gcbackuplauncher set before "export MIOS_BACKUP=yes"
Note: When you build, you need to enable Reloader "export RELOAD_SUPPORT=yes", if you don't use "make release".

There are dependencies, so use the root Makefile for build:

make release
