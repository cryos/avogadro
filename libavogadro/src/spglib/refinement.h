/* bravais.h */
/* Copyright (C) 2011 Atsushi Togo */

#ifndef __refinement_H__
#define __refinement_H__

#include "cell.h"
#include "mathfunc.h"

Cell * ref_refine_cell( SPGCONST Cell * cell,
			const double symprec );

#endif
