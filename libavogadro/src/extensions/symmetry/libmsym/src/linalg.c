//
//  linalg.c
//  libmsym
//
//  Created by Marcus Johansson on 12/04/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//
//  Simple linear algebra functions

#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "linalg.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510582
#endif

#define SQR(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))

void mleye(int l, double E[l][l]){
    
    memset(E, 0, sizeof(double[l][l]));
    for(int i = 0;i < l;i++){
        E[i][i] = 1.0;
    }
}


//#define EQUAL(A,B,T) ((fabs(((A)-(B)))) < (T))
//#define LT(A,B,T) ((B) - (A) > (T))
//#define VEQUAL(v1,v2) ((EQUAL((v1)[0],(v2)[0],2ERO_THRESHOLD)) && (EQUAL((v1)[1],(v2)[1],2ERO_THRESHOLD)) && (EQUAL((v1)[2],(v2)[2],2ERO_THRESHOLD))) //Stop using this
//#define S2ERO(v) (EQUAL((v),0.0,2ERO_THRESHOLD))

//#define PARALLEL(v1,v2) (S2ERO(fabs(vdot((v1),(v2)))-1.0))
//#define PERPENDICULAR(v1,v2) (S2ERO(vdot((v1),(v2))))


void vrotate(double theta, double v[3], double axis[3], double vr[3]){

    double m[3][3];
    
    mrotate(theta,axis,m);

    mvmul(v,m,vr);
}

/*
 //v_reflected = v − 2n * dot(v,n) where n is the normal vector to the plane (assuming through origo)
 void vreflect(double v[3], double plane[3], double vr[3]){
 double vp[3];
 double dot = vdot(v,plane);
 vscale(2*dot,plane,vp);
 vsub(v,vp,vr);
 }
 */

void vreflect(double v[3], double axis[3], double vr[3]){
    
    double m[3][3];
    
    mreflect(axis,m);
    
    mvmul(v,m,vr);
}

// skew symmetric matrix of axb = S ->
// T = I+S+(1-a.b/(||axb||^2))*S^2
void malign(double v[3], double axis[3], double m[3][3]){
    double vn[3], axisn[3], cross[3], dot, skew[3][3], across, k;
    vnorm2(v, vn);
    vnorm2(axis, axisn);
    dot = vdot(vn,axisn);
    if(dot >= 1.0){
        mleye(3,m);
        //m[0][0] = m[1][1] = m[2][2] = 1.0;
        //m[0][1] = m[0][2] = m[1][0] = m[1][2] = m[2][0] = m[2][1] = 0.0;
    
    } else if(dot <= -1.0){
        vcomplement(axis, cross);
        mrotate(M_PI, cross, m);
    } else {
    
        vcross(vn,axisn,cross);
        across = vabs(cross);
        
        
        
        k = (1-dot)/(SQR(across));
        
        skew[0][0] = 0.0;       skew[0][1] = -cross[2]; skew[0][2] = cross[1];
        skew[1][0] = cross[2];  skew[1][1] = 0.0;       skew[1][2] = -cross[0];
        skew[2][0] = -cross[1]; skew[2][1] = cross[0];  skew[2][2] = 0.0;
        
        mleye(3,m);
        
        madd(m,skew,m);
        mmmul(skew,skew,skew);
        mscale(k,skew,skew);
        madd(m,skew,m);
    }
}

void mrotate(double theta, double axis[3], double m[3][3]){
    double c = cos(theta);
    double s = sin(theta);
    
    //rotation matrix about an axis
    m[0][0] = c + (1 - c) * SQR(axis[0]);
    m[0][1] = (1 - c) * axis[0] * axis[1] - s * axis[2];
    m[0][2] = (1 - c) * axis[0] * axis[2] + s * axis[1];
    m[1][0] = (1 - c) * axis[0] * axis[1] + s * axis[2];
    m[1][1] = c + (1 - c) * axis[1] * axis[1];
    m[1][2] = (1 - c) * axis[2] * axis[1] - s * axis[0];
    m[2][0] = (1 - c) * axis[0] * axis[2] - s * axis[1];
    m[2][1] = (1 - c) * axis[1] * axis[2] + s * axis[0];
    m[2][2] = c + (1 - c) * axis[2] * axis[2];

}

void mreflect(double axis[3], double m[3][3]){
    
    m[0][0] = 1 - 2*(SQR(axis[0]));
    m[1][1] = 1 - 2*(SQR(axis[1]));
    m[2][2] = 1 - 2*(SQR(axis[2]));
    m[0][1] = m[1][0] = -2*axis[0]*axis[1];
    m[0][2] = m[2][0] = -2*axis[0]*axis[2];
    m[1][2] = m[2][1] = -2*axis[1]*axis[2];
    
}

