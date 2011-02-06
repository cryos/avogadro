/* symmetry_kpoints.h */
/* Copyright (C) 2008 Atsushi Togo */

#ifndef __symmetry_kpoints_H__
#define __symmetry_kpoints_H__

#include "symmetry.h"
#include "mathfunc.h"

int kpt_get_irreducible_kpoints( int map[],
				 SPGCONST double kpoints[][3], 
				 const int num_kpoint,
				 SPGCONST double lattice[3][3],
				 const Symmetry * symmetry,
				 const int is_time_reversal,
				 const double symprec );
int kpt_get_irreducible_reciprocal_mesh( int grid_point[][3],
					 int map[],
					 const int mesh[3],
					 const int is_shift[3],
					 const int is_time_reversal,
					 SPGCONST double lattice[3][3],
					 const Symmetry * symmetry,
					 const double symprec );
int kpt_get_stabilized_reciprocal_mesh( int grid_point[][3],
					int map[],
					const int mesh[3],
					const int is_shift[3],
					const int is_time_reversal,
					SPGCONST double lattice[3][3],
					const Symmetry * symmetry,
					const int num_q,
					SPGCONST double qpoints[][3],
					const double symprec );
int kpt_get_triplets_reciprocal_mesh( int triplets[][3],
				      int weight_triplets[],
				      int grid_point[][3],
				      const int num_triplets,
				      const int mesh[3],
				      const int is_time_reversal,
				      SPGCONST double lattice[3][3],
				      const Symmetry * symmetry,
				      const double symprec );
int kpt_get_triplets_reciprocal_mesh_with_q( int triplets_with_q[][3],
					     int weight_with_q[],
					     const int fixed_grid_number,
					     const int num_triplets,
					     SPGCONST int triplets[][3],
					     const int weight[],
					     const int mesh[3],
					     const int is_time_reversal,
					     SPGCONST double lattice[3][3],
					     const Symmetry * symmetry,
					     const double symprec );
#endif
