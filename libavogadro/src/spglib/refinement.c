/* bravais.c */
/* Copyright (C) 2011 Atsushi Togo */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "refinement.h"
#include "cell.h"
#include "lattice.h"
#include "mathfunc.h"
#include "pointgroup.h"
#include "primitive.h"
#include "spacegroup.h"
#include "spg_database.h"
#include "symmetry.h"

#include "debug.h"

#define REDUCE_RATE 0.95

static Cell * refine_cell( SPGCONST Cell * cell,
			   const double symprec );
static Cell * get_conventional_positions( SPGCONST Cell * conv_prim,
					  SPGCONST Symmetry * conv_sym );
static Cell * get_conventional_primitive( SPGCONST double bravais_lattice[3][3],
					  const double origin_shift[3],
					  SPGCONST Cell * primitive );
static Symmetry * get_IT_symmetry( const int hall_number );
static int get_symmetrized_positions( Cell * bravais,
				      SPGCONST Symmetry * conv_sym,
				      const double symprec );
static int get_exact_location( double position[3],
			       SPGCONST Symmetry * conv_sym,
			       SPGCONST double bravais_lattice[3][3],
			       const double symprec );
static int get_number_of_pure_translation( SPGCONST Symmetry * conv_sym );
static int get_conventional_lattice( double lattice[3][3],
			        const Holohedry holohedry,
			        SPGCONST double bravais_lattice[3][3] );
static void set_monocli( double lattice[3][3],
			 SPGCONST double metric[3][3] );
static void set_ortho( double lattice[3][3],
		       SPGCONST double metric[3][3] );
static void set_tetra( double lattice[3][3],
		       SPGCONST double metric[3][3] );
static void set_rhomb( double lattice[3][3],
		       SPGCONST double metric[3][3] );
static void set_trigo( double lattice[3][3],
		       SPGCONST double metric[3][3] );
static void set_cubic( double lattice[3][3],
		       SPGCONST double metric[3][3] );


static int identity[3][3] = {
  { 1, 0, 0},
  { 0, 1, 0},
  { 0, 0, 1},
};


Cell * ref_refine_cell( SPGCONST Cell * cell,
			const double symprec )
{
  return refine_cell( cell, symprec );
}

static Cell * refine_cell( SPGCONST Cell * cell,
			   const double symprec )
{
  Spacegroup spacegroup;
  Cell *primitive, *conv_prim, *bravais;
  Symmetry *conv_sym;
  
  primitive = prm_get_primitive( cell, symprec );
  if ( primitive->size == 0 ) {
    cel_free_cell( primitive );
    primitive = cel_copy_cell( cell );
  }
  if ( primitive->size == -1 ) {
    cel_free_cell( primitive );
    bravais = cel_alloc_cell( -1 );
    goto ret;
  }

  spacegroup = spa_get_spacegroup_with_primitive( primitive, symprec );
  conv_prim = get_conventional_primitive( spacegroup.bravais_lattice,
					  spacegroup.origin_shift,
					  primitive );


  debug_print("Primitive cell in refine_cell\n");
  debug_print_matrix_d3( primitive->lattice );
  debug_print_vectors_with_label( primitive->position,
				  primitive->types,
				  primitive->size );
  debug_print("Origin shift: %f %f %f\n",
	      spacegroup.origin_shift[0],
	      spacegroup.origin_shift[1],
	      spacegroup.origin_shift[2] );
  debug_print("Conventional primitive cell positions\n");
  debug_print_vectors_with_label( conv_prim->position,
				  conv_prim->types,
				  conv_prim->size );
  
  cel_free_cell( primitive );

  conv_sym = get_IT_symmetry( spacegroup.hall_number );
  bravais = get_conventional_positions( conv_prim, conv_sym );
  get_conventional_lattice( bravais->lattice,
			    spacegroup.holohedry,
			    spacegroup.bravais_lattice );

  debug_print("Conventional cell\n");
  debug_print_matrix_d3( bravais->lattice );
  debug_print_vectors_with_label( bravais->position,
				  bravais->types,
				  bravais->size );

  /* Symmetrize atomic positions of conventional unit cell */
  if ( ! get_symmetrized_positions( bravais, conv_sym, symprec ) ) {
    cel_free_cell( bravais );
    bravais = cel_alloc_cell( -1 );
  }

  debug_print("Conventional cell symmetrized positions\n");
  debug_print_vectors_with_label( bravais->position,
				  bravais->types,
				  bravais->size );

  sym_free_symmetry( conv_sym );
  cel_free_cell( conv_prim );

 ret:
  /* Return bravais->size = -1, if the bravais could not be found. */
  return bravais;
}

