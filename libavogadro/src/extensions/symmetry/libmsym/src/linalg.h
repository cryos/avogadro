//
//  linalg.h
//  libmsym
//
//  Created by Marcus Johansson on 13/04/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM_LINALG_h
#define __MSYM_LINALG_h

void mleye(int l, double **E);
int vzero(double v[3], double t);
int vparallel(double v1[3], double v2[3], double t);
int vperpendicular(double v1[3], double v2[3], double t);
double vnorm(double v[3]);
double vnorm2(double v1[3],double v2[3]);
double vlnorm(int l, double *v);
double vlnorm2(int l, double *v1, double *v2);
double vabs(double v[3]);
double vlabs(int l, double *v);
void vinv(double v[3]);
void vcopy(double vi[3], double vo[3]);
void vlcopy(int l, double *vi, double *vo);
void vcross(double v1i[3],double v2i[3], double vr[3]);
double vcrossnorm(double[3],double[3], double[3]);
double vdot(double[3],double[3]);
double vldot(int l, double *v1, double *v2);
int vequal(double v1[3],double v2[3], double t);
void vadd(double[3],double[3], double[3]);
void vladd(int l, double *v1,double *v2, double *vr);
void madd(double A[3][3], double B[3][3], double C[3][3]);
void mladd(int l, double **A, double **B, double **C);
void vsub(double[3],double[3], double[3]);
void vlsub(int l, double *v1,double *v2, double *vr);
void vscale(double,double[3], double[3]);
void vlscale(double s,int l, double *v, double *vr);
void mscale(double s,double m[3][3], double mr[3][3]);
void mlscale(double s,int l, double **m, double **mr);
void vproj_plane(double v[3], double plane[3], double proj[3]);
void vproj(double v[3], double u[3], double vo[3]);
void vlproj(int l, double *v, double *u, double *vo);
void vlprint(int l, double *v);
void vcomplement(double v1[3], double v2[3]);
double vangle(double[3],double[3]);
void vrotate(double theta, double v[3], double axis[3], double vr[3]);
void mrotate(double theta, double axis[3], double m[3][3]);
void vreflect(double v[3], double axis[3], double vr[3]);
void mreflect(double axis[3], double m[3][3]);
void mvmul(double v[3], double m[3][3], double r[3]);
void mvlmul(int r, int c, double **M, double *v, double *vo);
void mmmul(double A[3][3], double B[3][3], double C[3][3]);
void mmlmul(int rla, int cla, double **A, int clb, double **B, double **C);
void mmtlmul(int rla, int cla, double **A, int rlb, double **B, double **C);
void minv(double M[3][3], double I[3][3]);
double mdet(double M[3][3]);
void mcopy(double A[3][3], double B[3][3]);
void mlcopy(int l, double **A, double **B);
void mtranspose(double A[3][3], double B[3][3]);
void mltranspose(int rl, int cl, double **A, double **B);
double mltrace(int l, double **M);
int mequal(double A[3][3], double B[3][3], double t);
void malign(double v[3], double axis[3], double m[3][3]);
int ipow(int b, int e);
int mgs(int l, double **M, double **O, int n, double t);
void kron(int al, double **A, int bl, double **B, int cl, double **C);
void mlFilterSmall(int l, double **A);
void jacobi(double m[6], double e[3], double ev[3][3], double threshold);


#endif /* defined(__MSYM_LINALG_h) */
