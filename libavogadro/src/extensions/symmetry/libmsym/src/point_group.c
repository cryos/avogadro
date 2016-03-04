//
//  point_group.c
//  Symmetry
//
//  Created by Marcus Johansson on 14/04/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//


#include <math.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "symmetry.h"
#include "linalg.h"
#include "point_group.h"
#include "permutation.h"

#define PHI 1.618033988749894848204586834
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510582
#endif

#define CLASSIFICATION_THRESHOLD 0.01


msym_error_t symmetrizePointGroup(msym_point_group_t *ipg, msym_point_group_t **opg, msym_thresholds_t *thresholds);
msym_error_t copyPointGroup(msym_point_group_t *ipg, msym_point_group_t *opg);
msym_error_t reorientAxes(msym_point_group_t *pg, msym_thresholds_t *thresholds);
msym_error_t transformPrimary(msym_point_group_t *pg, msym_thresholds_t *thresholds);
msym_error_t transformSecondary(msym_point_group_t *pg, msym_thresholds_t *thresholds);
msym_error_t setPointGroupOrder(msym_point_group_t *pg);
msym_error_t setPointGroupName(size_t max, int n, msym_point_group_type_t type, char *name);

msym_error_t findSecondaryAxisSigma(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds);
msym_error_t findSecondaryAxisC2(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds);
msym_error_t findSecondaryAxisC3(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds);
msym_error_t findSecondaryAxisC4(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds);
msym_error_t findSecondaryAxisC2C5(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds);


msym_error_t generateSymmetryOperations(msym_point_group_t *pg, msym_thresholds_t *thresholds);

msym_error_t generateSymmetryOperationsImpliedS(msym_point_group_t *pg, msym_thresholds_t *thresholds);
msym_error_t generateSymmetryOperationsImpliedCPow(msym_point_group_t *pg, msym_thresholds_t *thresholds);
msym_error_t generateSymmetryOperationsImpliedSPow(msym_point_group_t *pg, msym_thresholds_t *thresholds);
msym_error_t generateSymmetryOperationsImpliedRot(msym_point_group_t *pg, msym_thresholds_t *thresholds);

void generateSymmetryOperationsCi(msym_point_group_t *pg);
void generateSymmetryOperationsCs(msym_point_group_t *pg);
void generateSymmetryOperationsCn(msym_point_group_t *pg);
void generateSymmetryOperationsCnh(msym_point_group_t *pg);
void generateSymmetryOperationsCnv(msym_point_group_t *pg);
void generateSymmetryOperationsDn(msym_point_group_t *pg);
void generateSymmetryOperationsDnh(msym_point_group_t *pg);
void generateSymmetryOperationsDnd(msym_point_group_t *pg);
void generateSymmetryOperationsS2n(msym_point_group_t *pg);
void generateSymmetryOperationsT(msym_point_group_t *pg);
void generateSymmetryOperationsTd(msym_point_group_t *pg);
void generateSymmetryOperationsTh(msym_point_group_t *pg);
void generateSymmetryOperationsO(msym_point_group_t *pg);
void generateSymmetryOperationsOh(msym_point_group_t *pg);
void generateSymmetryOperationsI(msym_point_group_t *pg);
void generateSymmetryOperationsIh(msym_point_group_t *pg);

void addSymmetryOperationsCi(msym_point_group_t *pg);
void addSymmetryOperationsCs(msym_point_group_t *pg);
void addSymmetryOperationsCn(msym_point_group_t *pg);
void addSymmetryOperationsCnh(msym_point_group_t *pg);
void addSymmetryOperationsCnv(msym_point_group_t *pg);
void addSymmetryOperationsDn(msym_point_group_t *pg);
void addSymmetryOperationsDnh(msym_point_group_t *pg);
void addSymmetryOperationsDnd(msym_point_group_t *pg);
void addSymmetryOperationsS2n(msym_point_group_t *pg);
void addSymmetryOperationsT(msym_point_group_t *pg);
void addSymmetryOperationsTd(msym_point_group_t *pg);
void addSymmetryOperationsTh(msym_point_group_t *pg);
void addSymmetryOperationsO(msym_point_group_t *pg);
void addSymmetryOperationsOh(msym_point_group_t *pg);
void addSymmetryOperationsI(msym_point_group_t *pg);
void addSymmetryOperationsIh(msym_point_group_t *pg);

msym_error_t pointGroupFromName(char *name, msym_point_group_t *pg);

int classifySymmetryOperations(msym_point_group_t *pg);
void sortSymmetryOperations(msym_point_group_t *pg, int classes);

void print_transform(double M[3][3], double axis[3]);



msym_error_t generatePointGroup(char *name, msym_thresholds_t *thresholds, msym_point_group_t **opg){
    msym_error_t ret = MSYM_SUCCESS;
    msym_point_group_t *pg = calloc(1,sizeof(msym_point_group_t));
    if(MSYM_SUCCESS != (ret = pointGroupFromName(name,pg))) goto err;
    if(MSYM_SUCCESS != (ret = generateSymmetryOperations(pg,thresholds))) goto err;
    int classes = classifySymmetryOperations(pg);
    sortSymmetryOperations(pg,classes);
    
    if((pg->type == POINT_GROUP_Cnv && pg->n == 0) || (pg->type == POINT_GROUP_Dnh && pg->n == 0)){
        pg->perm = NULL;
    } else {
        if(MSYM_SUCCESS != (ret = findSymmetryOperationPermutations(pg->sopsl,pg->sops, thresholds, &pg->perm))) goto err;
    }
    for(msym_symmetry_operation_t *s = pg->sops;s < (pg->sops + pg->sopsl);s++){
        if(pg->primary == NULL || (s->type == PROPER_ROTATION && s->order > pg->primary->order)) pg->primary = s;
    }
    
    mleye(3,pg->transform);
    
    *opg = pg;
    return ret;
    
err:
    *opg = NULL;
    free(pg);
    return ret;
}


msym_error_t pointGroupFromName(char *name, msym_point_group_t *pg){
    msym_error_t ret = MSYM_SUCCESS;
    int n = 0, gi = 0, ri = 0;;
    char g = 0, r = 0;
    
    int map[7][6];
    
    
    struct _pg_map {
        int i;
        msym_point_group_type_t type;
    } pg_map[] = {
        {1,  POINT_GROUP_Cn},
        {2,  POINT_GROUP_Cnv},
        {3,  POINT_GROUP_Cnh},
        {4,  POINT_GROUP_Ci},
        {5,  POINT_GROUP_Cs},
        {6,  POINT_GROUP_Dn},
        {7,  POINT_GROUP_Dnh},
        {8,  POINT_GROUP_Dnd},
        {9,  POINT_GROUP_S2n},
        {10, POINT_GROUP_T},
        {11, POINT_GROUP_Th},
        {12, POINT_GROUP_Td},
        {13, POINT_GROUP_O},
        {14, POINT_GROUP_Oh},
        {15, POINT_GROUP_I},
        {16, POINT_GROUP_Ih},
        {17, POINT_GROUP_K},
        {18, POINT_GROUP_Kh}
        
    };
    
    memset(map,0,sizeof(int[7][6]));
    
    map[0][0] =  1;
    map[0][1] =  2;
    map[0][2] =  3;
    map[0][4] =  4;
    map[0][5] =  5;
    map[1][0] =  6;
    map[1][2] =  7;
    map[1][3] =  8;
    map[2][0] =  9;
    map[3][0] =  10;
    map[3][2] =  11;
    map[3][3] =  12;
    map[4][0] =  13;
    map[4][2] =  14;
    map[5][0] =  15;
    map[5][2] =  16;
    map[6][0] =  17;
    map[6][2] =  18;
    
    if(sscanf(name,"%c%d%c",&g,&n,&r) < 2 && sscanf(name,"%c%c",&g,&r) < 1){
        ret = MSYM_INVALID_POINT_GROUP;
        msymSetErrorDetails("Invalid point group name %s",name);
        goto err;
    }
    
    if(n < 0) {
        ret = MSYM_INVALID_POINT_GROUP;
        msymSetErrorDetails("Invalid point group order %d",n);
        goto err;
    }
    
    switch(g){
        case 'C' : gi = 0; break;
        case 'D' : gi = 1; break;
        case 'S' : {
            if(n < 4 || n % 2 != 0){
                ret = MSYM_INVALID_POINT_GROUP;
                msymSetErrorDetails("Improper rotation order (%d) must be even",n);
                goto err;
            }
            gi = 2;
            break;
        }
        case 'T' : gi = 3; break;
        case 'O' : gi = 4; break;
        case 'I' : gi = 5; break;
        case 'K' : gi = 6; break;
        default :
            ret = MSYM_INVALID_POINT_GROUP;
            msymSetErrorDetails("Invalid point group type %c",g);
            goto err;
    }
    
    switch(r){
        case 0   : ri = 0; break;
        case 'v' : ri = 1; break;
        case 'h' : ri = 2; break;
        case 'd' : ri = 3; break;
        case 'i' : ri = 4; break;
        case 's' : ri = 5; break;
        default :
            ret = MSYM_INVALID_POINT_GROUP;
            msymSetErrorDetails("Invalid point group subtype %c",r);
            goto err;
    }
    
    int fi, fil = sizeof(pg_map)/sizeof(pg_map[0]);
    for(fi = 0;fi < fil;fi++){
        if(pg_map[fi].i == map[gi][ri]) break;
    }
    
    if(fi == fil){
        ret = MSYM_INVALID_POINT_GROUP;
        msymSetErrorDetails("Cannot find point group %s",name);
        goto err;
    }
    
    pg->type = pg_map[fi].type;
    pg->n = n;
    if(MSYM_SUCCESS != (ret = setPointGroupOrder(pg))) goto err;
    if(MSYM_SUCCESS != (ret = setPointGroupName(sizeof(pg->name)/sizeof(char),pg->n,pg->type,pg->name))) goto err;
    
err:
    return ret;
    
}


