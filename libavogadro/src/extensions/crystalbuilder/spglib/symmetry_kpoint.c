/* symmetry_kpoints.c */
/* Copyright (C) 2008 Atsushi Togo */

#include <stdio.h>
#include <stdlib.h>
#include "mathfunc.h"
#include "symmetry.h"

#include "debug.h"

/* #define QXYZ */
/* The addressing order of mesh grid is defined as running left */
/* element first. But when QXYZ is defined, it is changed to right */ 
/* element first. */

static PointSymmetry get_point_group_rotation( SPGCONST double lattice[3][3], 
					       const Symmetry * symmetry,
					       const int is_time_reversal,
					       const double symprec,
					       const int num_q,
					       SPGCONST double q[][3] );
static int get_ir_kpoints( int map[],
			   SPGCONST double kpoints[][3],
			   const int num_kpoint,
			   SPGCONST PointSymmetry * point_symmetry,
			   const double symprec );
static int get_ir_reciprocal_mesh( int grid_point[][3],
				   int map[],
				   const int mesh[3],
				   const int is_shift[3],
				   SPGCONST PointSymmetry * point_symmetry );
static int get_ir_triplets( int triplets[][3],
			    int weight_triplets[],
			    int grid[][3],
			    const int num_triplets,
			    const int mesh[3],
			    const int is_time_reversal,
			    SPGCONST double lattice[3][3],
			    const Symmetry *symmetry,
			    const double symprec );
static int get_ir_triplets_with_q( int triplets_with_q[][3], 
				   int weight_with_q[],
				   const int fixed_grid_number,
				   SPGCONST int triplets[][3],
				   const int num_triplets,
				   const int mesh[3],
				   SPGCONST PointSymmetry * point_symmetry );
static void get_grid_mapping_table( SPGCONST PointSymmetry * point_symmetry,
				    const int mesh[3],
				    int **map_sym,
				    const int is_shift[3] );
static void address_to_grid( int grid_double[3],
			     const int address,
			     const int mesh[3],
			     const int is_shift[3] );
static void get_grid_points( int grid_point[3],
			     const int grid[3],
			     const int mesh[3] );
static void get_vector_modulo( int v[3],
			       const int m[3] );
static int grid_to_address( const int grid[3],
			    const int mesh[3],
			    const int is_shift[3] );
static void free_array2D_int( int **array,
			      const int num_row );
static int ** allocate_array2d_int( const int num_row,
				    const int num_column );



int kpt_get_irreducible_kpoints( int map[],
				 SPGCONST double kpoints[][3],
				 const int num_kpoint,
				 SPGCONST double lattice[3][3],
				 const Symmetry * symmetry,
				 const int is_time_reversal,
				 const double symprec )
{
  PointSymmetry point_symmetry;

  point_symmetry = get_point_group_rotation( lattice,
					     symmetry,
					     is_time_reversal,
					     symprec,
					     0, NULL );

  return get_ir_kpoints(map, kpoints, num_kpoint, &point_symmetry, symprec);
}

/* grid_point (e.g. 4x4x4 mesh)                               */
/*    [[ 0  0  0]                                             */
/*     [ 1  0  0]                                             */
/*     [ 2  0  0]                                             */
/*     [-1  0  0]                                             */
/*     [ 0  1  0]                                             */
/*     [ 1  1  0]                                             */
/*     [ 2  1  0]                                             */
/*     [-1  1  0]                                             */
/*     ....      ]                                            */
/*                                                            */
/* Each value of 'map' correspnds to the index of grid_point. */
int kpt_get_irreducible_reciprocal_mesh( int grid_point[][3],
					 int map[],
					 const int mesh[3],
					 const int is_shift[3],
					 const int is_time_reversal,
					 SPGCONST double lattice[3][3],
					 const Symmetry * symmetry,
					 const double symprec )
{
  PointSymmetry point_symmetry;

  point_symmetry = get_point_group_rotation( lattice,
					     symmetry,
					     is_time_reversal,
					     symprec,
					     0, NULL );

  return get_ir_reciprocal_mesh(grid_point, map, mesh, is_shift, &point_symmetry);
}

