TEMPLATE      = lib

QT += opengl
CONFIG += link_pkgconfig 
PKGCONFIG += openbabel-2.0

CONFIG       += plugin
INCLUDEPATH  += ../
HEADERS       = ResidueSphereEngine.h vertexarray.h
SOURCES       = ResidueSphereEngine.cpp vertexarray.cpp
TARGET        = ResidueSphereEngine
win32:DESTDIR = ./
