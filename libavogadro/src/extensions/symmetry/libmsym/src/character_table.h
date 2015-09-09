//
//  character_table.h
//  libmsym
//
//  Created by Marcus Johansson on 28/11/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__CHARACTER_TABLE_h
#define __MSYM__CHARACTER_TABLE_h

#include <stdio.h>
#include "msym_error.h"


enum IrreducibleRepresentationEnum { A1, A2, Ag, Au, A1g, A2g, A3g, A1u, A2u, A3u, B1, B2, Bg, Bu, B1g, B2g, B3g, B1u, B2u, B3u, E, E1, E2, Eg, Eu, E1g, E2g, E1u, E2u, T, T1, T2,  T1g, T1u, T2g, T2u, Gg, Gu, Hg, Hu};

static const char *IrreducibleRepresentationName[] =
{
    [A1] = "A1",
    [A2] = "A2",
    [B1] = "B1",
    [B2] = "B2",
    [E] = "E",
    [E1] = "E1",
    [E2] = "E2",
    [Eg] = "Eg",
    [Eu] = "Eu",
    [E1g] = "E1g",
    [E2g] = "E2g",
    [E1u] = "E1u",
    [E2u] = "E2u",
    [T] = "T",
    [T1] = "T1",
    [T2] = "T2",
    [Ag] = "Ag",
    [Au] = "Au",
    [A1g] = "A1g",
    [A2g] = "A2g",
    [A3g] = "A3g",
    [A1u] = "A1u",
    [A2u] = "A2u",
    [A3u] = "A3u",
    [Bg] = "Bg",
    [Bu] = "Bu",
    [B1g] = "B1g",
    [B2g] = "B2g",
    [B3g] = "B3g",
    [B1u] = "B1u",
    [B2u] = "B2u",
    [B3u] = "B3u",
    [T1g] = "T1g",
    [T1u] = "T1u",
    [T2g] = "T2g",
    [T2u] = "T2u",
    [Gg] = "Gg",
    [Gu] = "Gu",
    [Hg] = "Hg",
    [Hu] = "Hu"

};

//This will consume some extra memory to make things simple

static const enum IrreducibleRepresentationEnum TdIrrep[] = {
    [0] = A1,
    [1] = A2,
    [2] = E,
    [3] = T1,
    [4] = T2,
};

static const enum IrreducibleRepresentationEnum C2hIrrep[] = {
    [0] = Ag,
    [1] = Bg,
    [2] = Au,
    [3] = Bu
};

static const enum IrreducibleRepresentationEnum C3vIrrep[] = {
    [0] = A1,
    [1] = A2,
    [2] = E
};

static const enum IrreducibleRepresentationEnum C4vIrrep[] = {
    [0] = A1,
    [1] = A2,
    [2] = B1,
    [3] = B2,
    [4] = E
};

static const enum IrreducibleRepresentationEnum D2hIrrep[] = {
    [0] = Ag,
    [1] = B1g,
    [2] = B2g,
    [3] = B3g,
    [4] = Au,
    [5] = B1u,
    [6] = B2u,
    [7] = B3u
};

static const enum IrreducibleRepresentationEnum D4hIrrep[] = {
    [0] = A1g,
    [1] = A2g,
    [2] = B1g,
    [3] = B2g,
    [4] = Eg,
    [5] = A1u,
    [6] = A2u,
    [7] = B1u,
    [8] = B2u,
    [9] = Eu
};

static const enum IrreducibleRepresentationEnum D6hIrrep[] = {
    [ 0] = A1g,
    [ 1] = A2g,
    [ 2] = B1g,
    [ 3] = B2g,
    [ 4] = E1g,
    [ 5] = E2g,
    [ 6] = A1u,
    [ 7] = A2u,
    [ 8] = B1u,
    [ 9] = B2u,
    [10] = E1u,
    [11] = E2u
};

static const enum IrreducibleRepresentationEnum IhIrrep[] = {
    [0] = Ag,
    [1] = T1g,
    [2] = T2g,
    [3] = Gg,
    [4] = Hg,
    [5] = Au,
    [6] = T1u,
    [7] = T2u,
    [8] = Gu,
    [9] = Hu
};


