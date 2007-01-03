set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

include_directories(${OPENBABEL_INCLUDE_DIR} ../)

link_directories(../)

# tell cmake to create .moc files for all files in the variable qtproject_SRCS that require such a file.
# note: this assumes that you use #include "header.moc" in your files
# also note that you don't actually require kde to use this command

set(bsengine_SRCS 
	bsengine.cpp
	vertexarray.cpp
)

qt4_automoc(${bsengine_SRCS})

ADD_LIBRARY(bsengine SHARED ${bsengine_SRCS})
TARGET_LINK_LIBRARIES(bsengine avogadro)

# ADD_LIBRARY(avogadroStatic STATIC ${libavogadro_SRCS})
# ADD_LIBRARY(avogadroShared SHARED ${libavogadro_SRCS})
# SET_TARGET_PROPERTIES(avogadroStatic avogadroShared PROPERTIES OUTPUT_NAME avogadro)

# install(TARGETS avogadroShared DESTINATION lib)
# install(TARGETS avogadroShared DESTINATION lib)
INSTALL(TARGETS bsengine DESTINATION lib/avogadro)
