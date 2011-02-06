/* pointgroup.c */
/* Copyright (C) 2008 Atsushi Togo */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bravais.h"
#include "pointgroup.h"
#include "symmetry.h"
#include "mathfunc.h"

#include "debug.h"

typedef struct {
  int table[10];
  char symbol[6];
  Holohedry holohedry;
  Laue laue;
} PointgroupType;

static int identity[3][3] = {
  { 1, 0, 0},
  { 0, 1, 0},
  { 0, 0, 1},
};

static int inversion[3][3] = {
  {-1, 0, 0},
  { 0,-1, 0},
  { 0, 0,-1},
};

static int rot_axes[37][3] = {
  { 1, 0, 0},
  { 0, 1, 0},
  { 0, 0, 1},
  { 0, 1, 1},
  { 1, 0, 1},
  { 1, 1, 0},
  { 0, 1,-1},
  {-1, 0, 1},
  { 1,-1, 0}, 
  { 1, 1, 1}, /* 10 */
  {-1, 1, 1},
  { 1,-1, 1},
  { 1, 1,-1},
  { 0, 1, 2},
  { 2, 0, 1},
  { 1, 2, 0},
  { 0, 2, 1},
  { 1, 0, 2},
  { 2, 1, 0},
  { 0,-1, 2}, /* 20 */
  { 2, 0,-1},
  {-1, 2, 0},
  { 0,-2, 1},
  { 1, 0,-2},
  {-2, 1, 0},
  { 2, 1, 1},
  { 1, 2, 1},
  { 1, 1, 2},
  { 2,-1,-1},
  {-1, 2,-1}, /* 30 */
  {-1,-1, 2},
  { 2, 1,-1},
  {-1, 2, 1},
  { 1,-1, 2},
  { 2,-1, 1}, /* 35 */
  { 1, 2,-1},
  {-1, 1, 2},
};

static Symmetry * get_cast_symmetry( const Symmetry * symmetry,
				     const PointgroupType * pointgroup_type );
static int check_pointgroup_condition( int check_count[32],
				       const int rotation_table[10] );
static int get_pointgroup_class_table( int table[10],
				       const Symmetry * symmetry );
static int check_pointgroup_class_table( const int a[10],
					 const int b[10] );
static PointgroupType get_pointgroup_data( const int num );
static PointgroupType get_pointgroup( const Symmetry * symmetry );
static int get_rotation_type( SPGCONST int rot[3][3] );
static int get_rotation_axis( SPGCONST int rot[3][3] );
static int get_orthogonal_axis( int ortho_axes[37],
				SPGCONST int proper_rot[3][3] );
static int laue2m( int axes[3],
		   const Symmetry * symmetry );
static int get_axes( int axes[3],
		     const Laue laue,
		     const Symmetry * symmetry );
static void get_proper_rotation( int prop_rot[3][3],
				 SPGCONST int rot[3][3] );
static void get_transform_matrix( int mat[3][3],
				  const int axes[3] );

#ifdef DEBUG
static void print_holohedry( const Holohedry holohedry );
#endif

Pointgroup ptg_get_pointgroup( const Symmetry * symmetry )
{
  PointgroupType pointgroup_type;
  Pointgroup pointgroup;
  int axes[3];
  int transform_mat[3][3];

  pointgroup_type = get_pointgroup( symmetry );
  get_axes( axes, pointgroup_type.laue, symmetry );
  get_transform_matrix( transform_mat, axes );

  strcpy( pointgroup.symbol, pointgroup_type.symbol );
  pointgroup.holohedry = pointgroup_type.holohedry;
  mat_copy_matrix_i3( pointgroup.transform_mat, transform_mat );

  return pointgroup;
}