static Cell * get_conventional_positions( SPGCONST Cell * conv_prim,
					  SPGCONST Symmetry * conv_sym )
{
  int i, j, k, num_pure_trans;
  int num_atom;
  Cell * bravais;

  num_pure_trans = get_number_of_pure_translation( conv_sym );
  bravais = cel_alloc_cell( conv_prim->size * num_pure_trans );

  num_atom = 0;
  for ( i = 0; i < conv_prim->size; i++ ) {
    for ( j = 0; j < conv_sym->size; j++ ) {
      /* Referred atoms in Bravais lattice */
      if ( mat_check_identity_matrix_i3( identity, conv_sym->rot[j] ) ) {
	bravais->types[num_atom] = conv_prim->types[i];
	mat_copy_vector_d3( bravais->position[num_atom],
			    conv_prim->position[i] );
	for ( k = 0; k < 3; k++ ) {
	  bravais->position[num_atom][k] += conv_sym->trans[j][k];
	  bravais->position[num_atom][k] -= 
	    mat_Nint( bravais->position[num_atom][k] );
	}
	num_atom++;
      }
    }
  }

  return bravais;
}

static int get_symmetrized_positions( Cell * bravais,
				      SPGCONST Symmetry * conv_sym,
				      const double symprec )
{
  int i, j, k, l, num_ind_atoms, is_found;
  double pos[3];
  int *ind_atoms;
  VecDBL *positions;

  debug_print("get_symmetrized_positions\n");

  ind_atoms = malloc( sizeof( double ) * bravais->size );
  positions = mat_alloc_VecDBL( bravais->size );
  num_ind_atoms = 0;

  for ( i = 0; i < bravais->size; i++ ) {
    is_found = 0;
    for ( j = 0; j < num_ind_atoms; j++ ) {
      for ( k = 0; k < conv_sym->size; k++ ) {
	/* check if this is site-symmetry */
	mat_multiply_matrix_vector_id3( pos,
					conv_sym->rot[k],
					positions->vec[ind_atoms[j]] );
	for ( l = 0; l < 3; l++ ) {
	  pos[l] += conv_sym->trans[k][l];
	}
	if ( cel_is_overlap( pos,
			     bravais->position[i],
			     bravais->lattice,
			     symprec ) ) {
	  is_found = 1;

	  goto escape;
	}
      }
    }
    
  escape:
    if ( is_found ) {
      /* This is not an independent atom. */
      for ( j = 0; j < 3; j++ ) { pos[j] -= mat_Nint( pos[j] ); }
      mat_copy_vector_d3( positions->vec[i], pos );
      debug_print("%d: %f %f %f (copy)\n", i+1, pos[0], pos[1], pos[2]);
    } else {
      /* This is an independent atom. */
      ind_atoms[ num_ind_atoms ] = i;
      num_ind_atoms++;

      mat_copy_vector_d3( positions->vec[i], bravais->position[i] );
      debug_print("%d: %f %f %f (before shift)\n", i+1,
		  positions->vec[i][0],
		  positions->vec[i][1],
		  positions->vec[i][2]);
      
      if (! get_exact_location( positions->vec[i],
				conv_sym,
				bravais->lattice,
				symprec ) ) {
	goto err;
      }
      debug_print("%d: %f %f %f (shift)\n", i+1,
		  positions->vec[i][0],
		  positions->vec[i][1],
		  positions->vec[i][2]);
    }
  }

  /* Copy back */
  for ( i = 0; i < bravais->size; i++ ) {
    mat_copy_vector_d3( bravais->position[i], positions->vec[i] );
  }

  mat_free_VecDBL( positions );
  free( ind_atoms );

  return 1;

 err:
  warning_print("spglib: Atomic positions could not be symmetrized  ");
  warning_print("(line %d, %s).\n", __LINE__, __FILE__);
  return 0;
}

