/* bravais_art.h */
/* Copyright (C) 2008 Atsushi Togo */

#ifndef __bravais_virtual_H__
#define __bravais_virtual_H__

#include "bravais.h"
#include "cell.h"
#include "symmetry.h"

int art_get_artificial_bravais( Bravais *bravais,
				const Symmetry *symmetry,
				SPGCONST Cell *cell,
				const Holohedry holohedry,
				const double symprec );

#endif
