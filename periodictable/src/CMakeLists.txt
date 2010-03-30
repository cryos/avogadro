include(${QT_USE_FILE})

# Ensure the Avogadro include directory is always first
get_directory_property(tmp_include_dirs INCLUDE_DIRECTORIES)
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES
  "${periodictable_BINARY_DIR}/include;${tmp_include_dirs}")
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${EIGEN2_INCLUDE_DIR}
  ${OPENBABEL2_INCLUDE_DIR}
)

# I think this is necessary now in order to tell the linker where openbabel is
link_directories(${OPENBABEL2_LIBRARY_DIRS})

file(GLOB periodictable_SRCS "*.cpp")

# Only headers with exported symbols should actually be installed
set(periodictable_HDRS
  elementtranslator.h
  periodictableview.h
)

set(PERIODICTABLE_LINK_LIBRARIES ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES})

qt4_automoc(${periodictable_SRCS})

# Set up the include directory and symlink/copy all headers to it.
# Note: Windows lacks symlinks and so the files must be copied.
make_directory("${libavogadro_BINARY_DIR}/include/avogadro")
if(UNIX)
  set(header_cmd "create_symlink")
else(UNIX)
  set(header_cmd "copy_if_different")
endif(UNIX)
foreach(headerFile ${periodictable_HDRS})
  set(from "${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}")
  set(to   "${libavogadro_BINARY_DIR}/include/avogadro/${headerFile}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E ${header_cmd} "${from}" "${to}")
endforeach(headerFile ${periodictable_HDRS})

add_library(periodictable SHARED
  ${periodictable_SRCS} )
set_target_properties(periodictable PROPERTIES 
  VERSION ${Avogadro_VERSION_FULL}
  SOVERSION 1
  LABELS periodictable)
set_source_files_properties(${periodictable_SRCS}
  PROPERTIES LABELS periodictable)
target_link_libraries(periodictable ${PERIODICTABLE_LINK_LIBRARIES})

install(TARGETS periodictable DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${periodictable_HDRS}
  DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro")