/* Site-symmetry is used to determine exact location of an atom */
/* R. W. Grosse-Kunstleve and P. D. Adams */
/* Acta Cryst. (2002). A58, 60-65 */
static int get_exact_location( double position[3],
			       SPGCONST Symmetry * conv_sym,
			       SPGCONST double bravais_lattice[3][3],
			       const double symprec )
{
  int i, j, k, num_sum;
  /* int sum_rot[3][3]; */
  double sum_rot[3][3];
  double pos[3], sum_trans[3];

  num_sum = 0;
  for ( i = 0; i < 3; i++ ) {
    sum_trans[i] = 0.0;
    for ( j = 0; j < 3; j++ ) {
      sum_rot[i][j] = 0;
    }
  }
  
  for ( i = 0; i < conv_sym->size; i++ ) {
    mat_multiply_matrix_vector_id3( pos,
				    conv_sym->rot[i],
				    position );
    for ( j = 0; j < 3; j++ ) {
      pos[j] += conv_sym->trans[i][j];
    }

    if ( cel_is_overlap( pos,
			 position,
			 bravais_lattice,
			 symprec ) ) {
      for ( j = 0; j < 3; j++ ) {
	sum_trans[j] += conv_sym->trans[i][j] - 
	  mat_Nint( pos[j] - position[j] );
	for ( k = 0; k < 3; k++ ) {
	  sum_rot[j][k] += conv_sym->rot[i][j][k];
	}
      }
      num_sum++;
    }
  }

  for ( i = 0; i < 3; i++ ) {
    sum_trans[i] /= num_sum;
    for ( j = 0; j < 3; j++ ) {
      sum_rot[i][j] /= num_sum;
    }
  }

  mat_multiply_matrix_vector_d3( position,
				 sum_rot,
				 position );

  for ( i = 0; i < 3; i++ ) {
    position[i] += sum_trans[i];
  }

  return 1;
}

static int get_number_of_pure_translation( SPGCONST Symmetry * conv_sym )
{
  int i, num_pure_trans = 0;
  
  for ( i = 0; i < conv_sym->size; i++ ) {
    if ( mat_check_identity_matrix_i3( identity, conv_sym->rot[i] ) ) {
      num_pure_trans++;
    }
  }

  return num_pure_trans;
}

static Cell * get_conventional_primitive( SPGCONST double bravais_lattice[3][3],
					  const double origin_shift[3],
					  SPGCONST Cell * primitive )
{
  int i, j;
  double inv_brv[3][3], trans_mat[3][3];
  Cell * conv_prim;

  conv_prim = cel_alloc_cell( primitive->size );

  mat_inverse_matrix_d3( inv_brv, bravais_lattice, 0 );
  mat_multiply_matrix_d3( trans_mat, inv_brv, primitive->lattice );
  
  for ( i = 0; i < primitive->size; i++ ) {
    conv_prim->types[i] = primitive->types[i];
    mat_multiply_matrix_vector_d3( conv_prim->position[i],
				   trans_mat,
				   primitive->position[i] );
    for ( j = 0; j < 3; j++ ) {
      conv_prim->position[i][j] -= origin_shift[j];
      conv_prim->position[i][j] -= mat_Nint( conv_prim->position[i][j] );
    }
  }

  return conv_prim;
}

static Symmetry * get_IT_symmetry( const int hall_number )
{
  int i, hall_num_db;
  int operation_index[2];
  int rot[3][3];
  double trans[3];
  Symmetry *symmetry;

  spgdb_get_operation_index( operation_index, hall_number );
  symmetry = sym_alloc_symmetry( operation_index[0] );

  for ( i = 0; i < operation_index[0]; i++ ) {
    /* rotation matrix matching and set difference of translations */
    hall_num_db = spgdb_get_operation( rot,
				       trans,
				       operation_index[1] + i );
    mat_copy_matrix_i3( symmetry->rot[i], rot );
    mat_copy_vector_d3( symmetry->trans[i], trans );
  }

  return symmetry;
}

static int get_conventional_lattice( double lattice[3][3],
				     const Holohedry holohedry,
				     SPGCONST double bravais_lattice[3][3] )
{
  int i, j;
  double metric[3][3];

  for ( i = 0; i < 3; i++ ) {
    for ( j = 0; j < 3; j++ ) {
      lattice[i][j] = 0;
    }
  }

  mat_get_metric( metric, bravais_lattice );

  switch ( holohedry ) {
  case TRICLI:
    mat_copy_matrix_d3( lattice, bravais_lattice );
    break;
  case MONOCLI: /* b-axis is the unique axis. */
    set_monocli( lattice, metric );
    break;
  case ORTHO:
    set_ortho( lattice, metric );
    break;
  case TETRA:
    set_tetra( lattice, metric );
    break;
  case RHOMB:
    set_rhomb( lattice, metric );
    break;
  case TRIGO:
    set_trigo( lattice, metric );
    break;
  case HEXA:
    set_trigo( lattice, metric );
    break;
  case CUBIC:
    set_cubic( lattice, metric );
    break;
  case NONE:
    break;
  }

  return 1;
}

