/* bravais_art.c */
/* Copyright (C) 2008 Atsushi Togo */

#include <stdio.h>
#include <stdlib.h>
#include "bravais.h"
#include "bravais_virtual.h"
#include "cell.h"
#include "mathfunc.h"
#include "spacegroup_type.h"
#include "symmetry.h"

#include "debug.h"

static double bcc_axes[13][3] = {
  { 1.0, 0.0, 0.0},
  { 0.0, 1.0, 0.0},
  { 0.0, 0.0, 1.0},
  { 0.0, 1.0, 1.0},
  { 0.0, 1.0,-1.0},
  { 1.0, 0.0, 1.0},
  {-1.0, 0.0, 1.0},
  { 1.0, 1.0, 0.0},
  { 1.0,-1.0, 0.0},
  {-0.5, 0.5, 0.5},
  { 0.5,-0.5, 0.5},
  { 0.5, 0.5,-0.5},
  { 0.5, 0.5, 0.5},
};

static double fcc_axes[13][3] = {
  { 1.0, 0.0, 0.0},
  { 0.0, 1.0, 0.0},
  { 0.0, 0.0, 1.0},
  { 0.0, 0.5, 0.5},
  { 0.0, 0.5,-0.5},
  { 0.5, 0.0, 0.5},
  {-0.5, 0.0, 0.5},
  { 0.5, 0.5, 0.0},
  { 0.5,-0.5, 0.0},
  { 1.0, 1.0, 1.0},
  {-1.0, 1.0, 1.0},
  { 1.0,-1.0, 1.0},
  { 1.0, 1.0,-1.0},
};

static double primitive_axes[13][3] = {
  { 1.0, 0.0, 0.0},
  { 0.0, 1.0, 0.0},
  { 0.0, 0.0, 1.0},
  { 0.0, 1.0, 1.0},
  { 0.0, 1.0,-1.0},
  { 1.0, 0.0, 1.0},
  {-1.0, 0.0, 1.0},
  { 1.0, 1.0, 0.0},
  { 1.0,-1.0, 0.0},
  { 1.0, 1.0, 1.0},
  {-1.0, 1.0, 1.0},
  { 1.0,-1.0, 1.0},
  { 1.0, 1.0,-1.0},
};

static int rot_axes[13][3] = {
  { 1, 0, 0},
  { 0, 1, 0},
  { 0, 0, 1},
  { 0, 1, 1},
  { 0,-1, 1},
  { 1, 0, 1},
  {-1, 0, 1},
  { 1, 1, 0},
  {-1, 1, 0},
  {-1, 1, 1},
  { 1,-1, 1},
  { 1, 1,-1},
  { 1, 1, 1},
};

static int get_rotation_axis( SPGCONST int rot[3][3],
			      const int axis_num );
static int is_monocli( Bravais *bravais,
		       const Symmetry *conv_sym,
		       const double symprec );
static int is_monocli_from_I( Bravais *bravais,
			      const Symmetry *conv_sym,
			      const double symprec );
static int is_monocli_from_F( Bravais *bravais,
			      const Symmetry *conv_sym,
			      const double symprec );
static int is_monocli_from_P( Bravais *bravais,
			      const Symmetry *conv_sym,
			      const double symprec );
static int get_monocli_bravais( double lattice[3][3],
				const Symmetry *conv_sym,
				const double relative_vol,
				SPGCONST double monocli_axes[13][3],
				const double symprec );
static void get_monocli_bcc_to_c_center( double lattice[3][3] );
static int is_monocli_orthogonal( const int b_axis,
				  const int naxis,
				  SPGCONST double monocli_axes[13][3],
				  const double symprec );
static void get_monocli_relative_axes( double relative_axis[3][3],
				       SPGCONST double new_lattice[3][3],
				       SPGCONST double old_lattice[3][3],
				       const double symprec );
static int is_tetra( Bravais *bravais,
		     const Symmetry *conv_sym );
static int is_rhombo( Bravais *bravais,
		      const Symmetry *conv_sym,
		      const double symprec );
static int is_ortho( Bravais *bravais,
		     const Symmetry *conv_sym,
		     const double symprec );
static int is_ortho_from_H( Bravais *bravais,
			    SPGCONST Cell *cell,
			    const Symmetry *symmetry,
			    const double symprec );
