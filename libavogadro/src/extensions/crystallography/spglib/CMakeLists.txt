set(spglib_SRCS
  cell.c
  debug.c
  hall_symbol.c
  lattice.c
  mathfunc.c
  pointgroup.c
  primitive.c
  refinement.c
  spacegroup.c
  spg_database.c
  spglib.c
  symmetry.c
  symmetry_kpoint.c
)

add_library(spglib STATIC ${spglib_SRCS})

# Suppress spglib warnings
set_target_properties(spglib PROPERTIES COMPILE_FLAGS "-w")

# Set -fPIC on x86_64
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
endif()
