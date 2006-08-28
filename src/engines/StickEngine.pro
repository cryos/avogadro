TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = StickEngine.h vertexarray.h
SOURCES       = StickEngine.cpp vertexarray.cpp
TARGET        = StickEngine
win32:DESTDIR = ./
