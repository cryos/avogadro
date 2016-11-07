//
//  orbital.c
//  libmsym
//
//  Created by Marcus Johansson on 07/11/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdarg.h>

#include "msym.h"
#include "orbital.h"
#include "linalg.h"
#include "symop.h"
#include "permutation.h"
#include "point_group.h"

void printTransform(int r, int c, double M[r][c]);
void tabPrintTransform(int r, int c, double M[r][c],int indent);
void printSubspaceTree(CharacterTable *ct, msym_subspace_t *ss,int indent);
void tabprintf(char *format, int indent, ...);

msym_error_t getOrbitalSubspaceCoefficients(msym_subspace_t *ss, int basisl, msym_orbital_t basis[basisl], int *offset, double c[basisl][basisl]);
int filterSubspace(msym_subspace_t *ss);



//These are not the real orbitals but linear combinations:
//lm+ = p(z)((x+iy)^m+(x-iy)^m)
msym_error_t orbitalFromQuantumNumbers(int n, int l, int m, msym_orbital_t *o){
    
    if(l > n || abs(m) > l) goto err;
    
    o->n = n;
    o->l = l;
    o->m = m;
    
    memset(o->name,0,sizeof(o->name));
        
    switch(l) {
        case 0 :
            snprintf(o->name, sizeof(o->name), "%ds",n);
            //o->v = spolynomial[m+l];
            break;
        case 1 : {
            char *d = "?";
            //o->v = ppolynomial[m+l];
            switch(m) {
                case -1 : //Not really the case but makes things easier to generate
                    d = "y";
                    break;
                case 1 :
                    d = "x";
                    break;
                case 0 :
                    d = "z";
                    break;
            }
            snprintf(o->name, sizeof(o->name), "%dp%s",n,d);
            break;
        }
        case 2 : {
            //o->v = dpolynomial[m+l];
            char *d = (signbit((float)m) == 1 ? "-" : "+");
            snprintf(o->name, sizeof(o->name), "%dd%d%s",n,abs(m),d);
            break;
        }
        default : {
            char t = 'f' - 3 + l;
            char *d = (signbit((float)m) == 1 ? "-" : "+");
            snprintf(o->name, sizeof(o->name), "%d%c%d%s",n,t,abs(m),d);
        }
    }
    return MSYM_SUCCESS;
err:
    msymSetErrorDetails("Invalid orbital quantum numbers n:%d l:%d m:%d",n,l,m);
    return MSYM_INVALID_ORBITALS;
}

msym_error_t orbitalFromName(char *name, msym_orbital_t *o){
    int n, l, m;
    char cl, cm1, cm2;
    
    sscanf(name,"%d%c%c%c",&n,&cl,&cm1,&cm2);
    
    switch(cl) {
        case 's' : l = m = 0; break;
        case 'p' : {
            l = 1;
            switch(cm1) {
                case 'x' : m = 1; break;
                case 'y' : m = -1; break;
                case 'z' : m = 0; break;
                default : goto err;
            }
            break;
        }
        default :
            if(cl < 'd' || cl == 'e' || cl > 'z') goto err;
            l = cl == 'd' ? 2 : cl + 3 - 'f';
            m = (cm1 - (int)'0')*(cm2 == '-' ? -1 : 1);
    }

    return orbitalFromQuantumNumbers(n,l,m,o);
    
err:
    msymSetErrorDetails("Invalid orbital name %s",name);
    return MSYM_INVALID_ORBITALS;

}


msym_error_t orbitalPolynomial(int l, int m, double *poly){
    msym_error_t ret = MSYM_SUCCESS;
    int pdim = ipow(3,l);
    if(abs(m) > l) {ret = MSYM_INVALID_ORBITALS; goto err;}
    switch (l) {
        case 0 : vlcopy(pdim, spolynomial[m+l],poly); break;
        case 1 : vlcopy(pdim, ppolynomial[m+l],poly); break;
        case 2 : vlcopy(pdim, dpolynomial[m+l],poly); break;
        default: {
            msymSetErrorDetails("Cannot handle azimithal %d",l);
            ret = MSYM_INVALID_ORBITALS;
        }
    }
err:
    return ret;
}

