//
//  symop.h
//  libmsym
//
//  See accompanying file LICENSE
//
//  Created by Marcus Johansson on 30/10/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__SYMOP_h
#define __MSYM__SYMOP_h

#include <stdio.h>

#include "msym.h"

void symopPow(msym_symmetry_operation_t *A, int pow, msym_symmetry_operation_t *O);
void applySymmetryOperation(msym_symmetry_operation_t *sop,double iv[3], double ov[3]);
void symmetryOperationMatrix(msym_symmetry_operation_t *sop, double m[3][3]);
void copySymmetryOperation(msym_symmetry_operation_t *dst, msym_symmetry_operation_t *src);
msym_symmetry_operation_t *findSymmetryOperation(msym_symmetry_operation_t*, msym_symmetry_operation_t*, int, msym_thresholds_t *thresholds);
void invertSymmetryOperation(msym_symmetry_operation_t *sop, msym_symmetry_operation_t *isop);
void symmetryOperationName(msym_symmetry_operation_t* sop, int l, char buf[l]);
void symmetryOperationShortName(msym_symmetry_operation_t* sop, int l, char buf[l]);
void printSymmetryOperation(msym_symmetry_operation_t *sop);

#endif /* defined(__MSYM__SYMOP_h) */
