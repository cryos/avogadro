project(avogadro) # the name of your project

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

cmake_minimum_required(VERSION 2.4.5)
include (CheckCXXCompilerFlag)
include (MacroEnsureVersion)
###########################################################
# Here I am adding GCC magic from KDE. With these settings#
# compiler issues are fixed and more warnings are issued. #
###########################################################
if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -std=iso9899:1990 -Wundef -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
#        set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wformat-security -fno-exceptions -fno-check-new -fno-common")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wformat-security -fno-check-new -fno-common -pedantic")
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

# Disabled the visibility flags for now - seem to break forcefields
	if (HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
		# We have all the parts necessary - use visibility support
		add_definitions(-DHAVE_GCC_VISIBILITY)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
		if (GCC_IS_NEWER_THAN_4_2)
			set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
		endif (GCC_IS_NEWER_THAN_4_2)
	endif (HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)

endif (CMAKE_COMPILER_IS_GNUCXX)

if(RELEASE_MODE)
 add_definitions(-DNDEBUG)
 if (CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -O3")
  set (CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -O3")
 endif (CMAKE_COMPILER_IS_GNUCXX)
endif(RELEASE_MODE)

set(QT_MIN_VERSION "4.3.0") # we need at least version 4.3 of Qt
find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
find_package(OpenGL REQUIRED) # find and setup OpenGL
find_package(OpenBabel2 REQUIRED) # find and setup OpenBabel
find_package(Eigen REQUIRED) # find and setup Eigen
find_package(Linguist REQUIRED) # find and setup Eigen

# The CMake 2.6 FindBoost function is far better than the 2.4 version
# So use it if possible
if(CMAKE_MAJOR_VERSION MATCHES 2 AND CMAKE_MINOR_VERSION MATCHES 6)
	message(STATUS "Using CMake 2.6")
	set(CMAKE_2_6 TRUE)
	find_package(Boost COMPONENTS python)
	if(Boost_PYTHON_FOUND)
		message(STATUS "Boost Python found!")
	endif(Boost_PYTHON_FOUND)
else(CMAKE_MAJOR_VERSION MATCHES 2 AND CMAKE_MINOR_VERSION MATCHES 6)
	find_package(BoostPython)
endif(CMAKE_MAJOR_VERSION MATCHES 2 AND CMAKE_MINOR_VERSION MATCHES 6)

find_package(PythonLibs)

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
		${CMAKE_CURRENT_SOURCE_DIR} -ts ${avogadro_TS}
		WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}
												                       )
# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(libavogadro)
add_subdirectory(avogadro)
add_subdirectory(doc)

# Linux Default: /usr/local/share/avogadro/fragments
set(avogadro_FRAGMENTS share/avogadro)
if(WIN32)
   set(avogadro_FRAGMENTS "C:\Program Files\Avogadro")
endif(WIN32)
if(APPLE)
   set(avogadro_FRAGMENTS "/Library/Application Support/Avogadro")   
endif(APPLE)

# When installing, make sure to install some fragments
INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fragments"
               DESTINATION ${avogadro_FRAGMENTS}
               PATTERN ".svn" EXCLUDE )