//We can split this into a part for each l and just build the subspaces, and we already have the permutation matrix so
msym_error_t findProjection(CharacterTable *ct, int sopsl, msym_symmetry_operation_t sops[sopsl], msym_permutation_t perm[sopsl], int l, msym_orbital_t *basis[2*l+1]){
    msym_error_t ret = MSYM_SUCCESS;
    int kdim = ipow(3,l), setl = perm[0].p_length;
    double (*mkron)[kdim] = malloc(sizeof(double[kdim][kdim]));
    double (*mperm)[setl] = malloc(sizeof(double[setl][setl]));
    
    for(int m = 0; m < 2*l+1;m++){
        permutationMatrix(&perm[m], mperm);
    }
    
    free(mperm);
    free(mkron);
    return ret;
}

msym_error_t generateOrbitalTransforms(int sopsl, msym_symmetry_operation_t sops[sopsl], int l, double transform[sopsl][2*l+1][2*l+1]){
    msym_error_t ret = MSYM_SUCCESS;
    int kdim = ipow(3,l), norbs = 2*l+1;
    double (*mkron)[kdim][kdim] = malloc(sizeof(double[2][kdim][kdim]));
    double (*poly)[kdim] = malloc(sizeof(double[norbs][kdim]));
    
    for(int m = -l; m <= l;m++){
        if(MSYM_SUCCESS != (ret = orbitalPolynomial(l,m,poly[m+l]))) goto err;
        
        //Normalization
        vlnorm(kdim, poly[m+l]);
    }
    
    
    for(int i = 0;i < sopsl;i++){
        double M[3][3];
        mkron[0][0][0] = 1.0;
        symmetryOperationMatrix(&sops[i], M);
        for(int j = 0, d = 1;j < l;j++, d *= 3){
            kron(3,M,d,mkron[j%2],3*d,mkron[(j+1)%2]);
        }
        mmlmul(norbs, kdim, poly, kdim, mkron[l%2], mkron[(l+1)%2]);
        mmtlmul(norbs, kdim, mkron[(l+1)%2], norbs, poly, transform[i]);
        
        /* Scaling
        for(int j = 0; j < norbs;j++){
            double scale = vldot(kdim, poly[j], poly[j]);
            vlscale(1.0/scale, norbs, transform[i][j], transform[i][j]);
        }*/
    }
    
    
err:
    free(mkron);
    free(poly);
    return ret;
}

