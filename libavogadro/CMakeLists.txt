project(libavogadro)
# Library versioning
# We probably want to adopt the libtool scheme once we hit 1.0.0
set(AVO_SOVERSION 1)

if(WIN32)
  add_definitions( -DWIN32 )
  # add definitions for OB in WIN32
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
endif()

macro(avogadro_plugin_helper)
  # Assume all MOC stuff is in the headers, replace .cpp and use qt4_wrap_cpp
  # Scan the header to verify the Q_OBJECT macro is used
  string(REPLACE ".cpp" ".h" hdr_list "${src_list}")
  set(moc_list "")
  foreach(file ${hdr_list})
    file(STRINGS "${file}" fileQObject REGEX "Q_OBJECT")
    if(fileQObject)
      list(APPEND moc_list "${file}")
    endif()
  endforeach()
  qt4_wrap_cpp(moc_files ${moc_list})
  # Now sort out the ui and qrc files, process them as appropriate
  set(ui_plugin_files)
  set(qrc_plugin_files)
  foreach(fileName ${ARGV2})
    if(${fileName} MATCHES "\\.qrc")
      set(qrc_plugin_files ${qrc_plugin_files} ${fileName})
    endif(${fileName} MATCHES "\\.qrc")
    if(${fileName} MATCHES "\\.ui")
      set(ui_plugin_files ${ui_plugin_files} ${fileName})
    endif(${fileName} MATCHES "\\.ui")
  endforeach()
  foreach(fileName ${ARGV3}) # Maintain compatibility with early function
    if(${fileName} MATCHES "\\.qrc")
      set(qrc_plugin_files ${qrc_plugin_files} ${fileName})
    endif()
  endforeach()
  if(NOT "${ui_plugin_files}" STREQUAL "")
    # Process the UI file for this plugin
    qt4_wrap_ui(plugin_UIS_H ${ui_plugin_files})
  endif()
  if(NOT "${qrc_plugin_files}" STREQUAL "")
    # Process the RC file and add it to the plugin
    qt4_add_resources(plugin_RC_SRCS ${qrc_plugin_files})
  endif()
endmacro()

macro(avogadro_plugin_target)
  # If we have set the PLUGIN_TARGET variable add this plugin to the custom
  # target. This assumes that the target was created before this function was
  # called.
  if(PLUGIN_TARGET)
    add_dependencies(${PLUGIN_TARGET} ${plugin_name})
  endif()
endmacro()

# Use this function to add a new plugin. It also uses the global variables
# LINK_LIBS to determine what libraries the plugin should link to and
# DESTINATION_DIR to determine where the plugin will be installed.
# ARGV2 is scanned for ui and or qrc files, ARGV3 is scanned for grc files
# in order to maintain compatibility.
function(avogadro_plugin_nogl plugin_name src_list)
  avogadro_plugin_helper()

  add_library(${plugin_name} MODULE ${src_list} ${moc_files} ${plugin_UIS_H}
              ${plugin_RC_SRCS})
  target_link_libraries(${plugin_name} ${LINK_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${OPENBABEL2_LIBRARIES})
  install(TARGETS ${plugin_name} DESTINATION ${DESTINATION_DIR})
  set_target_properties(${plugin_name} PROPERTIES
                        OUTPUT_NAME ${plugin_name}
                        PREFIX ""
                        LABELS "${PLUGIN_LABEL}")
  set_target_properties(${plugin_name} PROPERTIES
                        COMPILE_FLAGS "${NO_RTTI_FLAG}")
  avogadro_plugin_target()
endfunction()

function(avogadro_plugin plugin_name src_list)
  avogadro_plugin_nogl("${plugin_name}" "${src_list}")
  target_link_libraries(${plugin_name} ${OPENGL_LIBRARIES} )  
endfunction()

function(avogadro_static_plugin plugin_name src_list)
  avogadro_plugin_helper()
  add_library(${plugin_name} STATIC ${src_list} ${moc_files} ${plugin_UIS_H}
              ${plugin_RC_SRCS})
  set_target_properties(${plugin_name} PROPERTIES
                        OUTPUT_NAME ${plugin_name}
                        PREFIX ""
                        LABELS "${PLUGIN_LABEL}")
  set_target_properties(${plugin_name} PROPERTIES
                        COMPILE_FLAGS "${NO_RTTI_FLAG} -DQT_STATICPLUGIN")
  avogadro_plugin_target()
endfunction()

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)

if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()

