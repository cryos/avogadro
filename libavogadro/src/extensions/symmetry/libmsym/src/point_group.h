//
//  point_group.h
//  Symmetry
//
//  Created by Marcus Johansson on 28/11/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__POINT_GROUP_h
#define __MSYM__POINT_GROUP_h

#include "msym.h"
#include "symop.h"
#include "geometry.h"
#include "character_table.h"
#include "permutation.h"

//We cant handle names larger than e.g. D999v (should be more than sufficient, we can see the order in the group anyways)
typedef struct {
    msym_point_group_type_t type;
    int n;
    int order;
    msym_symmetry_operation_t *primary;
    msym_symmetry_operation_t *sops;
    msym_permutation_t *perm;
    int sopsl;
    double transform[3][3];
    CharacterTable *ct;
    char name[6];
} msym_point_group_t;

msym_error_t findPointGroup(int sopsl, msym_symmetry_operation_t *sops, msym_thresholds_t *thresholds, msym_point_group_t **pg);
msym_error_t findSubgroup(msym_subgroup_t *subgroup, msym_thresholds_t *thresholds);
msym_error_t findCharacterTable(msym_point_group_t *pg);
msym_error_t generatePointGroup(char *name, msym_thresholds_t *thresholds, msym_point_group_t **opg);
msym_error_t pointGroupFromSubgroup(msym_subgroup_t *sg, msym_thresholds_t *thresholds, msym_point_group_t **opg);
int numberOfSubgroups(msym_point_group_t *pg);

#endif /* defined(__MSYM__POINT_GROUP_h) */
