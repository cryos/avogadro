# check for lupdate and lrelease: we can't
# do it using qmake as it doesn't have
# QMAKE_LUPDATE and QMAKE_LRELEASE variables :(

# One problem is that FindQt4.cmake will look for these and cache the results
# If users have lrelease from Qt3 (e.g., Debian, Ubuntu)
#  then we will fail.

# First remove these from cache
set(QT_LUPDATE_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)
set(QT_LRELEASE_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)

FIND_PROGRAM(QT_LUPDATE_EXECUTABLE NAMES lupdate-qt4 lupdate PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
  $ENV{QTDIR}/bin
)

if(QT_LUPDATE_EXECUTABLE)
  message(STATUS "Found lupdate: ${QT_LUPDATE_EXECUTABLE}")
else(QT_LUPDATE_EXECUTABLE)
  message(FATAL_ERROR "Could NOT find lupdate")
endif(QT_LUPDATE_EXECUTABLE)

FIND_PROGRAM(QT_LRELEASE_EXECUTABLE NAMES lrelease-qt4 lrelease PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
  $ENV{QTDIR}/bin
)

if(QT_LRELEASE_EXECUTABLE)
  message(STATUS "Found lrelease: ${QT_LRELEASE_EXECUTABLE}")
else(QT_LRELEASE_EXECUTABLE)
  message(FATAL_ERROR "Could NOT find lrelease")
endif(QT_LRELEASE_EXECUTABLE)

mark_as_advanced(QT_LUPDATE_EXECUTABLE QT_LRELEASE_EXECUTABLE)

# QT4_WRAP_TS(outfiles infiles ...)
# outfiles receives .qm generated files from
# .ts files in arguments
# a target lupdate is created for you
# update/generate your translations files
# example: QT4_WRAP_TS(foo_QM ${foo_TS})
MACRO (QT4_WRAP_TS outfiles)
  # a target to manually run lupdate
  #ADD_CUSTOM_TARGET(lupdate
                    #COMMAND ${QT_LUPDATE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} -ts ${ARGN}
                    #WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  #)
  FOREACH (it ${ARGN})
    GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)

    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.qm)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
                       COMMAND ${QT_LRELEASE_EXECUTABLE}
                       ARGS -compress -removeidentical -silent ${it} -qm ${outfile}
                       DEPENDS ${it}
    )

    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)
ENDMACRO (QT4_WRAP_TS)
