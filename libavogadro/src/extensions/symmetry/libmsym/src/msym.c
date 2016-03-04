//
//  msym.c
//  libmsym
//
//  Created by Marcus Johansson on 30/01/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "msym.h"
#include "context.h"
#include "symmetry.h"
#include "equivalence_set.h"
#include "point_group.h"
#include "symmetrize.h"
#include "linalg.h"

msym_error_t msymFindSymmetry(msym_context ctx){
    msym_error_t ret = MSYM_SUCCESS;
    int elementsl = 0, esl = 0;
    msym_element_t *elements = NULL;
    msym_thresholds_t *t = NULL;
    msym_equivalence_set_t *es = NULL;
    msym_point_group_t *pg = NULL;
    int sopsl = 0;
    msym_symmetry_operation_t *sops = NULL;
    msym_equivalence_set_t *ses = NULL;
    int sesl = 0;
    
    clock_t start, end;

    double time;
    
    
    if(MSYM_SUCCESS != (ret = ctxGetElements(ctx, &elementsl, &elements))) goto err;
    
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))){
        if(MSYM_SUCCESS != (ret = msymFindEquivalenceSets(ctx))) goto err;
    }
    
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))){
        start = clock();
        if(MSYM_SUCCESS != (ret = findSymmetryOperations(esl,es,t,&sopsl,&sops))) goto err;
        end = clock();
        time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("time: %lf seconds to find %d symmetry operations in %d equivalence sets\n",time,sopsl,esl);
        start = clock();
        if(MSYM_SUCCESS != (ret = findPointGroup(sopsl, sops, t, &pg))) goto err;
        end = clock();
        time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("time: %lf seconds to find point group %s\n",time,pg->name);
        if(MSYM_SUCCESS != (ret = ctxSetPointGroup(ctx, pg))) {
            free(pg);
            goto err;
        }
        
        // Reuild equivalence sets after determining poing group in case they are very similar
        start = clock();
        //if(MSYM_SUCCESS != (ret = msymFindEquivalenceSets(ctx))) goto err;
        if(MSYM_SUCCESS != (ret = splitPointGroupEquivalenceSets(pg, esl, es, &sesl, &ses, t))) goto err;
        if(MSYM_SUCCESS != (ret = ctxSetEquivalenceSets(ctx, sesl, ses))) goto err;
        ses = NULL; sesl = 0;
        if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))) goto err;
        end = clock();
        time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("time: %lf seconds to regenerate %d equivalence sets\n",time,esl);
        
    }
    
    start = clock();
    if(MSYM_SUCCESS != (ret = msymFindEquivalenceSetPermutations(ctx))) goto err;
    
    end = clock();
    time = (double)(end - start) / CLOCKS_PER_SEC;
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))) goto err; //This is only for printing, since permutation may regenerate sets
    printf("time: %lf seconds to find permutations of %d symmetry operations in %d equivalence sets\n",time,pg->sopsl,esl);
    
    //for(int i = 0;i < pg->sopsl;i++){printSymmetryOperation(&pg->sops[i]);}
    
    free(sops);
    return ret;
    
err:
    free(ses);
    free(sops);
    return ret;
}

msym_error_t msymSetPointGroup(msym_context ctx, char *name){
    msym_error_t ret = MSYM_SUCCESS;
    msym_point_group_t *pg = NULL;
    msym_thresholds_t *t = NULL;
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = generatePointGroup(name, t, &pg))) goto err;
    if(MSYM_SUCCESS != (ret = ctxSetPointGroup(ctx, pg))) goto err;
    
    return ret;
    
err:
    free(pg);
    return ret;
}

