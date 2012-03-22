/* symmetry.c */
/* Copyright (C) 2008 Atsushi Togo */

#include <stdio.h>
#include <stdlib.h>
#include "cell.h"
#include "debug.h"
#include "lattice.h"
#include "mathfunc.h"
#include "pointgroup.h"
#include "primitive.h"
#include "symmetry.h"

#include "debug.h"

static int relative_axes[][3] = {
  { 1, 0, 0},
  { 0, 1, 0},
  { 0, 0, 1},
  {-1, 0, 0},
  { 0,-1, 0}, /* 5 */
  { 0, 0,-1},
  { 0, 1, 1},
  { 1, 0, 1},
  { 1, 1, 0},
  { 0,-1,-1}, /* 10 */
  {-1, 0,-1},
  {-1,-1, 0},
  { 0, 1,-1},
  {-1, 0, 1},
  { 1,-1, 0}, /* 15 */
  { 0,-1, 1},
  { 1, 0,-1},
  {-1, 1, 0},
  { 1, 1, 1},
  {-1,-1,-1}, /* 20 */
  {-1, 1, 1},
  { 1,-1, 1},
  { 1, 1,-1},
  { 1,-1,-1},
  {-1, 1,-1}, /* 25 */
  {-1,-1, 1},
};

static int identity[3][3] = { { 1, 0, 0 },
			      { 0, 1, 0 },
			      { 0, 0, 1 } };

static int get_index_with_least_atoms( const Cell *cell );
static VecDBL * get_translation( SPGCONST int rot[3][3],
				 SPGCONST Cell *cell,
				 const double symprec );
static int get_operation( int rot[][3][3],
			  double trans[][3],
			  SPGCONST Cell * cell,
			  const double symprec );
static Symmetry * reduce_operation( SPGCONST Cell * cell,
				    SPGCONST Symmetry * symmetry,
				    const double symprec );
static int is_overlap_all_atoms( const double test_trans[3],
				 SPGCONST int rot[3][3],
				 SPGCONST Cell * cell,
				 const double symprec );
static PointSymmetry transform_pointsymmetry( SPGCONST PointSymmetry * point_sym_prim,
					      SPGCONST double new_lattice[3][3],
					      SPGCONST double original_lattice[3][3] );
static int get_space_group_operation( int rot[][3][3],
				      double trans[][3],
				      SPGCONST PointSymmetry *lattice_sym,
				      SPGCONST Cell *primitive,
				      const double symprec );
static int get_operation_supercell( int rot[][3][3],
				    double trans[][3],
				    const int num_sym, 
				    const VecDBL * pure_trans,
				    SPGCONST Cell *cell,
				    SPGCONST Cell *primitive );
static void set_axes( int axes[3][3],
		      const int a1, const int a2, const int a3 );
static PointSymmetry get_lattice_symmetry( SPGCONST Cell *cell,
					   const double symprec );



Symmetry * sym_alloc_symmetry( const int size )
{
  Symmetry *symmetry;

  symmetry = malloc( sizeof( Symmetry ) );
  symmetry->size = size;
  if ( size > 0 ) {
    if ((symmetry->rot =
	 (int (*)[3][3]) malloc(sizeof(int[3][3]) * size)) == NULL) {
      warning_print("spglib: Memory could not be allocated ");
      warning_print("(line %d, %s).\n", __LINE__, __FILE__);
      exit(1);
    }
    if ((symmetry->trans =
	 (double (*)[3]) malloc(sizeof(double[3]) * size)) == NULL) {
      warning_print("spglib: Memory could not be allocated ");
      warning_print("(line %d, %s).\n", __LINE__, __FILE__);
      exit(1);
    }
  }
  return symmetry;
}

void sym_free_symmetry( Symmetry *symmetry )
{
  if ( symmetry->size > 0 ) {
    free( symmetry->rot );
    free( symmetry->trans );
  }
  free( symmetry );
}

