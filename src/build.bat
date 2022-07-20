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

cl -Od /Zi ../src/peyot.cpp -Fepeyot_debug.exe | msvc_color_release.exe

popd