/* symmetry.c */
/* Copyright (C) 2008 Atsushi Togo */

#include <stdio.h>
#include <stdlib.h>
#include "bravais.h"
#include "cell.h"
#include "debug.h"
#include "mathfunc.h"
#include "pointgroup.h"
#include "primitive.h"
#include "symmetry.h"

#include "debug.h"

static int identity[3][3] = {
  {1, 0, 0},              /* order 1 */
  {0, 1, 0},
  {0, 0, 1}
};

static int inversion[3][3] = {
  {-1, 0, 0},             /* order 2 */
  { 0,-1, 0},
  { 0, 0,-1}
};

static int generator6[3][3] = {
  { 1,-1, 0},             /* order 6 */
  { 1, 0, 0},
  { 0, 0, 1}
};

static int generator4[3][3] = {
  { 0,-1, 0},              /* order 4 */
  { 1, 0, 0},
  { 0, 0, 1}
};

static int generator3[3][3] = {
  { 0, 1, 0},              /* order 3 */
  { 0, 0, 1},
  { 1, 0, 0}
};

static int generator2y[3][3] = {
  {-1, 0, 0},             /* order 2 */
  { 0, 1, 0},
  { 0, 0,-1}
};

static int generator2z[3][3] = {
  {-1, 0, 0},             /* order 2 */
  { 0,-1, 0},
  { 0, 0, 1}
};

static int generator2r[3][3] = {
  { 0, 0,-1},             /* order 2 */
  { 0,-1, 0},
  {-1, 0, 0}
};

static int generator2xy[3][3] = {
  { 0, 1, 0},             /* order 2 */
  { 1, 0, 0},
  { 0, 0,-1}
};

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

static void generate_operation( int point_symmetry[][3][3],
				SPGCONST int generator[3][3],
				const int n_sym,
				const int n_gen );
static PointSymmetry get_conventional( const Holohedry holohedry );
static int get_index_with_least_atoms( const Cell *cell );
static VecDBL * get_translation( SPGCONST int rot[3][3],
				 SPGCONST Cell *cell,
				 const double symprec );
static int get_operation( int rot[][3][3],
			  double trans[][3],
			  SPGCONST Bravais * bravais,
			  SPGCONST Cell * cell,
			  const double symprec );
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
				    SPGCONST Cell *primitive,
				    const double symprec );
static PointSymmetry get_candidate( SPGCONST Bravais * bravais,
				    SPGCONST double lattice[3][3],
				    const double symprec );
static void set_axes( int axes[3][3],
		      const int a1, const int a2, const int a3 );
static PointSymmetry get_lattice_symmetry( SPGCONST Cell *cell,
					   const double symprec );



Symmetry * sym_alloc_symmetry( const int size )
{
  Symmetry *symmetry;

  symmetry = malloc( sizeof( Symmetry ) );
  symmetry->size = size;
  if ((symmetry->rot =
       (int (*)[3][3]) malloc(sizeof(int[3][3]) * size)) == NULL) {
    fprintf(stderr, "spglib: Memory could not be allocated.");
    exit(1);
  }
  if ((symmetry->trans =
       (double (*)[3]) malloc(sizeof(double[3]) * size)) == NULL) {
    fprintf(stderr, "spglib: Memory could not be allocated.");
    exit(1);
  }
  return symmetry;
}

void sym_free_symmetry( Symmetry *symmetry )
{
  free( symmetry->rot );
  free( symmetry->trans );
  free( symmetry );
}

