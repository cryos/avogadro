set(spglib_SRCS
  cell.c
  debug.c
  hall_symbol.c
  kpoint.c
  lattice.c
  mathfunc.c
  pointgroup.c
  primitive.c
  refinement.c
  site_symmetry.c
  sitesym_database.c
  spacegroup.c
  spg_database.c
  spglib.c
  spin.c
  symmetry.c
)
set(spglib_HDRS
  spglib.h
)

add_library(spglib SHARED ${spglib_SRCS})

# Suppress spglib warnings
set_target_properties(spglib PROPERTIES
  COMPILE_FLAGS "-w"
  OUTPUT_NAME avogadro_spglib
  LABELS spglib
  PUBLIC_HEADER ${spglib_HDRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Set -fPIC on x86_64
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
endif()
if(UNIX)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lm")
endif()

##I'm not sure I understand this, but it seems to work well for me.
# Set up the include directory and symlink/copy all headers to it.
# Note: Windows lacks symlinks and so the files must be copied.
#make_directory("${libavogadro_BINARY_DIR}/include/avogadro")
if(UNIX)
  set(header_cmd "create_symlink")
else()
  set(header_cmd "copy_if_different")
endif()
foreach(headerFile ${spglib_HDRS})
  set(from "${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}")
  set(to   "${libavogadro_BINARY_DIR}/include/avogadro/${headerFile}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E ${header_cmd} "${from}" "${to}")
endforeach()

install(TARGETS spglib
LIBRARY       DESTINATION ${LIB_INSTALL_DIR}
PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro")
