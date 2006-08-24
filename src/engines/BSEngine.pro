TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = BSEngine.h
SOURCES       = BSEngine.cpp
TARGET        = BSEngine
win32:DESTDIR = ./