static int is_ortho_from_H_axis( SPGCONST Bravais *bravais,
				 SPGCONST Cell *cell,
				 const Symmetry *symmetry,
				 const double symprec );
static int is_ortho_from_I( Bravais *bravais,
			    const Symmetry *conv_sym,
			    const double symprec );
static int is_ortho_from_F( Bravais *bravais,
			    const Symmetry *conv_sym,
			    const double symprec );
static int is_ortho_from_P( Bravais *bravais,
			    const Symmetry *conv_sym );
static int get_ortho_axis( int naxis[3],
			   const Symmetry *conv_sym );

/* bravais is going to be changed. */
int art_get_artificial_bravais( Bravais *bravais,
				const Symmetry *symmetry,
				SPGCONST Cell *cell,
				const Holohedry holohedry,
				const double symprec )
{
  int i, j;
  Symmetry *conv_sym;

  /* Triogonal */
  if (holohedry == TRIGO && bravais->holohedry == HEXA) {
    /* Just change the holohedry. */
    goto found;
  }

  /* Rhombohedral */
  if (holohedry == TRIGO && bravais->holohedry == RHOMB) {
    /* Do nothing */
    goto end;
  }

  /* Orthorhombic from Hexagonal */
  if (holohedry == ORTHO && bravais->holohedry == HEXA ) {
    if (is_ortho_from_H(bravais, cell, symmetry, symprec)) {
      goto end;
    }
    goto not_found;
  }

  conv_sym = typ_get_conventional_symmetry( bravais, cell->lattice,
					    symmetry, symprec );

  /* Rhombohedral from Cubic */
  if (holohedry == TRIGO && bravais->holohedry == CUBIC ) {
    if (is_rhombo(bravais, conv_sym, symprec)) {
      sym_free_symmetry( conv_sym );
      bravais->holohedry = RHOMB;
      goto end;
    }
    goto not_found_and_deallocate;
  }

  /* Monoclinic */
  if ( holohedry == MONOCLI ) {
    if (is_monocli(bravais, conv_sym, symprec)) {
      goto found_and_deallocate;
    }
    goto not_found_and_deallocate;
  }

  /* Tetragonal */
  if (holohedry == TETRA) {
    if ( is_tetra( bravais, conv_sym ) ) {
      goto found_and_deallocate;
    }
    goto not_found_and_deallocate;
  }

  /* Orthorhombic */
  if (holohedry == ORTHO) {
    if ( is_ortho( bravais, conv_sym, symprec ) ) {
      goto found_and_deallocate;
    }

    goto not_found_and_deallocate;
  }

  /* Triclinic */
  if ( holohedry == TRICLI ) {
    mat_copy_matrix_d3( bravais->lattice, cell->lattice );
    bravais->centering = NO_CENTER;
    goto found_and_deallocate;
  }

  goto not_found_and_deallocate;

  /*************/
  /*** Found ***/
  /*************/

 found_and_deallocate:
  sym_free_symmetry( conv_sym );
 found:
  bravais->holohedry = holohedry;

 end:
  /* Check if right hand system  */
  if (mat_get_determinant_d3(bravais->lattice) < -symprec) {
    for (i = 0; i < 3; i++) { 
      for (j = 0; j < 3; j++) {
	bravais->lattice[i][j] = -bravais->lattice[i][j];
      }
    }
  }
  return 1;

  /*****************/
  /*** Not found ***/
  /*****************/

 not_found_and_deallocate:
  sym_free_symmetry( conv_sym );
  
 not_found:
  return 0;
}

