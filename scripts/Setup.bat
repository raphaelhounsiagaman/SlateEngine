@echo off

pushd ..
vendor\Premake\premake5.exe --file=premake5.lua vs2026
popd
pause