static const int Degeneracy[] = {
    [A1] =  1,
    [A2] =  1,
    [B1] =  1,
    [B2] =  1,
    [E] =   2,
    [E1] =  2,
    [E2] =  2,
    [Eg] =  2,
    [Eu] =  2,
    [E1g] =  2,
    [E2g] =  2,
    [E1u] =  2,
    [E2u] =  2,
    [T] =   3,
    [T1] =  3,
    [T2] =  3,
    [Ag] =  1,
    [Au] =  1,
    [A1g] = 1,
    [A2g] = 1,
    [A3g] = 1,
    [A1u] = 1,
    [A2u] = 1,
    [A3u] = 1,
    [Bg] =  1,
    [Bu] =  1,
    [B1g] = 1,
    [B2g] = 1,
    [B3g] = 1,
    [B1u] = 1,
    [B2u] = 1,
    [B3u] = 1,
    [T1g] = 3,
    [T1u] = 3,
    [T2g] = 3,
    [T2u] = 3,
    [Gg] =  4,
    [Gu] =  4,
    [Hg] =  5,
    [Hu] =  5
};

#define PHIP 1.618033988749894848204586834  //(1+sqrt(5))/2
#define PHIN (-1/PHI)                       //(1-sqrt(5))/2

//          0          1        2        3        4        5        6        7
//          E          C2 (z)   σ (xy)   C2 (x)   C2 (y)   σ (xz)   σ (yz)   i
static const double D2hTable[][8] =
{
    [Ag]  = {1,        1,       1,       1,       1,       1,       1,       1},
    [B1g] = {1,        1,       1,      -1,      -1,      -1,      -1,       1},
    [B2g] = {1,       -1,      -1,      -1,       1,       1,      -1,       1},
    [B3g] = {1,       -1,      -1,       1,      -1,      -1,       1,       1},
    [Au]  = {1,        1,      -1,       1,       1,      -1,      -1,      -1},
    [B1u] = {1,        1,      -1,      -1,      -1,       1,       1,      -1},
    [B2u] = {1,       -1,       1,      -1,       1,      -1,       1,      -1},
    [B3u] = {1,       -1,       1,       1,      -1,       1,      -1,      -1}
};

//            E,      2C4,    σh,     2C'2,   2C''2,  2σv,    2σd,    C2,     2S4,    i,
static const double D4hTable[][10] =
{
    [A1g] =  {1,      1,      1,      1,      1,      1,      1,      1,      1,      1},
    [A2g] =  {1,      1,      1,     -1,     -1,     -1,     -1,      1,      1,      1},
    [B1g] =  {1,     -1,      1,      1,     -1,      1,     -1,      1,     -1,      1},
    [B2g] =  {1,     -1,      1,     -1,      1,     -1,      1,      1,     -1,      1},
    [Eg]  =  {2,      0,     -2,      0,      0,      0,      0,     -2,      0,      2},
    [A1u] =  {1,      1,     -1,      1,      1,     -1,     -1,      1,     -1,     -1},
    [A2u] =  {1,      1,     -1,     -1,     -1,      1,      1,      1,     -1,     -1},
    [B1u] =  {1,     -1,     -1,      1,     -1,     -1,      1,      1,      1,     -1},
    [B2u] =  {1,     -1,     -1,     -1,      1,      1,     -1,      1,      1,     -1},
    [Eu] =   {2,      0,      2,      0,      0,      0,      0,     -2,      0,     -2}
};


//           E      2C6      σh     3C'2    3C''2   3σv     3σd     2C3      C2     2S6     2S3      i
static const double D6hTable[][12] =
{
    [A1g] = {1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1},
    [A2g] = {1,      1,      1,     -1,     -1,     -1,     -1,      1,      1,      1,      1,      1},
    [B1g] = {1,     -1,     -1,      1,     -1,     -1,      1,      1,     -1,      1,     -1,      1},
    [B2g] = {1,     -1,     -1,     -1,      1,      1,     -1,      1,     -1,      1,     -1,      1},
    [E1g] = {2,      1,     -2,      0,      0,      0,      0,     -1,     -2,     -1,      1,      2},
    [E2g] = {2,     -1,      2,      0,      0,      0,      0,     -1,      2,     -1,     -1,      2},
    [A1u] = {1,      1,     -1,      1,      1,     -1,     -1,      1,      1,     -1,     -1,     -1},
    [A2u] = {1,      1,     -1,     -1,     -1,      1,      1,      1,      1,     -1,     -1,     -1},
    [B1u] = {1,     -1,      1,      1,     -1,      1,     -1,      1,     -1,     -1,      1,     -1},
    [B2u] = {1,     -1,      1,     -1,      1,     -1,      1,      1,     -1,     -1,      1,     -1},
    [E1u] = {2,      1,      2,      0,      0,      0,      0,     -1,     -2,      1,     -1,     -2},
    [E2u] = {2,     -1,     -2,      0,      0,      0,      0,     -1,      2,      1,      1,     -2}
};


