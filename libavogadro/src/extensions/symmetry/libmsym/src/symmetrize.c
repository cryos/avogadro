//
//  symmetrize.c
//  Symmetry
//
//  Created by Marcus Johansson on 04/02/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "symmetrize.h"
#include "orbital.h"
#include "linalg.h"

#define SQR(x) ((x)*(x))

msym_error_t projectOntoSubspace(int d, double orb[d], msym_subspace_t *ss, msym_orbital_t basis[d], double mem[d], double proj[d]);

msym_error_t symmetrizeMoleculeProject(msym_point_group_t *pg, int esl, msym_equivalence_set_t *es, msym_permutation_t **perm, msym_thresholds_t *thresholds, double *err);
msym_error_t symmetrizeMoleculeLinear(msym_point_group_t *pg, int esl, msym_equivalence_set_t *es, msym_permutation_t **perm, msym_thresholds_t *thresholds, double *err);

msym_error_t symmetrizeMolecule(msym_point_group_t *pg, int esl, msym_equivalence_set_t *es, msym_permutation_t **perm, msym_thresholds_t *thresholds, double *err){
    msym_error_t ret = MSYM_SUCCESS;
    if((pg->type == POINT_GROUP_Cnv || pg->type == POINT_GROUP_Dnh) && pg->n == 0){
        ret = symmetrizeMoleculeLinear(pg,esl,es,perm,thresholds,err);
    } else {
        ret = symmetrizeMoleculeProject(pg,esl,es,perm,thresholds,err);
    }
    
    return ret;
}
/* This is a projection into the fully symmetric space.
 * A little more computation than if we just recreate it from one atom,
 * but it is independant of the chosen atom and we can get the size
 * of the fully symmetric component.
 * The sizes of the individual equivalence sets are rather small anyways.
 */

msym_error_t symmetrizeMoleculeProject(msym_point_group_t *pg, int esl, msym_equivalence_set_t *es, msym_permutation_t **perm, msym_thresholds_t *thresholds, double *err){
    msym_error_t ret = MSYM_SUCCESS;
    double e = 0.0;
    double (*v)[3] = malloc(sizeof(double[pg->order][3]));
    for(int i = 0; i < esl;i++){
        if(es[i].length > pg->order){
            ret = MSYM_SYMMETRIZATION_ERROR;
            msymSetErrorDetails("Equivalence set (%d elements) larger than order of point group (%d)",es[i].length,pg->order);
            goto err;
        }
        memset(v, 0, sizeof(double[pg->order][3]));
        for(int j = 0; j < pg->sopsl;j++){
            for(int k = 0; k < es[i].length;k++){
                int p = perm[i][j].p[k];
                double sv[3];
                applySymmetryOperation(&pg->sops[j], es[i].elements[k]->v, sv);
                vadd(sv, v[p], v[p]);
            }
        }
        double sl = 0.0, ol = 0.0;
        for(int j = 0; j < es[i].length;j++){
            ol += vdot(es[i].elements[j]->v,es[i].elements[j]->v);
            sl += vdot(v[j],v[j]);
            vscale(1.0/((double)pg->order), v[j], es[i].elements[j]->v);
        }
        sl /= SQR((double)pg->order);
        if(!(es[i].length == 1 && ol <= thresholds->zero)) e += (ol-sl)/ol; //e = fmax(e,(ol-sl)/ol);
    }
    
    *err = sqrt(fmax(e,0.0)); //should never be < 0, but it's a dumb way to die
err:
    free(v);
    return ret;
}

msym_error_t symmetrizeMoleculeLinear(msym_point_group_t *pg, int esl, msym_equivalence_set_t *es, msym_permutation_t **perm, msym_thresholds_t *thresholds, double *err){
    msym_error_t ret = MSYM_SUCCESS;
    double e = 0.0;
    double (*v)[3] = malloc(sizeof(double[pg->order][3]));
    double (*vinf)[3] = malloc(sizeof(double[pg->order][3]));
    msym_symmetry_operation_t *cinf = NULL;
    
    for(int i = 0; i < pg->sopsl;i++){
        if(pg->sops[i].type == PROPER_ROTATION && pg->sops[i].order == 0) {
            cinf = &pg->sops[i];
            break;
        }
    }
    
    if(cinf == NULL){
        ret = MSYM_SYMMETRIZATION_ERROR;
        msymSetErrorDetails("Cannot find Cinf operation in linear point group");
        goto err;
    }
    
    for(int i = 0; i < esl;i++){
        if(es[i].length > pg->order){
            ret = MSYM_SYMMETRIZATION_ERROR;
            msymSetErrorDetails("Equivalence set (%d elements) larger than order of point group (%d)",es[i].length,pg->order);
            goto err;
        }
        
        memset(v, 0, sizeof(double[pg->order][3]));
        
        for(int k = 0; k < es[i].length;k++){
            vproj(es[i].elements[k]->v, cinf->v, vinf[k]);
        }
        
        for(int j = 0; j < pg->sopsl;j++){
            for(int k = 0; k < es[i].length;k++){
                int p = perm[i][j].p[k];
                double sv[3];
                applySymmetryOperation(&pg->sops[j], vinf[k], sv);
                vadd(sv, v[p], v[p]);
            }
        }
        double sl = 0.0, ol = 0.0;
        for(int j = 0; j < es[i].length;j++){
            ol += vdot(es[i].elements[j]->v,es[i].elements[j]->v);
            sl += vdot(v[j],v[j]);
            vscale(1.0/((double)pg->order), v[j], es[i].elements[j]->v);
        }
        sl /= SQR((double)pg->order);
        if(!(es[i].length == 1 && ol <= thresholds->zero)) e = fmax(e,(ol-sl)/ol);

        
    }
    
    *err = sqrt(e);
err:
    free(v);
    free(vinf);
    return ret;
}


