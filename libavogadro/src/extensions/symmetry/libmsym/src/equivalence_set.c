//
//  equivalence_set.c
//  libmsym
//
//  Created by Marcus Johansson on 08/02/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "equivalence_set.h"
#include "linalg.h"
#include "context.h"
#include "elements.h"

#define SQR(x) ((x)*(x))

msym_error_t partitionEquivalenceSets(int length, msym_element_t *elements[length], msym_element_t *pelements[length], msym_geometry_t g, int *esl, msym_equivalence_set_t **es, msym_thresholds_t *thresholds);
msym_error_t partitionPointGroupEquivalenceSets(msym_point_group_t *pg, int length, msym_element_t *elements[length], msym_element_t *pelements[length], int *esl, msym_equivalence_set_t **es, msym_thresholds_t *thresholds);



msym_error_t copyEquivalenceSets(int length, msym_equivalence_set_t es[length], msym_equivalence_set_t **ces){
    msym_error_t ret = MSYM_SUCCESS;
    int el = 0;
    
    for(int i = 0;i < length;i++) el += es[i].length;
    msym_equivalence_set_t *nes = malloc(sizeof(msym_equivalence_set_t[length]) + sizeof(msym_element_t *[el]));
    msym_element_t **ep = (msym_element_t **) &es[length];
    msym_element_t **nep = (msym_element_t **) &nes[length];
    memcpy(nes, es, sizeof(msym_equivalence_set_t[length]) + sizeof(msym_element_t *[el]));
    for(int i = 0;i < length;i++) nes[i].elements = nes[i].elements - ep + nep;
    *ces = nes;
err:
    return ret;
}

//TODO: Use a preallocated pointer array instead of multiple mallocs
msym_error_t generateEquivalenceSet(msym_point_group_t *pg, int length, msym_element_t elements[length], int *glength, msym_element_t **gelements, int *esl, msym_equivalence_set_t **es,msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    msym_element_t *ge = calloc(length,sizeof(msym_element_t[pg->order]));
    msym_equivalence_set_t *ges = calloc(length,sizeof(msym_equivalence_set_t));
    int gel = 0;
    int gesl = 0;
    for(int i = 0;i < length;i++){
        msym_equivalence_set_t *aes = NULL;
        int f;
        for(f = 0;f < gel;f++){
            if(ge[f].n == elements[i].n && ge[f].m == elements[i].m && 0 == strncmp(ge[f].name, elements[i].name, sizeof(ge[f].name)) && vequal(ge[f].v, elements[i].v, thresholds->permutation)){
                break;
            }
        }
        if(f == gel){
            aes = &ges[gesl++];
            aes->elements = calloc(pg->order,sizeof(msym_element_t*));
            aes->length = 0;
        } else {
            continue;
        }
        
        if(elements[i].aol > 0 || elements[i].ao != NULL){
            msymSetErrorDetails("Cannot (currently) generate equivalence sets from elements with orbitals");
            ret = MSYM_INVALID_ELEMENTS;
            goto err;
        }
        for(msym_symmetry_operation_t *s = pg->sops;s < (pg->sops + pg->sopsl);s++){
            double v[3];
            applySymmetryOperation(s, elements[i].v, v);
            
            for(f = 0;f < gel;f++){
                if(ge[f].n == elements[i].n && ge[f].m == elements[i].m && 0 == strncmp(ge[f].name, elements[i].name, sizeof(ge[f].name)) && vequal(ge[f].v, v, thresholds->permutation)){
                    break;
                }
            }
            if(f == gel){
                memcpy(&ge[gel],&elements[i],sizeof(msym_element_t));
                vcopy(v, ge[gel].v);
                aes->elements[aes->length++] = &ge[gel++];
            }
        }
        
        if(pg->order % aes->length != 0){
            msymSetErrorDetails("Equivalence set length (%d) not a divisor of point group order (%d)",pg->order);
            ret = MSYM_INVALID_EQUIVALENCE_SET;
            goto err;
        }
        
        aes->elements = realloc(aes->elements,sizeof(msym_element_t*[aes->length]));
    }
    
    msym_element_t *geo = ge;
    ge = realloc(ge,sizeof(msym_element_t[gel]));
    ges = realloc(ges,sizeof(msym_equivalence_set_t[gesl]) + sizeof(msym_element_t *[gel]));
    
    msym_element_t **ep = (msym_element_t **) &ges[gesl];
    for(int i = 0;i < gesl;i++){
        msym_element_t **tep = ep;
        for(int j = 0;j < ges[i].length;j++){
            *ep = ges[i].elements[j] - geo + ge;
            ep++;
        }
        free(ges[i].elements);
        ges[i].elements = tep;
    }

    *glength = gel;
    *gelements = ge;
    *es = ges;
    *esl = gesl;
    return ret;
    
err:
    free(ge);
    for(int i = 0; i < gesl;i++) free(ges[i].elements);
    free(ges);
    return ret;
}

