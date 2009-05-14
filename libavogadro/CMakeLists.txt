project(libavogadro)
# Library versioning
# We probably want to adopt the libtool scheme once we hit 1.0.0
set(SOVERSION 0)

if (WIN32)
  add_definitions( -DWIN32 )
  # add definitions for OB in WIN32
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
endif (WIN32)

# Use this function to add a new plugin. It also uses the global variables
# LINK_LIBS to determine what libraries the plugin should link to and
# DESTINATION_DIR to determine where the plugin will be installed.
# ARGV2 is scanned for ui and or qrc files, ARGV3 is scanned for grc files
# in order to maintain compatibility.
function(avogadro_plugin plugin_name src_list)
  # Assume all MOC stuff is in the headers, replace .cpp and use qt4_wrap_cpp
  # We should probably scan the header to verify the Q_OBJECT macro is used
  string(REPLACE ".cpp" ".h" hdr_list "${src_list}")
  qt4_wrap_cpp(moc_files ${hdr_list})
  # Now sort out the ui and qrc files, process them as appropriate
  unset(ui_plugin_files)
  unset(qrc_plugin_files)
  foreach(fileName ${ARGV2})
    if(${fileName} MATCHES "\\.qrc")
      set(qrc_plugin_files ${qrc_plugin_files} ${fileName})
    endif(${fileName} MATCHES "\\.qrc")
    if(${fileName} MATCHES "\\.ui")
      set(ui_plugin_files ${ui_plugin_files} ${fileName})
    endif(${fileName} MATCHES "\\.ui")
  endforeach(fileName ${ARGV2})
  foreach(fileName ${ARGV3}) # Maintain compatibility with early function
    if(${fileName} MATCHES "\\.qrc")
      set(qrc_plugin_files ${qrc_plugin_files} ${fileName})
    endif(${fileName} MATCHES "\\.qrc")
  endforeach(fileName ${ARGV3})
  if(NOT "${ui_plugin_files}" STREQUAL "")
    # Process the UI file for this plugin
    qt4_wrap_ui(plugin_UIS_H ${ui_plugin_files})
  endif(NOT "${ui_plugin_files}" STREQUAL "")
  if(NOT "${qrc_plugin_files}" STREQUAL "")
    # Process the RC file and add it to the plugin
    qt4_add_resources(plugin_RC_SRCS ${qrc_plugin_files})
  endif(NOT "${qrc_plugin_files}" STREQUAL "")
  add_library(${plugin_name} MODULE ${src_list} ${moc_files} ${plugin_UIS_H}
              ${plugin_RC_SRCS})
  target_link_libraries(${plugin_name} ${LINK_LIBS})
  install(TARGETS ${plugin_name} DESTINATION ${DESTINATION_DIR})
  set_target_properties(${plugin_name} PROPERTIES
                        OUTPUT_NAME ${plugin_name}
			PREFIX "")
endfunction(avogadro_plugin)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)

if(ENABLE_TESTS MATCHES ON)
  add_subdirectory(tests)
endif(ENABLE_TESTS MATCHES ON)