msym_error_t msymGenerateElements(msym_context ctx, int length, msym_element_t elements[length]){
    msym_error_t ret = MSYM_SUCCESS;
    msym_point_group_t *pg = NULL;
    msym_thresholds_t *t = NULL;
    msym_element_t *gelements = NULL;
    msym_equivalence_set_t *es = NULL;
    msym_element_t **pelements = NULL;
    double err = 0.0;
    
    
    int glength = 0, plength = 0, esl = 0;
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    if(MSYM_SUCCESS != (ret = generateEquivalenceSet(pg, length, elements, &glength, &gelements, &esl, &es,t))) goto err;
    if(MSYM_SUCCESS != (ret = msymSetElements(ctx, glength, gelements))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetElementPtrs(ctx, &plength, &pelements))) goto err;
    if(plength != glength){
        ret = MSYM_INVALID_ELEMENTS;
        msymSetErrorDetails("Inconsistency detected when setting elements");
        goto err;
    }
    for(int i = 0;i < esl;i++){
        for(int j = 0;j < es[i].length;j++){
            long int index = es[i].elements[j] - gelements;
            es[i].elements[j] = pelements[index];
        }
    }
    if(MSYM_SUCCESS != (ret = ctxSetEquivalenceSets(ctx, esl, es))) goto err;
    es = NULL; esl = 0;
    if(MSYM_SUCCESS != (ret = msymFindEquivalenceSetPermutations(ctx))) goto err;
    if(MSYM_SUCCESS != (ret = msymSymmetrizeMolecule(ctx, &err))) goto err;
    free(gelements);
    return ret;
    
err:
    free(gelements);
    free(es);
    return ret;
}

msym_error_t msymFindEquivalenceSets(msym_context ctx){
    msym_error_t ret = MSYM_SUCCESS;
    int pelementsl = 0;
    msym_element_t **pelements = NULL;
    msym_thresholds_t *t = NULL;
    msym_point_group_t *pg = NULL;
    msym_geometry_t g = GEOMETRY_UNKNOWN;
    double eigvec[3][3];
    double eigval[3];
    int esl = 0;
    msym_equivalence_set_t *es;
    
    clock_t start, end;
    double time;
    
    if(MSYM_SUCCESS != (ret = ctxGetElementPtrs(ctx, &pelementsl, &pelements))) goto err;
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) {
        if(MSYM_SUCCESS != (ret = ctxGetGeometry(ctx, &g, eigval, eigvec))) goto err;
        start = clock();
        if(MSYM_SUCCESS != (ret = findEquivalenceSets(pelementsl, pelements, g, &esl, &es, t))) goto err;
        end = clock();
        time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("time: %lf seconds to find %d equivalence sets in %d element molecule\n",time,esl,pelementsl);
    } else {
        if(MSYM_SUCCESS != (ret = findPointGroupEquivalenceSets(pg, pelementsl, pelements, &esl, &es, t))) goto err;
    }
    if(MSYM_SUCCESS != (ret = ctxSetEquivalenceSets(ctx, esl, es))) goto err;
err:
    return ret;
}


