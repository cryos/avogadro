/* spacegroup.c */
/* Copyright (C) 2010 Atsushi Togo */

#include <stdio.h>
#include <string.h>
#include "cell.h"
#include "hall_symbol.h"
#include "lattice.h"
#include "mathfunc.h"
#include "pointgroup.h"
#include "primitive.h"
#include "spacegroup.h"
#include "spg_database.h"
#include "symmetry.h"

#include "debug.h"

#define REDUCE_RATE 0.95

static Spacegroup get_spacegroup( SPGCONST Cell * primitive,
				  const double symprec );
static int get_hall_number( double origin_shift[3],
			    double conv_lattice[3][3],
			    Centering * centering,
			    double * tolerance,
			    SPGCONST Cell * primitive,
			    SPGCONST Symmetry * symmetry,
			    const double symprec );
static int get_hall_number_local( double origin_shift[3],
				  double conv_lattice[3][3],
				  Centering * centering,
				  SPGCONST Cell * primitive,
				  SPGCONST Symmetry * symmetry );
static Centering get_transformation_matrix( double trans_mat[3][3],
					    SPGCONST Symmetry * symmetry );
static Cell * get_primitive_with_smallest_lattice( SPGCONST Cell * cell,
						   const double symprec );
static Cell * get_cell_with_smallest_lattice( SPGCONST Cell * cell,
					      const double symprec );
static Symmetry * get_conventional_symmetry( SPGCONST double transform_mat[3][3],
					     const Centering centering,
					     const Symmetry *primitive_sym );

Spacegroup spa_get_spacegroup( SPGCONST Cell * cell,
			       const double symprec )
{
  Cell *primitive;
  Spacegroup spacegroup;

  primitive = get_primitive_with_smallest_lattice( cell, symprec );
  if ( primitive->size > -1 ) {
    spacegroup = get_spacegroup( primitive, symprec );
  } else {
    spacegroup.number = 0;
    warning_print("spglib: Space group could not be found ");
    warning_print("(line %d, %s).\n", __LINE__, __FILE__);
  }
  cel_free_cell( primitive );

  return spacegroup;
}

Spacegroup spa_get_spacegroup_with_primitive( SPGCONST Cell * primitive,
					      const double symprec )
{
  Cell *smallest_prim;
  Spacegroup spacegroup;

  smallest_prim = get_cell_with_smallest_lattice( primitive, symprec );
  if ( smallest_prim->size > 0 ) {
    spacegroup = get_spacegroup( primitive, symprec );
    cel_free_cell( smallest_prim );
  } else {
    spacegroup.number = 0;
    warning_print("spglib: Space group could not be found ");
    warning_print("(line %d, %s).\n", __LINE__, __FILE__);
  }
  return spacegroup;
}

Symmetry * spa_get_conventional_symmetry( SPGCONST double transform_mat[3][3],
					  const Centering centering,
					  const Symmetry *primitive_sym )
{
  return get_conventional_symmetry( transform_mat,
				    centering,
				    primitive_sym );
}

static Spacegroup get_spacegroup( SPGCONST Cell * primitive,
				  const double symprec )
{
  int hall_number;
  double tolerance;
  double conv_lattice[3][3];
  double origin_shift[3];
  Centering centering;
  Symmetry *symmetry;
  Spacegroup spacegroup;
  SpacegroupType spacegroup_type;

  symmetry = sym_get_operation( primitive, symprec );
  if ( symmetry->size == 0 ) {
    spacegroup.number = 0;
    warning_print("spglib: Space group could not be found ");
    warning_print("(line %d, %s).\n", __LINE__, __FILE__);
    goto ret;
  }

  hall_number = get_hall_number( origin_shift,
				 conv_lattice,
				 &centering,
				 &tolerance,
				 primitive,
				 symmetry,
				 symprec );

  if ( hall_number == 0 ) {
    spacegroup.number = 0;
    warning_print("spglib: Space group could not be found ");
    warning_print("(line %d, %s).\n", __LINE__, __FILE__);
    goto ret;
  }

  spacegroup_type = spgdb_get_spacegroup_type( hall_number );

  if ( spacegroup_type.number > 0 ) {
    mat_copy_matrix_d3( spacegroup.bravais_lattice, conv_lattice );
    mat_copy_vector_d3( spacegroup.origin_shift, origin_shift );
    spacegroup.number = spacegroup_type.number;
    spacegroup.hall_number = hall_number;
    spacegroup.holohedry = spacegroup_type.holohedry;
    spacegroup.centering = centering;
    strcpy(spacegroup.schoenflies,
	   spacegroup_type.schoenflies);
    strcpy(spacegroup.hall_symbol,
	   spacegroup_type.hall_symbol);
    strcpy(spacegroup.international,
	   spacegroup_type.international);
    strcpy(spacegroup.international_long,
	   spacegroup_type.international_full);
    strcpy(spacegroup.international_short,
	   spacegroup_type.international_short);
  } else {
    spacegroup.number = 0;
    warning_print("spglib: Space group could not be found ");
    warning_print("(line %d, %s).\n", __LINE__, __FILE__);
  }

 ret:
  /* spacegroup.number = 0 when space group was not found. */
  sym_free_symmetry( symmetry );
  return spacegroup;
}