int vzero(double v[3], double t){
    return vabs(v) <= t;
}

int vparallel(double v1[3], double v2[3], double t){
    double tv1[3], tv2[3];
    vnorm2(v1,tv1);
    vnorm2(v2,tv2);
    return fabs(fabs(vdot(tv1,tv2))-1.0) <= t;
}

int vperpendicular(double v1[3], double v2[3], double t){
    double tv1[3], tv2[3];
    vnorm2(v1,tv1);
    vnorm2(v2,tv2);
    return fabs(vdot(tv1,tv2)) <= t;
}

int vequal(double v1[3],double v2[3], double t){
    double vs[3], va[3];
    vsub(v1,v2,vs);
    vadd(v1,v2,va);
    return (vabs(vs) <= t && vabs(va) <= t)|| (vabs(vs)/vabs(va) <= t);
}

int vequalold(double v1[3],double v2[3], double t){
    int e = 1;
    for(int i = 0;i < 3;i++){
        e &= (fabs(v1[i] - v2[i]) <= t);
    }
    return e;
}

//v_projected = v - dot(v, n) * n  where n is the normal vector to the plane (assuming through origo)
void vproj_plane(double v[3], double plane[3], double proj[3]){
    double vp[3], nplane[3];
    vnorm2(plane,nplane);
    double dot = vdot(v,nplane);
    vscale(dot,nplane,vp);
    vsub(v,vp,proj);
}

void vproj(double v[3], double u[3], double vo[3]){
    vlproj(3, v, u, vo);
}

void vlproj(int l, double v[l], double u[l], double vo[l]){
    vlscale(vldot(l,u,v)/vldot(l,u,u),l,u,vo);
}

void vcomplement(double v1[3], double v2[3]){
    double c[2][3] = {{v1[2],v1[2],-v1[0]-v1[1]},{-v1[1]-v1[2],v1[0],v1[0]}};
    int i = ((v1[2] != 0.0) && (-v1[0] != v1[1]));
    vcopy(c[i], v2);
    vnorm(v2);
}

double vangle(double v1[3], double v2[3]){
    double c = vdot(v1,v2)/(vabs(v1)*vabs(v2));
    
    if(c > 1.0) c = 1.0;
    if(c < -1.0) c = -1.0;
    
    return acos(c);
}



void vcross(double v1i[3],double v2i[3], double vr[3]) {
    double v1[3], v2[3]; //use copies so we can do vcross(a,b,a);
    vcopy(v1i,v1);
    vcopy(v2i,v2);
    vr[0] = v1[1]*v2[2]-v1[2]*v2[1];
    vr[1] = v1[2]*v2[0]-v1[0]*v2[2];
    vr[2] = v1[0]*v2[1]-v1[1]*v2[0];
}

double vcrossnorm(double v1i[3],double v2i[3], double vr[3]) {
    vcross(v1i,v2i,vr);
    return vnorm(vr);
}

double vdot(double v1[3], double v2[3]) {
    return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
    
}

double vldot(int l, double v1[l], double v2[l]) {
    double d = 0;
    for(int i = 0; i < l; i++) d+= v1[i]*v2[i];
    return d;
}

void vadd(double v1[3],double v2[3], double vr[3]){
    vr[0] = v1[0] + v2[0];
    vr[1] = v1[1] + v2[1];
    vr[2] = v1[2] + v2[2];
}

void vladd(int l, double v1[l],double v2[l], double vr[l]){
    for(int i = 0; i < l;i++) vr[i] = v1[i] + v2[i];
}


void vscale(double s,double v[3], double vr[3]){
    vr[0] = s*v[0];
    vr[1] = s*v[1];
    vr[2] = s*v[2];
}

void vlscale(double s,int l, double v[l], double vr[l]){
    for(int i = 0;i < l;i++) vr[i] = s*v[i];
}

void mscale(double s,double m[3][3], double mr[3][3]){
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            mr[i][j] = s*m[i][j];
        }
    }
}

void mlscale(double s,int l, double m[l][l], double mr[l][l]){
    for(int i=0; i<l; ++i){
        for(int j=0; j<l; ++j){
            mr[i][j] = s*m[i][j];
        }
    }
}

double mltrace(int l, double M[l][l]) {
    double trace = 0;
    for(int i = 0; i < l; i++) trace += M[i][i];
    return trace;
}

void vsub(double v1[3],double v2[3], double vr[3]){
    vr[0] = v1[0] - v2[0];
    vr[1] = v1[1] - v2[1];
    vr[2] = v1[2] - v2[2];
}

void vlsub(int l, double v1[l],double v2[l], double vr[l]){
    for(int i = 0; i < l;i++) vr[i] = v1[i] - v2[i];
}

double vabs(double v[3]){
    return sqrt(SQR(v[0])+SQR(v[1])+SQR(v[2]));
}

