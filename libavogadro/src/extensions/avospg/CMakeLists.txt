#
# The SPGlib wrappers
#

find_package(Eigen2 REQUIRED)
find_package(Qt4 4.6 REQUIRED)
find_package(OpenBabel2 REQUIRED)
include_directories(${QT_INCLUDE_DIR} ${EIGEN2_INCLUDE_DIR} ${OPENBABE2_INCLUDE_DIR})

# Headers for our public API
set(avospg_HDRS
  avospglib.h
)

# Source files for our data.
set(avospg_SRCS
  avospglib.cpp
)


add_library(AvoSpg SHARED ${avospg_SRCS})

set_target_properties(AvoSpg PROPERTIES
  VERSION 0.1.0
  OUTPUT_NAME avogadro_AvoSpg
  LINK_INTERFACE_LIBRARIES ""
  SOVERSION 0
  LABELS avospg
  PUBLIC_HEADER ${avospg_HDRS})

if(Avogadro_USE_SYSTEM_SPGLIB)
  target_link_libraries(AvoSpg ${SPGLIB_LIBRARY})
else()
  add_subdirectory(spglib)
  target_link_libraries(AvoSpg spglib)
endif()


target_link_libraries(AvoSpg ${QT_QTCORE_LIBRARY})
target_link_libraries(AvoSpg ${OPENBABEL2_LIBRARIES})
target_link_libraries(AvoSpg avogadro)

#install(FILES ${avospg_HDRS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/avogadro")

##I'm not sure I understand this, but it seems to work well for me.
# Set up the include directory and symlink/copy all headers to it.
# Note: Windows lacks symlinks and so the files must be copied.
#make_directory("${libavogadro_BINARY_DIR}/include/avogadro")
if(UNIX)
  set(header_cmd "create_symlink")
else()
  set(header_cmd "copy_if_different")
endif()
foreach(headerFile ${avospg_HDRS})
  set(from "${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}")
  set(to   "${libavogadro_BINARY_DIR}/include/avogadro/${headerFile}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E ${header_cmd} "${from}" "${to}")
endforeach()

install(TARGETS AvoSpg
LIBRARY       DESTINATION ${LIB_INSTALL_DIR}
PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro")
