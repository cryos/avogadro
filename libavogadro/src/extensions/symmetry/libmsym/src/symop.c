//
//  symop.c
//  Symmetry
//
//  Created by Marcus Johansson on 30/10/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <math.h>
#include <stdlib.h>
#include "msym.h"
#include "context.h"
#include "symop.h"
#include "linalg.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510582
#endif

int igcd(int a, int b);

void symopPow(msym_symmetry_operation_t *A, int pow, msym_symmetry_operation_t *O){
    double origo[3] = {0.0,0.0,0.0};
    int apow, gcd;
    O->power = 1;
    switch (A->type){
        case IDENTITY :
            O->type = IDENTITY;
            O->order = 0;
            vcopy(origo,O->v);
            break;
        case REFLECTION :
        case INVERSION :
            if(pow % 2 == 0){
                O->type = IDENTITY;
                O->order = 0;
                vcopy(origo,O->v);
            } else {
                O->type = A->type;
                O->order = 0;
                vcopy(A->v,O->v);
            }
            break;
        case PROPER_ROTATION :
            // A->power = 0 is assumed to be uninitialized and set to 1
            apow = A->power == 0 ? pow % A->order : (A->power*pow) % A->order;
            gcd = igcd(apow, A->order);
            if(apow == 0){
                O->type = IDENTITY;
                O->order = 0;
                vcopy(origo,O->v);
            } else {
                O->type = PROPER_ROTATION;
                O->order = A->order/gcd;
                O->power = apow/gcd;
                vcopy(A->v,O->v);
            }
            break;
        case IMPROPER_ROTATION :
            A->type = PROPER_ROTATION;
            symopPow(A, pow, O);
            A->type = IMPROPER_ROTATION;
            apow = A->power == 0 ? pow % (2*A->order) : A->power*pow % (2*A->order);
            if(O->type == IDENTITY && pow == 0){} // Do nothing
            else if(O->type == IDENTITY && A->order % 2 == 1 && apow == A->order){
                O->type = REFLECTION;
                O->order = 0;
                vcopy(A->v,O->v);
            } else {
                if (apow > A->order && A->order % 2 == 1 && apow % 2 == 1){
                //if (apow > O->order && O->order % 2 == 1 && apow % 2 == 1){
                    O->power += A->order;
                    O->power %= 2*O->order;
                }
                if(apow % 2 == 1){
                    O->type = IMPROPER_ROTATION;
                }
                if(O->type == IMPROPER_ROTATION && O->order == 2){
                    O->type = INVERSION;
                    O->power = 1;
                }
            }
        default: break;
    }
}

void symmetryOperationName(msym_symmetry_operation_t* sop, int l, char buf[l]){
    switch (sop->type) {
        case PROPER_ROTATION   : snprintf(buf, l, "C%d^%d",sop->order,sop->power); break;
        case IMPROPER_ROTATION : snprintf(buf, l, "S%d^%d",sop->order,sop->power); break;
        case REFLECTION        : snprintf(buf, l, "R"); break;
        case INVERSION         : snprintf(buf, l, "i"); break;
        case IDENTITY          : snprintf(buf, l, "E"); break;
        default                : snprintf(buf, l, "?"); break;
    }
}

void symmetryOperationShortName(msym_symmetry_operation_t* sop, int l, char buf[l]){
    switch (sop->type) {
        case PROPER_ROTATION   : snprintf(buf, l, "C%d",sop->order); break;
        case IMPROPER_ROTATION : snprintf(buf, l, "S%d",sop->order); break;
        case REFLECTION        : snprintf(buf, l, "R"); break;
        case INVERSION         : snprintf(buf, l, "i"); break;
        case IDENTITY          : snprintf(buf, l, "E"); break;
        default                : snprintf(buf, l, "?"); break;
    }
}

