//
//  permutation.h
//  Symmetry
//
//  Created by Marcus Johansson on 01/02/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__PERMUTATION_h
#define __MSYM__PERMUTATION_h

#include <stdio.h>
#include "symop.h"
#include "msym_error.h"


//There are better ways of representing a permutation (lika a Lehmer code) but I'll leave that for later
typedef struct _msym_permutation_cycle_t {
    int l;
    int s;
} msym_permutation_cycle_t;

typedef struct _msym_permutation {
    int *p;
    int p_length;
    msym_permutation_cycle_t *c;
    int c_length;
} msym_permutation_t;

typedef struct _msym_permutation_morphism {
    enum {BIJECTION, SURJECTION} type;
    msym_permutation_t *domain;
    msym_permutation_t *codomain;
    int l;
    union {
        int *bijection;
        msym_permutation_cycle_t *surjection;
    } function;
    
} msym_permutation_morphism_t;


msym_error_t findSymmetryOperationPermutations(int l, msym_symmetry_operation_t sops[l], msym_thresholds_t *t, msym_permutation_t **ret);
msym_error_t findPermutation(msym_symmetry_operation_t *sop, int l, double (*v[l])[3], msym_thresholds_t *t, msym_permutation_t *perm);
void freePermutationData(msym_permutation_t *perm);
void permutationMatrix(msym_permutation_t *perm, double m[perm->p_length][perm->p_length]);
msym_error_t findPermutationSubgroups(int l, msym_permutation_t perm[l], int sgmax, msym_symmetry_operation_t *sops, int *subgroupl, msym_subgroup_t **subgroup);

void printPermutation(msym_permutation_t *perm);

#endif /* defined(__MSYM__PERMUTATION_h) */
