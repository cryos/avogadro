project(avogadro)
set(VERSION "0.9.0_beta1")

#execute_process(COMMAND cd ${CMAKE_SOURCE_DIR}; git rev-parse --sq --short HEAD
#                OUTPUT_VARIABLE SVN_REVISION)

if (WIN32)
  add_definitions( -DWIN32 )
  # add definitions for OB in WIN32
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
endif (WIN32)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