msym_error_t splitPointGroupEquivalenceSets(msym_point_group_t *pg, int esl, msym_equivalence_set_t es[esl], int *sesl, msym_equivalence_set_t **ses, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    int length = 0, gesl = 0;
    for(int i = 0;i < esl;i++) length += es[i].length;
    msym_equivalence_set_t *ges = NULL;
    msym_element_t **pelements = calloc(length,sizeof(msym_element_t*));
    msym_element_t **ep = (msym_element_t **) &es[esl];
    
    for(int i = 0; i < esl;i++){
        msym_equivalence_set_t *pes = NULL;
        int pesl = 0;
        if(MSYM_SUCCESS != (ret = partitionPointGroupEquivalenceSets(pg, es[i].length, es[i].elements, es[i].elements - ep + pelements, &pesl, &pes, thresholds))) goto err;
        ges = realloc(ges, sizeof(msym_equivalence_set_t[gesl+pesl]));
        memcpy(&ges[gesl], pes, sizeof(msym_equivalence_set_t[pesl]));
        free(pes);
        gesl += pesl;
    }
    
    ges = realloc(ges, sizeof(msym_equivalence_set_t[gesl]) + sizeof(msym_element_t *[length]));
    ep = (msym_element_t **) &ges[gesl];
    memcpy(ep, pelements, sizeof(msym_element_t *[length]));
    
    for(int i = 0;i < gesl;i++){
        ges[i].elements = ep;
        ep += ges[i].length;
    }
    
    *sesl = gesl;
    *ses = ges;
    
    free(pelements);
    return ret;
err:
    free(ges);
    free(pelements);
    return ret;
}

msym_error_t findPointGroupEquivalenceSets(msym_point_group_t *pg, int length, msym_element_t *elements[length], int *esl, msym_equivalence_set_t **es, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    msym_equivalence_set_t *ges = NULL;
    msym_element_t **pelements = calloc(length,sizeof(msym_element_t*));
    int gesl = 0;
    if(MSYM_SUCCESS != (ret = partitionPointGroupEquivalenceSets(pg, length, elements, pelements, &gesl, &ges, thresholds))) goto err;
    
    ges = realloc(ges,sizeof(msym_equivalence_set_t[gesl]) + sizeof(msym_element_t *[length]));
    msym_element_t **ep = (msym_element_t **) &ges[gesl];
    msym_element_t **epo = ep;
    memcpy(ep, pelements, sizeof(msym_element_t *[length]));
    for(int i = 0;i < gesl;i++){
        if(ep > epo + length){
            msymSetErrorDetails("Equivalence set pointer (%ld) extends beyond number of elements (%d)",ep-epo,length);
            ret = MSYM_INVALID_EQUIVALENCE_SET;
            goto err;
        }
        ges[i].elements = ep;
        ep += ges[i].length;
    }
    
    *es = ges;
    *esl = gesl;
    
    free(pelements);
    return ret;
err:
    free(pelements);
    return ret;

}

