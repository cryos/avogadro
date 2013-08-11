set(tubegen_SRCS
  Bitmap.c
  Core3DMath.c
  eprintf.c

  ANSR.cpp
  Cell.cpp
  CrystalCell.cpp
  diamond.cpp
  fcc.cpp
  graphite.cpp
  TubeGen.cpp
  TubuleBasis.cpp
)

add_library(tubegen STATIC ${tubegen_SRCS})

# Suppress warnings
set_target_properties(tubegen PROPERTIES COMPILE_FLAGS "-w")

# Set PIC flag
set_target_properties(tubegen PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
