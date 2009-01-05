project(avogadro) # the name of your project

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

cmake_minimum_required(VERSION 2.6.0)
cmake_policy(SET CMP0003 NEW)

# this is adapted from KDE's FindKDE4Internal.cmake : default the build type to
# "release with debug info".
#
# We will define two other build types: Debug and Release.
# These names are case-insensitive i.e. you can do -DCMAKE_BUILD_TYPE=debug
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)

message(STATUS "The build type is ${CMAKE_BUILD_TYPE}")

include (CheckCXXCompilerFlag)
include (MacroEnsureVersion)
include (TestIfWeNeedFPermissive)

# use mulithreaded dll runtime on windows when using MSVC
if (MSVC)
  set(CMAKE_CXX_FLAGS "/MD ${CMAKE_CXX_FLAGS}")
endif(MSVC)

###########################################################
# Here I am adding GCC magic from KDE. With these settings#
# compiler issues are fixed and more warnings are issued. #
###########################################################
# We should always respect the users CXXFLAGS if they have set them
if (CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wformat-security -fno-check-new -fno-common -pedantic")
  add_definitions (-D_BSD_SOURCE)
  # Set some linker flags
  if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
    set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-new-dtags ${CMAKE_SHARED_LINKER_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags ${CMAKE_EXE_LINKER_FLAGS}")
  endif (CMAKE_SYSTEM_NAME MATCHES Linux)
  # Now check if we can use visibility to selectively export symbols
  # Get the GCC version - from KDE4 cmake files
  exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE	_gcc_version_info)
  string (REGEX MATCH "[345]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
  # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the
  # patch level, handle this here:
  if (NOT _gcc_version)
    string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
  endif (NOT _gcc_version)
  check_cxx_compiler_flag(-fvisibility=hidden HAVE_GCC_VISIBILITY)
  set(HAVE_GCC_VISIBILITY ${HAVE_GCC_VISIBILITY} CACHE BOOL "GCC support for hidden visibility")
  macro_ensure_version("4.1.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_1)
  macro_ensure_version("4.2.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_2)
  set(_GCC_COMPILED_WITH_BAD_ALLOCATOR FALSE)
  if (GCC_IS_NEWER_THAN_4_1)
    exec_program(${CMAKE_C_COMPILER} ARGS -v OUTPUT_VARIABLE _gcc_alloc_info)
    string(REGEX MATCH "(--enable-libstdcxx-allocator=mt)" _GCC_COMPILED_WITH_BAD_ALLOCATOR "${_gcc_alloc_info}")
  endif (GCC_IS_NEWER_THAN_4_1)

  if (HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
    # We have all the parts necessary - use visibility support
    add_definitions(-DHAVE_GCC_VISIBILITY)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    if (GCC_IS_NEWER_THAN_4_2)
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
    endif (GCC_IS_NEWER_THAN_4_2)
  endif (HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)

  # Set up additional build flags for particular build types. These will be added to CMAKE_CXX_FLAGS,
  # they will not replace them.
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -DQT_NO_DEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DQT_NO_DEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG   "-g3 -fno-inline")

  TEST_IF_WE_NEED_FPERMISSIVE(NEED_FPERMISSIVE)
  if(NEED_FPERMISSIVE)
    #OK, adding -fpermissive to the C++ compiler flags
    #Workaround for a problem with Boost.Python and GCC >= 4.3.1
    #Further information: cmake/modules/TestIfWeNeedFPermissive.cmake
    set (CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fpermissive")
  endif(NEED_FPERMISSIVE)

  # define WITH_SSE2 to enable the SSE2 instruction set.
  # Available on Pentium 4, Athlon 64, and newer CPUs.
  # Enabled by default on the x86_64 architecture, but not on the x86 architecture.
  # If SSE2 is enabled, Eigen uses it by default; this can be disabled by
  # definining EIGEN_DONT_VECTORIZE.
  if(WITH_SSE2)
    message(STATUS "Enabling the SSE2 instruction set")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
  endif(WITH_SSE2)
else(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -DQT_NO_DEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-DQT_NO_DEBUG")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(QT_MIN_VERSION "4.4.0") # we need at least version 4.3 of Qt now
find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
find_package(OpenGL REQUIRED) # find and setup OpenGL
find_package(OpenBabel2 REQUIRED) # find and setup OpenBabel
find_package(Linguist REQUIRED) # find and setup Linguist
find_package(Eigen2 REQUIRED) # find and setup Eigen2
include_directories(${EIGEN2_INCLUDE_DIR})

option(ENABLE_PYTHON "Enable python support" ON)

if(ENABLE_PYTHON MATCHES ON)
  # Cmake's new FindBoost has an option to look for additional versions
  # e.g., there's now a 1.37.0 (and thus 1.37) release.
  SET(Boost_ADDITIONAL_VERSIONS "1.37" "1.37.0")
  find_package(Boost COMPONENTS python)
  if(Boost_PYTHON_FOUND)
    message(STATUS "Boost Python found - Python support enabled.")
    find_package(PythonLibs)
    find_package(PythonInterp)
    find_package(Numpy)
  endif(Boost_PYTHON_FOUND)
else(ENABLE_PYTHON MATCHES ON)
  message(STATUS "Python support disabled.")
endif(ENABLE_PYTHON MATCHES ON)

# Unit testing, mainly useful for developers and so defaults to off
option(ENABLE_TESTS "Enable unit testing" OFF)
if(ENABLE_TESTS MATCHES ON)
  set(QT_USE_QTTEST TRUE)
  enable_testing()
  message(STATUS "Unit testing enabled, run make test to run them")
endif(ENABLE_TESTS MATCHES ON)

# GLSL support is needed for OpenGL shaders
option(ENABLE_GLSL "Enable GLSL support" OFF)
if(ENABLE_GLSL MATCHES ON)
  find_package(GLEW)
  if(GLEW_FOUND)
    message(STATUS "GLEW found and GLSL support enabled")
    add_definitions( -DENABLE_GLSL )
  else(GLEW_FOUND)
    message(WARNING "GLEW not found, please install it if you require GLSL")
  endif(GLEW_FOUND)
endif(ENABLE_GLSL MATCHES ON)

# Multithreaded OpenGL. Seems mostly broken on Linux right now.
option(ENABLE_THREADEDGL "Enable threaded OpenGL rendering" OFF)
if(ENABLE_THREADEDGL MATCHES ON)
  add_definitions( -DENABLE_THREADED_GL )
  set(THREADED_GL true)
  message(STATUS "Threaded OpenGL rendering enabled - needs more testing")
else(ENABLE_THREADEDGL MATCHES ON)
  set(THREADED_GL false)
  message(STATUS "Threaded OpenGL rendering not enabled")
endif(ENABLE_THREADEDGL MATCHES ON)

# Default to using lib, other architectures can set alternate lib directories
if(NOT LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR lib)
endif(NOT LIB_INSTALL_DIR)

CONFIGURE_FILE(
  "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

FILE(GLOB_RECURSE avogadro_TS i18n/*.ts)
ADD_CUSTOM_TARGET(update-translations
    COMMAND ${QT_LUPDATE_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR} -noobsolete -ts ${avogadro_TS}
    WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR})
# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(libavogadro)
add_subdirectory(avogadro)
add_subdirectory(doc)

# Linux Default: /usr/local/share/avogadro/fragments
set(avogadro_FRAGMENTS share/avogadro)
if(WIN32)
  set(avogadro_FRAGMENTS "C:\\Program Files\\Avogadro")
endif(WIN32)
if(APPLE)
  set(avogadro_FRAGMENTS "/Library/Application Support/Avogadro")
endif(APPLE)

# When installing, make sure to install some fragments
INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fragments"
        DESTINATION ${avogadro_FRAGMENTS})
