TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = WireframeEngine.h
SOURCES       = WireframeEngine.cpp
TARGET        = WireframeEngine
win32:DESTDIR = ./
