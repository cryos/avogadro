/* cell.c */
/* Copyright (C) 2008 Atsushi Togo */

#include <stdlib.h>
#include <stdio.h>
#include "cell.h"
#include "mathfunc.h"

Cell * cel_alloc_cell( const int size )
{
    Cell *cell;
    int i, j;
    
    cell = malloc( sizeof( Cell ) );

    for ( i = 0; i < 3; i++ ) {
      for ( j = 0; j < 3; j++ ) {
	cell->lattice[i][j] = 0;
      }
    }
    cell->size = size;
    
    if ( size > 0 ) {
      if ((cell->types = (int *) malloc(sizeof(int) * size)) == NULL) {
        fprintf(stderr, "spglib: Memory of cell could not be allocated.");
        exit(1);
      }
      if ((cell->position =
	   (double (*)[3]) malloc(sizeof(double[3]) * size)) == NULL) {
        fprintf(stderr, "spglib: Memory of cell could not be allocated.");
        exit(1);
      }
    }

    return cell;
}

void cel_free_cell( Cell * cell )
{
  if ( cell->size > 0 ) {
    free( cell->position );
    free( cell->types );
  }
  free ( cell );
}

void cel_set_cell( Cell * cell,
		   SPGCONST double lattice[3][3],
		   SPGCONST double position[][3],
		   const int types[] )
{
  int i, j;
  mat_copy_matrix_d3(cell->lattice, lattice);
  for (i = 0; i < cell->size; i++) {
    for (j = 0; j < 3; j++) {
      cell->position[i][j] = position[i][j];
    }
    cell->types[i] = types[i];
  }
}

int cel_is_overlap( const double a[3],
		    const double b[3],
		    SPGCONST double lattice[3][3],
		    const double symprec )
{
  int i;
  double v_diff[3];

  for ( i = 0; i < 3; i++ ) {
    v_diff[i] = a[i] - b[i];
    v_diff[i] -= mat_Nint( v_diff[i] );
  }

  mat_multiply_matrix_vector_d3( v_diff, lattice, v_diff );
  if ( mat_norm_squared_d3( v_diff ) < symprec*symprec ) {
    return 1;
  } else {
    return 0;
  }
}

