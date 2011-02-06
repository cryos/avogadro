/* bravais.h */
/* Copyright (C) 2008 Atsushi Togo */

#ifndef __bravasis_H__
#define __bravasis_H__

#include "mathfunc.h"

typedef enum {
  TRICLI = 1,
  MONOCLI,
  ORTHO,
  TETRA,
  RHOMB,
  TRIGO,
  HEXA,
  CUBIC,
} Holohedry;

typedef enum {
  NO_CENTER = 0,
  BODY = -1,
  FACE = -3,
  A_FACE = 1,
  B_FACE = 2,
  C_FACE = 3,
  BASE = 4,
} Centering;

typedef struct {
  Holohedry holohedry;
  Centering centering;
  double lattice[3][3];
} Bravais;


Bravais brv_get_brv_lattice( SPGCONST double lattice_orig[3][3],
			     const double symprec );
int brv_get_brv_lattice_in_loop( Bravais *bravais,
				 SPGCONST double min_lattice[3][3],
				 const double symprec );
void brv_smallest_lattice_vector( double lattice_new[3][3],
				  SPGCONST double lattice[3][3],
				  const double symprec );
void brv_set_relative_lattice( void );

#endif
