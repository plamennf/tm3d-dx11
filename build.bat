@echo off

premake5.exe vs2022

compile_shaders.bat
msbuild tm3d-dx11.sln -v:m -property:Configuration="Debug"
xcopy /y /d ..\external\lib\*.dll run_tree\