Symmetry * sym_get_operation( SPGCONST Cell *cell,
			      const double symprec ) {
  int i, j, num_sym;
  MatINT *rot;
  VecDBL *trans;
  Symmetry *symmetry;
  
  rot = mat_alloc_MatINT( cell->size * 48 );
  trans = mat_alloc_VecDBL( cell->size * 48 );

  num_sym = get_operation( rot->mat, trans->vec, cell, symprec );

#ifdef DEBUG
  debug_print("*** get_symmetry (found symmetry operations) *** \n");
  debug_print("Lattice \n");
  debug_print_matrix_d3(cell->lattice);
  for ( i = 0; i < num_sym; i++ ) {
    debug_print("--- %d ---\n", i + 1);
    debug_print_matrix_i3(rot->mat[i]);
    debug_print("%f %f %f\n",
  		trans->vec[i][0], trans->vec[i][1], trans->vec[i][2]);
  }
#endif
  
  symmetry = sym_alloc_symmetry( num_sym );
  for ( i = 0; i < num_sym; i++ ) {
    mat_copy_matrix_i3(symmetry->rot[i], rot->mat[i]);
    for (j = 0; j < 3; j++)
      symmetry->trans[i][j] = trans->vec[i][j];
  }

  mat_free_MatINT( rot );
  mat_free_VecDBL( trans );

  return symmetry;
}

Symmetry * sym_reduce_operation( SPGCONST Cell * cell,
				 SPGCONST Symmetry * symmetry,
				 const double symprec )
{
  return reduce_operation( cell, symmetry, symprec );
}

int sym_get_multiplicity( SPGCONST Cell *cell,
			  const double symprec )
{
  int multi;
  
  VecDBL * trans = get_translation(identity, cell, symprec);
  multi = trans->size;
  mat_free_VecDBL( trans );
  return multi;
}

VecDBL * sym_get_pure_translation( SPGCONST Cell *cell,
				   const double symprec )
{
  return get_translation(identity, cell, symprec);
}

/* 1) A primitive cell of the input cell is searched. */
/* 2) Pointgroup operations of the primitive cell are obtained. */
/*    These are constrained by the input cell lattice pointgroup, */
/*    i.e., even if the lattice of the primitive cell has higher */
/*    symmetry than that of the input cell, it is not considered. */
/* 3) Spacegroup operations are searched for the primitive cell */
/*    using the constrained point group operations. */
/* 4) The spacegroup operations for the primitive cell are */
/*    transformed to those of original input cells, if the input cell */
/*    was not a primitive cell. */
static int get_operation( int rot[][3][3],
			  double trans[][3],
			  SPGCONST Cell *cell,
			  const double symprec )
{
  int num_sym;
  int multi;
  PointSymmetry lattice_sym;
  Cell *primitive;
  VecDBL *pure_trans;

  pure_trans = sym_get_pure_translation(cell, symprec);
  multi = pure_trans->size;

  /* Lattice symmetry for input cell*/
  lattice_sym = get_lattice_symmetry( cell, symprec );
  if ( lattice_sym.size == 0 ) {
    goto err;
  }

  /* Obtain primitive cell */
  if( multi > 1 ) {
    primitive = prm_get_primitive_with_pure_trans( cell,
  						   pure_trans,
  						   symprec );
    if ( primitive->size < 1 ) { goto err; }

    lattice_sym = transform_pointsymmetry( &lattice_sym,
    					   primitive->lattice,
    					   cell->lattice );
    if ( lattice_sym.size == 0 ) { goto err; }
  } else {
    primitive = cell;
  }

  /* Symmetry operation search for primitive cell */
  num_sym = get_space_group_operation( rot, trans, &lattice_sym,
  				       primitive, symprec );

  /* Recover symmetry operation for the input structure (overwritten) */
  if( multi > 1 ) {
    num_sym = get_operation_supercell( rot,
  				       trans,
  				       num_sym,
  				       pure_trans,
  				       cell,
  				       primitive );
    cel_free_cell( primitive );
    if ( num_sym == 0 ) { goto err; }
  }

  mat_free_VecDBL( pure_trans );
  return num_sym;

 err:
  mat_free_VecDBL( pure_trans );
  return 0;
}

