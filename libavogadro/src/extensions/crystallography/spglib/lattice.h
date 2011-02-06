/* lattice.h */
/* Copyright (C) 2008 Atsushi Togo */

#ifndef __lattice_H__
#define __lattice_H__

#include "mathfunc.h"
#include "pointgroup.h"

typedef enum {
  NO_CENTER,
  BODY,
  FACE,
  A_FACE,
  B_FACE,
  C_FACE,
  BASE,
  R_CENTER,
} Centering;

int lat_smallest_lattice_vector( double lattice_new[3][3],
				 SPGCONST double lattice[3][3],
				 const double symprec );
Centering lat_get_centering( double correction_mat[3][3],
			     SPGCONST int transform_mat[3][3],
			     const Laue laue );

#endif