Symmetry * sym_get_operation( SPGCONST Bravais *bravais,
			      SPGCONST Cell *cell,
			      const double symprec )
{
  int i, j, num_sym;
  MatINT *rot;
  VecDBL *trans;
  Symmetry *symmetry;
  

  rot = mat_alloc_MatINT( cell->size * 48 );
  trans = mat_alloc_VecDBL( cell->size * 48 );
  num_sym = get_operation(rot->mat, trans->vec,
			  bravais, cell, symprec);

  debug_print("*** get_symmetry (found symmetry operations) *** \n");
  debug_print("Lattice \n"); 
  debug_print_matrix_d3(cell->lattice);


  symmetry = sym_alloc_symmetry( num_sym );

  for ( i = 0; i < num_sym; i++ ) {
    debug_print("--- %d ---\n", i + 1);
    debug_print_matrix_i3(rot->mat[i]);
    debug_print("%f %f %f\n",
		trans->vec[i][0], trans->vec[i][1], trans->vec[i][2]);
    
    mat_copy_matrix_i3(symmetry->rot[i], rot->mat[i]);

    for (j = 0; j < 3; j++)
      symmetry->trans[i][j] = trans->vec[i][j];
  }

  mat_free_MatINT( rot );
  mat_free_VecDBL( trans );

  return symmetry;
}