msym_error_t setPointGroupName(size_t max, int n, msym_point_group_type_t type, char *name){
    msym_error_t ret = MSYM_SUCCESS;
    switch(type) {
        case POINT_GROUP_Ci  : snprintf(name,max,"Ci"); break;
        case POINT_GROUP_Cs  : snprintf(name,max,"Cs"); break;
        case POINT_GROUP_Cn  : snprintf(name,max,"C%d",n); break;
        case POINT_GROUP_Cnh : snprintf(name,max,"C%dh",n); break;
        case POINT_GROUP_Cnv : snprintf(name,max,"C%dv",n); break;
        case POINT_GROUP_Dn  : snprintf(name,max,"D%d",n); break;
        case POINT_GROUP_Dnh : snprintf(name,max,"D%dh",n); break;
        case POINT_GROUP_Dnd : snprintf(name,max,"D%dd",n); break;
        case POINT_GROUP_S2n : snprintf(name,max,"S%d",n); break;
        case POINT_GROUP_T   : snprintf(name,max,"T"); break;
        case POINT_GROUP_Td  : snprintf(name,max,"Td"); break;
        case POINT_GROUP_Th  : snprintf(name,max,"Th"); break;
        case POINT_GROUP_O   : snprintf(name,max,"O"); break;
        case POINT_GROUP_Oh  : snprintf(name,max,"Oh"); break;
        case POINT_GROUP_I   : snprintf(name,max,"I"); break;
        case POINT_GROUP_Ih  : snprintf(name,max,"Ih"); break;
        case POINT_GROUP_K   : snprintf(name,max,"K"); break;
        case POINT_GROUP_Kh  : snprintf(name,max,"Kh"); break;
        default :
            msymSetErrorDetails("Unknown point group when determining name");
            ret = MSYM_POINT_GROUP_ERROR;
            goto err;
    }
err:
    return ret;
}

//init point group, copy all point groups so we can free the original list later
msym_error_t createPointGroup(msym_thresholds_t *thresholds,int n, msym_point_group_type_t type, msym_symmetry_operation_t *primary, msym_symmetry_operation_t *sops, unsigned int sopsl, msym_point_group_t **rpg){
    msym_error_t ret = MSYM_SUCCESS;
    
    msym_point_group_t pg = {.n = n, .type = type, .sops = sops, .sopsl = sopsl, .primary = primary, .ct = NULL};

    if(MSYM_SUCCESS != (ret = setPointGroupOrder(&pg))) goto err;
    if(MSYM_SUCCESS != (ret = setPointGroupName(sizeof(pg.name)/sizeof(char),n,type,pg.name))) goto err;
    if(MSYM_SUCCESS != (ret = symmetrizePointGroup(&pg, rpg, thresholds))) goto err;
    if(((*rpg)->type == POINT_GROUP_Cnv && (*rpg)->n == 0) || ((*rpg)->type == POINT_GROUP_Dnh && (*rpg)->n == 0)){
        (*rpg)->perm = NULL;
    } else {
        if(MSYM_SUCCESS != (ret = findSymmetryOperationPermutations((*rpg)->sopsl,(*rpg)->sops, thresholds, &(*rpg)->perm))) goto err;
    }
    
err:
    return ret;
    
}


msym_error_t setPointGroupOrder(msym_point_group_t *pg){
    
    msym_error_t ret = MSYM_SUCCESS;
    switch (pg->type) {
        case (POINT_GROUP_Cs)  :
        case (POINT_GROUP_Ci)  : pg->order = 2; break;
        case (POINT_GROUP_Cn)  :
        case (POINT_GROUP_S2n) : pg->order = pg->n; break;
        case (POINT_GROUP_Cnh) :
        case (POINT_GROUP_Dn)  : pg->order = 2*pg->n; break;
        case (POINT_GROUP_Cnv) : pg->order = (pg->n == 0 ? 2 : 2*pg->n); break; //These will get a little tricky to symmetrize
        case (POINT_GROUP_Dnh) : pg->order = (pg->n == 0 ? 4 : 4*pg->n); break;
        case (POINT_GROUP_Dnd) : pg->order = 4*pg->n; break;
        case (POINT_GROUP_T)   : pg->order = 12; break;
        case (POINT_GROUP_Td)  :
        case (POINT_GROUP_Th)  :
        case (POINT_GROUP_O)   : pg->order = 24; break;
        case (POINT_GROUP_Oh)  : pg->order = 48; break;
        case (POINT_GROUP_I)   : pg->order = 60; break;
        case (POINT_GROUP_Ih)  : pg->order = 120; break;
        case (POINT_GROUP_K)   :
        case (POINT_GROUP_Kh)  : pg->order = 0;
        default                :
            msymSetErrorDetails("Point group has no primary axis for reorientation");
            goto err;
    }    
err:
    return ret;
}

msym_error_t findPointGroup(int sopsl, msym_symmetry_operation_t *sops, msym_thresholds_t *thresholds, msym_point_group_t **pg){
    msym_error_t ret = MSYM_SUCCESS;
    msymSetErrorDetails("");
    int inversion = 0, sigma = 0, nC[6] = {0,0,0,0,0,0}, linear = 0;
    msym_symmetry_operation_t *primary = NULL;
    msym_symmetry_operation_t *s = NULL;
    *pg = NULL;
    
    for(int i = 0;i < sopsl;i++){
        if(sops[i].type == PROPER_ROTATION && sops[i].order == 0){
            linear = 1;
            break;
        } else if (sops[i].type == PROPER_ROTATION && sops[i].order > 2){
            break;
        }
    }
    if(!linear){
        for(int i = 0;i < sopsl;i++){
            switch(sops[i].type){
                case PROPER_ROTATION :
                    if(primary == NULL || sops[i].order > primary->order) primary = &(sops[i]);
                    if(sops[i].order <= 5) nC[sops[i].order]++;
                    break;
                case INVERSION :
                    inversion = 1;
                    break;
                case REFLECTION :
                    sigma = 1;
                    break;
                case IMPROPER_ROTATION :
                    if(s == NULL || sops[i].order > s->order) s = &(sops[i]);
                    break;
                default :
                    break;

            }
        }
        if(nC[3] >= 2) {
            if(nC[5] >= 2){
                if(inversion){
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Ih, primary, sops, sopsl, pg);
                } else {
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_I, primary, sops, sopsl, pg);
                }
            } else if (nC[4] >= 2) {
                if(inversion){
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Oh, primary, sops, sopsl, pg);
                } else {
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_O, primary, sops, sopsl, pg);
                }
                
            } else if (sigma){
                if(inversion){
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Th, primary, sops, sopsl, pg);
                } else {
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Td, primary, sops, sopsl, pg);
                }
            } else {
                ret = createPointGroup(thresholds, primary->order, POINT_GROUP_T, primary, sops, sopsl, pg);
            }
            
        } else if (primary == NULL){
            if(sigma){
                ret = createPointGroup(thresholds, 1, POINT_GROUP_Cs, primary, sops, sopsl, pg);
            } else if(inversion){
                ret = createPointGroup(thresholds, 1, POINT_GROUP_Ci, primary, sops, sopsl, pg);
            } else {
                ret = createPointGroup(thresholds, 1, POINT_GROUP_Cn, primary, NULL, 0, pg);
            }
        } else {
            int nC2 = 0;
            int sigma_h = 0;
            int nsigma_v = 0;
            
            //Special case for D2d
            if(primary->order == 2 && s != NULL && !vparallel(primary->v, s->v, thresholds->angle)){
                for(int i = 0; i < sopsl;i++){
                    if(sops[i].type == PROPER_ROTATION && sops[i].order == 2 && vparallel(sops[i].v, s->v, thresholds->angle)){
                        primary = &sops[i];
                        break;
                    }
                }
            }
            
            for(int i = 0; i < sopsl;i++){
                nC2 += sops[i].type == PROPER_ROTATION && sops[i].order == 2 && vperpendicular(primary->v,sops[i].v, thresholds->angle);
                sigma_h = sigma_h || (sops[i].type == REFLECTION && vparallel(primary->v,sops[i].v,thresholds->angle));
                nsigma_v += (sops[i].type == REFLECTION && vperpendicular(primary->v,sops[i].v,thresholds->angle));
            }            
            if(nC2){ //actually nC2 == primary->order but less is acceptable here since we can generate the rest
                if(sigma_h){
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Dnh, primary, sops, sopsl, pg);
                } else if (nsigma_v){ //actually nsigma_v == primary->order but less is acceptable here since we can generate the rest
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Dnd, primary, sops, sopsl, pg);
                } else {
                    ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Dn, primary, sops, sopsl, pg);
                }
                
            } else if (sigma_h) {
                ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Cnh, primary, sops, sopsl, pg);
            } else if(nsigma_v) { //actually nsigma_v == primary->order but less is acceptable here since we can generate the rest
                ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Cnv, primary, sops, sopsl, pg);
            } else if(s != NULL){
                ret = createPointGroup(thresholds, s->order, POINT_GROUP_S2n, primary, sops, sopsl, pg);
            } else {
                ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Cn, primary, sops, sopsl, pg);
            }
        }
    } else {
        for(int i = 0; i < sopsl;i++){
            inversion = inversion || sops[i].type == INVERSION;
            
            if(sops[i].type == PROPER_ROTATION && (sops[i].order == 0 || primary == NULL)){ //TODO The primary == NULL is needed because of a bug!
                primary = &(sops[i]);
            }
        }
        
        if(inversion){
            ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Dnh, primary, sops, sopsl, pg);
        } else {
            ret = createPointGroup(thresholds, primary->order, POINT_GROUP_Cnv, primary, sops, sopsl, pg);
        }
    }
    
    return ret;
    
