TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = StickEngine.h vertexarray.h ../color.h
SOURCES       = StickEngine.cpp vertexarray.cpp ../color.cpp
TARGET        = StickEngine
win32:DESTDIR = ./
