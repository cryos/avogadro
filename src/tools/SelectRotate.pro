TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin
INCLUDEPATH  += ../
HEADERS       = SelectRotate.h
SOURCES       = SelectRotate.cpp
TARGET        = SelectRotate
win32:DESTDIR = ./