Pointgroup ptg_get_symmetry_pointgroup( Symmetry * symmetry )
{
  int i, max_equal, pg_num;
  int table[10];
  int chk_count[32];
  int axes[3];
  int transform_mat[3][3];
  PointgroupType pointgroup_type;
  Pointgroup pointgroup;
  Symmetry * sym_cast;

  /* Summarize rotation part of symmetry operations */
  if ( ! get_pointgroup_class_table( table, symmetry ) ) {
    fprintf(stderr, "spglib BUG: No point group symbol found\n");
    pointgroup_type = get_pointgroup_data( 0 );
  }

  /* Extract only the symmetry operations that agree with */
  /* point-group database */
  pg_num = check_pointgroup_condition( chk_count, table );
  if ( pg_num > -1 ) {
    pointgroup_type = get_pointgroup_data( pg_num );
  } else {
    max_equal = 0;
    debug_print("chk_count\n");
    for ( i = 0; i < 32; i++ ) {
      debug_print("%d(%d) ", chk_count[i],i);
      if ( max_equal < chk_count[i] ) {
	max_equal = chk_count[i];
	pg_num = i;
      }
    }
    debug_print("\n");

    pointgroup_type = get_pointgroup_data( pg_num );
    sym_cast = get_cast_symmetry( symmetry, &pointgroup_type );
    symmetry->size = sym_cast->size;
    for ( i = 0; i < sym_cast->size; i++ ) {
      mat_copy_matrix_i3( symmetry->rot[i], sym_cast->rot[i] );
      mat_copy_vector_d3( symmetry->trans[i], sym_cast->trans[i] );
    }
    sym_free_symmetry( sym_cast );
  }

  /* get_axes( axes, pointgroup_type.laue, symmetry ); */
  /* get_transform_matrix( transform_mat, axes ); */
  /* mat_copy_matrix_i3( pointgroup.transform_mat, transform_mat ); */
  strcpy( pointgroup.symbol, pointgroup_type.symbol );
  pointgroup.holohedry = pointgroup_type.holohedry;

  return pointgroup;
}

static int check_pointgroup_condition( int check_count[32],
				       const int rotation_table[10] ) 
{
  int i, j, pointgroup_number;
  PointgroupType pointgroup_type;

  /* Count number of rotation types that agree with database. */
  /* If all of them match, simply returns the point-group number. */
  /* If not, a table that describes a rotation condition between */ 
  /* database and found symmetry operations is written in check_count.*/
  /* The condition is: */
  /* 1. Number of rotation type matches */
  /* 2. The point group whose number of rotation in database is */
  /*    larger than that found in symmetry finder is removed. */
  pointgroup_number = -1;
  for ( i = 0; i < 32; i++ ) {
    check_count[i] = 0;
    pointgroup_type = get_pointgroup_data( i );
    for ( j = 0; j < 10; j++ ) {
      if ( pointgroup_type.table[j] == rotation_table[j] ) {
	check_count[i]++;
      } else {
	if ( pointgroup_type.table[j] > 0 ) {
	  check_count[i] = 0;
	  break;
	}
      }
    }
    if ( check_count[i] == 10 ) {
      pointgroup_number = i;
      break;
    }
  }

  return pointgroup_number;
}
				       

static Symmetry * get_cast_symmetry( const Symmetry * symmetry,
				     const PointgroupType * pointgroup_type )
{
  int i, j, num_sym;
  int table_count[10];
  Symmetry * sym_cast;

  num_sym = 0;
  for ( i = 0; i < 10; i++ ) {
    num_sym += pointgroup_type->table[i];
  }

  debug_print("Determined point group is \"%s\".\n",
	      pointgroup_type->symbol );

  sym_cast = sym_alloc_symmetry( num_sym );
  for ( i = 0; i < 10; i++ ) { table_count[i] = 0; }
  num_sym = 0;
  for ( i = 0; i < symmetry->size; i++ ) {
    for ( j = 0; j < 10; j++ ) {
      if ( pointgroup_type->table[j] == 0 ) {
	continue;
      }

      if ( get_rotation_type( symmetry->rot[i] ) == j ) {
	if ( table_count[j] == pointgroup_type->table[j] ) {
	  debug_print("Numuber of rotation type %d in symmetry operations\n", i);
	  debug_print("is too much. Break.\n");
	  break;
	}
	if ( num_sym == sym_cast->size ) {
	  debug_print("Numuber of rotations in new symmetry operations\n");
	  debug_print("is too much. Break.\n");
	  break;
	}

	mat_copy_matrix_i3( sym_cast->rot[num_sym], symmetry->rot[i] );
	mat_copy_vector_d3( sym_cast->trans[num_sym], symmetry->trans[i] );
	table_count[j]++;
	num_sym++;
	break;
      }
    }
  }

  return sym_cast;
}

static PointgroupType get_pointgroup( const Symmetry * symmetry )
{
  int i;
  int table[10];
  PointgroupType pointgroup;

  /* Summarize rotation part of symmetry operations */
  if ( ! get_pointgroup_class_table( table, symmetry ) ) {
    fprintf(stderr, "spglib BUG: No point group symbol found\n");
    goto err;
  }

#ifdef DEBUG
  printf("*** pointgroup class table ***\n");
  for (i = 0; i < 10; i++)
    printf("%2d ", table[i]);
  printf("\n");
#endif

  /* Search point group from 32 groups. */
  for ( i = 0; i < 32; i++ ) {
    pointgroup = get_pointgroup_data(i);
    if ( check_pointgroup_class_table( table, pointgroup.table ) ) {
      goto found;
    }
  }
  
 err:
  debug_print("No point group symbol found\n");
  fprintf(stderr, "spglib BUG: No point group symbol found\n");

 found:
#ifdef DEBUG
  print_holohedry(pointgroup.holohedry);
  printf("\n");
#endif
  return pointgroup;
}

