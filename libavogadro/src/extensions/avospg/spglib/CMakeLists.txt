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

if(UNIX)
  add_library(spglib SHARED ${spglib_SRCS})
  target_link_libraries(spglib m)
else() # Build statically on windows.
  add_library(spglib STATIC ${spglib_SRCS})
endif()

# Suppress spglib warnings
set_target_properties(spglib PROPERTIES
  COMPILE_FLAGS "-w"
  OUTPUT_NAME avogadro_spglib
  LABELS spglib
  PUBLIC_HEADER ${spglib_HDRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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