double vlabs(int l, double v[l]){
    double r = 0;
    for(int i = 0;i < l;i++) r += SQR(v[i]);
    return sqrt(r);
}

void vinv(double v[3]){
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
}

double vnorm(double v[3]){
    double norm = vabs(v);
    if (norm != 0.0) {
        v[0] /= norm;
        v[1] /= norm;
        v[2] /= norm;
    };
    return norm;
}

double vlnorm(int l, double v[l]){
    double norm = vlabs(l,v);
    if (norm != 0.0) {
        for(int i = 0; i < l;i++){
            v[i] /= norm;
        }
    };
    return norm;
}

double vlnorm2(int l, double v1[l], double v2[l]){
    double norm = vlabs(l,v1);
    if (norm != 0.0) {
        for(int i = 0; i < l;i++){
            v2[i] = v1[i]/norm;
        }
    };
    return norm;
}

double vnorm2(double v1[3], double v2[3]){
    
    double abs = vabs(v1);
    double norm = 1.0/(abs+DBL_MIN);
    vscale(norm, v1, v2);
    return abs;
}

void vcopy(double vi[3], double vo[3]){
    vo[0] = vi[0];
    vo[1] = vi[1];
    vo[2] = vi[2];
}

void vlcopy(int l, double vi[l], double vo[l]){
    for(int i = 0; i < l; i++) vo[i] = vi[i];
}

void mvmul(double v[3], double m[3][3], double r[3]){
    double t[3];
    t[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
    t[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
    t[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
    r[0] = t[0];
    r[1] = t[1];
    r[2] = t[2];
}

void mvlmul(int r, int c, double M[r][c], double v[c], double vo[r]){
    memset(vo, 0, sizeof(double[r]));
    for(int i = 0; i < r; i++){
        for(int j = 0; j < c;j++){
            vo[i] += M[i][j]*v[j];
        }
    }
}


void mmmul(double A[3][3], double B[3][3], double C[3][3]){
    double T[3][3];
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            T[i][j] = 0.0;
        }
    }
    
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            for(int k=0; k<3; ++k)
            {
                T[i][j]+=A[i][k]*B[k][j];
            }
        }
    }
    mcopy(T,C);
}

void mmtlmul(int rla, int cla, double A[rla][cla], int rlb, double B[rlb][cla], double C[rla][rlb]){
    double (*T)[rlb] = malloc(sizeof(double[cla][rlb]));
    mltranspose(rlb, cla, B, T);
    mmlmul(rla,cla,A,rlb,T,C);
    free(T);
}

void mmlmul(int rla, int cla, double A[rla][cla], int clb, double B[cla][clb], double C[rla][clb]){
    
    double (*T)[clb];
    if(A == C || B == C){
        T = malloc(sizeof(double[rla][clb]));
    } else {
        T = C;
    }
    
    for(int r=0; r < rla; r++){
        for(int c=0; c < clb; c++){
            T[r][c] = 0.0;
            for(int k=0; k < cla; k++)
            {
                T[r][c]+=A[r][k]*B[k][c];
                
            }
        }
    }
    if(A == C || B == C){
        for(int r=0; r < rla; r++){
            for(int c=0; c < clb; c++){
                C[r][c] = T[r][c];
            }
        }
        free(T);
    }
}

int mequal(double A[3][3], double B[3][3], double t){
    int e = 1;
    for(int i = 0;i < 3;i++){
        for(int j = 0;j < 3;j++){
            e &= (fabs(A[i][j] - B[i][j]) <= t);
        }
    }
    return e;
}

void mlFilterSmall(int l, double A[l][l]){
    for(int i = 0;i < l;i++){
        for(int j = 0;j < l;j++){
            if(fabs(A[i][j]) < DBL_EPSILON) A[i][j] = 0.0;
        }
    }
}

//only square matrixes for now
void kron(int al, double A[al][al], int bl, double B[bl][bl], int cl, double C[cl][cl]){
    for(int ai = 0; ai < al;ai++){
        for(int aj = 0; aj < al;aj++){
            for(int bi = 0; bi < bl;bi++){
                for(int bj = 0; bj < bl;bj++){
                    C[ai*bl+bi][aj*bl+bj] = A[ai][aj]*B[bi][bj];
                }
            }
        }
    }
}

void vlprint(int l, double v[l]) {
    printf("[");
    for(int i = 0;i < l;i++){
        printf("%lf%s",v[i],(i == (l - 1)) ? "]\n" : ";");
    }
}


