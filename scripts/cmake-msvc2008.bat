@echo off

SET build_system="NMake Makefiles"
SET build_type="Release"

REM requires NSIS (http://nsis.sourceforge.net)!
SET cpack_options=-DCPACK_BINARY_NSIS:=ON -DCPACK_BINARY_ZIP:=OFF

REM General

SET PATH=%PATH%;C:\src\qt-4.5.1\bin
SET openbabel_dir=C:\src\openbabel-2.2.1
SET openbabel_lib=%openbabel_dir%\output\Release\openbabel-2.lib
SET eigen2_dir=C:\src\eigen-2.0.1

REM GLSL Support

SET glew_dir=C:\src\glew\include
SET glew_lib=C:\src\glew\lib\glew32.lib

REM Python Support

SET python_dir=C:\src\Python-2.6.2
SET python_include=%python_dir%\Include
SET python_lib=%python_dir%\libs\python26.lib
SET python_exe=%python_dir%\libs\python.exe
SET boost_dir=C:\src\boost_1_38_0

REM Run cmake with GLSL & Python support
cmake.exe .. -DOPENBABEL2_INCLUDE_DIR=%openbabel_dir%\include -DOPENBABEL2_LIBRARIES=%openbabel_lib% -DEIGEN2_INCLUDE_DIR=%eigen2_dir% -DGLEW_INCLUDE_DIR=%glew_dir% -DGLEW_LIBRARY=%glew_lib% -DENABLE_GLSL:=ON -DPYTHON_INCLUDE_PATH=%python_include% -DPYTHON_LIBRARY=%python_lib% -DPYTHON_EXECUTABLE=%python_exe% -DBOOST_ROOT=%boost_dir% -DCMAKE_BUILD_TYPE=%build_type% -G %build_system% 


REM Run cmake with GLSL (no python)
REM cmake.exe .. -DOPENBABEL2_INCLUDE_DIR=%openbabel_dir%\include -DOPENBABEL2_LIBRARIES=%openbabel_lib% -DEIGEN2_INCLUDE_DIR=%eigen2_dir% -DGLEW_INCLUDE_DIR=%glew_dir% -DGLEW_LIBRARY=%glew_lib% -DENABLE_GLSL:=ON -DCMAKE_BUILD_TYPE=%build_type% -G %build_system% 

REM Run cmake without GLSL & python support
REM cmake.exe .. -DOPENBABEL2_INCLUDE_DIR=%openbabel_dir%\include -DOPENBABEL2_LIBRARIES=%openbabel_lib% -DEIGEN2_INCLUDE_DIR=%eigen2_dir% -DCMAKE_BUILD_TYPE=%build_type% -G %build_system% 