err:
    return ret;

}

msym_error_t findSubgroup(msym_subgroup_t *subgroup, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    int inversion = 0, sigma = 0, nC[6] = {0,0,0,0,0,0}, linear = 0;
    msym_symmetry_operation_t *primary = NULL;
    msym_symmetry_operation_t *s = NULL, *sop = NULL;;
    
    
    for(int i = 0;i < subgroup->sopsl;i++){
        if(subgroup->sops[i]->type == PROPER_ROTATION && subgroup->sops[i]->order == 0){
            linear = 1;
            break;
        } else if (subgroup->sops[i]->type == PROPER_ROTATION && subgroup->sops[i]->order > 2){
            break;
        }
    }
    if(!linear){
        for(int i = 0;i < subgroup->sopsl;i++){
            sop = subgroup->sops[i];
            if(sop->power > 1) continue;
            switch(subgroup->sops[i]->type){
                case PROPER_ROTATION :
                    if(primary == NULL || sop->order > primary->order) primary = sop;
                    if(sop->order <= 5) nC[sop->order]++;
                    break;
                case INVERSION :
                    inversion = 1;
                    break;
                case REFLECTION :
                    sigma = 1;
                    break;
                case IMPROPER_ROTATION :
                    if(s == NULL || sop->order > s->order) s = sop;
                    break;
                default :
                    break;
                    
            }
        }
        if(nC[3] >= 2) {
            if(nC[5] >= 2){
                if(inversion){
                    subgroup->type = POINT_GROUP_Ih;
                    subgroup->n = primary->order;
                } else {
                    subgroup->type = POINT_GROUP_I;
                    subgroup->n = primary->order;
                }
            } else if (nC[4] >= 2) {
                if(inversion){
                    subgroup->type = POINT_GROUP_Oh;
                    subgroup->n = primary->order;
                } else {
                    subgroup->type = POINT_GROUP_O;
                    subgroup->n = primary->order;
                }
                
            } else if (sigma){
                if(inversion){
                    subgroup->type = POINT_GROUP_Th;
                    subgroup->n = primary->order;
                } else {
                    subgroup->type = POINT_GROUP_Td;
                    subgroup->n = primary->order;
                }
            } else {
                subgroup->type = POINT_GROUP_T;
                subgroup->n = primary->order;
            }
            
        } else if (primary == NULL){
            if(sigma){
                subgroup->type = POINT_GROUP_Cs;
                subgroup->n = 1;
            } else if(inversion){
                subgroup->type = POINT_GROUP_Ci;
                subgroup->n = 1;
            } else {
                subgroup->type = POINT_GROUP_Cn;
                subgroup->n = 1;
            }
        } else {
            int nC2 = 0;
            int sigma_h = 0;
            int nsigma_v = 0;
            
            if(primary->order == 2 && s != NULL && !vparallel(primary->v, s->v, thresholds->angle)){
                for(int i = 0; i < subgroup->sopsl;i++){
                    sop = subgroup->sops[i];
                    if(sop->power > 1) continue;
                    if(sop->type == PROPER_ROTATION && sop->order == 2 && vparallel(sop->v, s->v, thresholds->angle)){
                        primary = sop;
                        break;
                    }
                }
            }
            
            for(int i = 0; i < subgroup->sopsl;i++){
                sop = subgroup->sops[i];
                if(sop->power > 1) continue;
                nC2 += sop->type == PROPER_ROTATION && sop->order == 2 && vperpendicular(primary->v,sop->v, thresholds->angle);
                sigma_h = sigma_h || (sop->type == REFLECTION && vparallel(primary->v,sop->v,thresholds->angle));
                nsigma_v += (sop->type == REFLECTION && vperpendicular(primary->v,sop->v,thresholds->angle));
            }
            if(nC2 == primary->order){
                if(sigma_h){
                    subgroup->type = POINT_GROUP_Dnh;
                    subgroup->n = primary->order;
                } else if (nsigma_v == primary->order){
                    subgroup->type = POINT_GROUP_Dnd;
                    subgroup->n = primary->order;
                } else {
                    subgroup->type = POINT_GROUP_Dn;
                    subgroup->n = primary->order;
                }
                
            } else if (sigma_h) {
                subgroup->type = POINT_GROUP_Cnh;
                subgroup->n = primary->order;
            } else if(nsigma_v == primary->order) {
                subgroup->type = POINT_GROUP_Cnv;
                subgroup->n = primary->order;
            } else if(s != NULL){
                subgroup->type = POINT_GROUP_S2n;
                subgroup->n = s->order;
            } else {
                subgroup->type = POINT_GROUP_Cn;
                subgroup->n = primary->order;
            }
        }
    } else {
        for(int i = 0; i < subgroup->sopsl;i++){
            inversion = inversion || subgroup->sops[i]->type == INVERSION;
            
            if(subgroup->sops[i]->type == PROPER_ROTATION && (subgroup->sops[i]->order == 0 || primary == NULL)){                 primary = subgroup->sops[i];
            }
        }
        
        if(inversion){
            subgroup->type = POINT_GROUP_Dnh;
            subgroup->n = primary->order;
        } else {
            subgroup->type = POINT_GROUP_Cnv;
            subgroup->n = primary->order;
        }
    }
    
    subgroup->primary = primary;
    if(MSYM_SUCCESS != (ret = setPointGroupName(sizeof(subgroup->name)/sizeof(char),subgroup->n,subgroup->type,subgroup->name))) goto err;
    return ret;
    
    
err:
    return ret;
    
}