static int is_rhombo( Bravais *bravais,
		      const Symmetry *conv_sym,
		      const double symprec )
{
  int i, j, k, l, naxis;
  double transform_matrix[3][3] = {
    { 1.0, 0.0, 0.0},
    { 0.0, 1.0, 0.0},
    { 0.0, 0.0, 1.0}
  };
  int bcc_basis[8][3] = {
    { 1, 1, 1},
    {-1, 1, 1},
    { 1,-1, 1},
    { 1, 1,-1},
    {-1,-1,-1},
    { 1,-1,-1},
    {-1, 1,-1},
    {-1,-1, 1},
  };
  int fcc_basis[6][3] = {
    { 0, 1, 1},
    { 1, 0, 1},
    { 1, 1, 0},
    { 0,-1,-1},
    {-1, 0,-1},
    {-1,-1, 0},
  };

  /* 'axis' has to be 9, 10, 11, or 12 in rot_axes. */
  for ( i = 0; i < conv_sym->size; i++ ) {
    naxis = get_rotation_axis(conv_sym->rot[i], 3);
    if ( naxis > 8 ) {
      break;
    }
  }

  if ( naxis < 9 ) {
    fprintf(stderr, "spglib: BUG in spglib in __LINE__, __FILE__.");
    goto err;
  }

  if (bravais->centering == BODY) {
    for ( i = 0; i < 8; i++ ) {
      for ( j = i+1; j < 8; j++ ) {
	for ( k = j+1; k < 8; k++ ) {
	  if ( bcc_basis[i][0] +
	       bcc_basis[j][0] +
	       bcc_basis[k][0] == rot_axes[naxis][0] &&
	       bcc_basis[i][1] +
	       bcc_basis[j][1] +
	       bcc_basis[k][1] == rot_axes[naxis][1] &&
	       bcc_basis[i][2] +
	       bcc_basis[j][2] +
	       bcc_basis[k][2] == rot_axes[naxis][2] ) {
	    for ( l = 0; l < 3; l++ ) {
	      transform_matrix[l][0] = ((double) bcc_basis[i][l]) / 2;
	      transform_matrix[l][1] = ((double) bcc_basis[j][l]) / 2;
	      transform_matrix[l][2] = ((double) bcc_basis[k][l]) / 2;
	    }
	    if ( mat_Dabs( mat_get_determinant_d3( transform_matrix ) ) > symprec ) {
	      goto found;
	    }
	  }
	}
      }
    }
  }  

  if (bravais->centering == FACE) {
    for ( i = 0; i < 6; i++ ) {
      for ( j = i+1; j < 6; j++ ) {
	for ( k = j+1; k < 6; k++ ) {
	  if ( fcc_basis[i][0] +
	       fcc_basis[j][0] +
	       fcc_basis[k][0] == 2*rot_axes[naxis][0] &&
	       fcc_basis[i][1] +
	       fcc_basis[j][1] +
	       fcc_basis[k][1] == 2*rot_axes[naxis][1] &&
	       fcc_basis[i][2] +
	       fcc_basis[j][2] +
	       fcc_basis[k][2] == 2*rot_axes[naxis][2] ) {
	    for ( l = 0; l < 3; l++ ) {
	      transform_matrix[l][0] = ((double) fcc_basis[i][l]) / 2;
	      transform_matrix[l][1] = ((double) fcc_basis[j][l]) / 2;
	      transform_matrix[l][2] = ((double) fcc_basis[k][l]) / 2;
	    }
	    if ( mat_Dabs( mat_get_determinant_d3( transform_matrix ) ) > symprec ) {
	      goto found;
	    }
	  }
	}
      }
    }
  }

  /* NO_CENTER */
  for ( i = 0; i < 3; i++ ) {
    transform_matrix[i][i] *= rot_axes[naxis][i];
  }

 found:
  mat_multiply_matrix_d3( bravais->lattice, bravais->lattice,
			  transform_matrix );
  bravais->holohedry = RHOMB;
  bravais->centering = NO_CENTER;

  /* Swap if determinant < 0 */
  if ( mat_get_determinant_d3( bravais->lattice ) < symprec ) {
    mat_copy_matrix_d3( transform_matrix, bravais->lattice );
    for ( i = 0; i < 3; i++ ) {
      bravais->lattice[i][1] = transform_matrix[i][2];
      bravais->lattice[i][2] = transform_matrix[i][1];
    }
  }

  return 1;

 err:
  return 0;
}

static int is_ortho( Bravais *bravais,
		     const Symmetry *conv_sym,
		     const double symprec )
{
  /* I-Ortho and F-Ortho from I-Cbuic or I-Tetragonal */
  if (bravais->centering == BODY) {
    if (is_ortho_from_I(bravais, conv_sym, symprec))
      goto found;
  }  

  if (bravais->centering == FACE) {
    if (is_ortho_from_F(bravais, conv_sym, symprec))
      goto found;
  }

  if (bravais->centering == NO_CENTER) {
    if (is_ortho_from_P(bravais, conv_sym))
      goto found;
  }

  return 0;

 found:
  return 1;
}

