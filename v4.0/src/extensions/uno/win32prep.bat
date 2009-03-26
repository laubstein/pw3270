@echo off

idlc -C -I. -I"C:\Arquivos de programas\BrOffice.org 3\Basis\sdk\idl" -Osrc src\Ooo3270.idl
regmerge src\Ooo3270.uno.rdb / "C:\Arquivos de programas\BrOffice.org 3\URE\misc\types.rdb"
regmerge src\Ooo3270.uno.rdb /UCR src\Ooo3270.urd
regmerge src\Ooo3270.uno.rdb / src\Ooo3270.urd


mkdir src\inc
cd src\inc
cppumaker -Gc -L -BUCR ..\Ooo3270.uno.rdb

cd ..