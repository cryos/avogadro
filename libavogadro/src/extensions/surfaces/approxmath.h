/**********************************************************************
  ApproxMath - Approximate functions for orbital rendering

  Copyright (C) 2010 Geoffrey Hutchison
  See comments for copyright on particular functions

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef APPROXMATH_H
#define APPROXMATH_H

#include <math.h>

// Should compiled with -fno-strict-aliasing
// From http://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
// "In my tests it usually within an error margin of 5% to 12%"
// Also points out another pow() implementation:
//   http://jrfonseca.blogspot.com/2008/09/fast-sse2-pow-tables-or-polynomials.html
#ifndef APOW_CONST
#define APOW_CONST 1072632447
#endif
inline double apow(double a, double b) {
    int tmp = (*(1 + (int *)&a));
    int tmp2 = (int)(b * (tmp - APOW_CONST) + APOW_CONST);
    double p = 0.0;
    *(1 + (int * )&p) = tmp2;
    return p;
}

// Approximate exp from the VMD project: http://www.ks.uiuc.edu/Research/vmd/
/*
 * David J. Hardy
 * 12 Dec 2008
 *
 * aexpfnx() - Approximate expf() for negative x.
 *
 * Assumes that x <= 0.
 *
 * Assumes IEEE format for single precision float, specifically:
 * 1 sign bit, 8 exponent bits biased by 127, and 23 mantissa bits.
 *
 * Interpolates exp() on interval (-1/log2(e), 0], then shifts it by
 * multiplication of a fast calculation for 2^(-N).  The interpolation
 * uses a linear blending of 3rd degree Taylor polynomials at the end
 * points, so the approximation is once differentiable.
 *
 * The error is small (max relative error per interval is calculated
 * to be 0.131%, with a max absolute error of -0.000716).
 *
 * The cutoff is chosen so as to speed up the computation by early
 * exit from function, with the value chosen to give less than the
 * the max absolute error.  Use of a cutoff is unnecessary, except
 * for needing to shift smallest floating point numbers to zero,
 * i.e. you could remove cutoff and replace by:
 *
 * #define MINXNZ  -88.0296919311130  // -127 * log(2)
 *
 *   if (x < MINXNZ) return 0.f;
 *
 * Use of a cutoff causes a discontinuity which can be eliminated
 * through the use of a switching function.
 *
 * We can obtain arbitrarily smooth approximation by taking k+1 nodes on
 * the interval and weighting their respective Taylor polynomials by the
 * kth order Lagrange interpolant through those nodes.  The wiggle in the
 * polynomial interpolation due to equidistant nodes (Runge's phenomenon)
 * can be reduced by using Chebyshev nodes.
 */

#define MLOG2EF    -1.44269504088896f

/*
 * Interpolating coefficients for linear blending of the
 * 3rd degree Taylor expansion of 2^x about 0 and -1.
 */
#define SCEXP0     1.0000000000000000f
#define SCEXP1     0.6987082824680118f
#define SCEXP2     0.2633174272827404f
#define SCEXP3     0.0923611991471395f
#define SCEXP4     0.0277520543324108f

/* for single precision float */
#define EXPOBIAS   127
#define EXPOSHIFT   23

/* cutoff is optional, but can help avoid unnecessary work */
#define ACUTOFF    -10

typedef union flint_t {
  float f;
  int n;
} flint;

float aexpfnx(float x) {
  /* assume x <= 0 */
  float mb;
  int mbflr;
  float d;
  float sy;
  flint scalfac;

  if (x < ACUTOFF) return 0.f;

  mb = x * MLOG2EF;    /* change base to 2, mb >= 0 */
  mbflr = (int) mb;    /* get int part, floor() */
  d = mbflr - mb;      /* remaining exponent, -1 < d <= 0 */
  sy = SCEXP0 + d*(SCEXP1 + d*(SCEXP2 + d*(SCEXP3 + d*SCEXP4)));
                       /* approx with linear blend of Taylor polys */
  scalfac.n = (EXPOBIAS - mbflr) << EXPOSHIFT;  /* 2^(-mbflr) */
  return (sy * scalfac.f);  /* scaled approx */
}

#endif