static int get_hall_number( double origin_shift[3],
			    double conv_lattice[3][3],
			    Centering * centering,
			    double * tolerance,
			    SPGCONST Cell * primitive,
			    SPGCONST Symmetry * symmetry,
			    const double symprec )
{
  int pg_num, hall_number, attempt;
  Symmetry * sym_reduced;

  *tolerance = symprec;
  pg_num = ptg_get_pointgroup_number( symmetry );
  if ( pg_num > -1 ) {
    hall_number = get_hall_number_local( origin_shift,
					 conv_lattice,
					 centering,
					 primitive,
					 symmetry );
    if ( hall_number > 0 ) { goto ret; }
  }

  /* Reduce tolerance and search hall symbol again when hall symbol */
  /* could not be found by the given tolerance. */
  /* The situation this happens is that symmetry operations found */
  /* don't match any of hall symbol database due to tricky */
  /* displacements of atoms from the exact points. */
  for ( attempt = 0; attempt < 100; attempt++ ) {
    *tolerance *= REDUCE_RATE;
    sym_reduced = sym_reduce_operation( primitive, symmetry, *tolerance );
    pg_num = ptg_get_pointgroup_number( sym_reduced );
    if ( pg_num > -1 ) {
      hall_number = get_hall_number_local( origin_shift,
					   conv_lattice,
					   centering,
					   primitive,
					   sym_reduced );
      if ( hall_number > 0 ) {
	sym_free_symmetry( sym_reduced );
	break;
      }
    }
    sym_free_symmetry( sym_reduced );
  }

  debug_print("spglib: Tolerance was changed to %f ", *tolerance);

 ret:
  return hall_number;
}

static int get_hall_number_local( double origin_shift[3],
				  double conv_lattice[3][3],
				  Centering * centering,
				  SPGCONST Cell * primitive,
				  SPGCONST Symmetry * symmetry )
{
  int hall_number;
  double trans_mat[3][3];
  Symmetry * conv_symmetry;

  *centering = get_transformation_matrix( trans_mat, symmetry );
  mat_multiply_matrix_d3( conv_lattice,
			  primitive->lattice,
			  trans_mat );
  conv_symmetry = get_conventional_symmetry( trans_mat,
					     *centering,
					     symmetry );
  hall_number = hal_get_hall_symbol( origin_shift,
				     *centering,
				     conv_symmetry );
  sym_free_symmetry( conv_symmetry );

  return hall_number;
}

static Centering get_transformation_matrix( double trans_mat[3][3],
					    SPGCONST Symmetry * symmetry )
{
  int pg_num;
  double correction_mat[3][3];
  Centering centering;
  Pointgroup pointgroup;

  pg_num = ptg_get_pointgroup_number( symmetry );
  pointgroup = ptg_get_pointgroup( pg_num );
  ptg_get_transformation_matrix( &pointgroup, symmetry );

  /* Centering is not determined only from symmetry operations */
  /* sometimes. Therefore centering and transformation matrix are */
  /* related. */
  centering = lat_get_centering( correction_mat,
				 pointgroup.transform_mat,
				 pointgroup.laue );
  mat_multiply_matrix_id3( trans_mat,
			   pointgroup.transform_mat,
			   correction_mat );

  debug_print("correction matrix\n");
  debug_print_matrix_d3( correction_mat );

  return centering;
}

static Cell * get_primitive_with_smallest_lattice( SPGCONST Cell * cell,
						   const double symprec )
{
  VecDBL *pure_trans;
  Cell *primitive;

  pure_trans = sym_get_pure_translation( cell, symprec );
  
  if ( pure_trans->size > 1 ) {
    primitive = prm_get_primitive_with_pure_trans( cell,
						   pure_trans,
						   symprec );
  } else {
    primitive = get_cell_with_smallest_lattice( cell, symprec );
  }

  mat_free_VecDBL( pure_trans );

  return primitive;
}