int kpt_get_stabilized_reciprocal_mesh( int grid_point[][3],
					int map[],
					const int mesh[3],
					const int is_shift[3],
					const int is_time_reversal,
					SPGCONST double lattice[3][3],
					const Symmetry * symmetry,
					const int num_q,
					SPGCONST double qpoints[][3],
					const double symprec )
{
  PointSymmetry point_symmetry;
  
  point_symmetry = get_point_group_rotation( lattice,
					     symmetry,
					     is_time_reversal,
					     symprec,
					     num_q, qpoints );

  return get_ir_reciprocal_mesh(grid_point, map, mesh, is_shift, &point_symmetry);
}

int kpt_get_triplets_reciprocal_mesh( int triplets[][3],
				      int weight_triplets[],
				      int grid_point[][3],
				      const int num_triplets,
				      const int mesh[3],
				      const int is_time_reversal,
				      SPGCONST double lattice[3][3],
				      const Symmetry * symmetry,
				      const double symprec )
{
  return get_ir_triplets( triplets,
			  weight_triplets,
			  grid_point,
			  num_triplets,
			  mesh,
			  is_time_reversal,
			  lattice,
			  symmetry,
			  symprec );
}

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
					     const double symprec )
{
  PointSymmetry point_symmetry;

  point_symmetry = get_point_group_rotation( lattice,
					     symmetry,
					     is_time_reversal,
					     symprec,
					     0, NULL );

  return get_ir_triplets_with_q( triplets_with_q,
				 weight_with_q,
				 fixed_grid_number,
				 triplets,
				 num_triplets,
				 mesh,
				 &point_symmetry );
				 
}

/* qpoints are used to find stabilizers (operations). */
/* num_q is the number of the qpoints. */
static PointSymmetry get_point_group_rotation( SPGCONST double lattice[3][3], 
					       const Symmetry * symmetry,
					       const int is_time_reversal,
					       const double symprec,
					       const int num_q,
					       SPGCONST double qpoints[][3])
{
  int i, j, k, l, is_found, count = 0;
  double volume;
  double rot_d[3][3], lat_inv[3][3], glat[3][3], tmp_mat[3][3], grot_d[3][3];
  double vec[3], diff[3];
  MatINT *rotations;
  PointSymmetry point_symmetry;
  SPGCONST int time_reversal_rotation[3][3] = {
    {-1, 0, 0 },
    { 0,-1, 0 },
    { 0, 0,-1 }
  };
  rotations = mat_alloc_MatINT(symmetry->size*2);

  volume = mat_get_determinant_d3(lattice);

  mat_inverse_matrix_d3(lat_inv, lattice, symprec);
  mat_transpose_matrix_d3(glat, lat_inv);
  mat_multiply_matrix_d3(tmp_mat, lat_inv, glat);
  
  for ( i = 0; i < symmetry->size; i++ ) {
    mat_cast_matrix_3i_to_3d(rot_d, symmetry->rot[i]);
    mat_get_similar_matrix_d3(grot_d, rot_d, tmp_mat,
			      symprec / volume / volume);
    mat_cast_matrix_3d_to_3i(rotations->mat[i], grot_d);
    mat_multiply_matrix_i3(rotations->mat[symmetry->size+i],
			   time_reversal_rotation,
			   rotations->mat[i]);
  }

  for ( i = 0; i < symmetry->size * (1 + (is_time_reversal != 0)); i++ ) {
    is_found = 1;

    for ( j = 0; j < count; j++ ) {
      if (mat_check_identity_matrix_i3(point_symmetry.rot[j],
				       rotations->mat[i])) {
	is_found = 0;
	break;
      }
    }

    if ( is_found ) {
      for ( k = 0; k < num_q; k++ ) { /* Loop to find stabilizers */
	is_found = 0;
	mat_multiply_matrix_vector_id3( vec, rotations->mat[i],
					qpoints[k] );

	for ( l = 0; l < num_q; l++ ) {
	  diff[0] = vec[0] - qpoints[l][0];
	  diff[1] = vec[1] - qpoints[l][1];
	  diff[2] = vec[2] - qpoints[l][2];
	
	  if ( mat_Dabs( diff[0] - mat_Nint( diff[0] ) ) < symprec &&
	       mat_Dabs( diff[1] - mat_Nint( diff[1] ) ) < symprec &&
	       mat_Dabs( diff[2] - mat_Nint( diff[2] ) ) < symprec ) {
	    is_found = 1;
	    break;
	  }
	}
	if ( is_found == 0 ) {
	  break;
	}
      }
      
      if ( is_found ) {
	mat_copy_matrix_i3(point_symmetry.rot[count], rotations->mat[i]);
	count++;
      }
    }
  }

  point_symmetry.size = count;

  mat_free_MatINT(rotations);

  return point_symmetry;
}

