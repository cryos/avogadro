# check for lupdate and lrelease: we can't
# do it using qmake as it doesn't have
# QMAKE_LUPDATE and QMAKE_LRELEASE variables :(
#
#  I18N_LANGUAGE - if not empty, wraps only chosen language
#

# One problem is that FindQt4.cmake will look for these and cache the results
# If users have lrelease from Qt3 (e.g., Debian, Ubuntu)
#  then we will fail.

# First remove these from cache
set(QT_LUPDATE_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)
set(QT_LRELEASE_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)
set(QT_LCONVERT_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)

FIND_PROGRAM(QT_LUPDATE_EXECUTABLE NAMES lupdate-qt4 lupdate PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
  $ENV{QTDIR}/bin
)

if(QT_LUPDATE_EXECUTABLE)
  message(STATUS "Found lupdate: ${QT_LUPDATE_EXECUTABLE}")
else(QT_LUPDATE_EXECUTABLE)
  if(Linguist_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find lupdate")
  endif(Linguist_FIND_REQUIRED)
endif(QT_LUPDATE_EXECUTABLE)

FIND_PROGRAM(QT_LRELEASE_EXECUTABLE NAMES lrelease-qt4 lrelease PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
  $ENV{QTDIR}/bin
)

if(QT_LRELEASE_EXECUTABLE)
  message(STATUS "Found lrelease: ${QT_LRELEASE_EXECUTABLE}")
else(QT_LRELEASE_EXECUTABLE)
  if(Linguist_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find lrelease")
  endif(Linguist_FIND_REQUIRED)
endif(QT_LRELEASE_EXECUTABLE)

FIND_PROGRAM(QT_LCONVERT_EXECUTABLE NAMES lconvert-qt4 lconvert PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
  $ENV{QTDIR}/bin
)

if(QT_LCONVERT_EXECUTABLE)
  message(STATUS "Found lconvert: ${QT_LCONVERT_EXECUTABLE}")
else(QT_LCONVERT_EXECUTABLE)
  if(Linguist_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find lconvert")
  endif(Linguist_FIND_REQUIRED)
endif(QT_LCONVERT_EXECUTABLE)

mark_as_advanced(QT_LUPDATE_EXECUTABLE QT_LRELEASE_EXECUTABLE QT_LCONVERT_EXECUTABLE)

if(QT_LUPDATE_EXECUTABLE AND QT_LRELEASE_EXECUTABLE AND QT_LCONVERT_EXECUTABLE)
  set(Linguist_FOUND TRUE)

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

# QT_WRAP_PO(outfiles infiles ...)
# outfiles receives .qm generated files from
# .po files in arguments
# example: QT4_WRAP_PO(foo_TS ${foo_PO})
MACRO (QT4_WRAP_PO outfiles)
   FOREACH (it ${ARGN})
      GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
      # PO files are foo-en_GB.po not foo_en_GB.po like Qt expects
      GET_FILENAME_COMPONENT(fileWithDash ${it} NAME_WE)
      if(NOT I18N_LANGUAGE)
        set(do_wrap ON)
      else(NOT I18N_LANGUAGE)
        string(REGEX MATCH "${I18N_LANGUAGE}" ln ${fileWithDash})
        if(ln)
          set(do_wrap ON)
        else(ln)
          set(do_wrap OFF)
        endif(ln)
      endif(NOT I18N_LANGUAGE)      
      if(do_wrap)
        STRING(REPLACE "-" "_" filenameBase "${fileWithDash}")
        SET(tsfile ${CMAKE_CURRENT_BINARY_DIR}/${filenameBase}.ts)
        SET(qmfile ${CMAKE_CURRENT_BINARY_DIR}/${filenameBase}.qm)

        if (NOT EXISTS "${it}")
           GET_FILENAME_COMPONENT(path ${it} PATH)
           STRING(REGEX MATCH "[^-]+$" lang "${fileWithDash}")
           set (it "${path}/${lang}.po")
        endif (NOT EXISTS "${it}")

        # lconvert from PO to TS and then run lupdate to generate the correct strings
        # finally run lrelease as used above
        ADD_CUSTOM_COMMAND(OUTPUT ${qmfile}
                         COMMAND ${QT_LCONVERT_EXECUTABLE}
                         ARGS -i ${it} -o ${tsfile}
                         COMMAND ${QT_LUPDATE_EXECUTABLE}
                         ARGS ${CMAKE_CURRENT_SOURCE_DIR} -silent -noobsolete -ts ${tsfile}
                         COMMAND ${QT_LRELEASE_EXECUTABLE}
                         ARGS -compress -removeidentical -silent ${tsfile} -qm ${qmfile}
                         DEPENDS ${it}
                         )

        SET(${outfiles} ${${outfiles}} ${qmfile})
      endif(do_wrap)
   ENDFOREACH (it)
ENDMACRO (QT4_WRAP_PO)

else(QT_LUPDATE_EXECUTABLE AND QT_LRELEASE_EXECUTABLE AND QT_LCONVERT_EXECUTABLE)
  set(Linguist_FOUND FALSE)
endif(QT_LUPDATE_EXECUTABLE AND QT_LRELEASE_EXECUTABLE AND QT_LCONVERT_EXECUTABLE)
