set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${LIB_INSTALL_DIR}/avogadro/projectplugins)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro-lib")

### selectionitems
#set(selectionitem_SRCS selectionitem.cpp)
#qt4_automoc(${selectionitem_SRCS})
#add_library(selectionitem MODULE ${selectionitem_SRCS})
#target_link_libraries(selectionitem ${LINK_LIBS})
#install(TARGETS selectionitem DESTINATION ${DESTINATION_DIR})

### smartsitems
#set(smartsitems_SRCS smartsitems.cpp)
#qt4_automoc(${smartsitems_SRCS})
#qt4_wrap_ui(smartsitems_UIS smartsitems.ui)
#add_library(smartsitems MODULE ${smartsitems_SRCS} ${smartsitems_UIS})
#target_link_libraries(smartsitems ${LINK_LIBS})
#install(TARGETS smartsitems DESTINATION ${DESTINATION_DIR})