msym_error_t partitionPointGroupEquivalenceSets(msym_point_group_t *pg, int length, msym_element_t *elements[length], msym_element_t *pelements[length], int *esl, msym_equivalence_set_t **es, msym_thresholds_t *thresholds){
    msym_error_t ret = MSYM_SUCCESS;
    msym_equivalence_set_t *ges = calloc(length,sizeof(msym_equivalence_set_t));
    int *eqi = malloc(sizeof(int[length]));
    memset(eqi,-1,sizeof(int[length]));
    int gesl = 0, pelementsl = 0;
    for(int i = 0;i < length;i++){
        if(eqi[i] >= 0) continue;
        if(pelementsl >= length){
            msymSetErrorDetails("Size of equivalence sets (%d) larger than number of elements (%d)",pelementsl,length);
            ret = MSYM_INVALID_EQUIVALENCE_SET;
            goto err;
        }
        
        msym_equivalence_set_t *aes = &ges[gesl++];
        aes->elements = &pelements[pelementsl];
        for(msym_symmetry_operation_t *s = pg->sops;s < (pg->sops + pg->sopsl);s++){
            double v[3];
            int f;
            applySymmetryOperation(s, elements[i]->v, v);
            for(f = 0;f < length;f++){
                if(elements[f]->n == elements[i]->n && elements[f]->m == elements[i]->m && 0 == strncmp(elements[f]->name, elements[i]->name, sizeof(elements[f]->name)) && vequal(elements[f]->v, v, thresholds->permutation)){
                    break;
                }
            }
            
            if(f < length && eqi[f] >= 0 && eqi[f] != gesl-1){
                char buf[64];
                symmetryOperationName(s, 64, buf);
                msymSetErrorDetails("Symmetry operation %s on element %d yeilded element (%d) in two diffenrent equivalence sets (%d and %d)",buf,i,f,eqi[f],gesl-1);
                ret = MSYM_INVALID_EQUIVALENCE_SET;
                goto err;
            } else if(f < length && eqi[f] == gesl-1){
                //printf("element[%d] %s belongs to equivalence set %d, but already added\n",f,elements[f]->name, eqi[f]);
            } else if(f < length){
                eqi[f] = gesl - 1;
                aes->elements[aes->length++] = elements[f];
                //printf("element[%d] %s belongs to equivalence set %d, adding\n",f,elements[f]->name, eqi[f]);
            } else {
                char buf[64];
                symmetryOperationName(s, 64, buf);
                msymSetErrorDetails("Cannot find permutation for %s when determining equivalence set from point group %s",buf,pg->name);
                ret = MSYM_INVALID_EQUIVALENCE_SET;
                goto err;
            }
        }
        //printf("generated equivalance set %d of length %d\n",gesl-1,aes->length);
        pelementsl += aes->length;
    }
    
    if(pelementsl != length){
        msymSetErrorDetails("Size of equivalence sets (%d) is not equal to number of elements (%d)",pelementsl,length);
        ret = MSYM_INVALID_EQUIVALENCE_SET;
        goto err;
    }
    
    *es = ges;
    *esl = gesl;
    
    free(eqi);
    return ret;
err:
    free(eqi);
    free(ges);
    return ret;

}

msym_error_t findEquivalenceSets(int length, msym_element_t *elements[length], msym_geometry_t g, int *esl, msym_equivalence_set_t **es, msym_thresholds_t *thresholds) {
    msym_error_t ret = MSYM_SUCCESS;
    int sesl = 0;
    msym_equivalence_set_t *ses = NULL;
    msym_element_t **pelements = calloc(length,sizeof(msym_element_t *));
    
    if(MSYM_SUCCESS != (ret = partitionEquivalenceSets(length, elements,pelements,g,&sesl,&ses,thresholds))) goto err;
    
    if(sesl > 1){
        for(int i = 0; i < sesl;i++){
            int rsesl = 0;
            msym_equivalence_set_t *rses = NULL;
            if(MSYM_SUCCESS != (ret = partitionEquivalenceSets(ses[i].length, ses[i].elements,ses[i].elements,g, &rsesl,&rses,thresholds))) goto err;
            
            if(rsesl > 1){
                ses[i].elements = rses[0].elements;
                ses[i].length = rses[0].length;
                ses = realloc(ses, sizeof(msym_equivalence_set_t[sesl+rsesl-1]));
                memcpy(&ses[sesl], &rses[1], sizeof(msym_equivalence_set_t[rsesl-1]));
                sesl += rsesl-1;
                i--;
            }
            free(rses);
        }
    }

    ses = realloc(ses, sizeof(msym_equivalence_set_t[sesl]) + sizeof(msym_element_t *[length]));
    msym_element_t **ep = (msym_element_t **) &ses[sesl];
    
    for(int i = 0;i < sesl;i++){
        memcpy(ep, ses[i].elements, sizeof(msym_element_t *[ses[i].length]));
        ses[i].elements = ep;
        ep += ses[i].length;
    }
    
    *esl = sesl;
    *es = ses;
    free(pelements);
    return ret;
err:
    free(pelements);
    free(ses);
    return ret;
    
}


