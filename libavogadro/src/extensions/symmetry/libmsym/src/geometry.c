//
//  geometry.c
//  libmsym
//
//  Created by Marcus Johansson on 28/11/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <math.h>
#include <float.h>

#include "msym.h"
#include "geometry.h"
#include "linalg.h"

#define SQR(x) ((x)*(x))

void inertialTensor(int length, msym_element_t *elements[length], double cm[3], double e[3], double v[3][3], msym_thresholds_t *thresholds);
msym_geometry_t eigenvaluesToGeometry(double e[3], msym_thresholds_t *thresholds);

msym_error_t findGeometry(int length, msym_element_t *elements[length], double cm[3], msym_thresholds_t *thresholds, msym_geometry_t *g, double e[3], double v[3][3]){
    inertialTensor(length, elements, cm, e, v, thresholds);
    *g = eigenvaluesToGeometry(e,thresholds);
    return MSYM_SUCCESS;
}

msym_error_t findCenterOfMass(int length, msym_element_t *elements[length], double v[3]){
    msym_error_t ret = MSYM_SUCCESS;
    double t = 0;
    v[0] = v[1] = v[2] = 0.0;
    for(int i = 0; i < length; i++){
        
        msym_element_t *a = elements[i];
        v[0] += a->v[0]*a->m;
        v[1] += a->v[1]*a->m;
        v[2] += a->v[2]*a->m;
        t += a->m;
    }
    
    if(t <= 0.0 || t != t){
        msymSetErrorDetails("Invalid element mass sum: %lf",t);
        ret = MSYM_INVALID_ELEMENTS;
    } else {
        v[0] /= t;
        v[1] /= t;
        v[2] /= t;
    }
    return ret;
}

#define EQUAL(A,B,T) ((fabs(((A)-(B)))) < (T))
#define EQUAL_REL(A,B,T) (fabs(((A)-(B))/((A)+(B))) < T)

//Lookup table would look alot nicer
msym_geometry_t eigenvaluesToGeometry(double e[3], msym_thresholds_t *thresholds){
    int e01 = EQUAL(0.0, e[0], thresholds->geometry);
    int e12, e23, planar;
    
    if(e[1] > 1.0){ //If the eigenvalues are large we want a relative value (can't always use since they may be 0, and very small values will give massive errors with division)
        e12 = EQUAL_REL(e[0], e[1], thresholds->geometry);
    } else {
        e12 = EQUAL(e[0], e[1], thresholds->geometry);
    }
    
    if(e[2] > 1.0){
        e23    = EQUAL_REL(e[1], e[2], thresholds->geometry);
        planar = EQUAL_REL((e[0]+e[1]),e[2],thresholds->geometry);
    } else {
        e23    = EQUAL(e[1], e[2], thresholds->geometry);
        planar = EQUAL(e[0]+e[1],e[2],thresholds->geometry);
    }
    
    if(e12 && e23) {
        return SPHERICAL;
    } else if (e01 && e23) {
        return LINEAR;
    } else if (planar) {
        if(e12){
            return PLANAR_REGULAR;
        } else {
            return PLANAR_IRREGULAR;
        }
    } else if (e12) {
        return POLYHEDRAL_OBLATE;
    } else if (e23) {
        return POLYHEDRAL_PROLATE;
    } else {
        return ASSYMETRIC;
    }
    
}

int geometryDegenerate(msym_geometry_t g){
    return !(g == PLANAR_IRREGULAR || g == ASSYMETRIC) && g != GEOMETRY_UNKNOWN;
}

void inertialTensor(int length, msym_element_t *elements[length], double cm[3], double e[3], double v[3][3], msym_thresholds_t *thresholds){
    double Ixx = 0, Iyy = 0, Izz = 0, Ixy = 0, Ixz = 0, Iyz = 0;
    for(int i = 0; i < length; i++){
        msym_element_t *a = elements[i];
        Ixx += a->m*(SQR(a->v[1]-cm[1])+SQR(a->v[2]-cm[2]));
        Iyy += a->m*(SQR(a->v[0]-cm[0])+SQR(a->v[2]-cm[2]));
        Izz += a->m*(SQR(a->v[0]-cm[0])+SQR(a->v[1]-cm[1]));
        Ixy -= a->m*(a->v[0]-cm[0])*(a->v[1]-cm[1]);
        Ixz -= a->m*(a->v[0]-cm[0])*(a->v[2]-cm[2]);
        Iyz -= a->m*(a->v[1]-cm[1])*(a->v[2]-cm[2]);
    }

    double I[6] = {Ixx,Ixy,Ixz,Iyy,Iyz,Izz}, ev[3][3], ei[3];
    
    jacobi(I,ei,ev,thresholds->eigfact);
    
    int m = ((((ei[0] < ei[1]) << 1) | (ei[1] < ei[2])) << 1) | (ei[2] < ei[0]);
    int min[7] = {0,2,1,1,0,2,0}, mid[7] = {1,1,0,2,2,0,1}, max[7] = {2,0,2,0,1,1,2};
    int o[3] = {min[m],mid[m],max[m]};
    
    for(int i = 0; i < 3;i++){
        e[i] = ei[o[i]];
        for(int j = 0; j < 3;j++){
            v[i][j] = ev[j][o[i]];
        }
    }
}

void printGeometry(msym_geometry_t g){
    char *s;
    switch(g) {
        case SPHERICAL		: s = "spherical"; break;
        case LINEAR		: s = "linear"; break;
        case PLANAR_REGULAR	: s = "planar regular polygon"; break;
        case PLANAR_IRREGULAR	: s = "planar irregular polygon"; break;
        case POLYHEDRAL_PROLATE	: s = "prolate symmetric polyhedron"; break;
        case POLYHEDRAL_OBLATE	: s = "oblate symmetric polyhedron"; break;
        case ASSYMETRIC		: s = "assymetric polyhedron"; break;
        default			: s = "unknown geometry";
    }
    printf("%s\n",s);
}