msym_error_t msymAlignAxes(msym_context ctx){
    
    msym_error_t ret = MSYM_SUCCESS;
    msym_element_t *elements = NULL;
    msym_point_group_t *pg;
    int elementsl = 0;
    double zero[3] = {0,0,0};
    
    if(MSYM_SUCCESS != (ret = ctxGetElements(ctx, &elementsl, &elements))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    
    
    if(pg->sops == NULL || pg->sopsl == 0){
        msymSetErrorDetails("No symmetry operations in point group");
        ret = MSYM_INVALID_POINT_GROUP;
        goto err;
    }
    
    if(MSYM_SUCCESS != (ret = ctxSetCenterOfMass(ctx, zero))) goto err;
    
    for(int i = 0; i < elementsl;i++) mvmul(elements[i].v, pg->transform, elements[i].v);
    for(int i = 0; i < pg->sopsl;i++) mvmul(pg->sops[i].v, pg->transform, pg->sops[i].v);
    mleye(3,pg->transform);
    
err:
    return ret;
}

msym_error_t msymGetAlignmentAxes(msym_context ctx, double primary[3], double secondary[3]){
    msym_error_t ret = MSYM_SUCCESS;
    msym_point_group_t *pg;
    
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    
    double m[3][3], x[3] = {1,0,0}, z[3] = {0,0,1};
    minv(pg->transform,m);
    mvmul(z, m, primary);
    mvmul(x, m, secondary);
    
err:
    return ret;

}


msym_error_t msymGetAlignmentTransform(msym_context ctx, double transform[3][3]){
    msym_error_t ret = MSYM_SUCCESS;
    msym_point_group_t *pg;
    
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    
    mcopy(pg->transform, transform);
    
err:
    return ret;
    
}

msym_error_t msymSetAlignmentTransform(msym_context ctx, double transform[3][3]){
    msym_error_t ret = MSYM_SUCCESS;
    msym_point_group_t *pg;
    msym_element_t *elements = NULL;
    msym_thresholds_t *t = NULL;
    int elementsl = 0;
    double m[3][3];
    
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetElements(ctx, &elementsl, &elements))){
        elements = NULL;
        elementsl = 0;
    }
    
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    
    if(pg->sops == NULL || pg->sopsl == 0){
        msymSetErrorDetails("No symmetry operations in point group for setting alignment transform");
        ret = MSYM_INVALID_POINT_GROUP;
        goto err;
    }
    
    for(int i = 0; i < elementsl;i++) mvmul(elements[i].v, pg->transform, elements[i].v);
    for(int i = 0; i < pg->sopsl;i++) mvmul(pg->sops[i].v, pg->transform, pg->sops[i].v);
    
    minv(transform,m);
    mcopy(transform, pg->transform);
    
    for(int i = 0; i < elementsl;i++) mvmul(elements[i].v, m, elements[i].v);
    for(int i = 0; i < pg->sopsl;i++) mvmul(pg->sops[i].v, m, pg->sops[i].v);
    
err:
    return ret;
}

msym_error_t msymSetAlignmentAxes(msym_context ctx, double primary[3], double secondary[3]){
    
    msym_error_t ret = MSYM_SUCCESS;
    msym_point_group_t *pg;
    msym_element_t *elements = NULL;
    msym_thresholds_t *t = NULL;
    int elementsl = 0;
    double x[3] = {1,0,0}, z[3] = {0,0,1}, m[3][3], p[3], s[3];
    
    vnorm2(primary, p);
    vnorm2(secondary,s);
    
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetElements(ctx, &elementsl, &elements))){
        elements = NULL;
        elementsl = 0;
    }
    
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    
    if(pg->sops == NULL || pg->sopsl == 0){
        msymSetErrorDetails("No symmetry operations in point group for setting alignment axes");
        ret = MSYM_INVALID_POINT_GROUP;
        goto err;
    }
    
    if(!vperpendicular(primary, secondary, t->angle)) {
        msymSetErrorDetails("Alignment axes are not orthogonal");
        ret = MSYM_INVALID_AXES;
        goto err;
    }
    
    for(int i = 0; i < elementsl;i++) mvmul(elements[i].v, pg->transform, elements[i].v);
    for(int i = 0; i < pg->sopsl;i++) mvmul(pg->sops[i].v, pg->transform, pg->sops[i].v);
    
    vproj_plane(s, p, s);
    malign(p,z,pg->transform);
    mvmul(s, pg->transform, s);
    malign(s,x,m);
    mmmul(m,pg->transform,pg->transform);
    minv(pg->transform,m);
    
    for(int i = 0; i < elementsl;i++) mvmul(elements[i].v, m, elements[i].v);
    for(int i = 0; i < pg->sopsl;i++) mvmul(pg->sops[i].v, m, pg->sops[i].v);
    
    
err:
    return ret;
}

msym_error_t msymSelectSubgroup(msym_context ctx, msym_subgroup_t *ext){
    msym_error_t ret = MSYM_SUCCESS;
    msym_subgroup_t *sg;
    msym_point_group_t *pg;
    msym_thresholds_t *t = NULL;
    
    if(MSYM_SUCCESS != (ret = ctxGetInternalSubgroup(ctx, ext, &sg))) goto err;
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = pointGroupFromSubgroup(sg, t, &pg))) goto err;
    if(MSYM_SUCCESS != (ret = ctxSetPointGroup(ctx, pg))) goto err;
    if(MSYM_SUCCESS != (ret = msymFindEquivalenceSets(ctx))) goto err;
    if(MSYM_SUCCESS != (ret = msymFindEquivalenceSetPermutations(ctx))) goto err;