static int is_ortho_from_H( Bravais *bravais,
			    SPGCONST Cell *cell,
			    const Symmetry *symmetry,
			    const double symprec )
{
  int i;
  double bravais_lattice[3][3];

  mat_copy_matrix_d3(bravais_lattice, bravais->lattice);
  bravais->holohedry = ORTHO;
  bravais->centering = C_FACE;

  /* Try three kinds of C-base orthorhombic lattice */
  /* Type 1 (no rotation) */
  for ( i = 0; i < 3; i++ ) {
    bravais->lattice[i][0] = bravais_lattice[i][0] - bravais_lattice[i][1];
    bravais->lattice[i][1] = bravais_lattice[i][0] + bravais_lattice[i][1];
    bravais->lattice[i][2] = bravais_lattice[i][2];
  }
  if ( is_ortho_from_H_axis(bravais, cell, symmetry, symprec) )
    goto found;

  /* Type 2 (60 degs) */
  for ( i = 0; i < 3; i++ ) {
    bravais->lattice[i][0] = 2* bravais_lattice[i][0] + bravais_lattice[i][1];
    bravais->lattice[i][1] = bravais_lattice[i][1];
    bravais->lattice[i][2] = bravais_lattice[i][2];
  }
  if ( is_ortho_from_H_axis(bravais, cell, symmetry, symprec) )
    goto found;

  /* Type 3 (120 degs) */
  for ( i = 0; i < 3; i++ ) {
    bravais->lattice[i][0] = bravais_lattice[i][0] + 2 * bravais_lattice[i][1];
    bravais->lattice[i][1] = - bravais_lattice[i][0];
    bravais->lattice[i][2] = bravais_lattice[i][2];
  }
  if ( is_ortho_from_H_axis(bravais, cell, symmetry, symprec) )
    goto found;


  /* Not found */
  mat_copy_matrix_d3(bravais->lattice, bravais_lattice);
  return 0;

  /* Found */
 found:
  return 1;
}

static int is_ortho_from_H_axis( SPGCONST Bravais *bravais,
				 SPGCONST Cell *cell,
				 const Symmetry *symmetry,
				 const double symprec )
{
  int naxis[3];
  Symmetry *conv_sym;

  conv_sym = typ_get_conventional_symmetry( bravais, cell->lattice,
					    symmetry, symprec );

  if ( get_ortho_axis( naxis, conv_sym ) ) {
    /* Found */
    sym_free_symmetry( conv_sym );
    return 1;
  }

  /* Not found */
  sym_free_symmetry( conv_sym );
  return 0;
}

static int get_ortho_axis( int naxis[3],
			   const Symmetry *conv_sym )
{
  int i, tmp_naxis, num_ortho_axis = 0;

  for (i = 0; i < conv_sym->size; i++) {
    tmp_naxis = get_rotation_axis(conv_sym->rot[i], 2);

    if (num_ortho_axis == 0) {
      if (tmp_naxis > -1) {
	naxis[0] = tmp_naxis;
	num_ortho_axis++;
      }
    }

    if (num_ortho_axis == 1) {
      if (tmp_naxis != naxis[0]) {
	naxis[1] = tmp_naxis;
	num_ortho_axis++;
      }
    }

    if (num_ortho_axis == 2) {
      if (tmp_naxis != naxis[0] && tmp_naxis != naxis[1]) {
	naxis[2] = tmp_naxis;
	num_ortho_axis++;
      }
    }

    if (num_ortho_axis > 2) {
      debug_print("Ortho axes(%d): %d %d %d\n", num_ortho_axis, naxis[0], naxis[1], naxis[2]);
      return 1;
    }
  }

  return 0;
}

static int is_ortho_from_I( Bravais *bravais,
			    const Symmetry *conv_sym,
			    const double symprec )
{
  int i, j, naxis[3];
  double relative_axis[3][3];

  if (! get_ortho_axis(naxis, conv_sym))
    goto not_found;

  /* Each axis has at least 2-fold rotation. */
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      relative_axis[j][i] = bcc_axes[naxis[i]][j];

  debug_print("is_ortho_from_I\n");
  debug_print_matrix_d3(relative_axis);
  mat_multiply_matrix_d3(bravais->lattice, bravais->lattice, relative_axis);

  /* F case */
  if (mat_Dabs(mat_Dabs(mat_get_determinant_d3(relative_axis)) - 2.0) < symprec) {
    bravais->centering = FACE;
  }
  
  return 1;

 not_found:
  return 0;
}

