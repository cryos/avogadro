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
  kpoint.c
  site_symmetry.c
  sitesym_database.c
  spglib_f.c
  spin.c
  tetrahedron_method.c
)

add_library(spglib STATIC ${spglib_SRCS})

# Suppress spglib warnings
set_target_properties(spglib PROPERTIES COMPILE_FLAGS "-w")

# Set PIC flag
set_target_properties(spglib PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