err:
    return ret;
}

msym_error_t msymSymmetrizeMolecule(msym_context ctx, double *err){
    msym_error_t ret = MSYM_SUCCESS;
    
    msym_point_group_t *pg;
    msym_equivalence_set_t *es;
    msym_element_t *elements;
    
    msym_permutation_t **perm;
    msym_thresholds_t *t = NULL;
    double error = 0.0;
    int perml = 0, esl = 0, elementsl = 0, sopsl = 0;
    
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetElements(ctx, &elementsl, &elements))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))){
        if(MSYM_SUCCESS != (ret = msymFindEquivalenceSets(ctx))) goto err;
        if(MSYM_SUCCESS != (ret = msymFindEquivalenceSetPermutations(ctx))) goto err;
        if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))) goto err;
    }
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSetPermutations(ctx, &perml, &sopsl, &perm))) goto err;
    if(sopsl != pg->sopsl || perml != esl) {
        msymSetErrorDetails("Detected inconsistency between point group, equivalence sets and permutaions");
        ret = MSYM_INVALID_PERMUTATION;
        goto err;
    }
    
    clock_t start = clock();
    
    if(MSYM_SUCCESS != (ret = symmetrizeMolecule(pg, esl, es, perm, t, &error))) goto err;
    
    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("time: %lf seconds to symmetrize %d equivalence sets in %d element molecule\n",time,esl,elementsl);
        
    *err = error;
err:
    return ret;
}

msym_error_t msymApplyTranslation(msym_context ctx, msym_element_t *ext, double v[3]){
    msym_error_t ret = MSYM_SUCCESS;
    
    msym_point_group_t *pg;
    msym_equivalence_set_t *es;
    msym_element_t *elements;
    msym_element_t *element;
    
    msym_permutation_t **perm;
    msym_thresholds_t *t = NULL;
    int perml = 0, esl = 0, elementsl = 0, sopsl = 0;
    
    if(MSYM_SUCCESS != (ret = ctxGetInternalElement(ctx, ext, &element))) goto err;
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetElements(ctx, &elementsl, &elements))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))){
        if(MSYM_SUCCESS != (ret = msymFindEquivalenceSets(ctx))) goto err;
        if(MSYM_SUCCESS != (ret = msymFindEquivalenceSetPermutations(ctx))) goto err;
        if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))) goto err;
    }
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSetPermutations(ctx, &perml, &sopsl, &perm))) goto err;
    if(sopsl != pg->sopsl || perml != esl) {
        msymSetErrorDetails("Detected inconsistency between point group, equivalence sets and permutaions");
        ret = MSYM_INVALID_PERMUTATION;
        goto err;
    }
    
    int fes = 0, fi = 0;
    for(fes = 0;fes < esl;fes++){
        for(fi = 0; fi < es[fes].length;fi++){
            if(element == es[fes].elements[fi]) break;
        }
        if(fi < es[fes].length) break;
    }
    
    if(fes >= esl){
        msymSetErrorDetails("Could not find element %s in any of the %d equivalence sets", element->name, esl);
        ret = MSYM_INVALID_ELEMENTS;
        goto err;
    }
    
    if(MSYM_SUCCESS != (ret = symmetrizeTranslation(pg, &es[fes], perm[fes], fi, v))) goto err;
    
    return ret;
err:
    return ret;
}