msym_error_t transformAxes(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    switch (pg->type){
        case (POINT_GROUP_Ci)  :
        case (POINT_GROUP_K)   :
        case (POINT_GROUP_Kh)  :
            break;
        case (POINT_GROUP_Cs)  :
            for(pg->primary = pg->sops; pg->primary < (pg->primary + pg->sopsl) && pg->primary->type != REFLECTION; pg->primary++){};
        case (POINT_GROUP_Cn)  :
        case (POINT_GROUP_Cnh) :
        case (POINT_GROUP_S2n) :
            if(MSYM_SUCCESS != (ret = reorientAxes(pg,thresholds))) goto err;
            if(MSYM_SUCCESS != (ret = transformPrimary(pg,thresholds))) goto err;
            break;
        case (POINT_GROUP_Cnv) :
        case (POINT_GROUP_Dnh) :
            if(MSYM_SUCCESS != (ret = reorientAxes(pg,thresholds))) goto err;
            if(MSYM_SUCCESS != (ret = transformPrimary(pg,thresholds))) goto err;
            if(pg->n > 0){
                if(MSYM_SUCCESS != (ret = transformSecondary(pg,thresholds))) goto err;
            }
            break;
        case (POINT_GROUP_Dn)  :
        case (POINT_GROUP_Dnd) :
        case (POINT_GROUP_O)   :
        case (POINT_GROUP_Oh)  :
            if(MSYM_SUCCESS != (ret = reorientAxes(pg,thresholds))) goto err;
            if(MSYM_SUCCESS != (ret = transformPrimary(pg,thresholds))) goto err;
            if(MSYM_SUCCESS != (ret = transformSecondary(pg,thresholds))) goto err;
            break;
        case (POINT_GROUP_T)   :
        case (POINT_GROUP_Td)  :
        case (POINT_GROUP_Th)  :
        case (POINT_GROUP_I)   :
        case (POINT_GROUP_Ih)  : {
            msym_symmetry_operation_t *t = pg->primary;
            pg->primary = NULL;
            msym_symmetry_operation_t *sop;
            for(sop = pg->sops; sop < (pg->sops + pg->sopsl); sop++){
                double z = -2.0;
                if(sop->type == PROPER_ROTATION && sop->order == 2){
                    double v[3];
                    vnorm2(sop->v,v);
                    if(v[2] > z || pg->primary == NULL) {
                        pg->primary = sop;
                        z = sop->v[2];
                    }
                }
            }
            if(pg->primary == NULL) {
                pg->primary = t;
                msymSetErrorDetails("Cannot find C2 axis for point group symmetrization of %s",pg->name);
                ret = MSYM_POINT_GROUP_ERROR;
                goto err;
            }
            if(MSYM_SUCCESS != (ret = reorientAxes(pg,thresholds))) goto err;
            if(MSYM_SUCCESS != (ret = transformPrimary(pg,thresholds))) goto err;
            if(MSYM_SUCCESS != (ret = transformSecondary(pg,thresholds))) goto err;
            pg->primary = t;
            break;
        }
            
    }
    
err:
    return ret;
}

msym_error_t pointGroupFromSubgroup(msym_subgroup_t *sg, msym_thresholds_t *thresholds, msym_point_group_t **opg){
    msym_error_t ret = MSYM_SUCCESS;
    *opg = calloc(1,sizeof(msym_point_group_t));
    msym_point_group_t *pg = *opg;
    pg->type = sg->type;
    pg->primary = sg->primary;
    pg->n = sg->n;
    pg->sops = malloc(sizeof(msym_symmetry_operation_t[sg->sopsl]));
    pg->sopsl = sg->sopsl;
    memcpy(pg->name,sg->name,sizeof(pg->name));
    
    if(MSYM_SUCCESS != (ret = setPointGroupOrder(pg))) goto err;
    
    for(int i = 0;i < sg->sopsl;i++){
        if(sg->primary == sg->sops[i]) pg->primary = &pg->sops[i];
        memcpy(&pg->sops[i], sg->sops[i], sizeof(msym_symmetry_operation_t));
    }
    
    mleye(3, pg->transform);
    
    if(MSYM_SUCCESS != (ret = transformAxes(pg, thresholds))) goto err;
    
    /* Unfortunately we need to regenerate these as they need a specific
     * class ordering for orbital symmetrization */
    free(pg->sops);
    pg->sops = NULL;
    pg->sopsl = 0;
    pg->primary = NULL;
    
    if(MSYM_SUCCESS != (ret = generateSymmetryOperations(pg,thresholds))) goto err;
    int classes = classifySymmetryOperations(pg);
    sortSymmetryOperations(pg,classes);
    
    double T[3][3];
    minv(pg->transform, T);
    
    for(int i = 0; i < pg->sopsl;i++){
        mvmul(pg->sops[i].v,T,pg->sops[i].v);
    }
    
    return ret;
err:
    *opg = NULL;
    free(pg->sops);
    free(pg);
    return ret;
}

msym_error_t symmetrizePointGroup(msym_point_group_t *ipg, msym_point_group_t **opg, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    
    *opg = calloc(1,sizeof(msym_point_group_t));
    msym_point_group_t *pg = *opg;
    if(MSYM_SUCCESS != (ret = copyPointGroup(ipg, pg))) goto err;
    if(MSYM_SUCCESS != (ret = generateSymmetryOperationsImpliedRot(pg, thresholds))) goto err;
    if(MSYM_SUCCESS != (ret = transformAxes(pg, thresholds))) goto err;
    
    free(pg->sops);
    pg->sops = NULL;
    pg->sopsl = 0;
    pg->primary = NULL;
    
    if(MSYM_SUCCESS != (ret = generateSymmetryOperations(pg,thresholds))) goto err;
    int classes = classifySymmetryOperations(pg);
    sortSymmetryOperations(pg,classes);
    
    //for(int i = 0;i < pg->sopsl;i++) printSymmetryOperation(&pg->sops[i]);
    
    double T[3][3];
    minv(pg->transform, T);
        
    for(int i = 0; i < pg->sopsl;i++){
        mvmul(pg->sops[i].v,T,pg->sops[i].v);
    }
    return ret;
err:
    free(pg->sops);
    free(pg);
    *opg = NULL;
    return ret;
    
    
}

/* Point primary axes above xy plane and all symops above the primary plane.
 * Not really needed but we may not have to deal with transforms that flip things over
 * This could be improved with some thresholds, since some things lie in the plane
 * and get flipped. Should try to align with x as well
 */
msym_error_t reorientAxes(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    double x[3] = {1.0,0.0,0.0}, y[3] = {0.0,1.0,0.0}, z[3] = {0.0, 0.0, 1.0};
    
    if(pg->primary == NULL) goto err;
    
    if(vdot(pg->primary->v,z) < 0.0) vinv(pg->primary->v);
    
    for(msym_symmetry_operation_t *sop = pg->sops; sop < (pg->sops + pg->sopsl); sop++){
        if(vperpendicular(sop->v, z, thresholds->angle)){
            double proj = vdot(sop->v, y)/vabs(sop->v);
            if(fabs(fabs(proj)-1.0) < thresholds->angle && proj < 0.0) { //along y axis
                vinv(sop->v);
            } else if (vdot(sop->v,x) < 0.0){ //in xy-plane not in y axis, reorient to positive x
                vinv(sop->v);
            }
            
        } else if(vdot(pg->primary->v,sop->v) < 0.0) vinv(sop->v); //not in xy-plane reorient to positive along primary axiz
    }
    
    return MSYM_SUCCESS;
    
err:
    msymSetErrorDetails("Point group has no primary axis for reorientation");
    return MSYM_POINT_GROUP_ERROR;
}


msym_error_t transformPrimary(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    if(pg->primary != NULL){
        double z[3] = {0.0, 0.0, 1.0};
        malign(pg->primary->v,z,pg->transform);
        for(msym_symmetry_operation_t *sop = pg->sops; sop < (pg->sops + pg->sopsl); sop++){
            mvmul(sop->v,pg->transform,sop->v);
        }
        vcopy(z,pg->primary->v); // get rid of small errors
    } else {
        msymSetErrorDetails("Point group has no primary axis for transformation");
        ret = MSYM_POINT_GROUP_ERROR;
    }
    return ret;
}

msym_error_t transformSecondary(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    double axis[3], x[3] = {1.0,0.0,0.0};
    switch(pg->type){
        case (POINT_GROUP_Cnv) :
            if(MSYM_SUCCESS != (ret = findSecondaryAxisSigma(pg, axis, thresholds))) goto err;
            break;
        case (POINT_GROUP_O)   :
        case (POINT_GROUP_Oh)  :
            if(MSYM_SUCCESS != (ret = findSecondaryAxisC4(pg, axis, thresholds))) goto err;
            break;
        case (POINT_GROUP_Dn)  :
        case (POINT_GROUP_Dnh) :
        case (POINT_GROUP_Dnd) :
        case (POINT_GROUP_T)   :
        case (POINT_GROUP_Td)  :
        case (POINT_GROUP_Th)  :
            if(MSYM_SUCCESS != (ret = findSecondaryAxisC2(pg, axis, thresholds))) goto err;
            break;
        case (POINT_GROUP_I)   :
        case (POINT_GROUP_Ih)  :
            if(MSYM_SUCCESS != (ret = findSecondaryAxisC2C5(pg, axis, thresholds))) goto err;
            break;
        default :
            msymSetErrorDetails("Unknown point group when determining secondary axis");
            ret = MSYM_POINT_GROUP_ERROR;
            goto err;
    }
    
    double m[3][3];
    
    malign(axis, x, m);

    for(msym_symmetry_operation_t *sop = pg->sops; sop < (pg->sops + pg->sopsl); sop++){
        mvmul(sop->v,m,sop->v);
    }    
    mmmul(m,pg->transform,pg->transform);
    
err:
    return ret;
}

