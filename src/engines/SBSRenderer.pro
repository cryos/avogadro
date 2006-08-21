TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = SBSRenderer.h
SOURCES       = SBSRenderer.cpp
TARGET        = SBSRenderer
win32:DESTDIR = ./