#define C4PI (-1.61803398874989484820458683436563811772030917980576286213544) //(2*cos(4*M_PI/5))
#define C2PI (0.618033988749894848204586834365638117720309179805762862135448) //(2*cos(2*M_PI/5))

//          E    C2   R    S6   C5   S10  C52  i    C3   S103
static const double IhTable[][10] =
{
    //       E,     15C2,   15σ,    20S6,   12C5,   12S10,  12C52,  i,      20C3,   12S103,
    [Ag]  = {1,     1,      1,      1,      1,      1,      1,      1,      1,      1},
    [T1g] = {3,     -1,     -1,     0,      -C4PI,  -C2PI,  -C2PI,  3,      0,      -C4PI},
    [T2g] = {3,     -1,     -1,     0,      -C2PI,  -C4PI,  -C4PI,  3,      0,      -C2PI},
    [Gg]  = {4,     0,      0,      1,      -1,     -1,     -1,     4,      1,      -1},
    [Hg]  = {5,     1,      1,      -1,     0,      0,      0,      5,      -1,     0},
    [Au]  = {1,     1,      -1,     -1,     1,      -1,     1,      -1,     1,      -1},
    [T1u] = {3,     -1,     1,      0,      -C4PI,  C2PI,   -C2PI,  -3,     0,      C4PI},
    [T2u] = {3,     -1,     1,      0,      -C2PI,  C4PI,   -C4PI,  -3,     0,      C2PI},
    [Gu]  = {4,     0,      0,      -1,     -1,     1,      -1,     -4,     1,      1},
    [Hu]  = {5,     1,      -1,     1,      0,      0,      0,      -5,     -1,     0}
};


//          E   C2  C3  S4  R
static const double TdTable[][5] =
{
    [A1] = {1,  1,  1,  1,  1},
    [A2] = {1,  1,  1, -1, -1},
    [E] =  {2,  2, -1,  0,  0},
    [T1] = {3, -1,  0,  1, -1},
    [T2] = {3, -1,  0, -1,  1}
};

        //E C2 R i
static const double C2hTable[][4] =
{
    [Ag] = {1,  1,  1,  1},
    [Bg] = {1, -1, -1,  1},
    [Au] = {1,  1, -1, -1},
    [Bu] = {1, -1,  1, -1}
};

//          E   C3  R
static const double C3vTable[][3] =
{
    [A1] = {1,  1,  1},
    [A2] = {1,  1, -1},
    [E] =  {2, -1,  0}
};

static const double C4vTable[][5] =
{
    [A1] = {1, 1, 1, 1, 1},
    [A2] = {1, 1, -1, -1, 1},
    [B1] = {1, -1, 1, -1, 1},
    [B2] = {1, -1, -1, 1, 1},
    [E] =  {2, 0, 0, 0, -2}
};

static const enum IrreducibleRepresentationEnum *CnhIrrep[] = {
    [2] = C2hIrrep
};

static const enum IrreducibleRepresentationEnum *CnvIrrep[] = {
    [3] = C3vIrrep,
    [4] = C4vIrrep
};


typedef struct {
    const char *name;
    const double *v;
    int l;
    int d;

} IrreducibleRepresentation;

typedef struct {
    IrreducibleRepresentation *irrep;
    int *classc;
    char (*name)[6];
    int l;
} CharacterTable;

msym_error_t characterTableUnknown(int n, CharacterTable *ct);
msym_error_t characterTableCnv(int n, CharacterTable *ct);
msym_error_t characterTableCnh(int n, CharacterTable *ct);
msym_error_t characterTableDnh(int n, CharacterTable *ct);
msym_error_t characterTableTd(int n, CharacterTable *ct);
msym_error_t characterTableIh(int n, CharacterTable *ct);

void printCharacterTable(CharacterTable *ct);

#endif /* defined(__MSYM__CHARACTER_TABLE_h) */
