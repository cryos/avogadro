set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${LIB_INSTALL_DIR}/avogadro/extensions)

include_directories(
  ../
  ${CMAKE_CURRENT_BINARY_DIR}
  ${OPENBABEL2_INCLUDE_DIR}
  ${EIGEN2_INCLUDE_DIR}
  ${AVOGADRO_INCLUDE_DIR}
  ${BOOST_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_PATH}
)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro-lib")

### Template extension - template for extensions
#set(templateextension_SRCS templateextension.cpp)
#qt4_automoc(${templateextension_SRCS})
#add_library(templateextension MODULE ${templateextension_SRCS})
#target_link_libraries(templateextension ${LINK_LIBS})
#install(TARGETS templateextension DESTINATION ${DESTINATION_DIR})

### gamess
set(gamessextension_SRCS gamessextension.cpp
  gamessinputdialog.cpp
  gamessinputdata.cpp
  gamessefpmatchdialog.cpp
)
qt4_wrap_ui(gamessextension_UIS_H gamessinputdialog.ui gamessefpmatchdialog.ui)
qt4_automoc(${gamessextension_SRCS})
add_library(gamessextension MODULE ${gamessextension_SRCS} ${gamessextension_UIS_H})
target_link_libraries(gamessextension ${LINK_LIBS})
install(TARGETS gamessextension DESTINATION ${DESTINATION_DIR})

### Properties
set(propextension_SRCS propmodel.cpp propextension.cpp)
qt4_automoc(${propextension_SRCS})
add_library(propextension MODULE ${propextension_SRCS} ${propextension_UIS_H})
target_link_libraries(propextension ${LINK_LIBS})
install(TARGETS propextension DESTINATION ${DESTINATION_DIR})

### Molecular Mechanics force fields
set(forcefieldextension_SRCS forcefieldextension.cpp forcefielddialog.cpp
  constraintsdialog.cpp constraintsmodel.cpp conformersearchdialog.cpp)
qt4_wrap_ui(forcefieldextension_UIS_H forcefielddialog.ui constraintsdialog.ui
  conformersearchdialog.ui)
qt4_automoc(${forcefieldextension_SRCS})
add_library(forcefieldextension MODULE ${forcefieldextension_SRCS}
  ${forcefieldextension_UIS_H})
target_link_libraries(forcefieldextension ${LINK_LIBS})
install(TARGETS forcefieldextension DESTINATION ${DESTINATION_DIR})

### hydrogens
set(hydrogensextension_SRCS hydrogensextension.cpp)
qt4_automoc(${hydrogensextension_SRCS})
add_library(hydrogensextension MODULE ${hydrogensextension_SRCS})
target_link_libraries(hydrogensextension ${LINK_LIBS})
install(TARGETS hydrogensextension DESTINATION ${DESTINATION_DIR})

### Selections
set(selectextension_SRCS selectextension.cpp)
qt4_automoc(${selectextension_SRCS})
add_library(selectextension MODULE ${selectextension_SRCS})
target_link_libraries(selectextension ${LINK_LIBS})
install(TARGETS selectextension DESTINATION ${DESTINATION_DIR})

### unit cells
#set(unitcellextension_SRCS unitcellextension.cpp unitcellparamdialog.cpp)
#qt4_wrap_ui(unitcellextension_UIS_H unitcellparamdialog.ui)
#qt4_automoc(${unitcellextension_SRCS})
#add_library(unitcellextension MODULE ${unitcellextension_SRCS} ${unitcellextension_UIS_H})
#target_link_libraries(unitcellextension ${LINK_LIBS})
#install(TARGETS unitcellextension DESTINATION ${DESTINATION_DIR})

### h->methyl
set(h2methylextension_SRCS h2methylextension.cpp)
qt4_automoc(${h2methylextension_SRCS})
add_library(h2methylextension MODULE ${h2methylextension_SRCS})
target_link_libraries(h2methylextension ${LINK_LIBS})
install(TARGETS h2methylextension DESTINATION ${DESTINATION_DIR})

### WiiTrack
#set(wiitrackextension_SRCS wiitrackextension.cpp)
#set(CWIID_LIBRARIES /usr/lib/libcwiid.so)
#qt4_automoc(${wiitrackextension_SRCS})
#ADD_LIBRARY(wiitrackextension SHARED ${wiitrackextension_SRCS})
#TARGET_LINK_LIBRARIES(wiitrackextension
#	${OPENBABEL2_LIBRARIES}
#	${QT_LIBRARIES}
#   	${OPENGL_LIBRARIES}
#        ${CWIID_LIBRARIES}
#	avogadro-lib
#)
#INSTALL(TARGETS wiitrackextension DESTINATION ${DESTINATION_DIR})

### Orbitals
set(orbitalextension_SRCS orbitalextension.cpp orbitaldialog.cpp
    basisset.cpp gaussianfchk.cpp slaterset.cpp mopacaux.cpp vdwsurface.cpp)
qt4_wrap_ui(orbitalextension_UIS_H orbitaldialog.ui)
qt4_automoc(${orbitalextension_SRCS})
add_library(orbitalextension MODULE ${orbitalextension_SRCS} ${orbitalextension_UIS_H})
target_link_libraries(orbitalextension ${LINK_LIBS})
install(TARGETS orbitalextension DESTINATION ${DESTINATION_DIR})