static void set_monocli( double lattice[3][3],
			 SPGCONST double metric[3][3] )
{
  double a, b, c, beta;
  a = sqrt( metric[0][0] );
  b = sqrt( metric[1][1] );
  c = sqrt( metric[2][2] );
  lattice[1][1] = b;
  lattice[2][2] = c;
  beta = acos( metric[0][2] / a / c );
  lattice[2][0] = a * cos( beta );
  lattice[0][0] = a * sin( beta );
}
			 
static void set_ortho( double lattice[3][3],
		       SPGCONST double metric[3][3] )
{
  double a, b, c;
  a = sqrt( metric[0][0] );
  b = sqrt( metric[1][1] );
  c = sqrt( metric[2][2] );
  lattice[0][0] = a;
  lattice[1][1] = b;
  lattice[2][2] = c;
}

static void set_tetra( double lattice[3][3],
		       SPGCONST double metric[3][3] )
{
  double a, b, c;
  a = sqrt( metric[0][0] );
  b = sqrt( metric[1][1] );
  c = sqrt( metric[2][2] );
  lattice[0][0] = ( a + b ) / 2;
  lattice[1][1] = ( a + b ) / 2;
  lattice[2][2] = c;
}

static void set_rhomb( double lattice[3][3],
		       SPGCONST double metric[3][3] )
{
  double a, b, c, angle, ahex, chex;


  a = sqrt( metric[0][0] );
  b = sqrt( metric[1][1] );
  c = sqrt( metric[2][2] );
  angle = acos( ( metric[0][1] / a / b +
		  metric[0][2] / a / c +
		  metric[1][2] / b / c ) / 3 );

  /* Reference, http://cst-www.nrl.navy.mil/lattice/struk/rgr.html */
  ahex = 2 * (a+b+c)/3 * sin( angle / 2 );
  chex = (a+b+c)/3 * sqrt( 3 * ( 1 + 2 * cos( angle ) ) ) ;
  lattice[0][0] = ahex / 2;
  lattice[1][0] = -ahex / ( 2 * sqrt(3) );
  lattice[2][0] = chex / 3;
  lattice[1][1] = ahex / sqrt(3);
  lattice[2][1] = chex / 3;
  lattice[0][2] = -ahex / 2;
  lattice[1][2] = -ahex / ( 2 * sqrt(3) );
  lattice[2][2] = chex / 3;


#ifdef DEBUG
  debug_print("Rhombo lattice: %f %f %f %f %f %f\n", a, b, c,
	      acos( metric[0][1] / a / b ) / 3.14 * 180,
	      acos( metric[0][2] / a / c ) / 3.14 * 180,
	      acos( metric[1][2] / b / c ) / 3.14 * 180 );
  double dmetric[3][3];
  mat_get_metric( dmetric, lattice );
  a = sqrt( dmetric[0][0] );
  b = sqrt( dmetric[1][1] );
  c = sqrt( dmetric[2][2] );
  debug_print("Rhombo lattice symmetrized: %f %f %f %f %f %f\n",
	      a, b, c,
	      acos( dmetric[0][1] / a / b ) / 3.14 * 180,
	      acos( dmetric[0][2] / a / c ) / 3.14 * 180,
	      acos( dmetric[1][2] / b / c ) / 3.14 * 180 );
#endif
}

static void set_trigo( double lattice[3][3],
		       SPGCONST double metric[3][3] )
{
  double a, b, c;
  a = sqrt( metric[0][0] );
  b = sqrt( metric[1][1] );
  c = sqrt( metric[2][2] );
  lattice[0][0] = ( a + b ) / 2;
  lattice[0][1] = - ( a + b ) / 4;
  lattice[1][1] = ( a + b ) / 4 * sqrt(3);
  lattice[2][2] = c;
}

static void set_cubic( double lattice[3][3],
		       SPGCONST double metric[3][3] )
{
  double a, b, c;
  a = sqrt( metric[0][0] );
  b = sqrt( metric[1][1] );
  c = sqrt( metric[2][2] );
  lattice[0][0] = ( a + b + c ) / 3;
  lattice[1][1] = ( a + b + c ) / 3;
  lattice[2][2] = ( a + b + c ) / 3;
}

