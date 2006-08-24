TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = SBSEngine.h GLEngine.h
SOURCES       = SBSEngine.cpp
TARGET        = SBSEngine
win32:DESTDIR = ./