msym_error_t msymGenerateOrbitalSubspaces(msym_context ctx){
    msym_error_t ret = MSYM_SUCCESS;
    
    msym_point_group_t *pg = NULL;
    msym_orbital_t *basis = NULL;
    msym_equivalence_set_t *es = NULL;
    msym_permutation_t **perm = NULL;
    msym_thresholds_t *t = NULL;
    msym_subspace_t *ss = NULL;
    int *span = NULL;
    
    clock_t start = clock();
    int basisl = 0, esl = 0, perml = 0, sopsl = 0, ssl = 0;
    
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    if(pg->ct == NULL){
        if(MSYM_SUCCESS != (ret = findCharacterTable(pg))) goto err;
    }
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetOrbitals(ctx, &basisl, &basis))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSetPermutations(ctx, &perml, &sopsl, &perm))) goto err;
    if(sopsl != pg->sopsl || perml != esl) {ret = MSYM_INVALID_PERMUTATION; goto err;}

    if(MSYM_SUCCESS != (ret = generateOrbitalSubspaces(pg, esl, es, perm, basisl, basis, t, &ssl, &ss, &span))) goto err;
    
    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("time: %lf seconds to generate %d root orbital subspaces from %d basis functions\n",time,ssl,basisl);
    
    //for(int i = 0;i < ssl;i++) printSubspace(pg->ct, &ss[i]);
    
    if(MSYM_SUCCESS != (ret = ctxSetOrbitalSubspaces(ctx, ssl, ss, span))) goto err;
    
    return ret;
err:
    for(int i = 0;i < ssl;i++){
        freeSubspace(&ss[i]);
    }
    free(ss);
    free(span);
    return ret;
}

msym_error_t msymGetOrbitalSubspaces(msym_context ctx, int l, double c[l][l]){
    msym_error_t ret = MSYM_SUCCESS;
    msym_subspace_t *ss = NULL;
    msym_orbital_t *basis = NULL;
    int *span = NULL;
    int ssl = 0, basisl = 0;
    
    if(MSYM_SUCCESS != (ret = ctxGetOrbitals(ctx, &basisl, &basis))) goto err;
    
    if(basisl != l) {
        ret = MSYM_INVALID_ORBITALS;
        msymSetErrorDetails("Number of orbital coefficients (%d) do not match orbital basis (%d)",l,basisl);
        goto err;
    }
    
    if(MSYM_SUCCESS != (ret = ctxGetOrbitalSubspaces(ctx, &ssl, &ss,&span))){
        if(MSYM_SUCCESS != (ret = msymGenerateOrbitalSubspaces(ctx))) goto err;
        if(MSYM_SUCCESS != (ret = ctxGetOrbitalSubspaces(ctx, &ssl, &ss,&span))) goto err;
    }
    
    //printf("getting orbital subspaces\n");
    
    if(MSYM_SUCCESS != (ret = getOrbitalSubspaces(ssl, ss, basisl, basis, c))) goto err;

    //msym_point_group_t *pg = NULL;
    //if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    //for(int i = 0;i < ssl;i++) printSubspace(pg->ct, &ss[i]);
    //printTransform(l,l,c);
    
    //printf("get ok\n");
    
    return ret;
err:
    return ret;

}

msym_error_t msymSymmetrizeOrbitals(msym_context ctx, int l, double c[l][l]){
    msym_error_t ret = MSYM_SUCCESS;

    msym_point_group_t *pg = NULL;
    msym_subspace_t *ss = NULL;
    msym_orbital_t *basis = NULL;
    msym_thresholds_t *t = NULL;
    int *span = NULL;
    
    double (*symc)[l] = NULL;
    
    int ssl = 0, basisl = 0;
    
    
    clock_t start;
    clock_t end;
    double time;
    
    
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    if(pg->ct == NULL){
        if(MSYM_SUCCESS != (ret = findCharacterTable(pg))) goto err;
    }
    
    if(MSYM_SUCCESS != (ret = ctxGetOrbitals(ctx, &basisl, &basis))) goto err;
    
    if(basisl != l) {
        ret = MSYM_INVALID_ORBITALS;
        msymSetErrorDetails("Number of orbital coefficients (%d) do not match orbital basis (%d)",l,basisl);
        goto err;
    }
    
    if(MSYM_SUCCESS != (ret = ctxGetOrbitalSubspaces(ctx, &ssl, &ss, &span))){
        if(MSYM_SUCCESS != (ret = msymGenerateOrbitalSubspaces(ctx))) goto err;
        if(MSYM_SUCCESS != (ret = ctxGetOrbitalSubspaces(ctx, &ssl, &ss, &span))) goto err;
    }
    
    
    symc = malloc(sizeof(double[l][l]));
    
    start = clock();

    if(MSYM_SUCCESS != (ret = symmetrizeOrbitals(pg, ssl, ss, span, basisl, basis, t, c , symc))) goto err;

    /*printf("Pre symmetrization\n");
    printTransform(l,l,c);
    printf("Posr symmetrization\n");
    printTransform(l,l,symc);*/
    
    end = clock();
    
    memcpy(c,symc,sizeof(double[l][l]));
    
    time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("time: %lf seconds to symmetrize %d orbitals\n",time,basisl);
    
err:
    free(symc);
    return ret;
}

