 VPATH += ./shared

 INCLUDEPATH += ./shared \
                ../../src \
                ../../../wiiuse/src

 LIBS += -L../../src/msvc_2008/Debug -lwiiuse
 LIBS += -L../../../wiiuse/src/msvc_2008/Debug -lwiiusecpp

 HEADERS       = glwidget.h \
                 window.h \
                 qtlogo.h \
                 wiimote.h \
                 extension.h \
                 wiimotedata_share.h \
            shared/qthread_ex.h \
            shared/wiwo.h

 SOURCES       = glwidget.cpp \
                 main.cpp \
                 window.cpp \
                 qtlogo.cpp \
                 wiimote.cpp \
                 extension.cpp \
                 wiimotedata_share.cpp \
            shared/qthread_ex.cpp \
            shared/wiwo.cpp

 QT           += opengl

 # install
 #target.path = ./build_debug
 #sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS example_qt.pro
 #sources.path = $$[QT_INSTALL_EXAMPLES]/opengl/hellogl
 #INSTALLS += target sources