static Symmetry * reduce_operation( SPGCONST Cell * cell,
				    SPGCONST Symmetry * symmetry,
				    const double symprec )
{
  int i, j, num_sym;
  Symmetry * sym_reduced;
  PointSymmetry point_symmetry;
  MatINT *rot;
  VecDBL *trans;

  point_symmetry = get_lattice_symmetry( cell, symprec );
  rot = mat_alloc_MatINT( symmetry->size );
  trans = mat_alloc_VecDBL( symmetry->size );

  num_sym = 0;
  for ( i = 0; i < point_symmetry.size; i++ ) {
    for ( j = 0; j < symmetry->size; j++ ) {
      if ( mat_check_identity_matrix_i3( point_symmetry.rot[i],
					 symmetry->rot[j] ) ) {
	if ( is_overlap_all_atoms( symmetry->trans[j],
				   symmetry->rot[j],
				   cell,
				   symprec ) ) {
	  mat_copy_matrix_i3( rot->mat[num_sym], symmetry->rot[j] );
	  mat_copy_vector_d3( trans->vec[num_sym], symmetry->trans[j] );
	  num_sym++;
	}
      }
    }
  }

  sym_reduced = sym_alloc_symmetry( num_sym );
  for ( i = 0; i < num_sym; i++ ) {
    mat_copy_matrix_i3( sym_reduced->rot[i], rot->mat[i] );
    mat_copy_vector_d3( sym_reduced->trans[i], trans->vec[i] );
  }

  mat_free_MatINT( rot );
  mat_free_VecDBL( trans );

  return sym_reduced;
}

/* Look for the translations which satisfy the input symmetry operation. */
/* This function is heaviest in this code. */
static VecDBL * get_translation( SPGCONST int rot[3][3],
				 SPGCONST Cell *cell,
				 const double symprec )
{
  int i, j, min_atom_index, num_trans = 0;
  double test_trans[3], origin[3];
  VecDBL *tmp_trans, *trans;

  tmp_trans = mat_alloc_VecDBL( cell->size );

  /* Look for the atom index with least number of atoms within same type */
  min_atom_index = get_index_with_least_atoms( cell );

  /* Set min_atom_index as the origin to measure the distance between atoms. */
  mat_multiply_matrix_vector_id3(origin, rot, cell->position[min_atom_index]);

  for (i = 0; i < cell->size; i++) {	/* test translation */
    if (cell->types[i] != cell->types[min_atom_index]) {
      continue;
    }
    for (j = 0; j < 3; j++) {
      test_trans[j] = cell->position[i][j] - origin[j];
    }

    if ( is_overlap_all_atoms( test_trans, rot, cell, symprec ) ) {
      for (j = 0; j < 3; j++) {
	tmp_trans->vec[num_trans][j] =
	  test_trans[j] - mat_Nint( test_trans[j] );
      }
      num_trans++;
    }
  }

  trans = mat_alloc_VecDBL( num_trans );
  for ( i = 0; i < num_trans; i++ ) {
    mat_copy_vector_d3( trans->vec[i], tmp_trans->vec[i] );
  }

  mat_free_VecDBL( tmp_trans );
  return trans;
}

static int is_overlap_all_atoms( const double trans[3],
				 SPGCONST int rot[3][3],
				 SPGCONST Cell * cell,
				 const double symprec )
{
  int i, j, count;
  double pos_rot[3], test_pos[3];

  count = 0;

  /* test nonsymmorphic operation for an atom */
  for (i = 0; i < cell->size; i++) { 
    mat_multiply_matrix_vector_id3( pos_rot,
				    rot,
				    cell->position[i]);
    for (j = 0; j < 3; j++) { 
      test_pos[j] = pos_rot[j] + trans[j];
    }

    /* check overlap of atom_j and atom_k */
    for (j = 0; j < cell->size; j++) { 
      if ( cell->types[i] == cell->types[j] ) {
	if ( cel_is_overlap( cell->position[j],
			     test_pos,
			     cell->lattice,
			     symprec) ) {
	  count++;
	  break;
	}
      }
    }

    if (count < i + 1) {
      goto not_found;
    }
  }

  return 1;  /* found */

 not_found:
  return 0;
}

