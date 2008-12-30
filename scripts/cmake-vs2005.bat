@echo off

cmake.exe .. -DOPENBABEL2_INCLUDE_DIR=C:\openbabel-trunk\include -DOPENBABEL2_LIBRARIES=C:\openbabel-trunk\output\Release\openbabel-2.lib -DEIGEN2_INCLUDE_DIR=C:\eigen2 -DBOOST_ROOT="c:\Program Files\boost\boost_1_36_0" -DNUMPY_INCLUDE_DIR="C:\Python25\Lib\site-packages\numpy\core\include" -DLIBRARY_OUPUT_PATH=c:\avogadro-git\output -DEXECUTABLE_OUPUT_PATH=c:\avogadro-git\output -DENABLE_GLSL:=ON -G "Visual Studio 9 2008"