static int get_ir_kpoints( int map[],
			   SPGCONST double kpoints[][3],
			   const int num_kpoint,
			   SPGCONST PointSymmetry * point_symmetry,
			   const double symprec )
{
  int i, j, k, l, num_ir_kpoint = 0, is_found;
  int *ir_map;
  double kpt_rot[3], diff[3];

  ir_map = (int*)malloc(num_kpoint*sizeof(int));

  for ( i = 0; i < num_kpoint; i++ ) {

    map[i] = i;

    is_found = 1;

    for ( j = 0; j < point_symmetry->size; j++ ) {
      mat_multiply_matrix_vector_id3(kpt_rot, point_symmetry->rot[j], kpoints[i]);

      for ( k = 0; k < 3; k++ ) {
	diff[k] = kpt_rot[k] - kpoints[i][k];
	diff[k] = diff[k] - mat_Nint(diff[k]);
      }

      if ( mat_Dabs(diff[0]) < symprec && 
	   mat_Dabs(diff[1]) < symprec && 
	   mat_Dabs(diff[2]) < symprec ) {
	continue;
      }
      
      for ( k = 0; k < num_ir_kpoint; k++ ) {
	mat_multiply_matrix_vector_id3(kpt_rot, point_symmetry->rot[j], kpoints[i]);

	for ( l = 0; l < 3; l++ ) {
	  diff[l] = kpt_rot[l] - kpoints[ir_map[k]][l];
	  diff[l] = diff[l] - mat_Nint(diff[l]);
	}

	if ( mat_Dabs(diff[0]) < symprec && 
	     mat_Dabs(diff[1]) < symprec && 
	     mat_Dabs(diff[2]) < symprec ) {
	  is_found = 0;
	  map[i] = ir_map[k];
	  break;
	}
      }

      if ( ! is_found )
	break;
    }

    if ( is_found ) {
      ir_map[num_ir_kpoint] = i;
      num_ir_kpoint++;
    }
  }

  free( ir_map );
  ir_map = NULL;

  return num_ir_kpoint;
}