static int get_pointgroup_class_table( int table[10],
					const Symmetry * symmetry )
{
  /* Look-up table */
  /* Operation   -6 -4 -3 -2 -1  1  2  3  4  6 */
  /* Trace     -  2 -1  0  1 -3  3 -1  0  1  2 */
  /* Determinant -1 -1 -1 -1 -1  1  1  1  1  1 */

  /* table[0] = -6 axis */
  /* table[1] = -4 axis */
  /* table[2] = -3 axis */
  /* table[3] = -2 axis */
  /* table[4] = -1 axis */
  /* table[5] =  1 axis */
  /* table[6] =  2 axis */
  /* table[7] =  3 axis */
  /* table[8] =  4 axis */
  /* table[9] =  6 axis */

  int i, rot_type;

  for ( i = 0; i < 10; i++ ) { table[i] = 0; }
  for ( i = 0; i < symmetry->size; i++ ) {
    rot_type = get_rotation_type( symmetry->rot[i] );
    if ( rot_type == -1 ) {
      goto err;
    } else {
      table[rot_type]++;
    }
  }
  
  return 1;

 err:
  return 0;
}

static int get_rotation_type( SPGCONST int rot[3][3] )
{
  int rot_type;

  if ( mat_get_determinant_i3( rot ) == -1 ) {
    switch ( mat_get_trace_i3( rot ) ) {
    case -2: /* -6 */
      rot_type = 0;
      break;
    case -1: /* -4 */
      rot_type = 1;
      break;
    case 0:  /* -3 */
      rot_type = 2;
      break;
    case 1:  /* -2 */
      rot_type = 3;
      break;
    case -3: /* -1 */
      rot_type = 4;
      break;
    default:
      rot_type = -1;
      break;
    }
  } else {
    switch ( mat_get_trace_i3( rot ) ) {
    case 3:  /* 1 */
      rot_type = 5;
      break;
    case -1: /* 2 */
      rot_type = 6;
      break;
    case 0:  /* 3 */
      rot_type = 7;
      break;
    case 1:  /* 4 */
      rot_type = 8;
      break;
    case 2:  /* 6 */
      rot_type = 9;
      break;
    default:
      rot_type = -1;
      break;
    }	
  }

  return rot_type;
}


static int check_pointgroup_class_table( const int a[10],
					 const int b[10] )
{
  int i;
  for (i = 0; i < 10; i++) {
    if ( a[i] - b[i] ) { return 0; }
  }
  return 1;
}