Symmetry * sym_get_operation_direct( SPGCONST Cell *cell,
				     const double symprec ) {
  int i, j, num_sym;
  MatINT *rot;
  VecDBL *trans;
  Symmetry *symmetry;
  PointSymmetry lattice_sym;
  
  rot = mat_alloc_MatINT( cell->size * 48 );
  trans = mat_alloc_VecDBL( cell->size * 48 );
  lattice_sym = get_lattice_symmetry( cell, symprec );

#ifdef DEBUG
  for ( i = 0; i < lattice_sym.size; i++ ) {
    printf("----- %d -----\n", i+1 );
    debug_print_matrix_i3( lattice_sym.rot[i] );
  }
#endif
  
  num_sym = get_space_group_operation( rot->mat, trans->vec,
				       &lattice_sym, cell, symprec);

  debug_print("*** get_symmetry (found symmetry operations) *** \n");
  debug_print("Lattice \n");
  debug_print_matrix_d3(cell->lattice);

  symmetry = sym_alloc_symmetry( num_sym );

  for ( i = 0; i < num_sym; i++ ) {
    debug_print("--- %d ---\n", i + 1);
    debug_print_matrix_i3(rot->mat[i]);
    debug_print("%f %f %f\n",
  		trans->vec[i][0], trans->vec[i][1], trans->vec[i][2]);
    
    mat_copy_matrix_i3(symmetry->rot[i], rot->mat[i]);

    for (j = 0; j < 3; j++)
      symmetry->trans[i][j] = trans->vec[i][j];
  }

  mat_free_MatINT( rot );
  mat_free_VecDBL( trans );

  return symmetry;
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

/* Look for the translations which satisfy the input symmetry operation. */
/* This function is heaviest in this code. */
static VecDBL * get_translation( SPGCONST int rot[3][3],
				 SPGCONST Cell *cell,
				 const double symprec )
{
  int i, j, k, l, count, min_atom_index, num_trans = 0;
  double test_trans[3], pos_rot[3], tmp_pos[3], origin[3];
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

    count = 0;
    for (j = 0; j < cell->size; j++) { /* test nonsymmorphic operation for an atom */
      mat_multiply_matrix_vector_id3(pos_rot, rot, cell->position[j]);

      for (k = 0; k < cell->size; k++) { /* check overlap of atom_k and atom_l */
	if (cell->types[j] != cell->types[k]) {
	  continue;
	}


	/* cell->position[k]      Position of reference atom     */
	/* pos_rot             Position of transformed atom   */
	/* test_trans             Guessed translation from above */
	for (l = 0; l < 3; l++) { 
	  tmp_pos[l] = pos_rot[l] + test_trans[l];
	}
	
	if ( cel_is_overlap( cell->position[k], tmp_pos,
			     cell->lattice, symprec) ) {
	  /* OK: atom_k on atom_j */
	  count++;
	  break;
	}
      }

      /* Is all atoms OK ? */
      if (count < j + 1) {
	break;
      }
    }

    if (count == cell->size) {	/* all atoms OK ? */
      for (j = 0; j < 3; j++) {
	tmp_trans->vec[num_trans][j] =
	  test_trans[j] - mat_Nint(test_trans[j] - symprec);
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

/* 1) A primitive cell of the input cell is searched. */
/* 2) Pointgroup operations of the primitive cell are obtained. */
/*    These are constrained by the input cell lattice pointgroup. */
/*    Therefore even if the lattice of the primitive cell has higher */
/*    symmetry than that of the input cell, it is not considered. */
/* 3) Spacegroup operations are searched for the primitive cell */
/*    through the point group operations. */
/* 4) The spacegroup operations for the primitive cell are */
/*    transformed to the original input cells, if the input cell */
/*    was not a primitive cell. */
static int get_operation( int rot[][3][3],
			  double trans[][3],
			  SPGCONST Bravais *bravais,
			  SPGCONST Cell *cell,
			  const double symprec )
{
  int num_sym, multi;
  PointSymmetry lattice_sym;
  Cell *primitive;
  VecDBL *pure_trans;

  pure_trans = sym_get_pure_translation(cell, symprec);
  multi = pure_trans->size;

  /* Obtain primitive cell */
  if( multi > 1 ) {
    primitive = prm_get_primitive_with_pure_trans( cell,
						   pure_trans,
						   symprec );
    if ( primitive->size == 0 ) {
      fprintf(stderr, "spglib: primitive cell could not found.");
      fprintf(stderr, "spglib: BUG in spglib in __LINE__, __FILE__.");
      goto err;
    }
  } else {
    primitive = cell;
  }

  /* Lattice symmetry for primitive cell*/
  lattice_sym = get_candidate(bravais, primitive->lattice, symprec);

  /* Symmetry operation search for primitive cell */
  num_sym = get_space_group_operation( rot, trans, &lattice_sym,
				       primitive, symprec );

  /* Recover symmetry operation for the input structure */
  if( multi > 1 ) {
    num_sym = get_operation_supercell( rot,
				       trans,
				       num_sym,
				       pure_trans,
				       cell,
				       primitive,
				       symprec );
    cel_free_cell( primitive );
  }

  mat_free_VecDBL( pure_trans );
  return num_sym;

 err:
  mat_free_VecDBL( pure_trans );
  return 0;
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
				    SPGCONST Cell *primitive,
				    const double symprec )
{
  int i, j, k, multi;
  double tmp_mat[3][3], coordinate[3][3], coordinate_inv[3][3];
  MatINT *rot_prim;
  VecDBL *trans_prim;

  rot_prim = mat_alloc_MatINT( num_sym );
  trans_prim = mat_alloc_VecDBL( num_sym );
  multi = pure_trans->size;

  debug_print("get_operation_supercell\n");

  /* Obtain ratio of bravais lattice and primitive lattice. P^-1*B */
  if (!(mat_inverse_matrix_d3( tmp_mat, primitive->lattice, symprec )))
    fprintf(stderr, "spglib: BUG in spglib in __LINE__, __FILE__.");

  mat_multiply_matrix_d3( coordinate, tmp_mat, cell->lattice );

  /* Obtain ratio of primitive lattice and bravais lattice. B^-1*P */
  if (!(mat_inverse_matrix_d3( coordinate_inv, coordinate, symprec )))
    fprintf(stderr, "spglib: BUG in spglib in __LINE__, __FILE__.");

  for( i = 0; i < num_sym; i++) {

    /* Translations for primitive cell in fractional coordinate */
    /* have to be recalculated for supercell */
    mat_multiply_matrix_vector_d3( trans[i], coordinate_inv, trans[i] );

    /* Rotations for primitive cell in fractional coordinate */
    /* have to be recalculated for supercell using similarity-like transformation */
    mat_cast_matrix_3i_to_3d(tmp_mat, rot[i]);
    if (!mat_get_similar_matrix_d3(tmp_mat, tmp_mat, coordinate, symprec)) {
      fprintf(stderr, "spglib: BUG in spglib in __LINE__, __FILE__.");
    }
    mat_cast_matrix_3d_to_3i(rot[i], tmp_mat);
    debug_print_matrix_i3(rot[i]);
  }

  /* Rotations and translations are backed up to re-use arrays rot[] and trans[]. */
  for( i = 0; i < num_sym; i++ ) {
    mat_copy_matrix_i3( rot_prim->mat[i], rot[i] );
    for( j = 0; j < 3; j++ )
      trans_prim->vec[i][j] = trans[i][j];
  }

  /* Rotations and translations plus pure translations are set. */
  for( i = 0; i < num_sym; i++ ) {
    for( j = 0; j < multi; j++ ) {
      mat_copy_matrix_i3( rot[ i * multi + j ], rot_prim->mat[i] );
      for ( k = 0; k < 3; k++ ) {
	trans[i * multi + j][k] =
	  mat_Dmod1( trans_prim->vec[i][k] + pure_trans->vec[j][k],
		     symprec );
      }
    }
  }

  mat_free_MatINT( rot_prim );
  mat_free_VecDBL( trans_prim );
  /* return number of symmetry operation of supercell */
  return num_sym * multi;
}

/* Pointgroup operations are obtained for the lattice of the input */
/* cell. Then they are transformed to those in the primitive cell */
/* using similarity transformation. */
static PointSymmetry get_candidate( SPGCONST Bravais * bravais,
				    SPGCONST double lattice[3][3],
				    const double symprec )
{
  int i;
  double coordinate[3][3], tmp_matrix_d3[3][3];
  PointSymmetry lattice_sym;

  /* Obtain ratio of lattice and bravais lattice. B^-1*P */
  if (!(mat_inverse_matrix_d3(tmp_matrix_d3, bravais->lattice, symprec)))
    fprintf(stderr, "spglib: BUG in spglib in __LINE__, __FILE__.");

  mat_multiply_matrix_d3(coordinate, tmp_matrix_d3, lattice);

  debug_print("Primitive lattice P\n");
  debug_print_matrix_d3(lattice);
  debug_print("Bravais lattice B\n");
  debug_print_matrix_d3(bravais->lattice);
  debug_print("Holohedry: %d\n", bravais->holohedry);
  debug_print("Ratio of lattice and bravais lattice. B^-1*P.\n");
  debug_print_matrix_d3(coordinate);

  debug_print("*** get_candidate ***\n");

  debug_print("*** candidate in unit cell ***\n");
  lattice_sym = get_conventional(bravais->holohedry);

#ifdef DEBUG
  for (i = 0; i < lattice_sym.size; i++) {
    debug_print("-------- %d -------\n", i+1);
    debug_print_matrix_i3(lattice_sym.rot[i]);
  }
#endif

  debug_print("*** candidate in primitive cell ***\n");

  for (i = 0; i < lattice_sym.size; i++) {
    mat_cast_matrix_3i_to_3d(tmp_matrix_d3, lattice_sym.rot[i]);
    if (!mat_get_similar_matrix_d3(tmp_matrix_d3, tmp_matrix_d3,
                                   coordinate, symprec)) {
      debug_print_matrix_d3(tmp_matrix_d3);
      fprintf(stderr, "spglib BUG: Determinant is zero.\n");
    }

    mat_cast_matrix_3d_to_3i(lattice_sym.rot[i], tmp_matrix_d3);

  }

#ifdef DEBUG
  for (i = 0; i < lattice_sym.size; i++) {
    debug_print("-------- %d -------\n", i+1);
    debug_print_matrix_i3(lattice_sym.rot[i]);
  }
#endif

  return lattice_sym;
}

static PointSymmetry get_conventional( const Holohedry holohedry )
{
  int i, j, k;
  PointSymmetry lattice_sym;

  /* all clear */
  for (i = 0; i < 48; i++)
    for (j = 0; j < 3; j++)
      for (k = 0; k < 3; k++)
	lattice_sym.rot[i][j][k] = 0;
  lattice_sym.size = 0;

  /* indentity: this is seed. */
  mat_copy_matrix_i3(lattice_sym.rot[0], identity);

  /* inversion */
  generate_operation(lattice_sym.rot, inversion, 1, 2);

  switch (holohedry) {
  case CUBIC:
    generate_operation(lattice_sym.rot, generator2y, 2, 2);
    generate_operation(lattice_sym.rot, generator2z, 4, 2);
    generate_operation(lattice_sym.rot, generator2xy, 8, 2);
    generate_operation(lattice_sym.rot, generator3, 16, 3);
    lattice_sym.size = 48;
    break;
  case HEXA:
  case TRIGO:
    generate_operation(lattice_sym.rot, generator2xy, 2, 2);
    generate_operation(lattice_sym.rot, generator6, 4, 6);
    lattice_sym.size = 24;
    break;
  case RHOMB:
    generate_operation(lattice_sym.rot, generator2r, 2, 2);
    generate_operation(lattice_sym.rot, generator3, 4, 3);
    lattice_sym.size = 12;
    break;
  case TETRA:
    generate_operation(lattice_sym.rot, generator4, 2, 4);
    generate_operation(lattice_sym.rot, generator2y, 8, 2);
    lattice_sym.size = 16;
    break;
  case ORTHO:
    generate_operation(lattice_sym.rot, generator2y, 2, 2);
    generate_operation(lattice_sym.rot, generator2z, 4, 2);
    lattice_sym.size = 8;
    break;
  case MONOCLI:
    generate_operation(lattice_sym.rot, generator2y, 2, 2);
    lattice_sym.size = 4;
    break;
  case TRICLI:
    lattice_sym.size = 2;
    break;
  default:
    fprintf(stderr, "spglib BUG: no Bravais lattice found.\n");
  }

  return lattice_sym;
}

static void generate_operation( int point_symmetry[][3][3],
				SPGCONST int generator[3][3],
				const int n_sym,
				const int n_gen )
{
  int i, j, count;
  int tmp_matrix[3][3];

  /* n_sym is number of symmetry operations, which was already counted. */
  /* n_gen is order, number of operations of the generator class. */
  for (i = 0; i < n_sym; i++) {
    for (j = 0; j < n_gen - 1; j++) {	/* "-1" comes from E (identity) in a class */
      count = i * (n_gen - 1) + j + n_sym;
      mat_multiply_matrix_i3(tmp_matrix, generator,
			     point_symmetry[count - n_sym]);
      mat_copy_matrix_i3(point_symmetry[count], tmp_matrix);
    }
  }
}

static PointSymmetry get_lattice_symmetry( SPGCONST Cell *cell,
					   const double symprec )
{
  int i, j, k, num_sym;
  int axes[3][3];
  double lattice[3][3], metric[3][3], cell_metric[3][3];
  PointSymmetry lattice_sym;

  mat_get_metric( cell_metric, cell->lattice );

  num_sym = 0;
  for ( i = 0; i < 26; i++ ) {
    for ( j = 0; j < 26; j++ ) {
      for ( k = 0; k < 26; k++ ) {
	set_axes( axes, i, j, k );
	if ( ! ( ( mat_get_determinant_i3( axes ) == 1 ) ||
		 ( mat_get_determinant_i3( axes ) == -1 ) ) ) {
	  continue;
	}
	mat_multiply_matrix_di3( lattice, cell->lattice, axes );
	mat_get_metric( metric, lattice );
	if ( mat_check_identity_matrix_d3( metric, cell_metric,
					   symprec ) ) {
	  mat_copy_matrix_i3( lattice_sym.rot[num_sym], axes );
	  num_sym++;
	}
	if ( num_sym > 48 ) {
	  fprintf(stderr, "spglib BUG: Too much lattice symmetries found.\n");
	  goto err;
	}
      }
    }
  }

  lattice_sym.size = num_sym;
  return lattice_sym;
  
 err:
  lattice_sym.size = 0;
  return lattice_sym;
}

static void set_axes( int axes[3][3],
		      const int a1, const int a2, const int a3 )
{
  int i;
  for ( i = 0; i < 3; i++ ) { axes[i][0] = relative_axes[a1][i]; }
  for ( i = 0; i < 3; i++ ) { axes[i][1] = relative_axes[a2][i]; }
  for ( i = 0; i < 3; i++ ) { axes[i][2] = relative_axes[a3][i]; }
}
