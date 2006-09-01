TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin 
INCLUDEPATH  += ../
HEADERS       = SphereEngine.h vertexarray.h ../color.h
SOURCES       = SphereEngine.cpp vertexarray.cpp ../color.cpp
TARGET        = SphereEngine
win32:DESTDIR = ./