msym_error_t copyPointGroup(msym_point_group_t *ipg, msym_point_group_t *opg){
    if(ipg->order < ipg->sopsl) goto err;
    memcpy(opg, ipg, sizeof(msym_point_group_t));
    opg->sops = calloc(opg->order,sizeof(msym_symmetry_operation_t));
    memcpy(opg->sops,ipg->sops,ipg->sopsl*sizeof(msym_symmetry_operation_t));
    if(ipg->primary != NULL){
        opg->primary = opg->sops + (ipg->primary - ipg->sops);
    }
    return MSYM_SUCCESS;
err:
    
    msymSetErrorDetails("More symmetry operations than order of point group (%s). Order: %d Number of operations: %d",ipg->name, ipg->order,ipg->sopsl);
    return MSYM_POINT_GROUP_ERROR;
}



/* For point groups where we use a perpendicular reflection plane to indicate direction.
   We use a vector where the the xy-plane and reflection plane cross
*/
msym_error_t findSecondaryAxisSigma(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    msym_symmetry_operation_t *sop = NULL;
    for(sop = pg->sops; sop < (pg->sops + pg->sopsl); sop++){
        if(sop->type == REFLECTION){
            vcross(sop->v, pg->primary->v, r);
            vnorm(r);
            break;
        }
    }
    if(sop == (pg->sops + pg->sopsl)){
        msymSetErrorDetails("Can't find secondary reflection plane when symmetrizing point group");
        ret = MSYM_POINT_GROUP_ERROR;
        goto err;
    }
err:
    return ret;
}

/* For point groups where we use a perpendicular C2 axis to indicate direction.
   Adjusted to make sure it's perfectly in the xy-plane.
 */
msym_error_t findSecondaryAxisC2(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    msym_symmetry_operation_t *sop = NULL;
    for(msym_symmetry_operation_t *sop = pg->sops; sop < (pg->sops + pg->sopsl); sop++){
        //Choose a C2 perpendicular to the primary axis, it'll make things relatively easy
        if(sop != pg->primary && sop->type == PROPER_ROTATION && sop->order == 2 && vperpendicular(sop->v, pg->primary->v,thresholds->angle)){
            vproj_plane(sop->v, pg->primary->v, r);
            vnorm(r);
            break;
        }
    }
    if(sop == (pg->sops + pg->sopsl)){
        msymSetErrorDetails("Can't find secondary C2 axis when symmetrizing point group");
        ret = MSYM_POINT_GROUP_ERROR;
        goto err;
    }
err:
    return ret;
}


msym_error_t findSecondaryAxisC2C5(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    msym_symmetry_operation_t *c2[2], *c5 = NULL;
    int c2i = 0;

    for(msym_symmetry_operation_t *sop = pg->sops; sop < (pg->sops + pg->sopsl) && (c5 == NULL || c2i < 2); sop++){
        if(vperpendicular(sop->v, pg->primary->v,thresholds->angle)){
            if(sop->type == PROPER_ROTATION && sop->order == 2){
                //printf("Found perpendicular C2\n");
                c2[c2i++] = sop;
            } else if (sop->type == PROPER_ROTATION && sop->order == 5){
                //printf("Found perpendicular C5\n");
                c5 = sop;
            }
        }
    }
    
    if(c5 == NULL || c2i < 2) {
        msymSetErrorDetails("Can't find secondary C2 axis when symmetrizing point group: (%s %s)",c5 == NULL ? "C5 axis missing" : "", c2i < 2 ? "C2 axis missing" : "");
        ret = MSYM_POINT_GROUP_ERROR;
        goto err;
    }
    
    if(fabs(vdot(c5->v, c2[0]->v)) > fabs(vdot(c5->v, c2[1]->v))){
        vproj_plane(c2[0]->v, pg->primary->v, r);
    } else {
        vproj_plane(c2[1]->v, pg->primary->v, r);
    }
    
err:
    return ret;
}


msym_error_t findSecondaryAxisC3(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    msym_symmetry_operation_t *sop = NULL;
    for(sop = pg->sops; sop < (pg->sops + pg->sopsl); sop++){
        //Choose any C3 axis and project
        if(sop != pg->primary && sop->type == PROPER_ROTATION && sop->order == 3){
            vproj_plane(sop->v, pg->primary->v, r);
            vnorm(r);
            break;
        }
    }
    if(sop == (pg->sops + pg->sopsl)){
        msymSetErrorDetails("Can't find secondary C3 axis when symmetrizing point group");
        ret = MSYM_POINT_GROUP_ERROR;
        goto err;
    }
err:
    return ret;
}

// Same as C2
msym_error_t findSecondaryAxisC4(msym_point_group_t *pg, double r[], msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    msym_symmetry_operation_t *sop = NULL;
    for(sop = pg->sops; sop < (pg->sops + pg->sopsl); sop++){
        //Choose a C2 perpendicular to the primary axis, it'll make things relatively easy
        if(sop != pg->primary && sop->type == PROPER_ROTATION && sop->order == 4 && vperpendicular(sop->v, pg->primary->v,thresholds->angle)){
            vproj_plane(sop->v, pg->primary->v, r);
            vnorm(r);
            break;
        }
    }
    if(sop == (pg->sops + pg->sopsl)){
        msymSetErrorDetails("Can't find secondary C4 axis when symmetrizing point group");
        ret = MSYM_POINT_GROUP_ERROR;
        goto err;
    }
err:
    return ret;
}

msym_error_t generateSymmetryOperations(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    double origo[3] = {0.0,0.0,0.0};

    pg->sops = malloc(sizeof(msym_symmetry_operation_t[pg->order+1]));
    vcopy(origo,pg->sops[0].v);
    
    pg->sops[0].type = IDENTITY;
    pg->sops[0].order = 0;
    pg->sopsl = 1;
    
    switch (pg->type){
        case (POINT_GROUP_Ci)  :
            generateSymmetryOperationsCi(pg);
            break;
        case (POINT_GROUP_Cs)  :
            generateSymmetryOperationsCs(pg);
            break;
        case (POINT_GROUP_Cn)  :
            generateSymmetryOperationsCn(pg);
            break;
        case (POINT_GROUP_Cnh) :
            generateSymmetryOperationsCnh(pg);
            break;
        case (POINT_GROUP_S2n) :
            generateSymmetryOperationsS2n(pg);
            break;
        case (POINT_GROUP_Cnv) :
            generateSymmetryOperationsCnv(pg);
            break;
        case (POINT_GROUP_Dn)  :
            generateSymmetryOperationsDn(pg);
            break;
        case (POINT_GROUP_Dnh) :
            generateSymmetryOperationsDnh(pg);
            break;
        case (POINT_GROUP_Dnd) :
            generateSymmetryOperationsDnd(pg);
            break;
        case (POINT_GROUP_T)   :
            generateSymmetryOperationsT(pg);
            break;
        case (POINT_GROUP_Td)  :
            generateSymmetryOperationsTd(pg);
            break;
        case (POINT_GROUP_Th)  :
            generateSymmetryOperationsTh(pg);
            break;
        case (POINT_GROUP_O)   :
            generateSymmetryOperationsO(pg);
            break;
        case (POINT_GROUP_Oh)  :
            generateSymmetryOperationsOh(pg);
            break;
        case (POINT_GROUP_I)   :
            generateSymmetryOperationsI(pg);
            break;
        case (POINT_GROUP_Ih)  :
            generateSymmetryOperationsIh(pg);
            break;
        case (POINT_GROUP_K)   :
        case (POINT_GROUP_Kh)  :
            pg->sops = NULL;
            pg->sopsl = 0;
            break;
        default :
            ret = MSYM_POINT_GROUP_ERROR;
            msymSetErrorDetails("Unknown point group when generating symmetry operations");
            goto err;
    }
    
    if(MSYM_SUCCESS != (ret = generateSymmetryOperationsImpliedCPow(pg,thresholds))) goto err;
    if(MSYM_SUCCESS != (ret = generateSymmetryOperationsImpliedS(pg,thresholds))) goto err;
    if(MSYM_SUCCESS != (ret = generateSymmetryOperationsImpliedSPow(pg,thresholds))) goto err;
    if(MSYM_SUCCESS != (ret = generateSymmetryOperationsImpliedRot(pg,thresholds))) goto err;
    
    if(pg->sopsl != pg->order){
        ret = MSYM_POINT_GROUP_ERROR;
        msymSetErrorDetails("Number of generated operations (%d) not equal to point group order (%d)",pg->sopsl,pg->order);
        goto err;
    }
    
    pg->sops = realloc(pg->sops,sizeof(msym_symmetry_operation_t[pg->order]));
    
    return ret;
    
err:
    free(pg->sops);
    pg->sops = NULL;
    return MSYM_POINT_GROUP_ERROR;
    
}

