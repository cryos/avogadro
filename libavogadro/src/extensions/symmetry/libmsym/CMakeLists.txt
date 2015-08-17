cmake_minimum_required (VERSION 2.8.11)

project(libmsym C)

set(LIBMSYM_MAJOR_VERSION 0)
set(LIBMSYM_MINOR_VERSION 1)
set(LIBMSYM_PATCH_VERSION 0)
set(LIBMSYM_VERSION ${LIBMSYM_MAJOR_VERSION}.${LIBMSYM_MINOR_VERSION}.${LIBMSYM_PATCH_VERSION})

set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for msym library")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake/libmsym)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

include_directories("${PROJECT_SOURCE_DIR}")

file (GLOB source src/*.c)
add_library (msym ${source})
if (BUILD_SHARED_LIBS)
        set_target_properties (msym PROPERTIES
                POSITION_INDEPENDENT_CODE "TRUE")
endif ()
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
#set_target_properties (msym PROPERTIES
#        C_STANDARD 99
#        C_STANDARD_REQUIRED ON)

set_target_properties(msym PROPERTIES PUBLIC_HEADER "src/msym.h;src/msym_error.h")

export(TARGETS msym FILE "${PROJECT_BINARY_DIR}/libmsymTargets.cmake")

export(PACKAGE libmsym)

file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(libmsymConfig.cmake.in "${PROJECT_BINARY_DIR}/libmsymConfig.cmake" @ONLY)
set(CONF_INCLUDE_DIRS "\${LIBMSYM_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(libmsymConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libmsymConfig.cmake" @ONLY)
configure_file(libmsymConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/libmsymConfigVersion.cmake" @ONLY)

install(FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libmsymConfig.cmake"
    "${PROJECT_BINARY_DIR}/libmsymConfigVersion.cmake"
    DESTINATION "${INSTALL_CMAKE_DIR}")

install(TARGETS msym
        EXPORT libmsymTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/libmsym"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )

install(EXPORT libmsymTargets DESTINATION "${INSTALL_CMAKE_DIR}")
