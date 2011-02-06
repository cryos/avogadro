/* hall_symbol.h */
/* Copyright (C) 2010 Atsushi Togo */

#ifndef __hall_symbol_H__
#define __hall_symbol_H__

#include "bravais.h"
#include "symmetry.h"
#include "mathfunc.h"


int hal_get_hall_symbol( double origin_shift[3],
			 SPGCONST Bravais *bravais,
			 SPGCONST Symmetry *conv_symmetry,
			 const double symprec );

#endif
