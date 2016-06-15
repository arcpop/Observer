set PATH=C:\Program Files (x86)\Windows Kits\10\TOOLS\SDV\bin\engine;%PATH%
set PATH=C:\Program Files (x86)\Windows Kits\10\TOOLS\SDV\bin\engine\engineq;%PATH%
set PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0\\VC\bin\x86_amd64;C:\Program Files (x86)\Microsoft Visual Studio 14.0\\VC\bin;C:\Program Files (x86)\Microsoft Visual Studio 14.0\\common7\ide\;%PATH%
set PATH=%PATH%;C:\Program Files (x86)\Windows Kits\10\\bin\x64
set _CL_=
wlimit /b /r /c /w 3000 /u 3000 /m 2500 slam -no_slamcl  -rerun  -precondition -platform wdm -target X64 -sdvpath "C:\Users\Raffi\documents\visual studio 2015\Projects\Observer\Observer\sdv" -halt_labels -gate RunDispatchFunction -halt_labels -gate RunDispatchFunction -driver -arrays -no_pa 3 -field_pa_version nocollapse -max_fields_nocollapse 100 -sourcedir "..\..\.."  -display_environment  startioroutine.fsm -tune_entry_points "C:\Users\Raffi\documents\visual studio 2015\Projects\Observer\Observer\SDV-map.h">wlimit.txt 2>wlimit.err
