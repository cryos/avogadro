######################################################################
# Example QMake project building a 3rd party extension against 
# installed Avogadro library. 
# See http://avogadro.openmolecules.net/ for more information.
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
