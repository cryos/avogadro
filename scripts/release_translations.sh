#!/bin/sh
echo Creating qmake project file
qmake -project -o avogadro_ts.pro
echo Releasing the translation files
lrelease -verbose avogadro_ts.pro
echo Removing qmake project file
rm avogadro_ts.pro
