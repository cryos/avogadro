/* spacegroup_type.h */
/* Copyright (C) 2008 Atsushi Togo */

#ifndef __spacegroup_type_H__
#define __spacegroup_type_H__

#include "bravais.h"
#include "cell.h"
#include "symmetry.h"
#include "mathfunc.h"

typedef struct {
  int number;
  char schoenflies[7];
  char hall_symbol[17];
  char international[32];
  char international_long[20];
  char international_short[11];
  double bravais_lattice[3][3];
} Spacegroup;

Spacegroup typ_get_spacegroup( SPGCONST Cell * cell,
			       const double symprec );
Symmetry * typ_get_conventional_symmetry( SPGCONST Bravais * bravais,
					  SPGCONST double primitive_lattice[3][3],
					  const Symmetry * primitive_sym,
					  const double symprec );

#endif
