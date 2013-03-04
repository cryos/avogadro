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
  LABELS spglib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
