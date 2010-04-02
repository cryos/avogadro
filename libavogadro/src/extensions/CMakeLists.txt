ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${Avogadro_PLUGIN_INSTALL_DIR}/extensions)

include_directories(
  ../
  ${CMAKE_CURRENT_BINARY_DIR}
)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS avogadro)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)
add_custom_target(extensions COMMENT "Meta target to build all Avogadro extensions.")

### Template extension - template for extensions
#avogadro_plugin(templateextension templateextension.cpp)

### Properties
avogadro_plugin_nogl(propextension
   "../tools/skeletontree.cpp;propmodel.cpp;propextension.cpp")

avogadro_plugin_nogl(cartesianextension
        "cartesianextension.cpp"
        cartesianeditor.ui
        edit.qrc)

avogadro_plugin_nogl(molecularpropextension
        "molecularpropextension.cpp"
        molecularpropdialog.ui)

### Molecular Mechanics force fields
set(forcefieldextension_SRCS forcefieldextension.cpp forcefielddialog.cpp
  constraintsdialog.cpp constraintsmodel.cpp conformersearchdialog.cpp)
avogadro_plugin_nogl(forcefieldextension
  "${forcefieldextension_SRCS}"
  "forcefielddialog.ui;constraintsdialog.ui;conformersearchdialog.ui")

### hydrogens
avogadro_plugin_nogl(hydrogenextension hydrogensextension.cpp)

### Selections
avogadro_plugin_nogl(selectextension selectextension.cpp)
target_link_libraries(selectextension periodictable)

### unit cells
avogadro_plugin_nogl(unitcellextension
  "unitcellextension.cpp;unitcellparamdialog.cpp"
  unitcellparamdialog.ui)

### insert fragment
avogadro_plugin_nogl(insertfragmentextension
"insertfragmentextension.cpp;insertfragmentdialog.cpp;insertcommand.cpp"
  insertfragmentdialog.ui)

### insert peptide (builder)
avogadro_plugin_nogl(insertpeptideextension
  "insertpeptideextension.cpp;insertcommand.cpp"
  insertpeptidedialog.ui)

### h->methyl
avogadro_plugin_nogl(h2methylextension h2methylextension.cpp)

### Surfaces
set(surfaceextension_SRCS surfaceextension.cpp surfacedialog.cpp
  basisset.cpp gaussianfchk.cpp slaterset.cpp mopacaux.cpp molpro.cpp
  vdwsurface.cpp ../../qtiocompressor/qtiocompressor.cpp)
avogadro_plugin_nogl(surfaceextension "${surfaceextension_SRCS}" surfacedialog.ui)
target_link_libraries(surfaceextension ${LINK_LIBS} ${ZLIB_LIBRARIES})

if(ENABLE_PYTHON AND ALL_PYTHON_FOUND)
  ### python terminal
  message(STATUS "Enabled python terminal")
  avogadro_plugin_nogl(pythonterminal
    "pythonterminal.cpp;highlighter.cpp"
    pythonterminalwidget.ui)

  file(GLOB python_SCRIPTS "python/*.py")
  install(FILES ${python_SCRIPTS} DESTINATION share/libavogadro/extensionScripts)
endif(ENABLE_PYTHON AND ALL_PYTHON_FOUND)

### Animation
set(animationextension_SRCS animationextension.cpp animationdialog.cpp
  povpainter.cpp trajvideomaker.cpp)
avogadro_plugin_nogl(animationextension
  "${animationextension_SRCS}"
  animationdialog.ui
  animation.qrc)

### POV-Ray extension
avogadro_plugin_nogl("povrayextension"
  "povrayextension.cpp;povpainter.cpp;povraydialog.cpp"
  "povraydialog.ui")

### File import extension
avogadro_plugin_nogl(fileimportextension fileimportextension.cpp fileimportdialog.ui)

### Shader extension
if(GLEW_FOUND)
  avogadro_plugin(shaderextension
    shaderextension.cpp
    shaderdialog.ui)
  file(GLOB shader_VERT "shaders/*.vert")
  file(GLOB shader_FRAG "shaders/*.frag")
  file(GLOB shader_PARAMS "shaders/*.params")
  install(FILES ${shader_VERT} ${shader_FRAG} ${shader_PARAMS}
          DESTINATION share/libavogadro/shaders)
endif(GLEW_FOUND)

### super cells
avogadro_plugin_nogl(supercellextension
  "supercellextension.cpp;supercelldialog.cpp"
  supercelldialog.ui)

# Network fetch
avogadro_plugin_nogl(networkfetchextension networkfetchextension.cpp)
target_link_libraries(networkfetchextension
  ${QT_QTNETWORK_LIBRARY}
)

add_subdirectory(spectra)
add_subdirectory(quantuminput)