static int get_ir_reciprocal_mesh( int grid[][3],
				   int map[],
				   const int mesh[3],
				   const int is_shift[3],
				   SPGCONST PointSymmetry * point_symmetry )
{
  /* In the following loop, mesh is doubled. */
  /* Even and odd mesh numbers correspond to */
  /* is_shift[i] = 0 and 1, respectively. */
  /* is_shift = [0,0,0] gives Gamma center mesh. */
  /* grid: reducible grid points */
  /* map: the mapping from each point to ir-point. */
  int i, j, k, l, address, address_rot, num_ir = 0;
  int grid_double[3], grid_rot[3], mesh_double[3];

  for ( i = 0; i < 3; i++ )
    mesh_double[i] = mesh[i] * 2;

  /* "-1" means the element is not touched yet. */
  for ( i = 0; i < mesh[0] * mesh[1] * mesh[2]; i++ ) {
    map[i] = -1;
  }

#ifndef QXYZ
  for ( i = 0; i < mesh_double[2]; i++ ) {
    if ( ( is_shift[2] && i % 2 == 0 ) ||
	 ( is_shift[2] == 0 && i % 2 != 0 ) ) 
      continue;

    for ( j = 0; j < mesh_double[1]; j++ ) {
      if ( ( is_shift[1] && j % 2 == 0 ) ||
	   ( is_shift[1] == 0 && j % 2 != 0 ) ) 
	continue;
      
      for ( k = 0; k < mesh_double[0]; k++ ) {
	if ( ( is_shift[0] && k % 2 == 0 ) ||
	     ( is_shift[0] == 0 && k % 2 != 0 ) ) 
	  continue;

	grid_double[0] = k;
	grid_double[1] = j;
	grid_double[2] = i;
#else
  for ( i = 0; i < mesh_double[0]; i++ ) {
    if ( ( is_shift[0] && i % 2 == 0 ) ||
  	 ( is_shift[0] == 0 && i % 2 != 0 ) )
      continue;

    for ( j = 0; j < mesh_double[1]; j++ ) {
      if ( ( is_shift[1] && j % 2 == 0 ) ||
  	   ( is_shift[1] == 0 && j % 2 != 0 ) )
  	continue;
      
      for ( k = 0; k < mesh_double[2]; k++ ) {
  	if ( ( is_shift[2] && k % 2 == 0 ) ||
  	     ( is_shift[2] == 0 && k % 2 != 0 ) )
  	  continue;

  	grid_double[0] = i;
  	grid_double[1] = j;
  	grid_double[2] = k;
#endif	

	address = grid_to_address( grid_double, mesh, is_shift );
	get_grid_points(grid[ address ], grid_double, mesh);

	for ( l = 0; l < point_symmetry->size; l++ ) {

	  mat_multiply_matrix_vector_i3( grid_rot, point_symmetry->rot[l], grid_double );
	  get_vector_modulo(grid_rot, mesh_double);
	  address_rot = grid_to_address( grid_rot, mesh, is_shift );

	  if ( address_rot > -1 ) { /* Invalid if even --> odd or odd --> even */
	    if ( map[ address_rot ] > -1 ) {
	      map[ address ] = map[ address_rot ];
	      break;
	    }
	  }
	}

	/* Set itself to the map when equivalent point */
	/* with smaller numbering could not be found. */
	if ( map[ address ] == -1 ) {
	  map[ address ] = address;
	  num_ir++;
	}
      }
    }
  }

  return num_ir;
}


/* Unique q-point triplets that conserve the momentum,  */
/* q+q'+q''=G, are obtained.                            */
/*                                                      */
/* The first q-point is selected among the ir-q-points. */
/* The second q-point is selected among the ir-q-points */
/* constrained by the first q-point (stabilizer)        */
/* The third q-point is searched through the all grid   */
/* points and is checked if it satisfies q+q'+q''=G,    */
/* here q, q', and q'' can be exchanged one another.    */
static int get_ir_triplets( int triplets[][3],
			    int weight_triplets[],
			    int grid[][3],
			    const int max_num_triplets,
			    const int mesh[3],
			    const int is_time_reversal,
			    SPGCONST double lattice[3][3],
			    const Symmetry *symmetry,
			    const double symprec )
{
  int i, j, l, m, num_ir, is_found, weight, weight_q;
  int num_triplets=0, num_unique_q=0;
  int mesh_double[3], address[3], is_shift[3];
  int grid_double[3][3];
  const int num_grid = mesh[0] * mesh[1] * mesh[2];
  int *map, *map_q, *unique_q;
  int **map_sym, **map_triplets;
  double q[3];
  PointSymmetry point_symmetry, point_symmetry_q;
  
  map_sym = allocate_array2d_int( symmetry->size, num_grid );
  map = (int*)malloc(num_grid * sizeof(int));
  map_q = (int*)malloc(num_grid * sizeof(int));
  unique_q = (int*)malloc(num_grid * sizeof(int));

  
  point_symmetry = get_point_group_rotation( lattice,
					     symmetry,
					     is_time_reversal,
					     symprec,
					     0, NULL );

  /* Only consider the gamma-point */
  for ( i = 0; i < 3; i++ )
    is_shift[i] = 0;

  num_ir = get_ir_reciprocal_mesh( grid, map, mesh, is_shift, &point_symmetry );
  map_triplets = allocate_array2d_int( num_ir, num_grid );

  for ( i = 0; i < 3; i++ )
    mesh_double[i] = mesh[i] * 2;

  /* Memory space check */
  if ( num_ir * num_grid < max_num_triplets ) {
    fprintf(stderr, "spglib: More memory space for triplets is required.");
    goto err;
  }

  /* Prepare triplet mapping table to enhance speed of query */
  /* 'unique_q' table is prepared for saving memory space */
  for ( i = 0; i < num_grid; i++ ) {
    if ( i == map[i] ) {
      unique_q[i] = num_unique_q;
      num_unique_q++;
    } 
    else {
      unique_q[i] = unique_q[map[i]];
    }
  }


  for ( i = 0; i < num_ir; i++ )
    for ( j = 0; j < num_grid; j++ )
      map_triplets[i][j] = 0;

  /* Prepare grid point mapping table */
  get_grid_mapping_table( &point_symmetry, mesh, map_sym, is_shift );

  /* Search triplets without considersing combination */
  for ( i = 0; i < num_grid; i++ ) {
    if ( i != map[ i ] )
      continue;

    weight = 0;
    for ( j = 0; j < num_grid; j++ ) {
      if ( i == map[j] )
	weight++;
    }

    /* Search irreducible q-points (map_q) with a stabilizer */
    address_to_grid( grid_double[0], i, mesh, is_shift );
    for ( j = 0; j < 3; j++ ) {
      q[j] = (double)grid_double[0][j] / mesh_double[j];
    }

    point_symmetry_q = get_point_group_rotation( lattice,
						 symmetry,
						 is_time_reversal,
						 symprec,
						 1, q );

    get_ir_reciprocal_mesh(grid, map_q, mesh, is_shift, &point_symmetry_q);


    for ( j = 0; j < num_grid; j++ ) {
      if ( j != map_q[ j ] ) {
	continue;
      }

      weight_q = 0;
      for ( l = 0; l < num_grid; l++ ) {
	if ( j == map_q[l] ) {
	  weight_q++;
	}
      }

      address_to_grid( grid_double[1], j, mesh, is_shift );

      grid_double[2][0] = - grid_double[0][0] - grid_double[1][0];
      grid_double[2][1] = - grid_double[0][1] - grid_double[1][1];
      grid_double[2][2] = - grid_double[0][2] - grid_double[1][2];
      get_vector_modulo( grid_double[2], mesh_double );

      /* Look for irreducible triplets exchanging three q-points */
      /* and searching by symmetry rotations */
      is_found = 0;
      for ( l = 0; l < point_symmetry.size; l++ ) {

	/* Check six combinations */
	for ( m = 0; m < 3; m++ ) {
	  address[m%3] = map_sym[l][i];
	  address[(m+1)%3] = map_sym[l][j];
	  address[(m+2)%3] = map_sym[l][grid_to_address( grid_double[2], mesh, is_shift )];
	
	  /* Found in the list (even) */
	  if ( address[0] == map[address[0]] ) {
	    if ( map_triplets[ unique_q[address[0]] ][ address[1] ] ) {
	      is_found = 1;
	      map_triplets[ unique_q[address[0]] ][ address[1] ] += weight * weight_q;
	      break;
	    }
	  }

	  /* Found in the list (odd) */
	  if ( address[1] == map[address[1]] ) {
	    if ( map_triplets[ unique_q[ address[1]] ][ address[0] ] ) {
	      is_found = 1;
	      map_triplets[ unique_q[ address[1]] ][ address[0] ] += weight * weight_q;
	      break;
	    }
	  }
	}
	
	if ( is_found )
	  break;
      }

      /* Not found in the list, then this is an irreducible triplet. */
      if (! is_found ) {
	triplets[num_triplets][0] = i;
	triplets[num_triplets][1] = j;
	triplets[num_triplets][2] = grid_to_address( grid_double[2], mesh, is_shift );
	num_triplets++;
	map_triplets[unique_q[i]][j] = weight * weight_q;
      }
    }
  }

  for ( i = 0; i < num_triplets; i++ )
    weight_triplets[i] = map_triplets[ unique_q[ triplets[i][0]] ][ triplets[i][1] ];

  free_array2D_int( map_sym, symmetry->size );
  free_array2D_int( map_triplets, num_ir );
  free( map );
  free( map_q );
  free( unique_q );
  return num_triplets;

 err:
  free_array2D_int( map_sym, symmetry->size );  
  free_array2D_int( map_triplets, num_ir );
  free( map );
  free( map_q );
  free( unique_q );
  return 0;
}

static int get_ir_triplets_with_q( int triplets_with_q[][3], 
				   int weight_with_q[],
				   const int fixed_grid_number,
				   SPGCONST int triplets[][3],
				   const int num_triplets,
				   const int mesh[3],
				   SPGCONST PointSymmetry *point_symmetry )
{
  int i, j, k, sym_num, rest_index, num_triplets_with_q;
  int address0, address1, address1_orig, found;
  int is_shift[3];
  const int num_grid = mesh[0] * mesh[1] * mesh[2];
  int **map_sym;

  map_sym = allocate_array2d_int( point_symmetry->size, num_grid );

  /* Only consider the gamma-point */
  for ( i = 0; i < 3; i++ ) {
    is_shift[i] = 0;
  }

  /* Prepare mapping tables */
  get_grid_mapping_table( point_symmetry, mesh, map_sym, is_shift );

  num_triplets_with_q = 0;

  for ( i = 0; i < num_triplets; i++ ) {
    sym_num = -1;
    for ( j = 0; j < point_symmetry->size; j++ ) {
      address0 = map_sym[j][fixed_grid_number];
      if ( triplets[i][0] == address0 ||
	   triplets[i][1] == address0 ||
	   triplets[i][2] == address0 ) {
	for ( k = 0; k < num_grid; k++ ) {
	  address1 = map_sym[j][k];
	  /* Matching indices 0 and 1 */
	  if ( ( triplets[i][0] == address0 && triplets[i][1] == address1 ) ||
	       ( triplets[i][1] == address0 && triplets[i][0] == address1 ) ) {
	    sym_num = j;
	    rest_index = 2;
	    address1_orig = k;
	    break;
	  }
	  /* Matching indices 1 and 2 */
	  if ( ( triplets[i][1] == address0 && triplets[i][2] == address1 ) ||
	       ( triplets[i][2] == address0 && triplets[i][1] == address1 ) ) {
	    sym_num = j;
	    rest_index = 0;
	    address1_orig = k;
	    break;
	  }
	  /* Matching indices 2 and 0 */
	  if ( ( triplets[i][2] == address0 && triplets[i][0] == address1 ) ||
	       ( triplets[i][0] == address0 && triplets[i][2] == address1 ) ) {
	    sym_num = j;
	    rest_index = 1;
	    address1_orig = k;
	    break;
	  }
	}
	if ( sym_num > -1 ) {
	  break;
	}
      }
    }

    /* Found? */
    if ( sym_num > -1 ) {
      for ( j = 0; j < num_grid; j++ ) {
	if ( map_sym[sym_num][j] == triplets[i][rest_index] ) {
	  triplets_with_q[num_triplets_with_q][0] = fixed_grid_number;
	  if ( j > address1_orig ) {
	    triplets_with_q[num_triplets_with_q][1] = address1_orig;
	    triplets_with_q[num_triplets_with_q][2] = j;
	  } else {
	    triplets_with_q[num_triplets_with_q][2] = address1_orig;
	    triplets_with_q[num_triplets_with_q][1] = j;
	  }
	  num_triplets_with_q++;
	  break;
	}
      }
    }
  }

  for ( i = 0; i < num_triplets_with_q; i++ ) {
    weight_with_q[i] = 0;
  }

  for ( i = 0; i < num_grid; i++ ) {
    found = 0;
    for ( j = 0; j < num_triplets_with_q; j++ ) {
      for ( k = 0; k < point_symmetry->size; k++ ) {

	if ( map_sym[k][fixed_grid_number] == triplets_with_q[j][0] ) {
	  if ( map_sym[k][i] == triplets_with_q[j][1] ||
	       map_sym[k][i] == triplets_with_q[j][2] ) {
	    weight_with_q[j]++;
	    found = 1;
	    break;
	  }	  
	}
	if ( map_sym[k][fixed_grid_number] == triplets_with_q[j][1] ) {
	  if ( map_sym[k][i] == triplets_with_q[j][2] ||
	       map_sym[k][i] == triplets_with_q[j][0] ) {
	    weight_with_q[j]++;
	    found = 1;
	    break;
	  }	  
	}
	if ( map_sym[k][fixed_grid_number] == triplets_with_q[j][2] ) {
	  if ( map_sym[k][i] == triplets_with_q[j][0] ||
	       map_sym[k][i] == triplets_with_q[j][1] ) {
	    weight_with_q[j]++;
	    found = 1;
	    break;
	  }
	}
      }
      if ( found ) {
	break;
      }
    }
    if ( ! found ) {
      fprintf(stderr, "spglib: Unexpected behavior in get_ir_triplets_with_q.\n");
    }
  }

  free_array2D_int( map_sym, point_symmetry->size );
  return num_triplets_with_q;
}

static void get_grid_mapping_table( SPGCONST PointSymmetry *point_symmetry,
				    const int mesh[3],
				    int **map_sym,
				    const int is_shift[3] )
{
  int i, j;
  int grid_rot[3], grid_double[3], mesh_double[3];

  for ( i = 0; i < 3; i++ )
    mesh_double[i] = mesh[i] * 2;

  for ( i = 0; i < point_symmetry->size; i++ ) {
    for ( j = 0; j < mesh[0]*mesh[1]*mesh[2]; j++ ) {
      address_to_grid( grid_double, j, mesh, is_shift );
      mat_multiply_matrix_vector_i3( grid_rot, point_symmetry->rot[i], grid_double );
      get_vector_modulo( grid_rot, mesh_double );
      map_sym[i][j] = grid_to_address( grid_rot, mesh, is_shift );
    }
  }
}  


static int grid_to_address( const int grid_double[3],
			    const int mesh[3],
			    const int is_shift[3] )
{
  int i, grid[3];

  for ( i = 0; i < 3; i++ ) {
    if ( grid_double[i] % 2 == 0 && (! is_shift[i])  ) {
      grid[i] = grid_double[i] / 2;
    } else {
      if ( grid_double[i] % 2 != 0 && is_shift[i] ) {
	grid[i] = ( grid_double[i] - 1 ) / 2;
      } else {
	return -1;
      }
    }
  }

#ifndef QXYZ
  return grid[2] * mesh[0] * mesh[1] + grid[1] * mesh[0] + grid[0];
#else
  return grid[0] * mesh[1] * mesh[2] + grid[1] * mesh[2] + grid[2];
#endif  
}

static void address_to_grid( int grid_double[3],
			     const int address,
			     const int mesh[3],
			     const int is_shift[3] )
{
  int i;
  int grid[3];

#ifndef QXYZ
  grid[2] = address / ( mesh[0] * mesh[1] );
  grid[1] = ( address - grid[2] * mesh[0] * mesh[1] ) / mesh[0];
  grid[0] = address % mesh[0];
#else
  grid[0] = address / ( mesh[1] * mesh[2] );
  grid[1] = ( address - grid[0] * mesh[1] * mesh[2] ) / mesh[2];
  grid[2] = address % mesh[2];
#endif

  for ( i = 0; i < 3; i++ ) {
    grid_double[i] = grid[i] * 2 + is_shift[i];
  }
}

static void get_grid_points( int grid[3],
			     const int grid_double[3],
			     const int mesh[3] )
{
  int i;

  for ( i = 0; i < 3; i++ ) {
    if ( grid_double[i] % 2 == 0 ) {
      grid[i] = grid_double[i] / 2;
    } else {
      grid[i] = ( grid_double[i] - 1 ) / 2;
    }
    
    grid[i] = grid[i] - mesh[i] * ( grid[i] > mesh[i] / 2 );
  }  
}

static void get_vector_modulo( int v[3],
			       const int m[3] )
{
  int i;

  for ( i = 0; i < 3; i++ ) {
    v[i] = v[i] % m[i];

    if ( v[i] < 0 )
      v[i] += m[i];
  }
}

static void free_array2D_int( int **array,
			  const int num_row )
{
  int i;
  for ( i = 0; i < num_row; i++ ) {
    free( array[i] );
    array[i] = NULL;
  }
  free( array );
  array = NULL;
}

static int ** allocate_array2d_int( const int num_row,
				    const int num_column )
{
  int i;
  int **array = (int**)malloc(num_row * sizeof(int*));
  for (i = 0; i < num_row; i++) {
    array[i] = (int*)malloc(num_column * sizeof(int));
  }
  return array;
}
