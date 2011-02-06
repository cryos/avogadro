/* spacegroup_type.c */
/* Copyright (C) 2008 Atsushi Togo */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spacegroup_type.h"
#include "spg_database.h"
#include "bravais.h"
#include "bravais_virtual.h"
#include "cell.h"
#include "mathfunc.h"
#include "pointgroup.h"
#include "primitive.h"
#include "symmetry.h"
#include "hall_symbol.h"

#include "debug.h"

Spacegroup typ_get_spacegroup( SPGCONST Cell * cell,
			       const double symprec )
{
  int hall_number;
  double origin_shift[3];
  VecDBL *pure_trans;
  Bravais bravais;
  Cell *primitive;
  Holohedry holohedry;
  Symmetry *symmetry, *conv_symmetry;
  Pointgroup pointgroup;
  Spacegroup spacegroup;
  SpacegroupType spacegroup_type;

#ifdef DEBUG
  int i;
  printf("Input structure\n");
  printf("lattice:\n");
  debug_print_matrix_d3( cell->lattice );
  debug_print("Metric tensor:\n");
  double metric[3][3];
  mat_get_metric( metric, cell->lattice );
  debug_print_matrix_d3( metric );
  printf("positions:\n");
  for ( i = 0; i < cell->size; i++ ) {
    printf("%f %f %f\n",
	   cell->position[i][0],
	   cell->position[i][1],
	   cell->position[i][2]);
  }
#endif

  /* get primitive cell */
  pure_trans = sym_get_pure_translation( cell, symprec );
  
  if ( pure_trans->size > 1) {

    debug_print("Cell is not primitive.\n");
    primitive = prm_get_primitive_with_pure_trans( cell,
						   pure_trans,
						   symprec );
    if ( primitive->size == 0 ) {
      spacegroup.number = 0;
    } else {
      spacegroup = typ_get_spacegroup( primitive, symprec );
      cel_free_cell( primitive );
    }
    mat_free_VecDBL( pure_trans );
    goto ret;
  }

  mat_free_VecDBL( pure_trans );


  bravais = brv_get_brv_lattice( cell->lattice, symprec );
  symmetry = sym_get_operation( &bravais, cell, symprec );
  /* Get correct Bravais lattice including internal symmetry */
  /* 'symmetry' may be changed along point group constraint.. */
  pointgroup = ptg_get_symmetry_pointgroup( symmetry );

  holohedry = pointgroup.holohedry;
  /* When holohedry from lattice doesn't correspond to holohedry   */
  /* determined by pointgroup, careful treatment is required. */
  if (holohedry < bravais.holohedry) {
    if ( ! ( art_get_artificial_bravais( &bravais, symmetry,
  					 cell, holohedry, symprec ) ) ) {
      spacegroup.number = 0;
      goto end;
    }
  }

  /* /\* This is new implementation of symmetry operation finder. *\/ */
  /* /\* Symmetry operation finder works perfectly, *\/ */
  /* /\* however virtual structure treatment is not implemented. *\/ */
  /* /\* Therefore space group type determination is not perfect. *\/ */
  /* symmetry = sym_get_operation_direct( cell, symprec ); */
  /* pointgroup = ptg_get_pointgroup( symmetry ); */
  /* bravais.holohedry = pointgroup.holohedry; */
  /* brv_set_relative_lattice(); */
  /* if ( ! brv_get_brv_lattice_in_loop( &bravais, */
  /* 				      cell->lattice, */
  /* 				      symprec ) ) { */
  /*   spacegroup.number = 0; */
  /*   goto end; */
  /* } */

  conv_symmetry = typ_get_conventional_symmetry( &bravais,
						 cell->lattice,
						 symmetry,
						 symprec );

  hall_number = hal_get_hall_symbol( origin_shift,
				     &bravais,
				     conv_symmetry,
				     symprec );
  sym_free_symmetry( conv_symmetry );

  spacegroup_type = spgdb_get_spacegroup_type( hall_number );
  
  debug_print("Hall number: %d\n", hall_number);
  debug_print("Spacegroup number: %d\n", spacegroup_type.number);
  
  if ( hall_number > 0 ) {
    mat_copy_matrix_d3(spacegroup.bravais_lattice, bravais.lattice);
    spacegroup.number = spacegroup_type.number;
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
    fprintf(stderr, "spglib BUG: Space group could not be found.\n");
  }

 end:
  sym_free_symmetry( symmetry );

 ret:
  return spacegroup;
}

Symmetry * typ_get_conventional_symmetry( SPGCONST Bravais *bravais,
					  SPGCONST double primitive_lattice[3][3],
					  const Symmetry *primitive_sym,
					  const double symprec )
{
  int i, j, k, multi, size;
  Centering centering;
  Holohedry holohedry;
  double tmp_trans;
  double coordinate_inv[3][3], tmp_matrix_d3[3][3], shift[4][3];
  double symmetry_rot_d3[3][3], primitive_sym_rot_d3[3][3];
  Symmetry *symmetry;

  centering = bravais->centering;
  holohedry = bravais->holohedry;
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

  /* C^-1 = P^-1*B */
  mat_inverse_matrix_d3(tmp_matrix_d3, primitive_lattice, symprec);
  mat_multiply_matrix_d3(coordinate_inv, tmp_matrix_d3, bravais->lattice);

  for (i = 0; i < size; i++) {
    mat_cast_matrix_3i_to_3d(primitive_sym_rot_d3, primitive_sym->rot[i]);

    /* C*S*C^-1: recover conventional cell symmetry operation */
    mat_get_similar_matrix_d3( symmetry_rot_d3, primitive_sym_rot_d3,
			       coordinate_inv, symprec );

    mat_cast_matrix_3d_to_3i( symmetry->rot[i], symmetry_rot_d3 );

    /* translation in conventional cell: C = B^-1*P */
    mat_inverse_matrix_d3(tmp_matrix_d3, coordinate_inv, symprec);
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
  	if ( tmp_trans < -symprec ) {
  	  tmp_trans += 1.0;
  	}
  	symmetry->trans[i * size + j][k] = tmp_trans;
      }
    }
  }

#ifdef DEBUG
  debug_print("*** get_conventional_symmetry ***\n");
  debug_print("--- Bravais lattice ---\n");
  debug_print_matrix_d3(bravais->lattice);
  debug_print("--- Prim lattice ---\n");
  debug_print_matrix_d3(primitive_lattice);
  debug_print("--- Coordinate inv ---\n");
  debug_print_matrix_d3(coordinate_inv);
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
