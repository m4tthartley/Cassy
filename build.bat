
@echo off

IF NOT EXIST build mkdir build
pushd build

REM dsound.lib
REM dxguid.lib
REM winmm.lib
REM ws2_32.lib

cl -Zi -nologo ../client.cc -Feclient.exe ws2_32.lib dsound.lib dxguid.lib user32.lib
cl -Zi -nologo ../server.cc -Feserver.exe ws2_32.lib

popd