msym_error_t msymFindEquivalenceSetPermutations(msym_context ctx) {
    msym_error_t ret = MSYM_SUCCESS;
    //We can't allocate this as a double[][] unless we typecast it every time, since the compiler doesn't have the indexing information in the context
    msym_permutation_t **perm = NULL;
    msym_permutation_t *bperm = NULL;
    msym_point_group_t *pg = NULL;
    msym_equivalence_set_t *es = NULL;
    msym_thresholds_t *t = NULL;
    double (**esv)[3] = NULL;
    int esl = 0;
    
    if(MSYM_SUCCESS != (ret = msymGetThresholds(ctx, &t))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetPointGroup(ctx, &pg))) goto err;
    if(MSYM_SUCCESS != (ret = ctxGetEquivalenceSets(ctx, &esl, &es))) goto err;
    
    perm = (msym_permutation_t**)malloc(esl*sizeof(msym_permutation_t*) + esl*pg->sopsl*sizeof(msym_permutation_t));
    bperm = (msym_permutation_t*)(perm + esl);
    memset(bperm,0,esl*pg->sopsl*sizeof(msym_permutation_t));
    
    
    
    for(int i = 0; i < esl;i++){
        perm[i] = bperm + i*pg->sopsl;
        if(es[i].length > pg->order){
            msymSetErrorDetails("Equivalence set has more elements (%d) than the order of the point group %s (%d)",es[i].length,pg->name,pg->order);
            ret = MSYM_INVALID_EQUIVALENCE_SET;
            goto err;
        }
    }
    /*
    if(perm == NULL){
        perm = (msym_permutation_t**)malloc(esl*sizeof(msym_permutation_t*) + esl*pg->sopsl*sizeof(msym_permutation_t));
        bperm = (msym_permutation_t*)(perm + esl);
        memset(bperm,0,esl*pg->sopsl*sizeof(msym_permutation_t));
        for(int i = 0; i < esl;i++){ //This really shouldn't happen
            perm[i] = bperm + i*pg->sopsl;
            if(es[i].length > pg->order){
                msymSetErrorDetails("Equivalence set has more elements (%d) than the order of the point group %s (%d)",es[i].length,pg->name,pg->order);
                ret = MSYM_INVALID_EQUIVALENCE_SET;
                goto err;
            }
        }
    }*/
    
    esv = malloc(sizeof(double (*[pg->order])[3]));
    for(int i = 0; i < esl;i++){
        for(int j = 0; j < es[i].length;j++){
            esv[j] = &es[i].elements[j]->v;
        }
        
        for(int j = 0; j < pg->sopsl;j++){
            if(MSYM_SUCCESS != (ret = findPermutation(&pg->sops[j], es[i].length, esv, t, &perm[i][j]))) goto err;
        }
    }
    if(MSYM_SUCCESS != (ret = ctxSetEquivalenceSetPermutations(ctx, esl, pg->sopsl, perm))) goto err;
    free(esv);
    return ret;
    
err:
    free(esv);
    free(perm);
    return ret;
}

