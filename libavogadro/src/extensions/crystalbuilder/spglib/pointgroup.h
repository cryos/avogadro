/* pointgroup.h */
/* Copyright (C) 2008 Atsushi Togo */

#ifndef __pointgroup_H__
#define __pointgroup_H__

#include "symmetry.h"
#include "bravais.h"

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
  int transform_mat[3][3];
} Pointgroup;

Pointgroup ptg_get_pointgroup( const Symmetry * symmetry );
Pointgroup ptg_get_symmetry_pointgroup( Symmetry * symmetry );

#endif
