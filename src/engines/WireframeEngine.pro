TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin
INCLUDEPATH  += ../
HEADERS       = WireframeEngine.h vertexarray.h ../color.h
SOURCES       = WireframeEngine.cpp vertexarray.cpp ../color.cpp
TARGET        = WireframeEngine
win32:DESTDIR = ./
