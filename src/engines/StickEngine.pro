TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = StickEngine.h GLEngine.h
SOURCES       = StickEngine.cpp
TARGET        = StickEngine
win32:DESTDIR = ./
