######################################################################
# This is an example QMake project for building Avogadro extension
######################################################################

TEMPLATE = lib
TARGET = templateextension
DEPENDPATH += .
INCLUDEPATH += .
QT += opengl
CONFIG += qt avogadro

# Input
HEADERS += templateextension.h
SOURCES += templateextension.cpp