static Cell * get_cell_with_smallest_lattice( SPGCONST Cell * cell,
					      const double symprec )
{
  int i, j;
  double min_lat[3][3], trans_mat[3][3], inv_lat[3][3];
  Cell * smallest_cell;

  if ( lat_smallest_lattice_vector( min_lat,
				    cell->lattice,
				    symprec ) ) {
    mat_inverse_matrix_d3( inv_lat, min_lat, 0 );
    mat_multiply_matrix_d3( trans_mat, inv_lat, cell->lattice );
    smallest_cell = cel_alloc_cell( cell->size );
    mat_copy_matrix_d3( smallest_cell->lattice, min_lat );
    for ( i = 0; i < cell->size; i++ ) {
      smallest_cell->types[i] = cell->types[i];
      mat_multiply_matrix_vector_d3( smallest_cell->position[i],
				     trans_mat, cell->position[i] );
      for ( j = 0; j < 3; j++ ) {
	cell->position[i][j] -= mat_Nint( cell->position[i][j] );
      }
    }
  } else {
    smallest_cell = cel_alloc_cell( -1 );
  }

  return smallest_cell;
}

static Symmetry * get_conventional_symmetry( SPGCONST double transform_mat[3][3],
					     const Centering centering,
					     const Symmetry *primitive_sym )
{
  int i, j, k, multi, size;
  double tmp_trans;
  double tmp_matrix_d3[3][3], shift[4][3];
  double symmetry_rot_d3[3][3], primitive_sym_rot_d3[3][3];
  Symmetry *symmetry;

  size = primitive_sym->size;

  if (centering == FACE) {
    symmetry = sym_alloc_symmetry(size * 4);
  }
  else {
    if (centering) {
      symmetry = sym_alloc_symmetry(size * 2);
    } else {
      symmetry = sym_alloc_symmetry(size);
    }
  }

  for (i = 0; i < size; i++) {
    mat_cast_matrix_3i_to_3d(primitive_sym_rot_d3, primitive_sym->rot[i]);

    /* C*S*C^-1: recover conventional cell symmetry operation */
    mat_get_similar_matrix_d3( symmetry_rot_d3,
			       primitive_sym_rot_d3,
			       transform_mat,
			       0 );
    mat_cast_matrix_3d_to_3i( symmetry->rot[i], symmetry_rot_d3 );

    /* translation in conventional cell: C = B^-1*P */
    mat_inverse_matrix_d3( tmp_matrix_d3,
			   transform_mat,
			   0 );
    mat_multiply_matrix_vector_d3( symmetry->trans[i], tmp_matrix_d3,
				   primitive_sym->trans[i] );
  }

  multi = 1;

  if (centering) {
    if (centering != FACE) {
      for (i = 0; i < 3; i++) {	shift[0][i] = 0.5; }
      if (centering == A_FACE) { shift[0][0] = 0; }
      if (centering == B_FACE) { shift[0][1] = 0; }
      if (centering == C_FACE) { shift[0][2] = 0; }

      multi = 2;
    }

    if (centering == FACE) {
      shift[0][0] = 0;
      shift[0][1] = 0.5;
      shift[0][2] = 0.5;
      shift[1][0] = 0.5;
      shift[1][1] = 0;
      shift[1][2] = 0.5;
      shift[2][0] = 0.5;
      shift[2][1] = 0.5;
      shift[2][2] = 0;

      multi = 4;
    }

    for (i = 0; i < multi - 1; i++) {
      for (j = 0; j < size; j++) {
	mat_copy_matrix_i3( symmetry->rot[(i+1) * size + j],
			    symmetry->rot[j] );
	for (k = 0; k < 3; k++) {
	  tmp_trans = symmetry->trans[j][k] + shift[i][k];
	  symmetry->trans[(i+1) * size + j][k] = tmp_trans;
	}
      }
    }
  }


  /* Reduce translations into -0 < trans < 1.0 */
  for (i = 0; i < multi; i++) {
    for (j = 0; j < size; j++) {
      for (k = 0; k < 3; k++) {
  	tmp_trans = symmetry->trans[i * size + j][k];
  	tmp_trans -= mat_Nint(tmp_trans);
  	if ( tmp_trans < 0 ) {
  	  tmp_trans += 1.0;
  	}
  	symmetry->trans[i * size + j][k] = tmp_trans;
      }
    }
  }

#ifdef DEBUG
  debug_print("Multi: %d\n", multi);
  debug_print("Centering: %d\n", centering);
  debug_print("sym size: %d\n", symmetry->size);
  
  for (i = 0; i < symmetry->size; i++) {
    debug_print("--- %d ---\n", i + 1);
    debug_print_matrix_i3(symmetry->rot[i]);
    debug_print("%f %f %f\n", symmetry->trans[i][0],
		symmetry->trans[i][1], symmetry->trans[i][2]);
  }
#endif

  return symmetry;
}

