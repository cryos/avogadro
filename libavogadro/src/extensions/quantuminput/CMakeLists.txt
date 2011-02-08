include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LINK_LIBS avogadro)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

### Input File Extension
set(inputfileextension_SRCS
    inputfileextension.cpp
    inputdialog.cpp
    abinitinputdialog.cpp
    daltoninputdialog.cpp
    gamessukinputdialog.cpp
    gaussianinputdialog.cpp
    molproinputdialog.cpp
    mopacinputdialog.cpp
    nwcheminputdialog.cpp
    qcheminputdialog.cpp)
set(inputfileextension_UIS
  abinitinputdialog.ui
  gaussianinputdialog.ui
  daltoninputdialog.ui
  gamessukinputdialog.ui
  molproinputdialog.ui
  mopacinputdialog.ui
  nwcheminputdialog.ui
  qcheminputdialog.ui)
avogadro_plugin_nogl(inputfileextension 
  "${inputfileextension_SRCS}"
  "${inputfileextension_UIS}"
 )

### gamess
set(gamessextension_SRCS
  inputdialog.cpp
  gamessextension.cpp
  gamessinputdialog.cpp
  gamessinputdata.cpp
  gamesshighlighter.cpp
  gamessefpmatchdialog.cpp)
avogadro_plugin_nogl(gamessextension
  "${gamessextension_SRCS}"
  "gamessinputdialog.ui;gamessefpmatchdialog.ui")