msym_error_t generateSymmetryOperationsImpliedS(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    double origo[3] = {0.0,0.0,0.0};
    int n = pg->sopsl;
    for(msym_symmetry_operation_t *sopi = pg->sops; sopi < (pg->sops + n); sopi++){
        if(sopi->type == REFLECTION){
            for(msym_symmetry_operation_t *sopj = pg->sops; sopj < (pg->sops + n) && pg->sopsl < pg->order; sopj++){
                if(sopj->type == PROPER_ROTATION && sopj->order == 2 && sopj->power == 1 && vparallel(sopi->v, sopj->v,thresholds->angle)){
                    pg->sops[pg->sopsl].type = INVERSION;
                    pg->sops[pg->sopsl].order = 0;
                    pg->sops[pg->sopsl].power = 1;
                    vcopy(origo, pg->sops[pg->sopsl].v);
                    pg->sopsl += !findSymmetryOperation(&(pg->sops[pg->sopsl]), pg->sops, pg->sopsl,thresholds);
                    if(pg->sopsl > pg->order) goto err;
                } else if (sopj->type == PROPER_ROTATION && sopj->power == 1 && sopj->order > 0 && vparallel(sopi->v, sopj->v,thresholds->angle)){
                    
                    copySymmetryOperation(&(pg->sops[pg->sopsl]), sopj);
                    pg->sops[pg->sopsl].type = IMPROPER_ROTATION;
                    pg->sopsl += !findSymmetryOperation(&(pg->sops[pg->sopsl]), pg->sops, pg->sopsl,thresholds);
                    if(pg->sopsl > pg->order) goto err;
                }
            }
        }
    }
    return MSYM_SUCCESS;
err:
    msymSetErrorDetails("Generation of implied symmetry operations by reflection resulted in more operations than point group order");
    return MSYM_POINT_GROUP_ERROR;
}

msym_error_t generateSymmetryOperationsImpliedCPow(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    int n = pg->sopsl;
    for(msym_symmetry_operation_t *sop = pg->sops; sop < (pg->sops + n); sop++){
        if(sop->type == PROPER_ROTATION){
            for(int pow = 2; pow < sop->order && pg->sopsl < pg->order; pow++){
                symopPow(sop, pow, &(pg->sops[pg->sopsl]));
                pg->sopsl += !findSymmetryOperation(&(pg->sops[pg->sopsl]), pg->sops, pg->sopsl,thresholds);
                if(pg->sopsl > pg->order) goto err;
            }
        }
    }
    return MSYM_SUCCESS;
err:
    msymSetErrorDetails("Generation of implied proper rotations resulted in more operations than point group order");
    return MSYM_POINT_GROUP_ERROR;
}

msym_error_t generateSymmetryOperationsImpliedSPow(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    int n = pg->sopsl;
    for(msym_symmetry_operation_t *sop = pg->sops; sop < (pg->sops + n); sop++){
        if(sop->type == IMPROPER_ROTATION){
            int mpow = sop->order % 2 == 1 ? 2*sop->order : sop->order;
            for(int j = 2; j < mpow; j++){
                symopPow(sop, j, &(pg->sops[pg->sopsl]));
                pg->sopsl += !findSymmetryOperation(&(pg->sops[pg->sopsl]), pg->sops, pg->sopsl,thresholds);
                if(pg->sopsl > pg->order) goto err;
            }
        }
    }
    return MSYM_SUCCESS;
err:
    msymSetErrorDetails("Generation of implied improper operations resulted in more operations than point group order");
    return MSYM_POINT_GROUP_ERROR;
}


msym_error_t generateSymmetryOperationsImpliedRot(msym_point_group_t *pg, msym_thresholds_t *thresholds){
    
    int n = pg->sopsl;
    for(msym_symmetry_operation_t *sopi = pg->sops; sopi < (pg->sops + n) && pg->sopsl < pg->order; sopi++){
        if(sopi->type == PROPER_ROTATION){
            for(msym_symmetry_operation_t *sopj = pg->sops; sopj < (pg->sops + n); sopj++){
                int istype = (sopj->type == REFLECTION || sopj->type == IMPROPER_ROTATION || (sopj->type == PROPER_ROTATION));
                if(sopi != sopj && istype && !vparallel(sopi->v, sopj->v,thresholds->angle)){
                    copySymmetryOperation(&(pg->sops[pg->sopsl]), sopj);
                    applySymmetryOperation(sopi,pg->sops[pg->sopsl].v,pg->sops[pg->sopsl].v);
                    pg->sopsl += !findSymmetryOperation(&(pg->sops[pg->sopsl]), pg->sops, pg->sopsl,thresholds);
                    if(pg->sopsl > pg->order) goto err;
                }
            }
        }
    }
    return MSYM_SUCCESS;
err:
    msymSetErrorDetails("Generation of implied symmetry operations by rotation resulted in more operations than point group order");
    return MSYM_POINT_GROUP_ERROR;
}

void generateSymmetryOperationsCi(msym_point_group_t *pg){
    double origo[3] = {0.0,0.0,0.0};
    int n = pg->sopsl;
    
    vcopy(origo,pg->sops[n].v);
    pg->sops[n].type = INVERSION;
    pg->sops[n].order = 0;
    pg->sops[n].power = 1;
    n++;
    
    pg->sopsl = n;
}

void generateSymmetryOperationsCs(msym_point_group_t *pg){
    double z[3] = {0.0,0.0,1.0};
    int n = pg->sopsl;
    
    vcopy(z,pg->sops[n].v);
    pg->sops[n].type = REFLECTION;
    pg->sops[n].order = 0;
    pg->sops[n].power = 1;
    n++;
    
    pg->sopsl = n;
}

void generateSymmetryOperationsCn(msym_point_group_t *pg){
    double z[3] = {0.0,0.0,1.0};
    int n = pg->sopsl;
    
    //Only need to generate the Cn the other will come through pow
    vcopy(z,pg->sops[n].v);
    pg->sops[n].type = PROPER_ROTATION;
    pg->sops[n].order = pg->n;
    pg->sops[n].power = 1;
    
    n++;
    pg->sopsl = n;
}

void generateSymmetryOperationsCnh(msym_point_group_t *pg){
    generateSymmetryOperationsCn(pg);
    generateSymmetryOperationsCs(pg);

}

void generateSymmetryOperationsCnv(msym_point_group_t *pg){
    double y[3] = {0.0,1.0,0.0}, z[3] = {0.0,0.0,1.0};
    int n;
    
    generateSymmetryOperationsCn(pg);
    
    if(pg->n > 0){
        n = pg->sopsl;
        
        vcopy(y,pg->sops[n].v);
        pg->sops[n].type = REFLECTION;
        pg->sops[n].order = 0;
        pg->sops[n].power = 1;
        copySymmetryOperation(&(pg->sops[n+1]), &(pg->sops[n]));
        vrotate(M_PI/pg->n, pg->sops[n].v, z, pg->sops[n+1].v);
        n += 2;
        
        pg->sopsl = n;
    }
    
}

void generateSymmetryOperationsDn(msym_point_group_t *pg){
    double x[3] = {1.0,0.0,0.0}, z[3] = {0.0,0.0,1.0};
    int n;
    
    generateSymmetryOperationsCn(pg);
    
    n = pg->sopsl;
    
    vcopy(x,pg->sops[n].v);
    pg->sops[n].type = PROPER_ROTATION;
    pg->sops[n].order = 2;
    pg->sops[n].power = 1;
    copySymmetryOperation(&(pg->sops[n+1]), &(pg->sops[n]));
    vrotate(M_PI/pg->n, pg->sops[n].v, z, pg->sops[n+1].v);
    n += 2;
    
    pg->sopsl = n;
    
}