### Vibrations
set(vibrationextension_SRCS vibrationextension.cpp vibrationdialog.cpp)
qt4_wrap_ui(vibrationextension_UIS_H vibrationdialog.ui)
qt4_automoc(${vibrationextension_SRCS})
add_library(vibrationextension MODULE ${vibrationextension_SRCS} ${vibrationextension_UIS_H})
target_link_libraries(vibrationextension ${LINK_LIBS})
install(TARGETS vibrationextension DESTINATION ${DESTINATION_DIR})

### Gaussian input deck generator
set(gaussianextension_SRCS gaussianextension.cpp
  gaussianinputdialog.cpp
  qcheminputdialog.cpp)
qt4_wrap_ui(gaussianextension_UIS_H
  gaussianinputdialog.ui
  qcheminputdialog.ui)
qt4_automoc(${gaussianextension_SRCS})
add_library(gaussianextension MODULE ${gaussianextension_SRCS}
  ${gaussianextension_UIS_H})
target_link_libraries(gaussianextension ${LINK_LIBS})
install(TARGETS gaussianextension DESTINATION ${DESTINATION_DIR})

if(ALL_PYTHON_FOUND)
  ### python terminal
  message(STATUS "Enabled python terminal")
  set(pythonterminal_SRCS pythonterminal.cpp highlighter.cpp)
  qt4_wrap_ui(pythonterminal_UIS_H pythonterminalwidget.ui)
  qt4_automoc(${pythonterminal_SRCS})
  add_library(pythonterminal MODULE ${pythonterminal_SRCS}
    ${pythonterminal_UIS_H})
  target_link_libraries(pythonterminal ${LINK_LIBS})
  install(TARGETS pythonterminal DESTINATION ${DESTINATION_DIR})


  file(GLOB python_SCRIPTS "python/*.py")
  install(FILES ${python_SCRIPTS} DESTINATION share/libavogadro/scripts)
endif(ALL_PYTHON_FOUND)

### Animation
set(animationextension_SRCS animationextension.cpp animationdialog.cpp
    povpainter.cpp trajvideomaker.cpp )
qt4_add_resources(animationextension_RC_SRCS animation.qrc)
qt4_wrap_ui(animationextension_UIS_H animationdialog.ui)
qt4_automoc(${animationextension_SRCS})
add_library(animationextension MODULE ${animationextension_SRCS}
	${animationextension_UIS_H} ${animationextension_RC_SRCS} )
target_link_libraries(animationextension ${LINK_LIBS})
install(TARGETS animationextension DESTINATION ${DESTINATION_DIR})

### Lin Morph Animation
#set(linmorphextension_SRCS linmorphextension.cpp linmorphdialog.cpp trajvideomaker.cpp)
#qt4_add_resources(linmorphextension_RC_SRCS linmorph.qrc)
#qt4_wrap_ui(linmorphdialog_UIS_H linmorphdialog.ui)
#qt4_automoc(${linmorphextension_SRCS})
#add_library(linmorphextension MODULE ${linmorphextension_SRCS}
#  ${linmorphdialog_UIS_H} ${linmorphextension_RC_SRCS} )
#target_link_libraries(linmorphextension ${LINK_LIBS})
#install(TARGETS linmorphextension DESTINATION ${DESTINATION_DIR})

### POV-Ray extension
set(povrayextension_SRCS povrayextension.cpp povpainter.cpp povraydialog.cpp)
qt4_wrap_ui(povrayextension_UIS_H povraydialog.ui)
qt4_automoc(${povrayextension_SRCS})
add_library(povrayextension MODULE ${povrayextension_SRCS} ${povrayextension_UIS_H})
target_link_libraries(povrayextension ${LINK_LIBS})
install(TARGETS povrayextension DESTINATION ${DESTINATION_DIR})

### File import extension
set(fileimportextension_SRCS fileimportextension.cpp)
qt4_wrap_ui(fileimportextension_UIS_H fileimportdialog.ui)
qt4_automoc(${fileimportextension_SRCS})
add_library(fileimportextension MODULE ${fileimportextension_SRCS}
  ${fileimportextension_UIS_H})
target_link_libraries(fileimportextension ${LINK_LIBS})
install(TARGETS fileimportextension DESTINATION ${DESTINATION_DIR})

### Shader extension
if(GLEW_FOUND)
  set(shaderextension_SRCS shaderextension.cpp)
  qt4_wrap_ui(shaderextension_UIS_H shaderdialog.ui)
  qt4_automoc(${shaderextension_SRCS})
  add_library(shaderextension MODULE ${shaderextension_SRCS}
              ${shaderextension_UIS_H})
  target_link_libraries(shaderextension ${LINK_LIBS})
  install(TARGETS shaderextension DESTINATION ${DESTINATION_DIR})
  file(GLOB shader_VERT "shaders/*.vert")
  file(GLOB shader_FRAG "shaders/*.frag")
  file(GLOB shader_PARAMS "shaders/*.params")
  install(FILES ${shader_VERT} ${shader_FRAG} ${shader_PARAMS}
          DESTINATION share/libavogadro/shaders)
endif(GLEW_FOUND)