void applySymmetryOperation(msym_symmetry_operation_t *sop,double iv[3], double ov[3]){
    switch (sop->type) {
        case PROPER_ROTATION   : {
            double theta = (sop->order == 0 ? 0.0 : sop->power*2*M_PI/sop->order);
            vrotate(theta, iv, sop->v, ov);
            break;
        }
        case IMPROPER_ROTATION : {
            double theta = sop->power*2*M_PI/sop->order;
            vrotate(theta, iv, sop->v, ov);
            vreflect(ov, sop->v, ov);
            break;
        }
        case REFLECTION        :
            vreflect(iv, sop->v, ov);
            break;
        case INVERSION         :
            ov[0] = -iv[0];
            ov[1] = -iv[1];
            ov[2] = -iv[2];
            break;
        case IDENTITY          :
            ov[0] = iv[0];
            ov[1] = iv[1];
            ov[2] = iv[2];
            break;
        default : fprintf(stderr,"UNKNOWN OPERATION\n"); //should never happen so will not handle this error atm
    }

}

void invertSymmetryOperation(msym_symmetry_operation_t *sop, msym_symmetry_operation_t *isop){
    copySymmetryOperation(isop, sop);
    switch (sop->type) {
        case PROPER_ROTATION   : {
            isop->power = sop->order - sop->power;
            break;
        }
        case IMPROPER_ROTATION : {
            if(sop->order % 2 == 0) isop->power = sop->order - sop->power;
            else isop->power = 2*sop->order - sop->power;
            break;
        }
        case REFLECTION        :
        case INVERSION         :
        case IDENTITY          :
            break;
        default : fprintf(stderr,"UNKNOWN OPERATION\n"); //should never happen so will not handle this error atm
    }
}

void symmetryOperationMatrix(msym_symmetry_operation_t *sop, double m[3][3]){
    switch (sop->type) {
        case PROPER_ROTATION   : {
            double theta = (sop->order == 0 ? 0.0 : sop->power*2*M_PI/sop->order);
            mrotate(theta, sop->v, m);
            break;
        }
        case IMPROPER_ROTATION : {
            double theta = sop->power*2*M_PI/sop->order;
            double m1[3][3], m2[3][3];
            mrotate(theta, sop->v, m1);
            mreflect(sop->v, m2);
            mmmul(m2, m1, m);
            break;
        }
        case REFLECTION        :
            mreflect(sop->v, m);
            break;
        case INVERSION         :
            for(int i = 0; i < 3; i++){
                for(int j = 0; j < 3; j++){
                    m[i][j] = -(i == j);
                }
            }
            break;
        case IDENTITY          :
            for(int i = 0; i < 3; i++){
                for(int j = 0; j < 3; j++){
                    m[i][j] = (i == j);
                }
            }
            break;
        default : fprintf(stderr,"UNKNOWN OPERATION\n"); //should never happen so will not handle this error atm
    }
}

void copySymmetryOperation(msym_symmetry_operation_t *dst, msym_symmetry_operation_t *src){
    dst->type = src->type;
    dst->order = src->order;
    dst->power = src->power;
    dst->cla = src->cla;
    vcopy(src->v, dst->v);
}

msym_symmetry_operation_t *findSymmetryOperation(msym_symmetry_operation_t *sop, msym_symmetry_operation_t *sops, int l, msym_thresholds_t *thresholds){
    msym_symmetry_operation_t *fsop = NULL;
    for(msym_symmetry_operation_t* s = sops; s < (sops + l);s++){
        if((s->type == INVERSION && sop->type == INVERSION) || (s->type == IDENTITY && sop->type == IDENTITY)){
            fsop = s;
            break;
        }
        
        else if(vparallel(s->v, sop->v,thresholds->angle) && s->type == sop->type && ((sop->type != PROPER_ROTATION && sop->type != IMPROPER_ROTATION) || (s->order == sop->order && s->power == sop->power))){
            fsop = s;
            break;
        }
    }
    
    return fsop;
}

void printSymmetryOperation(msym_symmetry_operation_t *sop){
    char buf[12];
    symmetryOperationName(sop, 12, buf);
    if(sop->type == INVERSION || sop->type == IDENTITY)
        printf("%s(%d)\n",buf,sop->cla);
    else
        printf("%s(%d) [%lf;%lf;%lf]\n",buf,sop->cla, sop->v[0],sop->v[1],sop->v[2]);
}

int igcd(int a, int b) {
    int c;
    while (a) {
        c = a;
        a = b % a;
        b = c;
    }
    return abs(b);
}