void generateSymmetryOperationsDnh(msym_point_group_t *pg){
    double x[3] = {1.0,0.0,0.0}, y[3] = {0.0,1.0,0.0}, z[3] = {0.0,0.0,1.0};
    int n;
    
    generateSymmetryOperationsCnh(pg);
    if(pg->n > 0){
        n = pg->sopsl;
        
        vcopy(x,pg->sops[n].v);
        pg->sops[n].type = PROPER_ROTATION;
        pg->sops[n].order = 2;
        pg->sops[n].power = 1;
        copySymmetryOperation(&(pg->sops[n+1]), &(pg->sops[n]));
        vrotate(M_PI/pg->n, pg->sops[n].v, z, pg->sops[n+1].v);
        n += 2;
        
        vcopy(y,pg->sops[n].v);
        pg->sops[n].type = REFLECTION;
        pg->sops[n].order = 0;
        pg->sops[n].power = 1;
        copySymmetryOperation(&(pg->sops[n+1]), &(pg->sops[n]));
        vrotate(M_PI/pg->n, pg->sops[n].v, z, pg->sops[n+1].v);
        n += 2;
        
        pg->sopsl = n;
    } else {
        n = pg->sopsl;
        
        pg->sops[n].type = INVERSION;
        pg->sops[n].order = 0;
        pg->sops[n].power = 1;
        
        pg->sopsl++;
    }
}

void generateSymmetryOperationsDnd(msym_point_group_t *pg){
    double z[3] = {0.0,0.0,1.0}, x[3] = {1.0,0.0,0.0};
    int n;
    
    generateSymmetryOperationsDn(pg);
    
    n = pg->sopsl;
    
    vrotate(M_PI/(2*pg->n),x,z,pg->sops[n].v);
    vcrossnorm(pg->sops[n].v,z,pg->sops[n].v);
    //vcopy(x,pg->sops[n].v);
    pg->sops[n].type = REFLECTION;
    pg->sops[n].order = 0;
    pg->sops[n].power = 1;
    copySymmetryOperation(&(pg->sops[n+1]), &(pg->sops[n]));
    vrotate(M_PI/pg->n, pg->sops[n].v, z, pg->sops[n+1].v);
    n += 2;
    
    vcopy(z,pg->sops[n].v);
    pg->sops[n].type = IMPROPER_ROTATION;
    pg->sops[n].order = 2*pg->n;
    pg->sops[n].power = 1;
    n++;
    
    pg->sopsl = n;
    
}

void generateSymmetryOperationsS2n(msym_point_group_t *pg){
    double z[3] = {0.0,0.0,1.0};
    int n = pg->sopsl;
    
    vcopy(z,pg->sops[n].v);
    pg->sops[n].type = IMPROPER_ROTATION;
    pg->sops[n].order = pg->n;
    pg->sops[n].power = 1;
    n++;
    
    pg->sopsl = n;
}

void generateSymmetryOperationsT(msym_point_group_t *pg){
    double v[4][3] = { {1.0,1.0,1.0}, {-1.0,1.0,1.0}, {1.0,-1.0,1.0}, {-1.0,-1.0,1.0} };
    
    int n;
    
    pg->n = 2;
    generateSymmetryOperationsDn(pg);
    pg->n = 3;
    
    n = pg->sopsl;
    
    for (int i = 0; i < 4; n++,i++){
        vnorm(v[i]);
        vcopy(v[i], pg->sops[n].v);
        pg->sops[n].type = PROPER_ROTATION;
        pg->sops[n].order = 3;
        pg->sops[n].power = 1;
    }
    
    pg->sopsl = n;
    
}

void generateSymmetryOperationsTd(msym_point_group_t *pg){
    double v[3][3] = { {1.0,0.0,0.0}, {0.0,1.0,0.0}, {0.0,0.0,1.0} };
    double xy[3] = {1.0,1.0,0.0};
    
    int n;
    
    generateSymmetryOperationsT(pg);
    
    n = pg->sopsl;
    
    for (int i = 0; i < 3; n++,i++){
        vnorm(v[i]);
        vcopy(v[i], pg->sops[n].v);
        pg->sops[n].type = IMPROPER_ROTATION;
        pg->sops[n].order = 4;
        pg->sops[n].power = 1;
    }
    
    vnorm(xy);
    vcopy(xy, pg->sops[n].v);
    pg->sops[n].type = REFLECTION;
    pg->sops[n].order = 0;
    pg->sops[n].power = 1;
    n++;
    
    pg->sopsl = n;
    
}

void generateSymmetryOperationsTh(msym_point_group_t *pg){
    double v[4][3] = { {1.0,1.0,1.0}, {-1.0,1.0,1.0}, {1.0,-1.0,1.0}, {-1.0,-1.0,1.0} };
    
    int n;
    
    pg->n = 2;
    generateSymmetryOperationsDnh(pg);
    pg->n = 3;
    
    n = pg->sopsl;
    
    for (int i = 0; i < 4; n++,i++){
        vnorm(v[i]);
        vcopy(v[i], pg->sops[n].v);
        pg->sops[n].type = IMPROPER_ROTATION;
        pg->sops[n].order = 6;
        pg->sops[n].power = 1;
    }
    
    pg->sopsl = n;
}

void generateSymmetryOperationsO(msym_point_group_t *pg){
    double v[3][3] = { {1.0,0.0,0.0}, {0.0,1.0,0.0}, {0.0,0.0,1.0} }, xy[3] = {1.0,1.0,0.0};
    int n;
    
    pg->n = 4;
    generateSymmetryOperationsT(pg);
    
    n = pg->sopsl;
    
    vnorm(xy);
    vcopy(xy, pg->sops[n].v);
    pg->sops[n].type = PROPER_ROTATION;
    pg->sops[n].order = 2;
    pg->sops[n].power = 1;
    n++;
    
    for (int i = 0; i < 3; n++,i++){
        vnorm(v[i]);
        vcopy(v[i], pg->sops[n].v);
        pg->sops[n].type = PROPER_ROTATION;
        pg->sops[n].order = 4;
        pg->sops[n].power = 1;
    }
    
    pg->sopsl = n;
    
    
}

void generateSymmetryOperationsOh(msym_point_group_t *pg){
    double v[3][3] = { {1.0,0.0,0.0}, {0.0,1.0,0.0}, {0.0,0.0,1.0} }, xy[3] = {1.0,1.0,0.0};
    int n;
    
    pg->n = 4;
    generateSymmetryOperationsTh(pg);
    
    n = pg->sopsl;
    
    vnorm(xy);
    vcopy(xy, pg->sops[n].v);
    pg->sops[n].type = PROPER_ROTATION;
    pg->sops[n].order = 2;
    pg->sops[n].power = 1;
    n++;
    
    vcopy(xy, pg->sops[n].v);
    pg->sops[n].type = REFLECTION;
    pg->sops[n].order = 0;
    pg->sops[n].power = 1;
    n++;
    
    
    for (int i = 0; i < 3; n++,i++){
        vnorm(v[i]);
        vcopy(v[i], pg->sops[n].v);
        pg->sops[n].type = PROPER_ROTATION;
        pg->sops[n].order = 4;
        pg->sops[n].power = 1;
    }
    
    pg->sopsl = n;
}

void generateSymmetryOperationsI(msym_point_group_t *pg){
    double v[6][3] = { {PHI,1.0,0.0}, {-PHI,1.0,0.0}, {0.0,PHI,1.0}, {0.0,-PHI,1.0}, {1.0,0.0,PHI}, {1.0,0.0,-PHI} };
    
    int n;
    
    pg->n = 4;
    generateSymmetryOperationsT(pg);
    
    n = pg->sopsl;
    
    for (int i = 0; i < 6; n++,i++){
        vnorm(v[i]);
        vcopy(v[i], pg->sops[n].v);
        pg->sops[n].type = PROPER_ROTATION;
        pg->sops[n].order = 5;
        pg->sops[n].power = 1;
    }
    
    pg->sopsl = n;
    
}

void generateSymmetryOperationsIh(msym_point_group_t *pg){
    double v[6][3] = { {PHI,1.0,0.0}, {-PHI,1.0,0.0}, {0.0,PHI,1.0}, {0.0,-PHI,1.0}, {1.0,0.0,PHI}, {1.0,0.0,-PHI} };
    
    int n;
    
    pg->n = 4;
    generateSymmetryOperationsTh(pg);
    
    n = pg->sopsl;
    
    for (int i = 0; i < 6; n++,i++){
        vnorm(v[i]);
        vcopy(v[i], pg->sops[n].v);
        pg->sops[n].type = PROPER_ROTATION;
        pg->sops[n].order = 5;
        pg->sops[n].power = 1;
    }
    
    for (int i = 0; i < 6; n++,i++){
        vnorm(v[i]);
        vcopy(v[i], pg->sops[n].v);
        pg->sops[n].type = IMPROPER_ROTATION;
        pg->sops[n].order = 10;
        pg->sops[n].power = 1;
    }
    
    pg->sopsl = n;
}