static PointgroupType get_pointgroup_data( const int num )
{
  PointgroupType pointgroup_data[32] = {
    {
      {0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
      "1    ",
      TRICLI,
      LAUE1,
    },
    {
      {0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
      "-1   ",
      TRICLI,
      LAUE1,
    },
    {
      {0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
      "2    ",
      MONOCLI,
      LAUE2M,
    },
    {
      {0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
      "m    ",
      MONOCLI,
      LAUE2M,
    },
    {
      {0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
      "2/m  ",
      MONOCLI,
      LAUE2M,
    },
    {
      {0, 0, 0, 0, 0, 1, 3, 0, 0, 0},
      "222  ",
      ORTHO,
      LAUEMMM,
    },
    {
      {0, 0, 0, 2, 0, 1, 1, 0, 0, 0},
      "mm2  ",
      ORTHO,
      LAUEMMM,
    },
    {
      {0, 0, 0, 3, 1, 1, 3, 0, 0, 0},
      "mmm  ",
      ORTHO,
      LAUEMMM,
    },
    {
      {0, 0, 0, 0, 0, 1, 1, 0, 2, 0},
      "4    ",
      TETRA,
      LAUE4M,
    },
    {
      {0, 2, 0, 0, 0, 1, 1, 0, 0, 0},
      "-4   ",
      TETRA,
      LAUE4M,
    },
    {
      {0, 2, 0, 1, 1, 1, 1, 0, 2, 0},
      "4/m  ",
      TETRA,
      LAUE4M,
    },
    {
      {0, 0, 0, 0, 0, 1, 5, 0, 2, 0},
      "422  ",
      TETRA,
      LAUE4MMM,
    },
    {
      {0, 0, 0, 4, 0, 1, 1, 0, 2, 0},
      "4mm  ",
      TETRA,
      LAUE4MMM,
    },
    {
      {0, 2, 0, 2, 0, 1, 3, 0, 0, 0},
      "-42m ",
      TETRA,
      LAUE4MMM,
    },
    {
      {0, 2, 0, 5, 1, 1, 5, 0, 2, 0},
      "4/mmm",
      TETRA,
      LAUE4MMM,
    },
    {
      {0, 0, 0, 0, 0, 1, 0, 2, 0, 0},
      "3    ",
      TRIGO,
      LAUE3,
    },
    {
      {0, 0, 2, 0, 1, 1, 0, 2, 0, 0},
      "-3   ",
      TRIGO,
      LAUE3,
    },
    {
      {0, 0, 0, 0, 0, 1, 3, 2, 0, 0},
      "32   ",
      TRIGO,
      LAUE3M,
    },
    {
      {0, 0, 0, 3, 0, 1, 0, 2, 0, 0},
      "3m   ",
      TRIGO,
      LAUE3M,
    },
    {
      {0, 0, 2, 3, 1, 1, 3, 2, 0, 0},
      "-3m  ",
      TRIGO,
      LAUE3M,
    },
    {
      {0, 0, 0, 0, 0, 1, 1, 2, 0, 2},
      "6    ",
      HEXA,
      LAUE6M,
    },
    {
      {2, 0, 0, 1, 0, 1, 0, 2, 0, 0},
      "-6   ",
      HEXA,
      LAUE6M,
    },
    {
      {2, 0, 2, 1, 1, 1, 1, 2, 0, 2},
      "6/m  ",
      HEXA,
      LAUE6M,
    },
    {
      {0, 0, 0, 0, 0, 1, 7, 2, 0, 2},
      "622  ",
      HEXA,
      LAUE6MMM,
    },
    {
      {0, 0, 0, 6, 0, 1, 1, 2, 0, 2},
      "6mm  ",
      HEXA,
      LAUE6MMM,
    },
    {
      {2, 0, 0, 4, 0, 1, 3, 2, 0, 0},
      "-62m ",
      HEXA,
      LAUE6MMM,
    },
    {
      {2, 0, 2, 7, 1, 1, 7, 2, 0, 2},
      "6/mmm",
      HEXA,
      LAUE6MMM,
    },
    {
      {0, 0, 0, 0, 0, 1, 3, 8, 0, 0},
      "23   ",
      CUBIC,
      LAUE3,
    },
    {
      {0, 0, 8, 3, 1, 1, 3, 8, 0, 0},
      "m-3  ",
      CUBIC,
      LAUE3,
    },
    {
      {0, 0, 0, 0, 0, 1, 9, 8, 6, 0},
      "432  ",
      CUBIC,
      LAUEM3M,
    },
    {
      {0, 6, 0, 6, 0, 1, 3, 8, 0, 0},
      "-43m ",
      CUBIC,
      LAUEM3M,
    },
    {
      {0, 6, 8, 9, 1, 1, 9, 8, 6, 0},
      "m-3m ",
      CUBIC,
      LAUEM3M,
    }
  };
  return pointgroup_data[num];
}

#ifdef DEBUG
static void print_holohedry( const Holohedry holohedry )
{
  switch (holohedry) {
  case TRICLI:
    debug_print("Triclinic");
    break;
  case MONOCLI:
    debug_print("Monoclinic");
    break;
  case ORTHO:
    debug_print("Orthorhombic");
    break;
  case TETRA:
    debug_print("Tetragonal");
    break;
  case RHOMB:
    debug_print("Rhombohedral");
    break;
  case TRIGO:
    debug_print("Trigonal");
    break;
  case HEXA:
    debug_print("Hexagonal");
    break;
  case CUBIC:
    debug_print("Cubic");
    break;
  }
}
#endif


static int get_axes( int axes[3],
		     const Laue laue,
		     const Symmetry * symmetry )
{
  /* This will be removed. */
  axes[0] = 0;
  axes[1] = 1;
  axes[2] = 2;

  switch (laue) {
  case LAUE1:
    break;
  case LAUE2M:
    /* laue2m( axes, symmetry ); */
    break;
  case LAUEMMM:
    break;
  case LAUE4M:
    break;
  case LAUE4MMM:
    break;
  case LAUE3:
    break;
  case LAUE3M:
    break;
  case LAUE6M:
    break;
  case LAUE6MMM:
    break;
  case LAUEM3:
    break;
  case LAUEM3M:
    break;
  default:
    break;
  }

  return 1;
}

static int get_rotation_axis( SPGCONST int proper_rot[3][3] )
{
  int i, axis = -1;
  int vec[3];

  /* No specific axis for I and -I */
  if ( mat_check_identity_matrix_i3( proper_rot, identity ) ) {
    goto end;
  }

  /* Look for eigenvector = rotation axis */
  for ( i = 0; i < 37; i++ ) {
    mat_multiply_matrix_vector_i3( vec, proper_rot, rot_axes[i] );
    if ( vec[0] == rot_axes[i][0] &&
	 vec[1] == rot_axes[i][1] &&
	 vec[2] == rot_axes[i][2] ) {
      axis = i;
      break;
    }
  }
  
 end:
  return axis;
}

static int get_orthogonal_axis( int ortho_axes[37],
				SPGCONST int proper_rot[3][3] )
{
  int i, num_ortho_axis;
  int vec[3];
  int sum_rot[3][3];

  num_ortho_axis = 0;

  mat_add_matrix_i3( sum_rot, proper_rot, identity );
  for ( i = 0; i < 37; i++ ) {
    mat_multiply_matrix_vector_i3( vec, sum_rot, rot_axes[i] );
    if ( vec[0] == 0 &&
	 vec[1] == 0 &&
	 vec[2] == 0 ) {
      ortho_axes[ num_ortho_axis ] = i;
      num_ortho_axis++;
    }
  }

  return num_ortho_axis;
}

static int laue2m( int axes[3],
		   const Symmetry * symmetry )
{
  int i, num_ortho_axis, norm, min_norm, is_found, tmpval;
  int prop_rot[3][3], t_mat[3][3];
  int ortho_axes[37];

  for ( i = 0; i < symmetry->size; i++ ) {
    get_proper_rotation( prop_rot, symmetry->rot[i] );

    /* Search two-fold rotation */
    if ( ! ( mat_get_trace_i3( prop_rot ) == -1 ) ) {
      continue;
    }

    /* The first axis */
    axes[1] = get_rotation_axis( prop_rot );
    break;
  }

  /* The second axis */
  num_ortho_axis = get_orthogonal_axis( ortho_axes, prop_rot );
  if ( ! num_ortho_axis ) { goto err; }
  
  min_norm = 8;
  is_found = 0;
  for ( i = 0; i < num_ortho_axis; i++ ) {
    norm = mat_norm_squared_i3( rot_axes[ortho_axes[i]] );
    if ( norm < min_norm ) {
      min_norm = norm;
      axes[0] = ortho_axes[i];
      is_found = 1;
    }
  }
  if ( ! is_found ) { goto err; }
  
  /* The third axis */
  min_norm = 8;
  is_found = 0;
  for ( i = 0; i < num_ortho_axis; i++ ) {
    norm = mat_norm_squared_i3( rot_axes[ortho_axes[i]] );
    if ( norm < min_norm && ( ! ( ortho_axes[i] == axes[0] ) ) ) {
      min_norm = norm;
      axes[2] = ortho_axes[i];
      is_found = 1;
    }
  }
  if ( ! is_found ) { goto err; }

  get_transform_matrix( t_mat, axes );
  if ( mat_get_determinant_i3( t_mat ) < 0 ) {
    tmpval = axes[0];
    axes[0] = axes[2];
    axes[2] = tmpval;
  }

#ifdef DEBUG
  printf("laue2m\n");
  for ( i = 0; i < num_ortho_axis; i++ ) {
    printf("%d: %d %d %d\n", i+1,
	   rot_axes[ortho_axes[i]][0],
	   rot_axes[ortho_axes[i]][1],
	   rot_axes[ortho_axes[i]][2] );
  }

  for ( i = 0; i < 3; i++ ) {
    printf("axis-%1d: %d %d %d\n", i,
	   rot_axes[axes[i]][0],
	   rot_axes[axes[i]][1],
	   rot_axes[axes[i]][2]);
  }

  debug_print_matrix_i3( t_mat );
#endif

  return 1;

 err:
  return 0;
}

static void get_proper_rotation( int prop_rot[3][3],
				 SPGCONST int rot[3][3] )
{
  if ( mat_get_determinant_i3( rot ) == -1 ) {
    mat_multiply_matrix_i3( prop_rot, inversion, rot );
  } else {
    mat_copy_matrix_i3( prop_rot, rot );
  }
}

static void get_transform_matrix( int mat[3][3],
				  const int axes[3] )
{
  int i;
  for ( i = 0; i < 3; i++ ) { mat[i][0] = rot_axes[axes[0]][i]; }
  for ( i = 0; i < 3; i++ ) { mat[i][1] = rot_axes[axes[1]][i]; }
  for ( i = 0; i < 3; i++ ) { mat[i][2] = rot_axes[axes[2]][i]; }
}