msym_error_t generateOrbitalSubspaces(msym_point_group_t *pg, int esl, msym_equivalence_set_t *es, msym_permutation_t **perm, int basisl, msym_orbital_t basis[basisl], msym_thresholds_t *thresholds, int *subspacel, msym_subspace_t **subspace, int **pspan){
    msym_error_t ret = MSYM_SUCCESS;
    int lmax = -1, nmax = -1, eslmax = -1;
    for(int i = 0;i < basisl;i++){
        lmax = basis[i].l > lmax ? basis[i].l : lmax;
        nmax = basis[i].n > nmax ? basis[i].n : nmax;
    }
    
    if(lmax < 0){ret = MSYM_INVALID_ORBITALS; return ret;} //if we goto err here, code will get ugly due to scope
    
    for(int i = 0;i < esl;i++) eslmax = es[i].length > eslmax ? es[i].length : eslmax;
    
    struct _ltransforms {int d; void *t;} *lts = calloc(lmax+1,sizeof(struct _ltransforms));
    double (*mkron)[(2*lmax+1)*pg->order] = malloc(sizeof(double[(2*lmax+1)*pg->order][(2*lmax+1)*pg->order]));
    double (*mperm)[pg->order] = malloc(sizeof(double[pg->order][pg->order]));
    
    double (*mproj)[pg->ct->l+1][(2*lmax+1)*pg->order][(2*lmax+1)*pg->order] = malloc(sizeof(double[lmax+1][pg->ct->l+1][(2*lmax+1)*pg->order][(2*lmax+1)*pg->order]));
    double (*lspan)[pg->ct->l] = malloc(sizeof(double[lmax+1][pg->ct->l]));
    int (*ispan) = calloc(pg->ct->l,sizeof(int));
    int *aspan = calloc(pg->ct->l,sizeof(int));
    int *nl = malloc(sizeof(int[lmax+1]));
    
    msym_orbital_t *(*omap)[nmax][lmax][2*lmax+1] = malloc(sizeof(msym_orbital_t *[eslmax][nmax+1][lmax+1][2*lmax+1]));
    
    *subspace = NULL;
    
    msym_subspace_t *iss = calloc(pg->ct->l, sizeof(msym_subspace_t));
    for(int k = 0;k < pg->ct->l;k++){
        iss[k].type = ATOMIC_ORBITAL;
        iss[k].irrep = k;
        iss[k].subspacel = esl;
        iss[k].subspace = calloc(esl, sizeof(msym_subspace_t));
    }
    
    for(int l = 0; l <= lmax;l++){
        lts[l].d = 2*l+1;
        lts[l].t = malloc(sizeof(double[pg->sopsl][lts[l].d][lts[l].d]));
        if(MSYM_SUCCESS != (ret = generateOrbitalTransforms(pg->sopsl, pg->sops, l, lts[l].t))) goto err;
    }
    
    for(int i = 0; i < esl;i++){
        int esilmax = -1, esinmax = -1;
        
        memset(nl,0,sizeof(int[lmax+1]));
        for(int j = 0;j < es[i].elements[0]->aol;j++){
            esilmax = esilmax < es[i].elements[0]->ao[j]->l ? es[i].elements[0]->ao[j]->l : esilmax;
            esinmax = esinmax < es[i].elements[0]->ao[j]->n ? es[i].elements[0]->ao[j]->n : esinmax;
            nl[es[i].elements[0]->ao[j]->l] += es[i].elements[0]->ao[j]->m == 0;
        }
        
        msym_orbital_t *(*esomap)[esinmax+1][esilmax+1][2*esilmax+1] = omap;
        
        memset(esomap,0,sizeof(msym_orbital_t *[es->length][esinmax+1][esilmax+1][2*esilmax+1]));
        for(int a = 0;a < es[i].length;a++){
            for(int ao = 0;ao < es[i].elements[a]->aol;ao++){
                msym_orbital_t *o = es[i].elements[a]->ao[ao];
                esomap[a][o->n][o->l][o->m+o->l] = o;
            }
        }
        
        memset(lspan,0,sizeof(double[lmax+1][pg->ct->l]));
        
        for(int l = 0;l <= esilmax;l++){
            int d = es[i].length*lts[l].d;
            double (*mlproj)[d][d] = mproj[l];
            memset(mlproj,0,sizeof(double[pg->ct->l][d][d]));
            memset(ispan,0,sizeof(int[pg->ct->l]));
            
            for(int s = 0;s < pg->sopsl;s++){
                double (*lt)[lts[l].d][lts[l].d] = lts[l].t;
                permutationMatrix(&perm[i][s], mperm);
                kron(perm[i][s].p_length,mperm,lts[l].d,lt[s],d,mkron);
                
                //printSymmetryOperation(&pg->sops[s]);
                //printTransform(d, d, mkron);
                
                for(int k = 0;k < pg->ct->l;k++){
                    lspan[l][k] += pg->ct->irrep[k].v[pg->sops[s].cla]*mltrace(d, mkron);
                    mlscale(pg->ct->irrep[k].v[pg->sops[s].cla], d, mkron, mlproj[pg->ct->l]); //mproj[pg->ct.l] is a workspace
                    mladd(d, mlproj[pg->ct->l], mlproj[k], mlproj[k]); //Could do this based on the span later, but it's not a huge amount of work
                }
            }
            memset(mlproj[pg->ct->l],0,sizeof(double[d][d]));
            int nirrepl = 0;
            for(int k = 0;k < pg->ct->l;k++){
                int lirrepl = nirrepl;
                msym_subspace_t *ss = &iss[k].subspace[i];
                ispan[k] = (((int)round(lspan[l][k]))/pg->order);
                mlscale(((double) pg->ct->irrep[k].d)/pg->order, d, mlproj[k], mlproj[k]);
                nirrepl = mgs(d, mlproj[k], mlproj[pg->ct->l], nirrepl, thresholds->orthogonalization/basisl);
                
                if(nirrepl - lirrepl != ispan[k]*pg->ct->irrep[k].d){
                    //printTransform(d, d, mlproj[k]);
                    ret = MSYM_ORBITAL_ERROR;
                    msymSetErrorDetails("Ortogonal subspace of dimension (%d) inconsistent with span (%d) in %s",nirrepl - lirrepl,ispan[k]*pg->ct->irrep[k].d,pg->ct->irrep[k].name);
                    goto err;
                    
                }
                
                ss->type = ATOMIC_ORBITAL;
                ss->irrep = k;
                
                if(ispan[k] > 0){
                    ss->subspace = realloc(ss->subspace, sizeof(msym_subspace_t[ss->subspacel+nl[l]]));
                    for(int n = l+1; n <= esinmax;n++){
                        if(esomap[0][n][l][0] == NULL) continue;
                        
                        aspan[k] += ispan[k]*pg->ct->irrep[k].d;
                        msym_subspace_t *nss = &ss->subspace[ss->subspacel];
                        memset(nss,0,sizeof(msym_subspace_t));

                        nss->type = ATOMIC_ORBITAL;
                        nss->irrep = ss->irrep;
                        nss->d = ispan[k]*pg->ct->irrep[k].d;
                        double (*space)[d] = malloc(sizeof(double[nss->d][d]));
                        for(int dim = 0; dim < ss->subspace[ss->subspacel].d;dim++){
                            vlnorm2(d, mlproj[pg->ct->l][lirrepl+dim], space[dim]);
                        }
                        nss->space = (double*) space;
                        nss->basisl = 0;
                        nss->basis.o = malloc(sizeof(msym_orbital_t *[d]));
                        for(int e = 0;e < es[i].length;e++){
                            for(int m = -l;m <= l;m++){
                                nss->basis.o[nss->basisl++] = esomap[e][n][l][m+l];
                            }
                        }
                        ss->subspacel++;
                        if(nss->basisl != d) {
                            ret = MSYM_ORBITAL_ERROR;
                            msymSetErrorDetails("Basis length (%d) inconsistent with projection operator dimensions (%d)",nss->basisl,d);
                            goto err;
                        }
                    }
                }
            }
        }
    }

    printf("Subspace span (vectors) = ");
    for(int k = 0;k < pg->ct->l;k++){
        printf(" + %d%s",aspan[k],pg->ct->irrep[k].name);
    }
    printf("\n");
    
    msym_subspace_t tss = {.subspacel = pg->ct->l, .subspace = iss, .d = 0, .basisl = 0, .space = NULL};
    filterSubspace(&tss);
    *subspace = tss.subspace;
    *subspacel = tss.subspacel;
    *pspan = aspan;
    
    free(omap);
    free(nl);
    free(ispan);
    free(lspan);
    free(mproj);
    free(mperm);
    free(mkron);
    for(int l = 0;l <= lmax;l++){
        free(lts[l].t);
    }
    free(lts);
    
    return ret;
    
err:
    free(aspan);
    free(omap);
    free(nl);
    free(ispan);
    free(lspan);
    free(mproj);
    free(mperm);
    free(mkron);
    for(int l = 0;l < lmax;l++){
        free(lts[l].t);
    }
    free(lts);
    for(int k = 0;k < pg->ct->l;k++){
        freeSubspace(&iss[k]);
    }
    free(iss);
    return ret;
}

