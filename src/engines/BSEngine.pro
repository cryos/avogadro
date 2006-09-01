TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin
INCLUDEPATH  += ../
HEADERS       = BSEngine.h vertexarray.h ../color.h
SOURCES       = BSEngine.cpp vertexarray.cpp ../color.cpp
TARGET        = BSEngine
win32:DESTDIR = ./
