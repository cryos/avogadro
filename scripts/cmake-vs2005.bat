@echo off

cmake.exe .. -DOPENBABEL2_INCLUDE_DIR=C:\openbabel\include -DOPENBABEL2_LIBRARIES=C:\openbabel\windows-vc2005\OpenBabelDLL\OpenBabel.dll -DEIGEN_INCLUDE_DIR=C:\eigen\include -G "Visual Studio 8 2005"
