set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${LIB_INSTALL_DIR}/avogadro/colors)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro-lib")

### elementcolor
set(elementcolor_SRCS elementcolor.cpp)
qt4_automoc(${elementcolor_SRCS})
add_library(elementcolor MODULE ${elementcolor_SRCS})
target_link_libraries(elementcolor ${LINK_LIBS})
install(TARGETS elementcolor DESTINATION ${DESTINATION_DIR})

### residuecolor
set(residuecolor_SRCS residuecolor.cpp)
qt4_automoc(${residuecolor_SRCS})
add_library(residuecolor MODULE ${residuecolor_SRCS})
target_link_libraries(residuecolor ${LINK_LIBS})
install(TARGETS residuecolor DESTINATION ${DESTINATION_DIR})

