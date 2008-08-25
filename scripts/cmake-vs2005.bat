@echo off

cmake.exe .. -DOPENBABEL2_INCLUDE_DIR=C:\openbabel-SVN\include -DOPENBABEL2_LIBRARIES=C:\openbabel-SVN\windows-vc2005\obdll.dll -DEIGEN2_INCLUDE_DIR=C:\eigen\include -G "Visual Studio 8 2005"
