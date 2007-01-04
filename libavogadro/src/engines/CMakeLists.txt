set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

include_directories(${OPENBABEL_INCLUDE_DIR} ../)

link_directories(../)

set(bsengine_SRCS 
	bsengine.cpp
)

qt4_automoc(${bsengine_SRCS})

ADD_LIBRARY(bsengine SHARED ${bsengine_SRCS})
TARGET_LINK_LIBRARIES(bsengine avogadro)

INSTALL(TARGETS bsengine DESTINATION lib/avogadro)