static int get_index_with_least_atoms( const Cell *cell )
{
  int i, j, min, min_index;
  int *mapping;
  mapping = (int *) malloc(sizeof(int) * cell->size);
  
  for ( i = 0; i < cell->size; i++ ) {
    mapping[i] = 0;
  }
  
  for ( i = 0; i < cell->size; i++ ) {
    for ( j = 0; j < cell->size; j++ ) {
      if ( cell->types[i] == cell->types[j] ) {
	mapping[j]++;
	break;
      }
    }
  }
  
  min = mapping[0];
  min_index = 0;
  for ( i = 0; i < cell->size; i++ ) {
    if ( min > mapping[i] && mapping[i] >0 ) {
      min = mapping[i];
      min_index = i;
    }
  }

  free(mapping);

  return min_index;
}

static int get_space_group_operation( int rot[][3][3],
				      double trans[][3],
				      SPGCONST PointSymmetry *lattice_sym,
				      SPGCONST Cell *cell,
				      const double symprec )
{
  int i, j, k, num_sym;
  VecDBL *tmp_trans;

  num_sym = 0;
  for (i = 0; i < lattice_sym->size; i++) {
    /* get translation corresponding to a rotation */
    tmp_trans = get_translation( lattice_sym->rot[i], cell, symprec );

    for (j = 0; j < tmp_trans->size; j++) {
      for (k = 0; k < 3; k++) {
	trans[num_sym + j][k] = tmp_trans->vec[j][k];
      }
      mat_copy_matrix_i3(rot[num_sym + j], lattice_sym->rot[i]);
    }
    num_sym += tmp_trans->size;
  }

  mat_free_VecDBL( tmp_trans );
  return num_sym;
}

static int get_operation_supercell( int rot[][3][3],
				    double trans[][3],
				    const int num_sym, 
				    const VecDBL * pure_trans,
				    SPGCONST Cell *cell,
				    SPGCONST Cell *primitive )
{
  int i, j, k, multi;
  double inv_prim_lat[3][3], drot[3][3], trans_mat[3][3], trans_mat_inv[3][3];
  MatINT *rot_prim;
  VecDBL *trans_prim;

  rot_prim = mat_alloc_MatINT( num_sym );
  trans_prim = mat_alloc_VecDBL( num_sym );
  multi = pure_trans->size;

  debug_print("get_operation_supercell\n");

  mat_inverse_matrix_d3( inv_prim_lat, primitive->lattice, 0 );
  mat_multiply_matrix_d3( trans_mat, inv_prim_lat, cell->lattice );
  mat_inverse_matrix_d3( trans_mat_inv, trans_mat, 0 );

  for( i = 0; i < num_sym; i++) {

    /* Translations  */
    mat_multiply_matrix_vector_d3( trans[i], trans_mat_inv, trans[i] );

    /* Rotations */
    mat_cast_matrix_3i_to_3d( drot, rot[i] );
    mat_get_similar_matrix_d3( drot, drot, trans_mat, 0 );
    mat_cast_matrix_3d_to_3i( rot[i], drot );
  }

  for( i = 0; i < num_sym; i++ ) {
    mat_copy_matrix_i3( rot_prim->mat[i], rot[i] );
    for( j = 0; j < 3; j++ )
      trans_prim->vec[i][j] = trans[i][j];
  }

  /* Rotations and translations are copied with the set of */
  /* pure translations. */
  for( i = 0; i < num_sym; i++ ) {
    for( j = 0; j < multi; j++ ) {
      mat_copy_matrix_i3( rot[ i * multi + j ], rot_prim->mat[i] );
      for ( k = 0; k < 3; k++ ) {
	trans[i * multi + j][k] =
	  mat_Dmod1( trans_prim->vec[i][k] + pure_trans->vec[j][k] );
      }
    }
  }

  mat_free_MatINT( rot_prim );
  mat_free_VecDBL( trans_prim );

  /* return number of symmetry operation of supercell */
  return num_sym * multi;
}

