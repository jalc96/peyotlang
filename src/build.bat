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

set params=-DWIN32=1 -DDEVELOPMENT=1
set windows_libs=kernel32.lib

cl -diagnostics:column -Od /Zi %params% ../src/peyot.cpp -Fepeyot_debug.exe /link %windows_libs%| msvc_color_release.exe

popd