static int is_ortho_from_F( Bravais *bravais,
			    const Symmetry *conv_sym,
			    const double symprec )
{
  int i, j, naxis[3];
  double relative_axis[3][3];

  if (! get_ortho_axis(naxis, conv_sym))
    goto not_found;
      
  /* Each axis has at least 2-fold rotation. */
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      relative_axis[j][i] = fcc_axes[naxis[i]][j];

  debug_print("is_ortho_from_F\n");
  debug_print_matrix_d3(relative_axis);
  mat_multiply_matrix_d3(bravais->lattice, bravais->lattice, relative_axis);
  
  /* I case */
  if (mat_Dabs(mat_Dabs(mat_get_determinant_d3(relative_axis)) - 0.5) < symprec) {
    bravais->centering = BODY;
  }

  return 1;

 not_found:
  return 0;
}

static int is_ortho_from_P( Bravais *bravais,
			    const Symmetry *conv_sym )
{
  int i, j, naxis[3];
  double relative_axis[3][3];

  if (! get_ortho_axis(naxis, conv_sym))
    goto not_found;
      
  /* Each axis has at least 2-fold rotation. */
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      relative_axis[j][i] = primitive_axes[naxis[i]][j];
  mat_multiply_matrix_d3(bravais->lattice, bravais->lattice, relative_axis);
  if (naxis[0] < 3 &&  naxis[1] > 2 && naxis[2] > 2)
    bravais->centering = A_FACE;
  if (naxis[0] > 2 &&  naxis[1] < 3 && naxis[2] > 2)
    bravais->centering = B_FACE;
  if (naxis[0] > 2 &&  naxis[1] > 2 && naxis[2] < 3)
    bravais->centering = C_FACE;

  debug_print("is_ortho_from_P\n");
  debug_print_matrix_d3(relative_axis);

  return 1;

 not_found:
  return 0;
}

static int is_tetra( Bravais *bravais,
		     const Symmetry *conv_sym )
{
  int i, tmp_naxis = -1;
  double permutate_axis_a[3][3] = {
    { 0.0, 0.0, 1.0},
    { 1.0, 0.0, 0.0},
    { 0.0, 1.0, 0.0},
  };
  double permutate_axis_b[3][3] = {
    { 0.0, 1.0, 0.0},
    { 0.0, 0.0, 1.0},
    { 1.0, 0.0, 0.0},
  };
  double permutate_axis_F_a[3][3] = {
    { 0.0, 0.0, 1.0},
    { 0.5, 0.5, 0.0},
    {-0.5, 0.5, 0.0},
  };
  double permutate_axis_F_b[3][3] = {
    {-0.5, 0.5, 0.0},
    { 0.0, 0.0, 1.0},
    { 0.5, 0.5, 0.0},
  };
  double permutate_axis_F_c[3][3] = {
    { 0.5, 0.5, 0.0},
    {-0.5, 0.5, 0.0},
    { 0.0, 0.0, 1.0},
  };

  /* Get 4 fold axis */
  for (i = 0; i < conv_sym->size; i++) {
    tmp_naxis = get_rotation_axis(conv_sym->rot[i], 4);
    if (tmp_naxis > -1)
      break;
  }

  if (tmp_naxis < 0)
    goto not_found;

  /* I from F-Cubic */
  if (bravais->centering == FACE) {
    if (tmp_naxis == 0 || tmp_naxis == 1 || tmp_naxis == 2) {
      if (tmp_naxis == 0) { /* b c a*/
	mat_multiply_matrix_d3(bravais->lattice, bravais->lattice, permutate_axis_F_a);
      }
      if (tmp_naxis == 1) { /* c a b*/
	mat_multiply_matrix_d3(bravais->lattice, bravais->lattice, permutate_axis_F_b);
      }
      if (tmp_naxis == 2) { /* a b c*/
	mat_multiply_matrix_d3(bravais->lattice, bravais->lattice, permutate_axis_F_c);
      }
      
      bravais->centering = BODY;
      goto found;
    }
  }

  /* P or I: just exchange the 4-fold axis to c-axis */
  if (bravais->centering != FACE) {
    if (tmp_naxis == 0 || tmp_naxis == 1 || tmp_naxis == 2) {
      if (tmp_naxis == 0) { /* c a b*/
	mat_multiply_matrix_d3(bravais->lattice, bravais->lattice, permutate_axis_a);
      }
      if (tmp_naxis == 1) { /* b c a*/
	mat_multiply_matrix_d3(bravais->lattice, bravais->lattice, permutate_axis_b);
      }

      debug_print("is_tetra\n");
      debug_print_matrix_d3(bravais->lattice);
      goto found;
    }
  }

 not_found: /* This may not happen. */
  return 0;

 found:
  return 1;
}