/* Graam-Schmidt */
int mgs(int l, double M[l][l], double O[l][l], int n, double t){
    double *tmp = malloc(sizeof(double[l]));
    for(int i = 0; i < l;i++){
        if(vlabs(l,M[i]) < t){
            continue;
        }
        if(n == 0){
            vlcopy(l, M[i], O[n]);
            n++;
        } else if (n < l) {
            vlcopy(l, M[i], O[n]);
            for(int j = 0; j < n;j++){
                vlproj(l, O[n], O[j], tmp);
                vlsub(l, O[n], tmp, O[n]);
            }
            n += !(vlabs(l, O[n]) < t);
        }
    }
    free(tmp);
    return n;
}


void jacobi(double m[6], double e[3], double ev[3][3], double threshold){
    double err = 1.0;
    e[0] = m[0];
    e[1] = m[3];
    e[2] = m[5];

    mleye(3,ev);
    
    while(err > 0){
        err = 0.0;
        for(int od = 0;od < 3;od++){
            int i = 1 << od, row = od >> 1, col = 1 + (od >> (od >> 1));
            double ami = fabs(m[i]), eps = ami/threshold;
            
            if(fabs(e[row]) + eps == fabs(e[row]) && fabs(e[col]) + eps == fabs(e[col])){
                m[i] = 0.0;
            } else if(ami > 0.0) {
                err = fmax(ami, err);
                double d = e[col] - e[row],
                t = copysign(2,d)*m[i]/(fabs(d) + sqrt(SQR(d)+4*SQR(m[i]))),
                c = 1/sqrt(1+SQR(t)),
                s = c*t;
                
                e[row] -= t * m[i];
                e[col] += t * m[i];
                
                m[i] = 0.0;
                
                /* rotate eigenvectors */
                for (int k = 0; k < 3; k++){
                    double evr = ev[k][row], evc = ev[k][col];
                    ev[k][row] = c*evr - s*evc;
                    ev[k][col] = s*evr + c*evc;
                }
                
                /* rotate index */
                int ix = col ^ 3, iy = 4 >> row;
                double mix = m[ix], miy = m[iy];
                m[ix] = c*mix - s*miy;
                m[iy] = s*mix + c*miy;
            }
        }
    }
}


void madd(double A[3][3], double B[3][3], double C[3][3]){
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            C[i][j] = A[i][j] + B[i][j];
        }
    }
}

void mladd(int l, double A[l][l], double B[l][l], double C[l][l]){
    for(int i=0; i<l; ++i){
        for(int j=0; j<l; ++j){
            C[i][j] = A[i][j] + B[i][j];
        }
    }
}

void mcopy(double A[3][3], double B[3][3]){
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            B[i][j] = A[i][j];
        }
    }
}

double mdet(double M[3][3]){
    double d0 = M[1][1] * M[2][2] - M[2][1] * M[1][2];
    double d1 = M[1][0] * M[2][2] - M[1][2] * M[2][0];
    double d2 = M[1][0] * M[2][1] - M[1][1] * M[2][0];
    return (M[0][0]*d0 - M[0][1]*d1 + M[0][2]*d2);
}

void minv(double M[3][3], double I[3][3]){
    
    double d0 = M[1][1] * M[2][2] - M[2][1] * M[1][2];
    double d1 = M[1][0] * M[2][2] - M[1][2] * M[2][0];
    double d2 = M[1][0] * M[2][1] - M[1][1] * M[2][0];
    double det = (M[0][0]*d0 - M[0][1]*d1 + M[0][2]*d2);
    
    I[0][0] = d0/det;
    I[0][1] = (M[0][2] * M[2][1] - M[0][1] * M[2][2])/det;
    I[0][2] = (M[0][1] * M[1][2] - M[0][2] * M[1][1])/det;
    I[1][0] = -d1/det;
    I[1][1] = (M[0][0] * M[2][2] - M[0][2] * M[2][0])/det;
    I[1][2] = (M[1][0] * M[0][2] - M[0][0] * M[1][2])/det;
    I[2][0] = d2/det;
    I[2][1] = (M[2][0] * M[0][1] - M[0][0] * M[2][1])/det;
    I[2][2] = (M[0][0] * M[1][1] - M[1][0] * M[0][1])/det;
    
    
}

void mlcopy(int l, double A[l][l], double B[l][l]){
    for(int i=0; i<l; ++i){
        for(int j=0; j<l; ++j){
            B[i][j] = A[i][j];
        }
    }
}

void mtranspose(double A[3][3], double B[3][3]){
    mltranspose(3, 3, A, B);
}



void mltranspose(int rl, int cl, double A[rl][cl], double B[cl][rl]){
    for(int r = 0; r < rl;r++){
        for(int c = 0; c < cl;c++){
            B[c][r] = A[r][c];
        }
    }
}

int ipow(int b, int e)
{
    int r = 1;
    while (e){
        if (e & 1)
            r *= b;
        e >>= 1;
        b *= b;
    }
    return r;
}