msym_error_t getOrbitalSubspaces(int ssl, msym_subspace_t ss[ssl], int basisl, msym_orbital_t basis[basisl], double c[basisl][basisl]){
    msym_error_t ret = MSYM_SUCCESS;
    int index = 0;
    memset(c,0,sizeof(double[basisl][basisl]));
    for(int i = 0;i < ssl;i++){
        if(MSYM_SUCCESS != (ret = getOrbitalSubspaceCoefficients(&ss[i],basisl,basis,&index,c))) goto err;
    }
    
    if(index != basisl){
        msymSetErrorDetails("Subspace index (%d) does not match basis length (%d)",index,basisl);
        ret = MSYM_INVALID_SUBSPACE;
        goto err;
    }
    
    return ret;
err:
    return ret;
}

msym_error_t getOrbitalSubspaceCoefficients(msym_subspace_t *ss, int basisl, msym_orbital_t basis[basisl], int *offset, double c[basisl][basisl]){
    msym_error_t ret = MSYM_SUCCESS;
    
    int index = *offset;
    if(index >= basisl) {
        msymSetErrorDetails("Subspace index (%d) is larger than basis length (%d)",index,basisl);
        ret = MSYM_INVALID_SUBSPACE;
        goto err;
    }
    
    if(ss->subspacel == 0){
        double (*space)[ss->basisl] = (double (*)[ss->basisl]) ss->space;
        if(index+ss->d > basisl) {
            msymSetErrorDetails("Generated subspaces (%d) is larger than basis length (%d)",index+ss->d,basisl);
            ret = MSYM_INVALID_SUBSPACE;
            goto err;
        }
        for(int d = 0;d < ss->d;d++){
            for(int b = 0;b < ss->basisl;b++){
                c[index][ss->basis.o[b]-basis] = space[d][b];
            }
            //printf("orbital %d is in irrep %d\n",index,ss->irrep);
            index++;
        }
    } else {
        for(int i = 0;i < ss->subspacel;i++){
            if(MSYM_SUCCESS != (ret = getOrbitalSubspaceCoefficients(&ss->subspace[i],basisl,basis,&index,c))) goto err;
        }
    }
    
    *offset = index;
    
    return ret;
err:
    return ret;
}

