//
//  symmetrize.h
//  Symmetry
//
//  Created by Marcus Johansson on 04/02/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__SYMMETRIZE_h
#define __MSYM__SYMMETRIZE_h

#include <stdio.h>

#include "msym.h"
#include "point_group.h"
#include "permutation.h"

msym_error_t symmetrizeMolecule(msym_point_group_t *pg, int esl, msym_equivalence_set_t *es, msym_permutation_t **perm, msym_thresholds_t *thresholds, double *err);
msym_error_t symmetrizeOrbitals(msym_point_group_t *pg, int ssl, msym_subspace_t *ss, int *span, int basisl, msym_orbital_t basis[basisl], msym_thresholds_t *thresholds, double orb[basisl][basisl],double symorb[basisl][basisl]);
msym_error_t symmetrizeTranslation(msym_point_group_t *pg, msym_equivalence_set_t *es, msym_permutation_t *perm, int pi, double translation[3]);


#endif /* defined(__MSYM__SYMMETRIZE_h) */
