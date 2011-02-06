/* pointgroup.h */
/* Copyright (C) 2008 Atsushi Togo */

#ifndef __pointgroup_H__
#define __pointgroup_H__

#include "symmetry.h"

typedef enum {
  NONE,
  TRICLI,
  MONOCLI,
  ORTHO,
  TETRA,
  RHOMB,
  TRIGO,
  HEXA,
  CUBIC,
} Holohedry;

typedef enum {
  LAUE1,
  LAUE2M,
  LAUEMMM,
  LAUE4M,
  LAUE4MMM,
  LAUE3,
  LAUE3M,
  LAUE6M,
  LAUE6MMM,
  LAUEM3,
  LAUEM3M,
} Laue;

typedef struct {
  char symbol[6];
  Holohedry holohedry;
  Laue laue;
  int transform_mat[3][3];
} Pointgroup;

int ptg_get_pointgroup_number( const Symmetry * symmetry );
Pointgroup ptg_get_pointgroup( const int pointgroup_number );
void ptg_get_transformation_matrix( Pointgroup * pointgroup,
				    const Symmetry * symmetry );
#endif
