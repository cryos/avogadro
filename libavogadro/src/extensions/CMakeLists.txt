ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${Avogadro_PLUGIN_INSTALL_DIR}/extensions)

include_directories(
  ../
  ${CMAKE_CURRENT_BINARY_DIR}
)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro")

### Template extension - template for extensions
#avogadro_plugin(templateextension templateextension.cpp)

### Input File Extension
set(inputfileextension_SRCS inputfileextension.cpp
#    daltoninputdialog.cpp
    gaussianinputdialog.cpp
    molproinputdialog.cpp
    mopacinputdialog.cpp
    nwcheminputdialog.cpp
    qcheminputdialog.cpp)
avogadro_plugin(inputfileextension "${inputfileextension_SRCS}"
 "gaussianinputdialog.ui;molproinputdialog.ui;mopacinputdialog.ui;nwcheminputdialog.ui;qcheminputdialog.ui")

### gamess
set(gamessextension_SRCS gamessextension.cpp
  gamessinputdialog.cpp
  gamessinputdata.cpp
  gamesshighlighter.cpp
  gamessefpmatchdialog.cpp)
avogadro_plugin(gamessextension
  "${gamessextension_SRCS}"
  "gamessinputdialog.ui;gamessefpmatchdialog.ui")

### Properties
avogadro_plugin(propextension
   "../tools/skeletontree.cpp;propmodel.cpp;propextension.cpp")

avogadro_plugin(molecularpropextension
        "molecularpropextension.cpp"
        molecularpropdialog.ui)

### Molecular Mechanics force fields
set(forcefieldextension_SRCS forcefieldextension.cpp forcefielddialog.cpp
  constraintsdialog.cpp constraintsmodel.cpp conformersearchdialog.cpp)
avogadro_plugin(forcefieldextension
  "${forcefieldextension_SRCS}"
  "forcefielddialog.ui;constraintsdialog.ui;conformersearchdialog.ui")

### hydrogens
avogadro_plugin(hydrogenextension hydrogensextension.cpp)

### Selections
avogadro_plugin(selectextension selectextension.cpp)

### unit cells
avogadro_plugin(unitcellextension
  "unitcellextension.cpp;unitcellparamdialog.cpp"
  unitcellparamdialog.ui)

### insert fragment
avogadro_plugin(insertfragmentextension
"insertfragmentextension.cpp;insertfragmentdialog.cpp;insertcommand.cpp"
  insertfragmentdialog.ui)

### insert peptide (builder)
avogadro_plugin(insertpeptideextension
  "insertpeptideextension.cpp;insertcommand.cpp"
  insertpeptidedialog.ui)

### h->methyl
avogadro_plugin(h2methylextension h2methylextension.cpp)

### Surfaces
set(surfaceextension_SRCS surfaceextension.cpp surfacedialog.cpp
  basisset.cpp gaussianfchk.cpp slaterset.cpp mopacaux.cpp molpro.cpp
  vdwsurface.cpp ../../qtiocompressor/qtiocompressor.cpp)
avogadro_plugin(surfaceextension "${surfaceextension_SRCS}" surfacedialog.ui)
target_link_libraries(surfaceextension ${LINK_LIBS} ${ZLIB_LIBRARIES})

### Vibrations
avogadro_plugin(vibrationextension
  "vibrationextension.cpp;vibrationdialog.cpp"
  vibrationdialog.ui)

### Spectra
avogadro_plugin(spectraextension
  "spectraextension.cpp;spectradialog.cpp;spectratype.cpp;spectratype_ir.cpp;spectratype_nmr.cpp"
  "spectradialog.ui;spectratabir.ui;spectratabnmr.ui")

if(ENABLE_PYTHON AND ALL_PYTHON_FOUND)
  ### python terminal
  message(STATUS "Enabled python terminal")
  avogadro_plugin(pythonterminal
    "pythonterminal.cpp;highlighter.cpp"
    pythonterminalwidget.ui)

  file(GLOB python_SCRIPTS "python/*.py")
  install(FILES ${python_SCRIPTS} DESTINATION share/libavogadro/extensionScripts)
endif(ENABLE_PYTHON AND ALL_PYTHON_FOUND)

### Animation
set(animationextension_SRCS animationextension.cpp animationdialog.cpp
  povpainter.cpp trajvideomaker.cpp)
avogadro_plugin(animationextension
  "${animationextension_SRCS}"
  animationdialog.ui
  animation.qrc)

### POV-Ray extension
avogadro_plugin("povrayextension"
  "povrayextension.cpp;povpainter.cpp;povraydialog.cpp"
  "povraydialog.ui")

### File import extension
avogadro_plugin(fileimportextension fileimportextension.cpp fileimportdialog.ui)

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
avogadro_plugin(supercellextension
  "supercellextension.cpp;supercelldialog.cpp"
  supercelldialog.ui)

avogadro_plugin(networkfetchextension networkfetchextension.cpp)

# Subdirs
add_subdirectory(crystalbuilder)
