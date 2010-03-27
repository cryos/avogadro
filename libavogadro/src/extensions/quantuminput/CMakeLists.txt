include_directories(
  ../
  ${CMAKE_CURRENT_BINARY_DIR}
)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS avogadro)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

# These are two independent extensions, but they may be merged in future

### Input File Extension
set(inputfileextension_SRCS inputfileextension.cpp
    daltoninputdialog.cpp
    gaussianinputdialog.cpp
    molproinputdialog.cpp
    mopacinputdialog.cpp
    nwcheminputdialog.cpp
    qcheminputdialog.cpp)
avogadro_plugin(inputfileextension "${inputfileextension_SRCS}"
 "gaussianinputdialog.ui;daltoninputdialog.ui;molproinputdialog.ui;mopacinputdialog.ui;nwcheminputdialog.ui;qcheminputdialog.ui")

### gamess
set(gamessextension_SRCS gamessextension.cpp
  gamessinputdialog.cpp
  gamessinputdata.cpp
  gamesshighlighter.cpp
  gamessefpmatchdialog.cpp)
avogadro_plugin(gamessextension
  "${gamessextension_SRCS}"
  "gamessinputdialog.ui;gamessefpmatchdialog.ui")