int classifySymmetryOperations(msym_point_group_t *pg){
    int c = 1;
    double (*mop)[3][3] = malloc(sizeof(double[pg->sopsl][3][3]));
    double (*imop)[3][3] = malloc(sizeof(double[pg->sopsl][3][3]));
    
    //There may be a better way to do this
    for(int i = 0; i < pg->sopsl;i++){
        if(pg->sops[i].type == IDENTITY){
            pg->sops[i].cla = 0;
        } else {
            pg->sops[i].cla = -1;
        }
        msym_symmetry_operation_t isop;
        invertSymmetryOperation(&(pg->sops[i]), &isop);
        symmetryOperationMatrix(&(pg->sops[i]), mop[i]);
        symmetryOperationMatrix(&isop, imop[i]);
    }
    
    for(int i = 0; i < pg->sopsl;i++){
        if(pg->sops[i].cla < 0){
            pg->sops[i].cla = c;
            for(int j = 0; j < pg->sopsl;j++){
                double m[3][3];
                mmmul(mop[i], imop[j], m);
                mmmul(mop[j],m,m);
                for(int k = 0; k < pg->sopsl;k++){
                    if(mequal(mop[k],m,CLASSIFICATION_THRESHOLD)){ //Don't need to be dynamic, this is done on generated point groups (always the same)
                        pg->sops[k].cla = c;
                    }
                }
            }
            c++;
        }
    }
    
    free(mop);
    free(imop);
    
    return c;
    
    
}

//cant be bothereed writing an efficient sorting alg for this
void sortSymmetryOperations(msym_point_group_t *pg, int classes){
    msym_symmetry_operation_t *tmp = malloc(pg->sopsl*sizeof(msym_symmetry_operation_t));
    int n = 0;
    
    for(int c = 0; c < classes;c++){
        for(int i = 0; i < pg->sopsl;i++){
            if(pg->sops[i].cla == c){
                copySymmetryOperation(&tmp[n], &pg->sops[i]);
                n++;
            }
        }
    }
    memcpy(pg->sops, tmp,pg->sopsl*sizeof(msym_symmetry_operation_t));

    free(tmp);
}

int numberOfSubgroups(msym_point_group_t *pg){
    
    int n = pg->n;
    int size = 0, ndiv = n >= 2, sdiv = 0, nodd = 0, sodd = 0, neven = 0, seven = 0;
    
    switch (pg->type) {
        case POINT_GROUP_Kh  : size = -1; break;
        case POINT_GROUP_K   : size = -1; break;
        case POINT_GROUP_Ih  : size = 162; break;
        case POINT_GROUP_I   : size = 57; break;
        case POINT_GROUP_Oh  : size = 96; break;
        case POINT_GROUP_O   : size = 28; break;
        case POINT_GROUP_Th  : size = 24; break;
        case POINT_GROUP_Td  : size = 28; break;
        case POINT_GROUP_T   : size = 9; break;
        case POINT_GROUP_Ci  : size = 0; break;
        case POINT_GROUP_Cs  : size = 0; break;
        default: {
            for(int i = 2; i < n;i++){
                if(n % i == 0){ ndiv++; sdiv += i; }
            }
            for(int i = 3; i < n;i += 2){
                if(n % i == 0){ nodd++; sodd += i;}
            }
            for(int i = 4; i <= n;i += 2){
                if(n % i == 0){ neven++; seven += (n << 1)/i; }
            }
            switch (pg->type) {
                case POINT_GROUP_Cnh : {
                    size = 2*ndiv;
                    if(n % 2 == 0){
                        int n2 = n >> 1;
                        for(int i = 2; i < n2;i++){
                            if(n2 % i == 0){ size++;}
                        }
                        size += 1 + (n2 >= 2);
                    }
                    break;
                }
                case POINT_GROUP_Dn  :
                case POINT_GROUP_Cnv : size = n + ndiv + sdiv; break;
                case POINT_GROUP_Cn  : size = ndiv - 1; break;
                case POINT_GROUP_Dnh : {
                    if(n % 2 == 0) size = 4*n + 2*ndiv + 3*sdiv + 4 + neven + seven;
                    else size = 3*(n+sdiv+1) + 2*ndiv;
                    break;
                }
                case POINT_GROUP_Dnd : {
                    if(n % 2 == 0) size = 2*n + 3 + ndiv + 2*sdiv + nodd + sodd;
                    else size = 3*(n+sdiv+1) + 2*ndiv;
                    break;
                }
                case POINT_GROUP_S2n : size = ndiv - 1; break;
                default : break;
            }
        }
    }
    
    return size;
}

msym_error_t findCharacterTable(msym_point_group_t *pg){
    
    const struct _fmap {
        msym_point_group_type_t type;
        msym_error_t (*f)(int, CharacterTable*);
        
    } fmap[18] = {
        
        [ 0] = {POINT_GROUP_Ci, characterTableUnknown},
        [ 1] = {POINT_GROUP_Cs, characterTableUnknown},
        [ 2] = {POINT_GROUP_Cn, characterTableUnknown},
        [ 3] = {POINT_GROUP_Cnh,characterTableCnh},
        [ 4] = {POINT_GROUP_Cnv,characterTableCnv},
        [ 5] = {POINT_GROUP_Dn, characterTableUnknown},
        [ 6] = {POINT_GROUP_Dnh,characterTableDnh},
        [ 7] = {POINT_GROUP_Dnd,characterTableUnknown},
        [ 8] = {POINT_GROUP_S2n,characterTableUnknown},
        [ 9] = {POINT_GROUP_T,  characterTableUnknown},
        [10] = {POINT_GROUP_Td, characterTableTd},
        [11] = {POINT_GROUP_Th, characterTableUnknown},
        [12] = {POINT_GROUP_O,  characterTableUnknown},
        [13] = {POINT_GROUP_Oh, characterTableUnknown},
        [14] = {POINT_GROUP_I,  characterTableUnknown},
        [15] = {POINT_GROUP_Ih, characterTableIh},
        [16] = {POINT_GROUP_K,  characterTableUnknown},
        [17] = {POINT_GROUP_Kh, characterTableUnknown}
    };
    
    msym_error_t ret = MSYM_SUCCESS;
    
    CharacterTable *ct = malloc(sizeof(CharacterTable));
    
    int fi, fil = sizeof(fmap)/sizeof(fmap[0]);
    for(fi = 0; fi < fil;fi++){
        if(fmap[fi].type == pg->type) {
            if(MSYM_SUCCESS != (ret = fmap[fi].f(pg->n,ct))) goto err;
            break;
        }
    }
    
    if(fi == fil){
        msymSetErrorDetails("Unknown point group when finding character table");
        ret = MSYM_POINT_GROUP_ERROR;
        goto err;
    }
    
    ct = realloc(ct, sizeof(CharacterTable)+sizeof(int[ct->l])+ct->l*sizeof(*ct->name));

    ct->classc = (int*)(ct + 1);
    ct->name = (char (*)[6]) ((int *)ct->classc + ct->l);
        
    memset(ct->classc, 0, sizeof(int[ct->l]));
    memset(ct->name, 0, ct->l*sizeof(*(ct->name)));
    for(int i = 0; i < pg->sopsl;i++){
        ct->classc[pg->sops[i].cla]++;
        symmetryOperationShortName(&pg->sops[i], sizeof(*(ct->name)), ct->name[pg->sops[i].cla]);
    }
    pg->ct = ct;
    return ret;
err:
    free(ct);
    return ret;
}

void printPointGroup(msym_point_group_t *pg){
    char buf[64];
    if(pg == NULL){
        printf("No point group\n");
        return;
    }
    printf("PointGroup %s (%d,%d)\nPrimary:\n",pg->name, pg->order, pg->sopsl);
    if(pg->primary != NULL) {
        symmetryOperationName(pg->primary, 64, buf);
        printf("%s\n",buf);
    } else {
        printf("No primary rotation axis\n");
    }
    for(int i = 0; i < pg->sopsl;i++){
        symmetryOperationName(&pg->sops[i], 64, buf);
        printf("\t%s\n",buf);
    }
}


void print_transform(double M[3][3], double axis[3]){
    
    fprintf(stderr,"M = \n");
    fprintf(stderr,"[[%lf, %lf, %lf], ",M[0][0],M[0][1],M[0][2]);
    fprintf(stderr,"[%lf, %lf, %lf], ",M[1][0],M[1][1],M[1][2]);
    fprintf(stderr,"[%lf, %lf, %lf]]\n",M[2][0],M[2][1],M[2][2]);
    
    double v[3];
    mvmul(axis,M,v);
    fprintf(stderr,"After transform:\n");
    fprintf(stderr,"[%lf, %lf, %lf]\n",v[0],v[1],v[2]);
    
}