msym_error_t symmetrizeOrbitals(msym_point_group_t *pg, int ssl, msym_subspace_t *ss, int *span, int basisl, msym_orbital_t basis[basisl], msym_thresholds_t *thresholds, double orb[basisl][basisl],double symorb[basisl][basisl]){
    msym_error_t ret = MSYM_SUCCESS;
    double (*proj)[pg->ct->l][basisl] = malloc(sizeof(double[basisl][pg->ct->l][basisl]));
    double *mem = malloc(sizeof(double[basisl]));
    double (*comp)[pg->ct->l] = malloc(sizeof(double[basisl][pg->ct->l]));
    int *icomp = calloc(basisl,sizeof(int));
    int (*ispan) = calloc(pg->ct->l,sizeof(int));
    memset(proj,0,sizeof(double[basisl][pg->ct->l][basisl]));
    
    for(int o = 0;o < basisl;o++){
        double mcomp = -1.0;
        for(int k = 0;k < pg->ct->l;k++){
            for(int s = 0;s < ssl;s++){
                if(ss[s].irrep == k){
                    if(MSYM_SUCCESS != (ret = projectOntoSubspace(basisl, orb[o], &ss[s], basis, mem, proj[o][k]))) goto err;
                }
            }
            comp[o][k] = vlabs(basisl, proj[o][k]);
            //printf("orbital %d compinent in %s = %lf\n",o,pg->ct->irrep[k].name,comp[o][k]);
            if(comp[o][k] > mcomp){
                icomp[o] = k;
                mcomp = comp[o][k];
            }
        }
    }
    
    for(int o = 0;o < basisl;o++){
        ispan[icomp[o]]++;
        //printf("orbital %d (%lf) has largest component (%lf) in %s\n",o,vlabs(basisl,orb[o]),vlabs(basisl,proj[o][icomp[o]]),pg->ct->irrep[icomp[o]].name);
        //scale back to full length, this is a more reasonable option, but will look at that later
        //vlnorm2(basisl, proj[o][icomp[o]], symorb[o]);
        //vlscale(vlabs(basisl, orb[o]), basisl, symorb[o], symorb[o]);
        
        //just throw away
        vlcopy(basisl, proj[o][icomp[o]], symorb[o]);
    }
    
    //printf("Orbital span (vectors) = ");
    for(int k = 0;k < pg->ct->l;k++){
        if(ispan[k] != span[k]){
            msymSetErrorDetails("Projected orbitals do not span the expected irredicible representations. Expected %d%s, got %d",span[k],pg->ct->irrep[k].name,ispan[k]);
            ret = MSYM_SYMMETRIZATION_ERROR;
            goto err;
        }
        //printf(" + %d%s",ispan[k],pg->ct->irrep[k].name);
    }
    //printf("\n");
    
    
    free(ispan);
    free(icomp);
    free(comp);
    free(mem);
    free(proj);
    return ret;
err:
    free(ispan);
    free(icomp);
    free(comp);
    free(mem);
    free(proj);
    return ret;
}

msym_error_t symmetrizeTranslation(msym_point_group_t *pg, msym_equivalence_set_t *es, msym_permutation_t *perm, int pi, double translation[3]){
    msym_error_t ret = MSYM_SUCCESS;
    double (*v)[3] = calloc(es->length,sizeof(double[3]));
    
    for(int j = 0; j < pg->sopsl;j++){
        int p = perm[j].p[pi];
        double stranslation[3];
        applySymmetryOperation(&pg->sops[j], translation, stranslation);
        vadd(stranslation, v[p], v[p]);
    }
    
    double scale = ((double)es->length)/pg->order;
    
    for(int i = 0;i < es->length;i++){
        vscale(scale, v[i], v[i]);
        vadd(es->elements[i]->v,v[i],es->elements[i]->v);
    }
    
err:
    free(v);
    return ret;
}

msym_error_t projectOntoSubspace(int d, double orb[d], msym_subspace_t *ss, msym_orbital_t basis[d], double mem[d], double proj[d]){
    msym_error_t ret = MSYM_SUCCESS;
    if(ss->subspacel){
        for(int i = 0;i < ss->subspacel;i++){
            if(MSYM_SUCCESS != (ret = projectOntoSubspace(d, orb, &ss->subspace[i], basis, mem, proj))) goto err;
        }
    } else {
        for(int i = 0; i < ss->d;i++){
            double (*space)[ss->basisl] = (double (*)[ss->basisl]) ss->space;
            memset(mem, 0, sizeof(double[d]));
            for(int j = 0; j < ss->basisl;j++){
                mem[ss->basis.o[j] - basis] = space[i][j];
            }
            vlproj(d, orb, mem, mem);
            vladd(d, mem, proj, proj);
        }
    }

err:
    return ret;
}


