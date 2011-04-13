@echo off

REM
REM All of this variables are used either in CMakeLists.txt or in cmake/modules/AvoCPack.cmake
REM


REM SET build_system="NMake Makefiles"
SET build_system="Visual Studio 9 2008"
SET build_type="Release"
SET build_options=-DCMAKE_BUILD_TYPE=%build_type% -G %build_system% 

REM requires NSIS (http://nsis.sourceforge.net)!
SET cpack_options=-DCPACK_BINARY_NSIS:=ON -DCPACK_BINARY_ZIP:=OFF

REM General

SET qt_bin_dir=C:\Qt\4.7.2\bin
SET PATH=%PATH%;%qt_bin_dir%
SET qt_opt=-DQT_BIN_DIR=%qt_bin_dir%

SET openbabel_dir=C:\src\openbabel-2.3.0\
SET openbabel_install_dir=C:\src\openbabel-2.3.0\windows-vc2008\install
SET openbabel_version=2.3.0
SET openbabel_include_dir=%openbabel_install_dir%\include\openbabel-2.0
SET openbabel_lib=%openbabel_install_dir%\bin\openbabel-2.lib
SET openbabel_dll_dir=%openbabel_install_dir%%\bin\
SET ob_opt=-DOPENBABEL2_DIR=%openbabel_dir% -DOPENBABEL2_INCLUDE_DIR=%openbabel_include_dir% -DOPENBABEL2_LIBRARIES=%openbabel_lib% -DOPENBABEL2_DLL_DIR=%openbabel_dll_dir% -DOPENBABEL2_VERSION=%openbabel_version%

REM libxml_dir has "libxml-2.0.pc.in"
SET libxml_dir=C:\src\libxml2-2.7.3
REM libxml_dll_dir has "libxml2.dll"
SET libxml_dll_dir=%libxml_dir%\win32\bin.msvc
SET libxml_opt=-DLIBXML_DIR=%libxml_dir% -DLIBXML_DLL_DIR=%libxml_dll_dir%

SET zlib_dir=C:\src\zlib-1.2.5
SET zlib_lib=%zlib_dir%\old\msvc_2008\Win32_DLL_Release\zlib1.lib
REM where is "zlib1.dll" :
SET zlib_dll_dir=%zlib_dir%\old\msvc_2008\Win32_DLL_Release
SET zlib_opt=-DZLIB_INCLUDE_DIR=%zlib_dir% -DZLIB_LIBRARY=%zlib_lib% -DZLIB_DLL_DIR=%zlib_dll_dir%

SET eigen2_dir=C:\src\eigen-2.0.15
SET eigen_opt=-DEIGEN2_INCLUDE_DIR=%eigen2_dir%


REM GLSL Support
SET glsl_options=-DENABLE_GLSL:=ON

SET glew_dir=C:\src\glew-1.5.8\include
SET glew_lib=C:\src\glew-1.5.8\lib\glew32.lib
SET glew_dll_dir=C:\src\glew-1.5.8\bin
SET glew_opt=-DGLEW_INCLUDE_DIR=%glew_dir% -DGLEW_LIBRARY=%glew_lib% -DGLEW_DLL_DIR=%glew_dll_dir%

REM Python Support

SET python_dir=C:\src\Python-2.6.2
SET python_include=%python_dir%\Include
SET python_lib=%python_dir%\libs\python26.lib
SET python_exe=%python_dir%\libs\python.exe
SET boost_dir=C:\src\boost_1_38_0

REM Run cmake with GLSL & Python support
REM cmake.exe .. -DOPENBABEL2_INCLUDE_DIR=%openbabel_dir%\include -DOPENBABEL2_LIBRARIES=%openbabel_lib% -DEIGEN2_INCLUDE_DIR=%eigen2_dir% -DGLEW_INCLUDE_DIR=%glew_dir% -DGLEW_LIBRARY=%glew_lib% -DENABLE_GLSL:=ON -DPYTHON_INCLUDE_PATH=%python_include% -DPYTHON_LIBRARY=%python_lib% -DPYTHON_EXECUTABLE=%python_exe% -DBOOST_ROOT=%boost_dir% -DCMAKE_BUILD_TYPE=%build_type% -G %build_system% 

REM Run cmake with GLSL (no python)
REM cmake.exe .. -DOPENBABEL2_INSTALL_DIR=%OPENBABEL_INSTALL_DIR% -DOPENBABEL2_INCLUDE_DIR=%openbabel_dir% -DOPENBABEL2_LIBRARIES=%openbabel_lib% -DOPENBABEL_BINDIR=%openbabel_bindir% -DZLIB_INCLUDE_DIR=%zlib_dir% -DZLIB_LIBRARY=%zlib_lib% -DZLIB_DLL=%zlib_dll% -DEIGEN2_INCLUDE_DIR=%eigen2_dir% -DGLEW_INCLUDE_DIR=%glew_dir% -DGLEW_LIBRARY=%glew_lib% -DGLEW_DLL=%glew_dll% -DENABLE_GLSL:=ON -DCMAKE_BUILD_TYPE=%build_type% -G %build_system% 
cmake.exe .. %qt_opt% %ob_opt% %libxml_opt% %zlib_opt% %eigen_opt% %glew_opt% %cpack_options% %glsl_options% %build_options%

REM Run cmake without GLSL & python support
REM cmake.exe .. -DOPENBABEL2_INCLUDE_DIR=%openbabel_dir%\include -DOPENBABEL2_LIBRARIES=%openbabel_lib% -DEIGEN2_INCLUDE_DIR=%eigen2_dir% -DCMAKE_BUILD_TYPE=%build_type% -G %build_system% 