//Should filter so we don't get subspaces with just one subspace as well
int filterSubspace(msym_subspace_t *ss){
    int ret = 0;
    if(ss->subspacel == 0){
        ret = ss->d > 0 && ss->basisl > 0;
    } else {
        for(int i = 0;i < ss->subspacel;i++){
            if(!filterSubspace(&ss->subspace[i])){
                ss->subspacel--;
                
                if(ss->subspacel == 0){
                    free(ss->subspace);
                    ss->subspace = NULL;
                    break;
                } else {
                    memcpy(&ss->subspace[i], &ss->subspace[ss->subspacel], sizeof(msym_subspace_t));
                    ss->subspace = realloc(ss->subspace, sizeof(msym_subspace_t[ss->subspacel]));
                    i--;
                }
                
            }
        }
        ret = ss->subspacel > 0;
    }
    return ret;
}


void freeSubspace(msym_subspace_t *ss){
    free(ss->basis.o);
    free(ss->space);
    for(int i = 0; i < ss->subspacel;i++){
        freeSubspace(&ss->subspace[i]);
    }
    
    free(ss->subspace);
}


void printSubspace(CharacterTable *ct, msym_subspace_t *ss){
    printSubspaceTree(ct,ss,0);
}

void printSubspaceTree(CharacterTable *ct, msym_subspace_t *ss,int indent){
    if(ct == NULL){
        tabprintf("Subspace irrep: %d\n", indent,ss->irrep);
    } else {
        tabprintf("Subspace irrep: %s\n", indent,ct->irrep[ss->irrep].name);
    }
    if(ss->subspacel == 0){
        if(ss->d > 0 && ss->basisl > 0){
            tabprintf("", indent);
            for(int i = 0;i < ss->basisl;i++) printf("  %s\t",ss->basis.o[i]->name);
            printf("\n");
            double (*space)[ss->basisl] = (double (*)[ss->basisl]) ss->space;
            tabPrintTransform(ss->d,ss->basisl,space,indent);
        } else {
            tabprintf("No subspaces spaned\n", indent);
        }
    } else {
        for(int i = 0; i < ss->subspacel;i++){
            printSubspaceTree(ct,&ss->subspace[i],indent+1);
        }
    }
}

void tabprintf(char *format, int indent, ...){
    for(int i = 0; i < indent;i++) printf("\t");
    va_list args;
    va_start (args, indent);
    vprintf (format, args);
    va_end (args);
}


//Density matrix without occupation numbers
void densityMatrix(int l, double M[l][l], double D[l][l]){
    memset(D,0,sizeof(double[l][l]));
    for(int i = 0; i < l;i++){
        for(int j = 0;j < l;j++){
            for(int k = 0;k < l;k++){
                D[i][j] += M[k][i]*M[k][j];
            }
        }
    }
}

void printOrbital(msym_orbital_t *orb){
    printf("Orbital(%d,%d,%d) : %s\n",orb->n, orb->l, orb->m, orb->name);
}

void printTransform(int r, int c, double M[r][c]) {
    
    printf("\n[");
    for(int i = 0;i < r;i++){
        for(int j = 0;j<c;j++){
            char *pre = signbit(M[i][j]) ? "" : " ";
            char *post1 = "";
            char *post2 = (j == (c - 1)) ? (i == (r - 1)) ? "" : ";" : " ";
            
            printf("%s%.8lf%s%s",pre,M[i][j],post1,post2);
        }
        printf("%s",(i == (r - 1)) ? "]\n" : "\n ");
    }
    
}

void tabPrintTransform(int r, int c, double M[r][c],int indent) {
    if(r == 0 || c == 0) {tabprintf("[]\n",indent);return;}
    //printf("\n");
    tabprintf("[",indent);
    for(int i = 0;i < r;i++){
        for(int j = 0;j<c;j++){
            char *pre = signbit(M[i][j]) ? "" : " ";
            char *post1 = "\t";
            char *post2 = (j == (c - 1)) ? (i == (r - 1)) ? "" : ";" : " ";
            
            printf("%s%.3lf%s%s",pre,M[i][j],post1,post2);
        }
        printf("%s",(i == (r - 1)) ? "]\n" : "\n ");
        tabprintf(" ", indent);
    }
    printf("\n");
    
}