msym_error_t partitionEquivalenceSets(int length, msym_element_t *elements[length], msym_element_t *pelements[length], msym_geometry_t g, int *esl, msym_equivalence_set_t **es, msym_thresholds_t *thresholds) {
    
    int ns = 0, gd = geometryDegenerate(g);
    double *e = calloc(length,sizeof(double));
    double *s = calloc(length,sizeof(double));
    
    int *sp = calloc(length,sizeof(int)); //set partition
    int *ss  = calloc(length,sizeof(int)); //set size
    
    double (*ev)[3] = calloc(length,sizeof(double[3]));
    double (*ep)[3] = calloc(length,sizeof(double[3]));
    
    double (*vec)[3] = calloc(length, sizeof(double[3]));
    double *m = calloc(length, sizeof(double));
    
    for(int i = 0;i < length;i++){
        vcopy(elements[i]->v, vec[i]);
        m[i] = elements[i]->m;
    }

    for(int i=0; i < length; i++){
        for(int j = i+1; j < length;j++){
            double w = m[i]*m[j]/(m[i]+m[j]);
            double dist;
            double v[3];
            double proji[3], projj[3];
            
            vnorm2(vec[i],v);
            vproj_plane(vec[j], v, proji);
            vscale(w, proji, proji);
            vadd(proji,ep[i],ep[i]);
            
            vnorm2(vec[j],v);
            vproj_plane(vec[i], v, projj);
            vscale(w, projj, projj);
            vadd(projj,ep[j],ep[j]);
            
            vsub(vec[j],vec[i],v);
            
            dist = vabs(v);
            
            vscale(w/dist,v,v);
            
            vadd(v,ev[i],ev[i]);
            vsub(ev[j],v,ev[j]);
            
            double dij = w*dist; //This is sqrt(I) for a diatomic molecule along an axis perpendicular to the bond with O at center of mass.
            e[i] += dij;
            e[j] += dij;
            
            s[i] += SQR(dij);
            s[j] += SQR(dij);
        }
        vsub(vec[i],ev[i],ev[i]);
        
    }

    for(int i = 0; i < length; i++){
        
        double v[3];
        double w = m[i]/2.0;
        double dist = vabs(elements[i]->v);
        double dii = w*dist;
        vscale(w,elements[i]->v,v);
        vsub(ev[i],v,ev[i]);
        
        // Plane projection can't really differentiate certain types of structures when we add the initial vector,
        // but not doing so will result in huge cancellation errors on degenerate point groups,
        // also large masses will mess up the eq check when this is 0.
        if(gd) vadd(ep[i],v,ep[i]);
        
        e[i] += dii;
        s[i] += SQR(dii);
    }
    for(int i = 0; i < length; i++){
        if(e[i] >= 0.0){
            sp[i] = i;
            for(int j = i+1; j < length;j++){
                if(e[j] >= 0.0){
                    double vabsevi = vabs(ev[i]), vabsevj = vabs(ev[j]), vabsepi = vabs(ep[i]), vabsepj = vabs(ep[j]);
                    double eep = 0.0, eev = fabs((vabsevi)-(vabsevj))/((vabsevi)+(vabsevj)), ee = fabs((e[i])-(e[j]))/((e[i])+(e[j])), es = fabs((s[i])-(s[j]))/((s[i])+(s[j]));
                    
                    if(!(vabsepi < thresholds->zero && vabsepj < thresholds->zero)){
                        eep = fabs((vabsepi)-(vabsepj))/((vabsepi)+(vabsepj));
                    }
                    
                    double max = fmax(eev,fmax(eep,fmax(ee, es)));
                    
                    if(max < thresholds->equivalence && elements[i]->n == elements[j]->n){
                        e[j] = max > 0.0 ? -max : -1.0;
                        sp[j] = i;
                    }
                }
            }
            e[i] = -1.0;
        }
    }
    
    for(int i = 0; i < length;i++){
        int j = sp[i];
        ns += (ss[j] == 0);
        ss[j]++;
    }

    msym_equivalence_set_t *eqs = calloc(ns,sizeof(msym_equivalence_set_t));
    msym_element_t **lelements = elements;
    msym_element_t **pe = pelements;
    
    if(elements == pelements){
        lelements = malloc(sizeof(msym_element_t *[length]));
        memcpy(lelements, elements, sizeof(msym_element_t *[length]));
    }
    
    for(int i = 0, ni = 0; i < length;i++){
        if(ss[i] > 0){
            int ei = 0;
            eqs[ni].elements = pe;
            eqs[ni].length = ss[i];
            for(int j = 0; j < length;j++){
                if(sp[j] == i){
                    double err = (e[j] > -1.0) ? fabs(e[j]) : 0.0;
                    eqs[ni].err = fmax(eqs[ni].err,err);
                    eqs[ni].elements[ei++] = lelements[j];
                }
            }
            pe += ss[i];
            ni++;
        }
    }

    if(elements == pelements){
        free(lelements);
    }
    free(m);
    free(vec);
    free(s);
    free(e);
    free(sp);
    free(ss);
    free(ev);
    free(ep);
    *es = eqs;
    *esl = ns;
    return MSYM_SUCCESS;
}
