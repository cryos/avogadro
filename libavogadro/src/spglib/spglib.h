/* spglib.h */
/* Copyright (C) 2008 Atsushi Togo */

/*
  ------------------------------------------------------------------

  See usages written in spglib.c.

  ------------------------------------------------------------------
 */

#ifndef __spglib_H__
#define __spglib_H__

/* SPGCONST is used instead of 'const' so to avoid gcc warning. */
/* However there should be better way than this way.... */
#define SPGCONST

int spg_get_symmetry( int rotation[][3][3],
		      double translation[][3],
		      const int max_size,
		      SPGCONST double lattice[3][3],
		      SPGCONST double position[][3],
		      const int types[],
		      const int num_atom,
		      const double symprec );
int spg_get_conventional_symmetry( double bravais_lattice[3][3], 
				   int rotation[][3][3],
				   double translation[][3],
				   const int max_size,
				   SPGCONST double lattice[3][3],
				   SPGCONST double position[][3],
				   const int types[],
				   const int num_atom,
				   const double symprec );
int spg_get_multiplicity( SPGCONST double lattice[3][3],
			  SPGCONST double position[][3],
			  const int types[],
			  const int num_atom,
			  const double symprec );
int spg_get_smallest_lattice( double smallest_lattice[3][3],
			      SPGCONST double lattice[3][3],
			      const double symprec );
int spg_get_max_multiplicity( SPGCONST double lattice[3][3],
			      SPGCONST double position[][3],
			      const int types[],
			      const int num_atom,
			      const double symprec );
int spg_find_primitive( double lattice[3][3],
			double position[][3],
			int types[],
			const int num_atom,
			const double symprec );
void spg_show_symmetry( SPGCONST double lattice[3][3],
			SPGCONST double position[][3],
			const int types[],
			const int num_atom,
			const double symprec );
int spg_get_international( char symbol[11],
			   SPGCONST double lattice[3][3],
			   SPGCONST double position[][3],
			   const int types[],
			   const int num_atom,
			   const double symprec );
int spg_refine_cell( double lattice[3][3],
		     double position[][3],
		     int types[],
		     const int num_atom,
		     const double symprec );
int spg_get_ir_kpoints( int map[],
			SPGCONST double kpoints[][3],
			const int num_kpoints,
			SPGCONST double lattice[3][3],
			SPGCONST double position[][3],
			const int types[],
			const int num_atom,
			const int is_time_reversal,
			const double symprec );
int spg_get_ir_reciprocal_mesh( int grid_point[][3],
				int map[],
				const int mesh[3],
				const int is_shift[3],
				const int is_time_reversal,
				SPGCONST double lattice[3][3],
				SPGCONST double position[][3],
				const int types[],
				const int num_atom,
				const double symprec );
int spg_get_stabilized_reciprocal_mesh( int grid_point[][3],
				        int map[],
				        const int mesh[3],
				        const int is_shift[3],
				        const int is_time_reversal,
				        SPGCONST double lattice[3][3],
					const int num_rot,
				        SPGCONST int rotations[][3][3],
				        const int num_q,
				        SPGCONST double qpoints[][3],
				        const double symprec );
int spg_get_triplets_reciprocal_mesh( int triplets[][3],
				      int weight_triplets[],
				      int grid_point[][3],
				      const int num_triplets,
				      const int mesh[3],
				      const int is_time_reversal,
				      SPGCONST double lattice[3][3],
				      const int num_rot,
				      SPGCONST int rotations[][3][3],
				      const double symprec );
int spg_get_triplets_reciprocal_mesh_with_q( int triplets_with_q[][3],
					     int weight_triplets_with_q[],
					     const int fixed_grid_number,
					     const int num_triplets,
					     SPGCONST int triplets[][3],
					     const int weight_triplets[],
					     const int mesh[3],
					     const int is_time_reversal,
					     SPGCONST double lattice[3][3],
					     const int num_rot,
					     SPGCONST int rotations[][3][3],
					     const double symprec );
int spg_get_schoenflies( char symbol[10],
			 SPGCONST double lattice[3][3],
			 SPGCONST double position[][3],
			 const int types[],
			 const int num_atom,
			 const double symprec );

#endif