static int is_monocli( Bravais *bravais,
		       const Symmetry *conv_sym,
		       const double symprec )
{
  /* Monoclinic from I-cubic, I-tetra, and I-ortho */
  if (bravais->centering == BODY) {
    debug_print("Monoclinic from I\n");
    if (is_monocli_from_I(bravais, conv_sym, symprec))
      goto found;
  }

  /* Monoclinic from F-cubic */
  if (bravais->centering == FACE) {
    debug_print("Monoclinic from F\n");
    if (is_monocli_from_F(bravais, conv_sym, symprec))
      goto found;
  }

  /* Monoclinic from P */
  if (bravais->centering == NO_CENTER || A_FACE || B_FACE || C_FACE) {
    debug_print("Monoclinic from P, A, B, C\n");
    if (is_monocli_from_P(bravais, conv_sym, symprec))
      goto found;
  }

  return 0;

 found:
  return 1;
}

static int is_monocli_from_I( Bravais *bravais,
			      const Symmetry *conv_sym,
			      const double symprec )
{
  double lattice[3][3], relative_axis[3][3];

  mat_copy_matrix_d3(lattice, bravais->lattice);
  
  /* base center */
  if (get_monocli_bravais(bravais->lattice, conv_sym, 1.0, bcc_axes, symprec)) {
    get_monocli_relative_axes(relative_axis, bravais->lattice, lattice, symprec);
    if (mat_Dabs(mat_Dabs(relative_axis[0][0]+relative_axis[0][1]) - 1.0) < symprec &&
	mat_Dabs(mat_Dabs(relative_axis[1][0]+relative_axis[1][1]) - 1.0) < symprec &&
	mat_Dabs(mat_Dabs(relative_axis[2][0]+relative_axis[2][1]) - 1.0) < symprec) {
      bravais->centering = C_FACE;
      goto found;
    }
    if (mat_Dabs(mat_Dabs(relative_axis[0][1]+relative_axis[0][2]) - 1.0) < symprec &&
	mat_Dabs(mat_Dabs(relative_axis[1][1]+relative_axis[1][2]) - 1.0) < symprec &&
	mat_Dabs(mat_Dabs(relative_axis[2][1]+relative_axis[2][2]) - 1.0) < symprec) {
      bravais->centering = A_FACE;
      goto found;
    }	

    /* otherwise bcc */
    get_monocli_bcc_to_c_center(bravais->lattice);
    bravais->centering = C_FACE;
    goto found;
  }

  /* primitive */
  if (get_monocli_bravais(bravais->lattice, conv_sym, 0.5, bcc_axes, symprec)) {
    bravais->centering = NO_CENTER;
    goto found;
  }

  return 0;

 found:
  return 1;
}

static int is_monocli_from_F( Bravais *bravais,
			      const Symmetry *conv_sym,
			      const double symprec )
{
  double lattice[3][3], relative_axis[3][3];

  mat_copy_matrix_d3(lattice, bravais->lattice);
  
  /* base center */
  if (get_monocli_bravais(bravais->lattice, conv_sym, 0.5, fcc_axes, symprec)) {
    get_monocli_relative_axes(relative_axis, bravais->lattice, lattice, symprec);

    if ((mat_Dabs(mat_Dabs(relative_axis[0][0]+relative_axis[0][1]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][0]+relative_axis[1][1]) - 1.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][0]+relative_axis[2][1]) - 1.0) < symprec) ||
	(mat_Dabs(mat_Dabs(relative_axis[0][0]+relative_axis[0][1]) - 1.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][0]+relative_axis[1][1]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][0]+relative_axis[2][1]) - 1.0) < symprec) ||
	(mat_Dabs(mat_Dabs(relative_axis[0][0]+relative_axis[0][1]) - 1.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][0]+relative_axis[1][1]) - 1.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][0]+relative_axis[2][1]) - 0.0) < symprec)) {
      bravais->centering = C_FACE;
      goto found;
    }
    if ((mat_Dabs(mat_Dabs(relative_axis[0][1]+relative_axis[0][2]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][1]+relative_axis[1][2]) - 1.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][1]+relative_axis[2][2]) - 1.0) < symprec) ||
	(mat_Dabs(mat_Dabs(relative_axis[0][1]+relative_axis[0][2]) - 1.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][1]+relative_axis[1][2]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][1]+relative_axis[2][2]) - 1.0) < symprec) ||
	(mat_Dabs(mat_Dabs(relative_axis[0][1]+relative_axis[0][2]) - 1.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][1]+relative_axis[1][2]) - 1.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][1]+relative_axis[2][2]) - 0.0) < symprec)) {
      bravais->centering = A_FACE;
      goto found;
    }
    /* otherwise bcc */
    get_monocli_bcc_to_c_center(bravais->lattice);
    bravais->centering = C_FACE;
    goto found;
  }

  /* primitive */
  if (get_monocli_bravais(bravais->lattice, conv_sym, 0.25, fcc_axes, symprec)) {
    bravais->centering = NO_CENTER;
    goto found;
  }

  return 0;

 found:
  return 1;
}

