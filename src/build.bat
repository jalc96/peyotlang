@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

del *.pdb > NUL 2> NUL
del *.obj > NUL 2> NUL

@echo on
@echo -------------------------------------------------
@echo DEBUG BUILD
@echo -------------------------------------------------
@echo off

set params=-DWIN32=1 -D_WIN32=1 -DDEVELOPMENT=1 -DPERFORMANCEAPI_ENABLED=1
set windows_libs=kernel32.lib libcmtd.lib advapi32.lib
set external_libs=PerformanceAPI_MTd.lib /DEBUG:FULL
set options=-Od /Zi /MTd /Zc:wchar_t
set linker=/link %external_libs% %windows_libs%

cl %options% %params% ../src/peyot.cpp -Fepeyot_debug.exe %linker%|msvc_color_release.exe
rem cl %options% %params% ../src/peyot.cpp -Fepeyot_debug.exe %linker%

popd