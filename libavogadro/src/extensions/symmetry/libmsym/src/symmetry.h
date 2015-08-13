//
//  symmetry.h
//  libmsym
//
//  Created by Marcus Johansson on 12/04/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM_SYMMETRY_h
#define __MSYM_SYMMETRY_h

#include "msym.h"
#include "symop.h"

msym_error_t findSymmetryOperations(int esl, msym_equivalence_set_t es[esl], msym_thresholds_t *t, int *lsops, msym_symmetry_operation_t **sops);

#endif /* defined(__MSYM_SYMMETRY_h) */