static int is_monocli_from_P( Bravais *bravais,
			      const Symmetry *conv_sym, 
			      const double symprec )
{
  double lattice[3][3], relative_axis[3][3];

  /* primitive */
  if ( bravais->centering == NO_CENTER ) {
    if (get_monocli_bravais(bravais->lattice, conv_sym, 1.0, primitive_axes, symprec)) {
      bravais->centering = NO_CENTER;
      debug_print("No centering found\n");
      goto found;
    }
  }

  /* base center */
  mat_copy_matrix_d3(lattice, bravais->lattice);
  if (get_monocli_bravais(bravais->lattice, conv_sym, 2.0, primitive_axes, symprec)) {

    get_monocli_relative_axes(relative_axis, bravais->lattice, lattice, symprec);

    debug_print("is_monocli_from_P\n");
    debug_print_matrix_d3(relative_axis);

    if ((mat_Dabs(mat_Dabs(relative_axis[0][0]+relative_axis[0][1]) - 2.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][0]+relative_axis[1][1]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][0]+relative_axis[2][1]) - 0.0) < symprec) ||
	(mat_Dabs(mat_Dabs(relative_axis[0][0]+relative_axis[0][1]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][0]+relative_axis[1][1]) - 2.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][0]+relative_axis[2][1]) - 0.0) < symprec) ||
	(mat_Dabs(mat_Dabs(relative_axis[0][0]+relative_axis[0][1]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][0]+relative_axis[1][1]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][0]+relative_axis[2][1]) - 2.0) < symprec)) {
      bravais->centering = C_FACE;
      goto found;
    }

    if ((mat_Dabs(mat_Dabs(relative_axis[0][1]+relative_axis[0][2]) - 2.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][1]+relative_axis[1][2]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][1]+relative_axis[2][2]) - 0.0) < symprec) ||
	(mat_Dabs(mat_Dabs(relative_axis[0][1]+relative_axis[0][2]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][1]+relative_axis[1][2]) - 2.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][1]+relative_axis[2][2]) - 0.0) < symprec) ||
	(mat_Dabs(mat_Dabs(relative_axis[0][1]+relative_axis[0][2]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[1][1]+relative_axis[1][2]) - 0.0) < symprec &&
	 mat_Dabs(mat_Dabs(relative_axis[2][1]+relative_axis[2][2]) - 2.0) < symprec)) {
      bravais->centering = A_FACE;
      goto found;
    }

    /* otherwise bcc */
    get_monocli_bcc_to_c_center(bravais->lattice);
    bravais->centering = C_FACE;
    goto found;
  }

  return 0;

 found:
  debug_print("New centering: %d\n", bravais->centering);
  return 1;
}

static void get_monocli_relative_axes( double relative_axis[3][3],
				       SPGCONST double new_lattice[3][3],
				       SPGCONST double old_lattice[3][3],
				       const double symprec )
{
  double inv_lattice[3][3];

  if (!(mat_inverse_matrix_d3(inv_lattice, old_lattice, symprec)))
    fprintf(stderr, "spglib: BUG in spglib in __LINE__, __FILE__.");

  mat_multiply_matrix_d3(relative_axis, inv_lattice, new_lattice);
}

