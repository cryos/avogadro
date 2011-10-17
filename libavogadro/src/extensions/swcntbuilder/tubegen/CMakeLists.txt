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

# Set -fPIC on x86_64
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"  )
endif()
