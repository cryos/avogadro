TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = BSRenderer.h
SOURCES       = BSRenderer.cpp
TARGET        = BSRenderer
win32:DESTDIR = ./