static void get_monocli_bcc_to_c_center( double lattice[3][3] )
{
  int i;
  double tmp_lattice[3][3];

  mat_copy_matrix_d3(tmp_lattice, lattice);
  for (i = 0; i < 3; i++)
    lattice[i][0] = tmp_lattice[i][0] + tmp_lattice[i][2];
}

static int get_monocli_bravais( double lattice[3][3],
				const Symmetry *conv_sym,
				const double relative_vol,
				SPGCONST double monocli_axes[13][3],
				const double symprec )
{
  int i, j, k, l, naxis[3];
  double relative_axis[3][3], volume;

  /* get 2-fold axis */
  for (i = 0; i < conv_sym->size; i++) {
    naxis[1] = get_rotation_axis(conv_sym->rot[i], 2);
    if (naxis[1] > -1) {
      break;
    }
  }

  /* get the other axes orthonormal to the 2-fold axis */
  for (i = 0; i < 13; i++) {
    if (is_monocli_orthogonal(naxis[1], i, monocli_axes, symprec)) {
      naxis[0] = i;
      for (j = 0; j < 13; j++) {
	if (is_monocli_orthogonal(naxis[1], j, monocli_axes, symprec)) {
	  naxis[2] = j;
	  for (k = 0; k < 3; k++)
	    for (l = 0; l < 3; l++)
	      relative_axis[l][k] = monocli_axes[naxis[k]][l];

	  volume = mat_Dabs(mat_get_determinant_d3(relative_axis));
	  debug_print("axes: %d %d %d, volume: %f \n", naxis[0], naxis[1], naxis[2], volume);
	  if (mat_Dabs(volume - relative_vol) < symprec) {
	    goto found;
	  }
	}
      }
    }
  }

  /* not found */
  return 0;

 found:
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      relative_axis[j][i] = monocli_axes[naxis[i]][j];
  mat_multiply_matrix_d3(lattice, lattice, relative_axis);
  return 1;
}

static int is_monocli_orthogonal( const int b_axis,
				  const int naxis,
				  SPGCONST double monocli_axes[13][3],
				  const double symprec )
{
  if (mat_Dabs(monocli_axes[b_axis][0] * monocli_axes[naxis][0] +
	       monocli_axes[b_axis][1] * monocli_axes[naxis][1] +
	       monocli_axes[b_axis][2] * monocli_axes[naxis][2]) < symprec) {
    return 1;
  }

  return 0;
}

/* axis_num: Rotation type N */
static int get_rotation_axis( SPGCONST int rot[3][3],
			      const int axis_num )
{
  int i, axis = -1, tmp_rot[3][3], test_rot[3][3], vec[3];
  int identity[3][3] = {
    { 1, 0, 0},
    { 0, 1, 0},
    { 0, 0, 1},
  };

  int inversion[3][3] = {
    {-1, 0, 0},
    { 0,-1, 0},
    { 0, 0,-1},
  };

  mat_copy_matrix_i3(test_rot, identity);
  mat_copy_matrix_i3(tmp_rot, rot);

  /* If improper, multiply inversion and get proper rotation */
  if (mat_get_determinant_i3(rot) != 1)
    mat_multiply_matrix_i3(tmp_rot, inversion, rot);

  /* Look for rotation axis */
  if (!(mat_check_identity_matrix_i3(tmp_rot, identity))) {

    /* check rotation order */
    for (i = 0; i < axis_num; i++) {
      mat_multiply_matrix_i3(test_rot, tmp_rot, test_rot);
      if (mat_check_identity_matrix_i3(test_rot, identity) && i < axis_num - 1)
	goto end;
    }
    if (mat_check_identity_matrix_i3(test_rot, identity)) {

      /* Look for eigenvector = rotation axis */
      for (i = 0; i < 13; i++) {
	mat_multiply_matrix_vector_i3(vec, tmp_rot, rot_axes[i]);
	
	if (vec[0] == rot_axes[i][0] &&
	    vec[1] == rot_axes[i][1] &&
	    vec[2] == rot_axes[i][2]) {

	  axis = i;

	  debug_print("--------\n");
	  debug_print("invariant axis: %d %d %d\n", rot_axes[i][0], rot_axes[i][1], rot_axes[i][2]);
	  debug_print_matrix_i3(tmp_rot);
	  break;
	}
      }
    }
  }
  
 end:
  return axis;
}
