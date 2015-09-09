//
//  symmetry.c
//  libmsym
//
//  Created by Marcus Johansson on 12/04/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include "symmetry.h"
#include "msym.h"
#include "linalg.h"
#include "symop.h"
#include "geometry.h"
#include "equivalence_set.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>


//Special case of less than (basically with some margin)
#define LT(A,B,T) ((B) - (A) > (T))
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510582
#endif

int divisors(int n, int* div);

msym_error_t findEquivalenceSetSymmetryOperations(msym_equivalence_set_t *es, msym_thresholds_t *t, int *lsops, msym_symmetry_operation_t **sops);
msym_error_t findSymmetryLinear(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetryPlanarRegular(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *t, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetryPlanarIrregular(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetryPolyhedralProlate(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetryPolyhedralOblate(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetrySymmetricPolyhedron(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int prim, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetryAsymmetricPolyhedron(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetrySpherical(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetryCubic(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t findSymmetryUnknown(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *t, int *rsopsl, msym_symmetry_operation_t **rsops);
msym_error_t reduceSymmetry(int sopsl, msym_symmetry_operation_t sops[sopsl], msym_thresholds_t *thresholds, int *isopsl, msym_symmetry_operation_t **isops);
msym_error_t filterSymmetryOperations(int sopsl, msym_symmetry_operation_t sops[sopsl], msym_thresholds_t *thresholds, int *isopsl, msym_symmetry_operation_t **isops);

msym_error_t findSymmetryOperations(int esl, msym_equivalence_set_t es[esl], msym_thresholds_t *t, int *lsops, msym_symmetry_operation_t **sops){
    msym_error_t ret = MSYM_SUCCESS;
    msym_symmetry_operation_t *rsops = NULL;
    int lrsops = 0;
    for(int i = 0; i < esl;i++){
        int llsops = lrsops;
        if(MSYM_SUCCESS != (ret = findEquivalenceSetSymmetryOperations(&es[i], t, &lrsops, &rsops))) goto err;
        
        if(llsops > 0 && lrsops == 0) {
            free(rsops);
            rsops = NULL;
            break;
        }
    }
    
    for(int i = 0;i < lrsops;i++){
        vnorm(rsops[i].v);
    }
    
    *lsops = lrsops;
    *sops = rsops;
    return ret;
err:
    free(rsops);
    *sops = NULL;
    *lsops = 0;
    return ret;
}

msym_error_t findEquivalenceSetSymmetryOperations(msym_equivalence_set_t *es, msym_thresholds_t *t, int *lsops, msym_symmetry_operation_t **sops){
    //function pointer syntax is a little ambiguous, this is technically less correct, but nicer to read
    
    const struct _fmap {
        msym_geometry_t g;
        msym_error_t (*f)(msym_equivalence_set_t*,double[3],double[3][3],msym_thresholds_t*,int*,msym_symmetry_operation_t**);
        
    } fmap[8] = {
        [0] = {SPHERICAL,           findSymmetrySpherical},
        [1] = {LINEAR,              findSymmetryLinear},
        [2] = {PLANAR_REGULAR,      findSymmetryPlanarRegular},
        [3] = {PLANAR_IRREGULAR,    findSymmetryPlanarIrregular},
        [4] = {POLYHEDRAL_PROLATE,  findSymmetryPolyhedralProlate},
        [5] = {POLYHEDRAL_OBLATE,   findSymmetryPolyhedralOblate},
        [6] = {ASSYMETRIC,          findSymmetryAsymmetricPolyhedron},
        [7] = {GEOMETRY_UNKNOWN,    findSymmetryUnknown}
    };
    
    msym_error_t ret = MSYM_SUCCESS;
    msym_symmetry_operation_t *fsops = NULL;
    int lfsops = 0;
    double cm[3];
    msym_geometry_t g;
    double eigvec[3][3];
    double eigval[3];

    
    if(MSYM_SUCCESS != (ret = findCenterOfMass(es->length, es->elements, cm))) goto err;
    if(MSYM_SUCCESS != (ret = findGeometry(es->length, es->elements, cm, t, &g, eigval, eigvec))) goto err;
    
    int fi, fil = sizeof(fmap)/sizeof(fmap[0]);
    for(fi = 0; fi < fil;fi++){
        if(fmap[fi].g == g) {
            if(MSYM_SUCCESS != (ret = fmap[fi].f(es,cm,eigvec,t,&lfsops,&fsops))) goto err;
            break;
        }
    }
    if(fi == fil){
        msymSetErrorDetails("Unknown geometry of equivalence set");
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    
    if(*sops == NULL){
        *sops = fsops;
        *lsops = lfsops;
    } else if (lfsops != 0) {
        if(MSYM_SUCCESS != (ret = reduceSymmetry(lfsops, fsops, t, lsops, sops))) goto err;
        free(fsops);
    } else if (fsops == 0 && es->length > 1) {
        msymSetErrorDetails("No symmetry operations found in equivalence set with %d elements",es->length);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    } else {
        free(fsops);
    }

    return ret;
    
err:
    free(fsops);
    return ret;

}

msym_error_t findSymmetryUnknown(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    return MSYM_INVALID_GEOMETRY;
}

msym_error_t findSymmetryLinear(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    msym_error_t ret = MSYM_SUCCESS;
    int prim = 0, sopsl = 0;
    msym_symmetry_operation_t *sops = NULL;
    if(es->length != 2){
        msymSetErrorDetails("Expected two elements in linear equivalence set, got %d",es->length);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    if(vzero(cm,thresholds->zero)){
        double t[3], v[3];
        vnorm2(es->elements[0]->v,v);
        vnorm2(es->elements[1]->v,t);
        vadd(v,t,t);
        vscale(0.5, t, t);
        vsub(v,t,v);
        vnorm(v);
        sopsl = 3;
        sops = malloc(sopsl*sizeof(msym_symmetry_operation_t));
        vcopy(v,sops[0].v);
        vcopy(v,sops[1].v);
        sops[0].type = PROPER_ROTATION;
        sops[0].order = 0;
        sops[0].power = 1;
        sops[1].type = REFLECTION;
        sops[2].type = INVERSION;
        
    } else {
        sopsl = 3;
        sops = malloc(sopsl*sizeof(msym_symmetry_operation_t));
        vcopy(cm,sops[0].v);
        vnorm(sops[0].v);
        vcopy(ev[prim],sops[1].v);
        vnorm(sops[1].v);
        vcrossnorm(sops[0].v,sops[1].v,sops[2].v);
        sops[0].type = PROPER_ROTATION;
        sops[0].order = 2;
        sops[0].power = 1;
        sops[1].type = REFLECTION;
        sops[2].type = REFLECTION;
    }
    
    *rsops = sops;
    *rsopsl = sopsl;
    
err:
    return ret;
    
}

msym_error_t findSymmetryPlanarRegular(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    msym_error_t ret = MSYM_SUCCESS;
    
    int sigma_h = vzero(cm,thresholds->zero), order = es->length, prim = 2, div_len, even, inversion, n = 0, split = 0;
    int *div, sopsl;
    double v0[3], v0_proj[3], v_init[3], theta;
    msym_symmetry_operation_t *sops = NULL;
    
    vnorm2(es->elements[0]->v,v0);
    vproj_plane(v0, ev[prim], v0_proj);
    vnorm(v0_proj);
    
    vcopy(v0_proj,v_init);
    
    for(int i = 1; i < es->length;i++){
        double vi[3], vi_proj[3];
        vcopy(es->elements[i]->v,vi);
        vproj_plane(vi, ev[prim], vi_proj);
        vnorm(vi);
        vnorm(vi_proj);
        theta = vangle(v0_proj,vi_proj);
        
        if(LT(theta,2*M_PI/es->length,asin(thresholds->angle)) && es->length % 2 == 0){
            order = es->length/2;
            split = 1;
            vadd(v0_proj, vi_proj, v_init);
            vnorm(v_init);
            break;
        }
    }
    
    div = malloc(order*sizeof(int));
    div_len = divisors(order,div);
    
    even = order % 2 == 0;
    inversion = even && sigma_h;
    
    sopsl = (div_len + sigma_h + order + order*sigma_h + inversion + sigma_h*(div_len - even));
    sops = malloc(sopsl*sizeof(msym_symmetry_operation_t));
    
    for(; n < div_len; n++){
        sops[n].type = PROPER_ROTATION;
        sops[n].order = div[n];
        sops[n].power = 1;
        vcopy(ev[prim],sops[n].v);
    }
    if(sigma_h) {
        sops[n].type = REFLECTION;
        vcopy(ev[prim],sops[n].v);
        n++;
        for(int s = 0; s < div_len; s++){
            if(div[s] > 2){
                sops[n].type = IMPROPER_ROTATION;
                sops[n].order = div[s];
                sops[n].power = 1;
                vcopy(ev[prim],sops[n].v);
                n++;
            }
        }
    }
    
    if(inversion){
        sops[n].type = INVERSION;
        n++;
    }
    
    theta = M_PI/order;
    for (int i = 0; i < order && n < sopsl; i++) {
        double r[3];
        vrotate(i*theta, v_init, ev[prim], r);
        vnorm(r); //Not really nessecary
        vcrossnorm(r,ev[prim],sops[n].v);
        sops[n].type = REFLECTION;
        
        if(!findSymmetryOperation(&sops[n], sops, n, thresholds)){
            n++;
            if(sigma_h){
                vcopy(r,sops[n].v);
                sops[n].type = PROPER_ROTATION;
                sops[n].order = 2;
                sops[n].power = 1;
                n++;
            }
        }
    }
    
    free(div);
    if(n != sopsl) {
        msymSetErrorDetails("Unexpected number of generated symmetry operations in planar regular polygon. Got %d expected %d",n,sopsl);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    
    *rsops = sops;
    *rsopsl = sopsl;
    
    return ret;
    
err:
    free(sops);
    return ret;
}

msym_error_t findSymmetryPlanarIrregular(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    msym_error_t ret = MSYM_SUCCESS;
    int sopsl = 0;
    msym_symmetry_operation_t *sops = NULL;
    
    if(es->length != 4){
        msymSetErrorDetails("Unexpected number of elements (%d) in planar irregular polygon",es->length);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    
    int iscm = vzero(cm,thresholds->zero);
    
    if(iscm){
        //3xC2 + 3xSigma + inversion
        sopsl = 7;
        sops = malloc(sopsl*sizeof(msym_symmetry_operation_t));
    } else {
        // 2xSigma + 1xC2
        sopsl = 3;
        sops = malloc(sopsl*sizeof(msym_symmetry_operation_t));
    }
    
    //The CM vector must be pointing in the same direction as the largest moment ov inertia vector, otherwise these would not be equal.
    
    vcopy(ev[2],sops[0].v);
    vnorm(sops[0].v);
    sops[0].type = PROPER_ROTATION;
    sops[0].order = 2;
    sops[0].power = 1;
    
    vcopy(ev[1],sops[1].v);
    vnorm(sops[1].v);
    sops[1].type = REFLECTION;
    
    vcopy(ev[0],sops[2].v);
    vnorm(sops[2].v);
    sops[2].type = REFLECTION;
    
    if(iscm){
        vcopy(sops[0].v, sops[3].v);
        sops[3].type = REFLECTION;
        
        vcopy(sops[1].v, sops[4].v);
        sops[4].type = PROPER_ROTATION;
        sops[4].order = 2;
        sops[4].power = 1;
        
        vcopy(sops[2].v, sops[5].v);
        sops[5].type = PROPER_ROTATION;
        sops[5].order = 2;
        sops[5].power = 1;
        
        sops[6].type = INVERSION;
        
    }
    
    *rsopsl = sopsl;
    *rsops = sops;
    
    return ret;
err:
    return ret;
}

msym_error_t findSymmetryPolyhedralProlate(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    return findSymmetrySymmetricPolyhedron(es,cm,ev,thresholds,0,rsopsl,rsops);
}

msym_error_t findSymmetryPolyhedralOblate(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    return findSymmetrySymmetricPolyhedron(es,cm,ev,thresholds,2,rsopsl,rsops);
}

msym_error_t findSymmetrySymmetricPolyhedron(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int prim, int *rsopsl, msym_symmetry_operation_t **rsops){
    msym_error_t ret = MSYM_SUCCESS;
    int sopsl = 0;
    msym_symmetry_operation_t *sops = NULL;
    
    int sigma_h = 0, staggered = 0, split = 0, order = es->length/2, even, inversion, div_len;
    int *div;
    
    int n = 0;
    double v0[3], v0_proj[3], v_init[3], dot0, theta, theta_C2 = 0.0, theta_sigma = 0.0;
    
    if(!(vzero(cm,thresholds->zero))){
        msymSetErrorDetails("Symmetric polyhedron not at center of mass. Vector length: %e > %e (zero threshold)", vabs(cm),thresholds->zero);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    
    vcopy(es->elements[0]->v,v0);
    dot0 = vdot(v0,ev[prim]);
    
    vproj_plane(v0, ev[prim], v0_proj);
    vnorm(v0);
    vnorm(v0_proj);
    vcopy(v0_proj,v_init);
    
    
    for(int i = 1; i < es->length;i++){
        double vi[3], vi_proj[3], v0i[3], doti;
        vcopy(es->elements[i]->v,vi);
        doti = vdot(vi,ev[prim]);
        
        vproj_plane(vi,ev[prim], vi_proj);
        vnorm(vi);
        vnorm(vi_proj);
        vsub(v0,vi, v0i);
        vnorm(v0i);
        
        double tangle = fabs((fabs(dot0)-fabs(doti))/(fabs(dot0)+fabs(doti)));
        
        if(!(tangle <= thresholds->angle)){
            msymSetErrorDetails("Elements in symmetric polyhedron do not lie on two parallel circles %e > %e (angle threshold)",tangle,thresholds->angle);
            ret = MSYM_SYMMETRY_ERROR;
            goto err;
        }
        
        if(vparallel(v0i,ev[prim],thresholds->angle)){
            sigma_h = 1;
            staggered = 0;
        }
        
        theta = vangle(v0_proj, vi_proj);
        
        if(dot0*doti > 0.0){
            theta_sigma = theta/2;
            if(LT(theta, 4*M_PI/es->length, asin(thresholds->angle)) && es->length % 4 == 0){
                vadd(v0,vi,v_init);
                vproj_plane(v_init, ev[prim], v_init);
                vnorm(v_init);
                order = es->length/4;
                even = order % 2 == 0;
                split = 1;
                //theta_split = theta;
                
            }
        } else {
            //This can potentially find a staggered form if split by squal amounts (should be ok, TODO: check)
            if(fabs(theta - 2*M_PI/es->length) < asin(thresholds->angle)){
                staggered = 1;
            }
            // if we have not yet found that we are are staggerd/eclipsed or split this will either be over-written or this is a C2 axis for Dn
            if(!split && !staggered && !sigma_h){
                if(LT(theta, 2*M_PI/es->length, asin(thresholds->angle))){
                    vadd(v0_proj, vi_proj, v_init);
                    vnorm(v_init);
                }
            }
        }
    }
    
    if(split){
        staggered = !sigma_h;
    }
    
    even = order % 2 == 0;
    
    inversion = (staggered && !even) || (sigma_h && even);
    div = malloc(order*sizeof(int));
    div_len = divisors(order,div);
    
    //Symmetry operations:
    //Primary rotation and all divisors
    //sigma_h if eclipsed
    //n x C2
    //n x sigma_v if we are in staggered or eclipsed form
    //Possible inversion (see above)
    //Smax*2 if staggerd Sn (n > 2) for sigma_h
    
    sopsl = (div_len + sigma_h + order + order*(sigma_h || staggered) + inversion + staggered + sigma_h*(div_len - even));
    sops = malloc(sopsl*sizeof(msym_symmetry_operation_t));
    
    int max;
    for(max = 0; n < div_len; n++){
        if(div[n] > max){
            max = div[n];
        }
        sops[n].type = PROPER_ROTATION;
        sops[n].order = div[n];
        sops[n].power = 1;
        vcopy(ev[prim],sops[n].v);
    }
    
    if(sigma_h) {
        sops[n].type = REFLECTION;
        vcopy(ev[prim],sops[n].v);
        n++;
        for(int s = 0; s < div_len; s++){
            if(div[s] > 2){
                sops[n].type = IMPROPER_ROTATION;
                sops[n].order = div[s];
                sops[n].power = 1;
                vcopy(ev[prim],sops[n].v);
                n++;
            }
        }
    }
    
    if(inversion){
        sops[n].type = INVERSION;
        n++;
    }
    
    //PI/order angle between C2 & sigma_v
    //PI/(2*order) start angle if split & staggered
    //start angle if not split & staggered
    //0 start angle if sigma_h
    
    if(staggered){
        theta_C2 = M_PI/(2*order);
        sops[n].type = IMPROPER_ROTATION;
        sops[n].order = 2*max;
        sops[n].power = 1;
        vcopy(ev[prim],sops[n].v);
        n++;
    }
    
    theta = M_PI/order;
    for (int i = 0; i < order; i++) {
        vrotate(theta_C2 + i*theta, v_init, ev[prim], sops[n].v);
        vnorm(sops[n].v); //Not really nessecary
        sops[n].type = PROPER_ROTATION;
        sops[n].order = 2;
        sops[n].power = 1;
        n++;
        
        if(staggered || sigma_h){
            vrotate(i*theta, v_init,ev[prim], sops[n].v);
            vcrossnorm(sops[n].v,ev[prim],sops[n].v);
            sops[n].type = REFLECTION;
            n++;
        }
        
    }
    if(n != sopsl){
        msymSetErrorDetails("Unexpected number of generated symmetry operations in symmetric polyhedron. Got %d expected %d",n,sopsl);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    
    *rsopsl = sopsl;
    *rsops = sops;
    return ret;
    
err:
    free(sops);
    *rsops = NULL;
    *rsopsl = 0;
    return ret;
    
}

msym_error_t findSymmetryAsymmetricPolyhedron(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    msym_error_t ret = MSYM_SUCCESS;
    int sopsl = 0;
    msym_symmetry_operation_t *sops = NULL;

    if(es->length == 4){
        //Only C2 axis
        sopsl = 3;
    } else if(es->length == 8){
        sopsl = 7;
    } else {
        msymSetErrorDetails("Unexpected number of elements (%d) in asymmetric polyhedron",es->length);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    
    if(!(vzero(cm,thresholds->zero))){
        msymSetErrorDetails("Asymmetric polyhedron not at center of mass. Vector length: %e > %e (zero threshold)", vabs(cm),thresholds->zero);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    
    sops = malloc(sopsl*sizeof(msym_symmetry_operation_t));
    vcopy(ev[0], sops[0].v);
    vcopy(ev[1], sops[1].v);
    vcopy(ev[2], sops[2].v);
    vnorm(sops[0].v);
    vnorm(sops[1].v);
    vnorm(sops[2].v);
    sops[0].type = PROPER_ROTATION;
    sops[0].order = 2;
    sops[0].power = 1;
    sops[1].type = PROPER_ROTATION;
    sops[1].order = 2;
    sops[1].power = 1;
    sops[2].type = PROPER_ROTATION;
    sops[2].order = 2;
    sops[2].power = 1;
    
    
    if(es->length == 8){
        vcopy(sops[0].v,sops[3].v);
        vcopy(sops[1].v,sops[4].v);
        vcopy(sops[2].v,sops[5].v);
        sops[3].type = REFLECTION;
        sops[4].type = REFLECTION;
        sops[5].type = REFLECTION;
        sops[6].type = INVERSION;
        
    }
    
    *rsopsl = sopsl;
    *rsops = sops;
    return ret;
    
err:
    free(sops);
    *rsops = NULL;
    *rsopsl = 0;
    return ret;
    
}

msym_error_t findSymmetrySpherical(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    msym_error_t ret = MSYM_SUCCESS;
    int sopsl = 0;
    msym_symmetry_operation_t *sops = NULL;
    if(es->length == 1){
        if(!(vzero(cm,thresholds->zero))){
            double t[3];
            vcopy(es->elements[0]->v,t);
            sopsl = 1;
            sops = malloc(sopsl*sizeof(msym_symmetry_operation_t));
            vcopy(t,sops[0].v);
            vnorm(sops[0].v);
            sops[0].type = PROPER_ROTATION;
            sops[0].order = 0;
            sops[0].power = 1;
            
        }
        *rsopsl = sopsl;
        *rsops = sops;
    } else {
        ret = findSymmetryCubic(es,cm,ev,thresholds,rsopsl,rsops);
    }

    return ret;
err:
    return ret;
    
}


msym_error_t findSymmetryCubic(msym_equivalence_set_t *es, double cm[3], double ev[3][3], msym_thresholds_t *thresholds, int *rsopsl, msym_symmetry_operation_t **rsops){
    
    msym_error_t ret = MSYM_SUCCESS;
    /*struct _pair {
        msym_element_t *e[2];
        double d;
    } *pairs = malloc(sizeof(struct _pair[150]));*/
    double c2d = 0, c4d = 0, sigmad = 0;
    int found = 0, nsigma = 0, esigma = 0, inversion = 0, nc[6] = {0,0,0,0,0,0}, ec[6] = {0,0,0,0,0,0}, *ncb = &nsigma, *ecb = &esigma,*nc3b = &nsigma;
    msym_symmetry_operation_t **(ac[6]);
    double thetac[6] = {0.0,M_PI,M_PI/2,M_PI/3,M_PI/4,M_PI/5};
    
    msym_symmetry_operation_t *sops = malloc(sizeof(msym_symmetry_operation_t[120]));
    int sopsl = 0;
    
    double (**esv)[3] = malloc(sizeof(double (*[es->length])[3]));
    
    msym_symmetry_operation_t **sigma = malloc(16*sizeof(msym_symmetry_operation_t*)); //only 15, but we can overflow
    
    msym_symmetry_operation_t **c3b = NULL;
    msym_symmetry_operation_t **cb = sigma;
    
    msym_permutation_t perm;
    msym_symmetry_operation_t sopc = {.type = PROPER_ROTATION, .order = 2, .power = 1};
    msym_symmetry_operation_t sopsigma = {.type = REFLECTION, .order = 1, .power = 1};
    msym_symmetry_operation_t sopinversion = {.type = INVERSION, .order = 1, .power = 1};
    
    //15 C2 (I), 10 C3 (I), 3 C4 (O), 6 C5 (I)
    ac[0] = malloc((15+10+3+6)*sizeof(msym_symmetry_operation_t*));
    ac[1] = ac[0];
    ac[2] = ac[1];
    ac[3] = ac[2] + 15;
    ac[4] = ac[3] + 10;
    ac[5] = ac[4] + 3;
    
    for(int i = 0; i < es->length;i++){
        esv[i] = &es->elements[i]->v;
    }
    
    for(int i = 0; i < es->length && !found;i++){
        for(int j = i+1;j < es->length;j++){
            if(vparallel(es->elements[i]->v, es->elements[j]->v, thresholds->angle)) continue;
            double d;
            int psopsl = sopsl;
            vsub(es->elements[i]->v,es->elements[j]->v,sopsigma.v);
            d = vabs(sopsigma.v);
            vadd(es->elements[i]->v,es->elements[j]->v,sopc.v);
            vnorm(sopc.v);
            //vsub(es->elements[i]->v,es->elements[j]->v,sopsigma.v);
            vnorm(sopsigma.v);
            
            //Do this first, otherwise we might find a c2, at this distance and not look for more.
            if(es->length % 5 != 0 && (c4d == 0 || fabs(d - c4d)/(d + c4d) < thresholds->equivalence)){
                sopc.order = 4;
                if(!findSymmetryOperation(&sopc, sops, sopsl, thresholds)){
                    if(MSYM_SUCCESS == findPermutation(&sopc, es->length, esv, thresholds, &perm)){
                        c4d = d;
                        copySymmetryOperation(&sops[sopsl], &sopc);
                        (ac[4])[nc[4]++] = &sops[sopsl++];
                        free(perm.p);
                        free(perm.c);
                        sopc.order = 2;
                        if(!findSymmetryOperation(&sopc, sops, sopsl, thresholds)){
                            copySymmetryOperation(&sops[sopsl], &sopc);
                            (ac[2])[nc[2]++] = &sops[sopsl++];
                            
                        }
                    }

                    if(!findSymmetryOperation(&sopsigma, sops, sopsl, thresholds)){
                        if(MSYM_SUCCESS == findPermutation(&sopsigma, es->length, esv, thresholds, &perm)){
                            //sigmad = d; //This is a bit dangerous, but the C2 axes dhould generate the rest
                            copySymmetryOperation(&sops[sopsl], &sopsigma);
                            sigma[nsigma++] = &sops[sopsl++];
                            free(perm.p);
                            free(perm.c);
                        }
                    }
                }
                sopc.order = 2;
            }
            
            if(c2d == 0 || fabs(d - c2d)/(d + c2d) < thresholds->equivalence){
                if(!findSymmetryOperation(&sopc, sops, sopsl, thresholds)){
                    if(MSYM_SUCCESS == findPermutation(&sopc, es->length, esv, thresholds, &perm)){
                        c2d = d;
                        copySymmetryOperation(&sops[sopsl], &sopc);
                        (ac[2])[nc[2]++] = &sops[sopsl++];
                        free(perm.p);
                        free(perm.c);
                    }
                }
            }
            
            if(sigmad == 0 || fabs(d - sigmad)/(d + sigmad) < thresholds->equivalence){
                if(!findSymmetryOperation(&sopsigma, sops, sopsl, thresholds)){
                    if(MSYM_SUCCESS == findPermutation(&sopsigma, es->length, esv, thresholds, &perm)){
                        sigmad = d;
                        
                        copySymmetryOperation(&sops[sopsl], &sopsigma);
                        sigma[nsigma++] = &sops[sopsl++];
                        free(perm.p);
                        free(perm.c);
                    }
                }
            }
            
            //we have generated something, we have more than 2 operations
            if(sopsl > psopsl && sopsl >= 2 && !(nsigma == 15 && nc[2] == 0) && !(nsigma == 0 && nc[2] == 15)){
               
                for(msym_symmetry_operation_t *sopi = sops; sopi < (sops + sopsl);sopi++){
                    for(msym_symmetry_operation_t *sopj = sops; sopj < (sops + sopsl); sopj++){
                        if(sopi == sopj) continue;
                        if(sopi->type == PROPER_ROTATION && sopj->type == PROPER_ROTATION && sopj->order == 2 && vperpendicular(sopi->v, sopj->v, thresholds->angle)){
                            copySymmetryOperation(&sops[sopsl], sopj);
                            vcrossnorm(sopi->v, sopj->v, sops[sopsl].v);
                            if(!findSymmetryOperation(&sops[sopsl], sops, sopsl,thresholds)){
                                (ac[2])[nc[2]++] = &sops[sopsl++];
                            }
                            
                        } else if(!vparallel(sopi->v, sopj->v,thresholds->angle)){
                            copySymmetryOperation(&sops[sopsl], sopj);
                            applySymmetryOperation(sopi,sops[sopsl].v,sops[sopsl].v);
                            if(!findSymmetryOperation(&sops[sopsl], sops, sopsl,thresholds)){
                                if(sopj->type == REFLECTION){
                                    sigma[nsigma++] = &sops[sopsl++];
                                } else {
                                    (ac[sopj->order])[nc[sopj->order]++] = &sops[sopsl++];
                                }
                            }
                        }
                        if(nsigma > 15 || nc[2] > 15){
                            ret = MSYM_SYMMETRY_ERROR;
                            msymSetErrorDetails("Inconsistency when generating symmetry axes for cubic point group, %d C2 axes, %d reflection planes",nc[2],nsigma);
                            goto err;
                        }
                    }
                }
            }
            
            //A little speedup, I point group may require up to 5 iterations, to check if it might be an I
            if(((nsigma >= 3 || (nc[2] >= 3 && nsigma == 0)) && nc[4] == 0 && i > 3 && es->length % 5 != 0) ||
               ((nsigma >= 9 || (nc[2] >= 9 && nsigma == 0)) && i > 0) ||
               (nsigma == 15 && nc[2] == 15)){
                found = 1;
                break;
            }
            if(nsigma == 0 && nc[2] == 0 && nc[4] == 0 && i > 3 && es->length > 120){
                ret = MSYM_SYMMETRY_ERROR;
                msymSetErrorDetails("Found no symmetry operations in cubic group of size %d, thresholds are too high ",es->length);
                goto err;
            }
        }
    }
    
    if(MSYM_SUCCESS == findPermutation(&sopinversion, es->length, esv, thresholds, &perm)){
        inversion = 1;
        copySymmetryOperation(&sops[sopsl++], &sopinversion);
        free(perm.p);
        free(perm.c);
    }
    
    esigma = (((nsigma+2) / 3) + (nsigma / 10) - (nsigma / 13))*3;
    esigma = esigma == 1 ? 0 : esigma; //We cannot generate the remaining axes from one sigma try to generate from C2
    
    switch(esigma) {
        case 15 : ec[5] = 6; ec[3] = 10; ec[2] = 15; break; //Ih
        case 9  : ec[4] = 3; ec[3] = 4; ec[2] = 9; break; //Oh
        case 6  :
        { //Td or Oh (a cube won't generate the pair reflection planes, but we can look for inversion)
            if(inversion){
                int gsigma = 0;
                ec[4] = 3; ec[3] = 4; ec[2] = 9;
                for(int i = 0; i < nsigma && gsigma < 3;i++){
                    for(int j = i+1; j < nsigma && gsigma < 3;j++){
                        double theta = vangle(sigma[i]->v, sigma[j]->v);
                        if(fabs(theta - M_PI/2) < asin(thresholds->angle)){
                            sops[sopsl].type = REFLECTION;
                            vadd(sigma[i]->v, sigma[j]->v, sops[sopsl].v);
                            vnorm(sops[sopsl].v);
                            if(!findSymmetryOperation(&(sops[sopsl]), sops, sopsl,thresholds)){
                                sigma[nsigma+gsigma++] = &(sops[sopsl++]);
                                //gsigma++;
                                //sopsl++;
                            }
                            if(gsigma < 3){
                                sops[sopsl].type = REFLECTION;
                                vsub(sigma[i]->v, sigma[j]->v, sops[sopsl].v);
                                vnorm(sops[sopsl].v);
                                if(!findSymmetryOperation(&(sops[sopsl]), sops, sopsl,thresholds)){
                                    sigma[nsigma+gsigma++] = &(sops[sopsl++]);
                                    //gsigma++;
                                    //sopsl++;
                                }
                            }
                        }
                    }
                }
                nsigma += gsigma;
                break;
            } else {
                ec[3] = 4; ec[2] = 3; break;
            }
        }
        case 3  : ec[3] = 4; ec[2] = 3; c3b = sigma; nc3b = &nsigma; break; //Th
        case 0  :
        {
            ec[2] = ((nc[2] > 0) + ((nc[2] > 3) << 1) + ((nc[2] > 9) << 1))*3;
            switch(ec[2]){
                case 3  : ec[3] = 4; ec[2] = 3; c3b = ac[2]; nc3b = &(nc[2]); break;
                case 9  :
                    ec[4] = 3; ec[3] = 4; ec[2] = 9;
                    for(int i = 0; i < nc[2] && nc[4] < 3; i++){ //This should not happen anymore, but it may segfault on an error, so keep it for now
                        int p = 0;
                        for(int j = 0; j < nc[2] && nc[4] < 3; j++){
                            p += vperpendicular((ac[2])[i]->v, (ac[2])[j]->v,thresholds->angle);
                        }
                        
                        if(p == 4){
                            sops[sopsl].type = PROPER_ROTATION;
                            sops[sopsl].order = 4;
                            sops[sopsl].power = 1;
                            vcopy((ac[2])[i]->v,sops[sopsl].v);
                            if(!findSymmetryOperation(&sops[sopsl], sops, sopsl,thresholds)){
                                (ac[4])[nc[4]] = &(sops[sopsl++]);
                                nc[4]++;
                            }
                        }
                    }
                    c3b = ac[4];
                    nc3b = &(nc[4]);
                    break;
                    //Look for C2 that have 4 other perpendicular C2 this will be a C4
                case 15 :
                    ec[5] = 6; ec[3] = 10; ec[2] = 15;
                    ncb = &(nc[2]);
                    ecb = &(ec[2]);
                    cb = ac[2];
                    break;
                default :
                    msymSetErrorDetails("Unexpected number of C2 axes (%d) in cubic point group",ec[2]);
                    ret = MSYM_SYMMETRY_ERROR;
                    goto err;
                    
            }
            break;
        }
            
        default :
            msymSetErrorDetails("Unexpected number of reflection planes (%d) in cubic point group",nsigma);
            ret = MSYM_SYMMETRY_ERROR;
            goto err;
    }
    
    
    //In a Th, we might not find the C3 axes so we generated them from the 3 reflection planes.
    //In the T, O, groups we generate them from C2 and C4 axes resp.
    //In I they will be generated later by the c2 axis (cbase = C2 in that case)
    if(c3b != NULL && nc[3] == 0 && *nc3b >= 3){
        vadd(c3b[0]->v,c3b[1]->v,sops[sopsl].v);
        vadd(sops[sopsl].v,c3b[2]->v,sops[sopsl].v);
        vnorm(sops[sopsl].v);
        sops[sopsl].type = PROPER_ROTATION;
        sops[sopsl].order = 3;
        sops[sopsl].power = 1;
        (ac[3])[nc[3]] = &(sops[sopsl++]);
        //sopsl++;
        nc[3]++;
        
        vadd(c3b[0]->v,c3b[1]->v,sops[sopsl].v);
        vsub(sops[sopsl].v,c3b[2]->v,sops[sopsl].v);
        vnorm(sops[sopsl].v);
        sops[sopsl].type = PROPER_ROTATION;
        sops[sopsl].order = 3;
        sops[sopsl].power = 1;
        (ac[3])[nc[3]] = &(sops[sopsl++]);
        //sopsl++;
        nc[3]++;
        
        vadd(c3b[0]->v,c3b[2]->v,sops[sopsl].v);
        vsub(sops[sopsl].v,c3b[1]->v,sops[sopsl].v);
        vnorm(sops[sopsl].v);
        sops[sopsl].type = PROPER_ROTATION;
        sops[sopsl].order = 3;
        sops[sopsl].power = 1;
        (ac[3])[nc[3]] = &(sops[sopsl++]);
        //sopsl++;
        nc[3]++;
        
        vsub(c3b[0]->v,c3b[1]->v,sops[sopsl].v);
        vsub(sops[sopsl].v,c3b[2]->v,sops[sopsl].v);
        vnorm(sops[sopsl].v);
        sops[sopsl].type = PROPER_ROTATION;
        sops[sopsl].order = 3;
        sops[sopsl].power = 1;
        (ac[3])[nc[3]] = &(sops[sopsl++]);
        //sopsl++;
        nc[3]++;
    }
    
    found = nc[2] == ec[2] && nc[3] == ec[3] && nc[4] == ec[4] && nc[5] == ec[5];
    
    for(int i = 0; i < *ncb && !found && sopsl < 120;i++){
        for(int j = i+1; j < *ncb && !found;j++){
            double theta = fabs(vangle(cb[i]->v, cb[j]->v));
            if(theta > M_PI/2){
                theta = M_PI - theta;
            }
            for(int k = 2; k < 6;k++){
                
                if(fabs(theta - thetac[k]) < asin(thresholds->angle)){
                    vcrossnorm(cb[i]->v, cb[j]->v, sops[sopsl].v);
                    sops[sopsl].type = PROPER_ROTATION;
                    sops[sopsl].order = k;
                    sops[sopsl].power = 1;
                    if(!findSymmetryOperation(&(sops[sopsl]), sops, sopsl,thresholds)){
                        (ac[k])[nc[k]] = &(sops[sopsl]);
                        sopsl++;
                        nc[k]++;
                    }
                    break;
                }
            }
            found = nc[2] == ec[2] && nc[3] == ec[3] && nc[4] == ec[4] && nc[5] == ec[5];
        }
    }
    
    int nS = 0;
    if(nsigma > 0){
        for(int i = 0; i < sopsl && sopsl < 120;i++){
            if(sops[i].type == PROPER_ROTATION){
                //Td
                if(!inversion && sops[i].order == 2){
                    vcopy(sops[i].v, sops[sopsl].v);
                    sops[sopsl].type = IMPROPER_ROTATION;
                    sops[sopsl].order = 4;
                    sops[sopsl].power = 1;
                    sopsl++;
                    nS++;
                }
                else if (inversion && sops[i].order > 2) {
                    vcopy(sops[i].v, sops[sopsl].v);
                    sops[sopsl].type = IMPROPER_ROTATION;
                    sops[sopsl].order = sops[i].order + (sops[i].order % 2)*sops[i].order;
                    sops[sopsl].power = 1;
                    sopsl++;
                    nS++;
                }
            }
        }
    }
    
    if(sopsl > 63) {
        msymSetErrorDetails("Generated more than 63 symmetry operations (%d) in cubic point group, not including powers",sopsl);
        ret = MSYM_SYMMETRY_ERROR;
        goto err;
    }
    
    *rsopsl = sopsl;
    *rsops = sops;
    
    free(esv);
    free(ac[0]);
    free(sigma);
    
    return ret;
    
err:
    free(ac[0]);
    free(sigma);
    free(sops);
    free(esv);
    *rsopsl = 0;
    *rsops = NULL;
    return ret;
}

msym_error_t reduceSymmetry(int sopsl, msym_symmetry_operation_t sops[sopsl], msym_thresholds_t *thresholds, int *isopsl, msym_symmetry_operation_t **isops){
    msym_error_t ret = MSYM_SUCCESS;
    
    int rsopsl = *isopsl;
    msym_symmetry_operation_t *rsops = *isops;
    msym_symmetry_operation_t *cinf[2] = {NULL,NULL};
    
    int inv[2] = {0,0};
    int inversion = 0;
    for(int i = 0;i < rsopsl;i++){
        if(!((rsops[i].type == PROPER_ROTATION && rsops[i].order == 0) || rsops[i].type == INVERSION || rsops[i].type == REFLECTION)) break;
        if(inv[0] && cinf[0] != NULL) break;
        inv[0] = inv[0] || rsops[i].type == INVERSION;
        if(rsops[i].type == PROPER_ROTATION && rsops[i].order == 0) cinf[0] = &rsops[i];
    }
    
    for(int i = 0;i < sopsl;i++){
        if(!((sops[i].type == PROPER_ROTATION && sops[i].order == 0) || sops[i].type == INVERSION || sops[i].type == REFLECTION)) break;
        if(inv[1] && cinf[1] != NULL) break;
        inv[1] = inv[1] || sops[i].type == INVERSION;
        if(sops[i].type == PROPER_ROTATION && sops[i].order == 0) cinf[1] = &sops[i];
    }
    
    inversion = inv[0] && inv[1];
    
    if(cinf[0] != NULL && cinf[1] != NULL){
        double cross[3];
        int perpendicular = vperpendicular(cinf[0]->v,cinf[1]->v,thresholds->angle);
        int parallel = vparallel(cinf[0]->v,cinf[1]->v,thresholds->angle);
        vcrossnorm(cinf[0]->v, cinf[1]->v, cross);
        if(inversion && perpendicular){
            double v[3];
            vcopy(cinf[0]->v, v);
            rsops = realloc(rsops, sizeof(msym_symmetry_operation_t[7]));
            rsopsl = 7;
            
            rsops[0].type = INVERSION;
            rsops[1].type = REFLECTION;
            rsops[2].type = REFLECTION;
            rsops[3].type = REFLECTION;
            rsops[4].type = PROPER_ROTATION;
            rsops[4].order = 2;
            rsops[4].power = 1;
            rsops[5].type = PROPER_ROTATION;
            rsops[5].order = 2;
            rsops[5].power = 1;
            rsops[6].type = PROPER_ROTATION;
            rsops[6].order = 2;
            rsops[6].power = 1;
            
            vcopy(v, rsops[1].v);
            vcopy(cinf[1]->v, rsops[2].v);
            vcopy(cross, rsops[3].v);
            vcopy(v, rsops[4].v);
            vcopy(cinf[1]->v, rsops[5].v);
            vcopy(cross, rsops[6].v);
        } else if (inversion & !parallel){
            rsops = realloc(rsops, sizeof(msym_symmetry_operation_t[3]));
            rsopsl = 3;
            rsops[0].type = INVERSION;
            rsops[1].type = REFLECTION;
            rsops[2].type = PROPER_ROTATION;
            rsops[2].order = 2;
            rsops[2].power = 1;
            
            vcopy(cross, rsops[1].v);
            vcopy(cross, rsops[2].v);
        } else if(perpendicular && !inv[0] && !inv[1]){
            rsops = realloc(rsops, sizeof(msym_symmetry_operation_t[1]));
            rsopsl = 1;
            rsops[0].type = REFLECTION;
            vcopy(cross, rsops[0].v);
        } else if (perpendicular){
            int index = inv[0] ? 0 : 1;
            double v[2][3];
            vcopy(cinf[0]->v, v[0]);
            vcopy(cinf[1]->v, v[1]);
            
            rsops = realloc(rsops, sizeof(msym_symmetry_operation_t[3]));
            rsopsl = 3;
            rsops[0].type = REFLECTION;
            rsops[1].type = REFLECTION;
            rsops[2].type = PROPER_ROTATION;
            rsops[2].order = 2;
            rsops[2].power = 1;
            
            vcopy(cross, rsops[0].v);
            vcopy(v[index], rsops[1].v);
            vcopy(v[(index+1)%2], rsops[2].v);
            
        } else if (parallel){
            if(MSYM_SUCCESS != (ret = filterSymmetryOperations(sopsl,sops,thresholds,&rsopsl,&rsops))) goto err;
        } else {
            rsops = realloc(rsops, sizeof(msym_symmetry_operation_t[1]));
            rsopsl = 1;
            rsops[0].type = REFLECTION;
            vcopy(cross, rsops[0].v);
        }
    } else if (cinf[0] != NULL){
        
        for(int i = 0;i < sopsl;i++){
            int add = 0;
            if(sops[i].type == IMPROPER_ROTATION){
                add = vparallel(sops[i].v,cinf[0]->v,thresholds->angle);
            } else if(sops[i].type == PROPER_ROTATION){
                if(sops[i].order != 2){
                    add = vparallel(sops[i].v,cinf[0]->v,thresholds->angle);
                } else {
                    add = vparallel(sops[i].v,cinf[0]->v,thresholds->angle) || (vperpendicular(sops[i].v,cinf[0]->v,thresholds->angle) && inv[0]);
                }
            } else if(sops[i].type == REFLECTION){
                add = vperpendicular(sops[i].v,cinf[0]->v,thresholds->angle);
            }
            if(add){
                rsops = realloc(rsops, sizeof(msym_symmetry_operation_t[rsopsl+1]));
                copySymmetryOperation(&rsops[rsopsl], &sops[i]);
                rsopsl++;
            }
        }
        if(MSYM_SUCCESS != (ret = filterSymmetryOperations(sopsl,sops,thresholds,&rsopsl,&rsops))) goto err;
    } else if (cinf[1] != NULL){
        for(int i = 0;i < rsopsl && rsopsl > 0;i++){
            msym_symmetry_operation_t *fsop = findSymmetryOperation(&rsops[i], sops, sopsl,thresholds);
            if(!fsop){
                int remove = 1;
                if(rsops[i].type == IMPROPER_ROTATION){
                    remove = !vparallel(rsops[i].v,cinf[1]->v,thresholds->angle);
                } else if(rsops[i].type == PROPER_ROTATION){
                    if(rsops[i].order != 2){
                        remove = !vparallel(rsops[i].v,cinf[1]->v,thresholds->angle);
                    } else {
                        remove = !(vparallel(rsops[i].v,cinf[1]->v,thresholds->angle) || (vperpendicular(rsops[i].v,cinf[1]->v,thresholds->angle) && inv[1]));
                    }
                } else if(rsops[i].type == REFLECTION){
                    remove = !vperpendicular(rsops[i].v,cinf[1]->v,thresholds->angle);
                }
                if(remove){
                    rsopsl--;
                    copySymmetryOperation(&rsops[i], &rsops[rsopsl]);
                    rsops = realloc(rsops, sizeof(msym_symmetry_operation_t[rsopsl]));
                    i--;
                } else if(vparallel(rsops[i].v,cinf[1]->v,thresholds->angle)){
                    if(vdot(rsops[i].v,cinf[1]->v) < 0){
                        vsub(rsops[i].v,cinf[1]->v,rsops[i].v);
                    } else {
                        vadd(rsops[i].v,cinf[1]->v,rsops[i].v);
                    }
                }
            }
        }
    } else {
        if(MSYM_SUCCESS != (ret = filterSymmetryOperations(sopsl,sops,thresholds, &rsopsl,&rsops))) goto err;
    }
    
    *isopsl = rsopsl;
    *isops = rsops;
    return ret;
err:
    return ret;
}

msym_error_t filterSymmetryOperations(int sopsl, msym_symmetry_operation_t sops[sopsl], msym_thresholds_t *thresholds, int *isopsl, msym_symmetry_operation_t **isops){
    msym_error_t ret = MSYM_SUCCESS;
    int rsopsl = *isopsl;
    msym_symmetry_operation_t *rsops = *isops;
    
    for(int i = 0;i < rsopsl && rsopsl > 0;i++){
        msym_symmetry_operation_t *fsop = findSymmetryOperation(&rsops[i], sops, sopsl,thresholds);
        if(!fsop){
            rsopsl--;
            copySymmetryOperation(&rsops[i], &rsops[rsopsl]);
            rsops = realloc(rsops, sizeof(msym_symmetry_operation_t[rsopsl]));
            i--;
        } else if (rsops[i].type == PROPER_ROTATION || rsops[i].type == IMPROPER_ROTATION || rsops[i].type == REFLECTION){
            if(vdot(rsops[i].v,fsop->v) < 0){
                vsub(rsops[i].v,fsop->v,rsops[i].v);
            } else {
                vadd(rsops[i].v,fsop->v,rsops[i].v);
            }
        }
    }

    *isopsl = rsopsl;
    *isops = rsops;
    return ret;
err:
    return ret;
    
}

int divisors(int n, int* div){
    int max = floor(sqrt(n));
    div[0] = n;
    int l = 1;
    for(int i = 2; i <= max;i++) {
        if(n % i == 0){
            int fact = n/i;
            div[l++] = i;
            if(i != fact){
                div[l++] = n/i;
            }
        }
    }
    return l;
}
