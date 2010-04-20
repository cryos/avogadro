include(${QT_USE_FILE})

# Ensure the Avogadro include directory is always first
get_directory_property(tmp_include_dirs INCLUDE_DIRECTORIES)
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES
  "${libavogadro_BINARY_DIR}/include;${tmp_include_dirs}")
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB plotwidget_SRCS "*.cpp")

# Only headers with exported symbols should actually be installed
set(plotwidget_HDRS
  plotaxis.h # dlonie: Are these necessary here?
  plotobject.h
  plotpoint.h
  plotwidget.h
)

set(PLOTWIDGET_LINK_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

qt4_automoc(${plotwidget_SRCS})

# Set up the include directory and symlink/copy all headers to it.
# Note: Windows lacks symlinks and so the files must be copied.
make_directory("${libavogadro_BINARY_DIR}/include/qplotwidget")
if(UNIX)
  set(header_cmd "create_symlink")
else(UNIX)
  set(header_cmd "copy_if_different")
endif(UNIX)
foreach(headerFile ${plotwidget_HDRS} global.h)
  set(from "${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}")
  set(to   "${libavogadro_BINARY_DIR}/include/qplotwidget/${headerFile}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E ${header_cmd} "${from}" "${to}")
endforeach(headerFile ${periodictable_HDRS})

add_library(plotwidget SHARED
  ${plotwidget_SRCS} )
set_target_properties(plotwidget PROPERTIES
  LINK_INTERFACE_LIBRARIES ""
  VERSION ${Avogadro_VERSION_FULL}
  SOVERSION 1
  OUTPUT_NAME QPlotWidget
  LABELS plotwidget)
set_source_files_properties(${plotwidget_SRCS}
  PROPERTIES LABELS plotwidget)
target_link_libraries(plotwidget ${PLOTWIDGET_LINK_LIBRARIES})

install(TARGETS plotwidget DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${plotwidget_HDRS}
  DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro")
install(FILES ${plotwidget_HDRS} global.h
  DESTINATION "${INCLUDE_INSTALL_DIR}/qplotwidget")