static PointSymmetry get_lattice_symmetry( SPGCONST Cell *cell,
					   const double symprec )
{
  int i, j, k, num_sym;
  int axes[3][3];
  double lattice[3][3], min_lattice[3][3];
  double metric[3][3], cell_metric[3][3];
  PointSymmetry lattice_sym;

  if ( ! lat_smallest_lattice_vector( min_lattice,
				      cell->lattice,
				      symprec ) ) {
    goto err;
  }
  mat_get_metric( cell_metric, min_lattice );

  num_sym = 0;
  for ( i = 0; i < 26; i++ ) {
    for ( j = 0; j < 26; j++ ) {
      for ( k = 0; k < 26; k++ ) {
	set_axes( axes, i, j, k );
	if ( ! ( ( mat_get_determinant_i3( axes ) == 1 ) ||
		 ( mat_get_determinant_i3( axes ) == -1 ) ) ) {
	  continue;
	}
	mat_multiply_matrix_di3( lattice, min_lattice, axes );
	mat_get_metric( metric, lattice );
	if ( mat_check_identity_matrix_d3( metric,
					   cell_metric,
					   symprec ) ) {
	  mat_copy_matrix_i3( lattice_sym.rot[num_sym], axes );
	  num_sym++;
	}
	if ( num_sym > 48 ) {
	  warning_print("spglib: Too many lattice symmetries was found.\n");
	  warning_print("        Tolerance may be too large ");
	  warning_print("(line %d, %s).\n", __LINE__, __FILE__);
	  goto err;
	}
      }
    }
  }

  lattice_sym.size = num_sym;
  return transform_pointsymmetry( &lattice_sym,
				  cell->lattice,
				  min_lattice );
  
 err:
  lattice_sym.size = 0;
  return lattice_sym;
}

static PointSymmetry transform_pointsymmetry( SPGCONST PointSymmetry * lat_sym_orig,
					      SPGCONST double new_lattice[3][3],
					      SPGCONST double original_lattice[3][3] )
{
  int i, size;
  double trans_mat[3][3], inv_mat[3][3], drot[3][3];
  PointSymmetry lat_sym_new;

  mat_inverse_matrix_d3( inv_mat, original_lattice, 0 );
  mat_multiply_matrix_d3( trans_mat, inv_mat, new_lattice );

  size = 0;
  for ( i = 0; i < lat_sym_orig->size; i++ ) {
    mat_cast_matrix_3i_to_3d( drot, lat_sym_orig->rot[i] );
    mat_get_similar_matrix_d3( drot, drot, trans_mat, 0 );

    /* new_lattice may have lower point symmetry than original_lattice.*/
    /* The operations that have non-integer elements are not counted. */
    if ( mat_is_int_matrix( drot, mat_Dabs( mat_get_determinant_d3( trans_mat ) ) / 10 ) ) {
      mat_cast_matrix_3d_to_3i( lat_sym_new.rot[size], drot );
      if ( ! abs( mat_get_determinant_i3( lat_sym_new.rot[size] ) ) == 1 ) {
	warning_print("spglib: A point symmetry operation is not unimodular.");
	warning_print("(line %d, %s).\n", __LINE__, __FILE__);
	goto err;
      }
      size++;
    }
  }

#ifdef SPGWARNING
  if ( ! ( lat_sym_orig->size == size ) ) {
    warning_print("spglib: Some of point symmetry operations were dropted.");
    warning_print("(line %d, %s).\n", __LINE__, __FILE__);
  }
#endif

  lat_sym_new.size = size;
  return lat_sym_new;

 err:
  lat_sym_new.size = 0;
  return lat_sym_new;
}

static void set_axes( int axes[3][3],
		      const int a1, const int a2, const int a3 )
{
  int i;
  for ( i = 0; i < 3; i++ ) { axes[i][0] = relative_axes[a1][i]; }
  for ( i = 0; i < 3; i++ ) { axes[i][1] = relative_axes[a2][i]; }
  for ( i = 0; i < 3; i++ ) { axes[i][2] = relative_axes[a3][i]; }
}
