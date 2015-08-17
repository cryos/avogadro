//
//  equivalence_set.h
//  libmsym
//
//  Created by Marcus Johansson on 08/02/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__EQUIVALENCE_SET_h
#define __MSYM__EQUIVALENCE_SET_h

#include <stdio.h>
#include "msym.h"
#include "point_group.h"

msym_error_t copyEquivalenceSets(int length, msym_equivalence_set_t es[length], msym_equivalence_set_t **ces);
msym_error_t findEquivalenceSets(int length, msym_element_t *elements[length], msym_geometry_t g, int *esl, msym_equivalence_set_t **es, msym_thresholds_t *thresholds);
msym_error_t findPointGroupEquivalenceSets(msym_point_group_t *pg, int length, msym_element_t *elements[length], int *esl, msym_equivalence_set_t **es, msym_thresholds_t *thresholds);
msym_error_t splitPointGroupEquivalenceSets(msym_point_group_t *pg, int esl, msym_equivalence_set_t es[esl], int *sesl, msym_equivalence_set_t **ses, msym_thresholds_t *thresholds);
msym_error_t generateEquivalenceSet(msym_point_group_t *pg, int length, msym_element_t elements[length], int *glength, msym_element_t **gelements, int *esl, msym_equivalence_set_t **es,msym_thresholds_t *thresholds);

#endif /* defined(__MSYM__EQUIVALENCE_SET_h) */
