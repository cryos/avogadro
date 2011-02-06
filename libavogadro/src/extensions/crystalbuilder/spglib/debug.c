/* debug.c */
/* Copyright (C) 2008 Atsushi Togo */

#ifdef DEBUG
#include <stdio.h>
#include "bravais.h"
#include "debug.h"

void dbg_print_matrix_d3(double a[3][3])
{
    int i;
    for (i = 0; i < 3; i++) {
        printf("%f %f %f\n", a[i][0], a[i][1], a[i][2]);
    }
}

void dbg_print_matrix_i3(int a[3][3])
{
    int i;
    for (i = 0; i < 3; i++) {
        printf("%d %d %d\n", a[i][0], a[i][1], a[i][2]);
    }
}

void dbg_print_vectors_d3(double a[][3], int size)
{
    int i;
    for (i = 0; i < size; i++) {
        printf("%d: %f %f %f\n", i + 1, a[i][0], a[i][1], a[i][2]);
    }
}

void dbg_print_vectors_with_label(double a[][3], int b[], int size)
{
    int i;
    for (i = 0; i < size; i++) {
        printf("%d: %f %f %f\n", b[i], a[i][0], a[i][1], a[i][2]);
    }
}

void dbg_print_holohedry(Bravais *bravais)
{
  switch (bravais->centering) {
  case NO_CENTER:
    break;
  case BASE:
    printf("Bace center ");
    break;
  case BODY:
    printf("Body center ");
    break;
  case FACE:
    printf("Face center ");
    break;
  case A_FACE:
    printf("A center ");
    break;
  case B_FACE:
    printf("B center ");
    break;
  case C_FACE:
    printf("C center ");
    break;
  }

  switch (bravais->holohedry) {
  case CUBIC:
    printf("Cubic\n");
    break;
  case HEXA:
    printf("Hexagonal\n");
    break;
  case TRIGO:
    printf("Trigonal\n");
    break;
  case RHOMB:
    printf("Rhombohedral\n");
    break;
  case TETRA:
    printf("Tetragonal\n");
    break;
  case ORTHO:
    printf("Orthogonal\n");
    break;
  case MONOCLI:
    printf("Monoclinic\n");
    break;
  case TRICLI:
    printf("Triclinic\n");
    break;
  }
}

#endif
