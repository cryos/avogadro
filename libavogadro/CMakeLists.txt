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
# Optional ARGV2 - UI list
# Optional ARGV3 - RC files
function(avogadro_plugin plugin_name src_list)
  qt4_automoc(${src_list})
  if(NOT "${ARGV2}" STREQUAL "")
    # Process the UI file for this plugin
    qt4_wrap_ui(plugin_UIS_H ${ARGV2})
  endif(NOT "${ARGV2}" STREQUAL "")
  if(NOT "${ARGV3}" STREQUAL "")
    # Process the RC file and add it to the plugin
    qt4_add_resources(plugin_RC_SRCS ${ARGV3})
  endif(NOT "${ARGV3}" STREQUAL "")
  add_library(${plugin_name} MODULE ${src_list} ${plugin_UIS_H}
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
