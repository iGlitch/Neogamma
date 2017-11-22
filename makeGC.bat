cd GC_source
make clean
make
dollz3 gcbackuplauncher\gcbackuplauncher.dol gcbackuplauncher\gcbackuplauncherzib.dol -m
cd..
copy GC_source\gcbackuplauncher\gcbackuplauncherzib.dol data\gcbackuplauncher.dol /Y
pause