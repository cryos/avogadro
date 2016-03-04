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

void mleye(int l, double E[l][l]);
int vzero(double v[3], double t);
int vparallel(double v1[3], double v2[3], double t);
int vperpendicular(double v1[3], double v2[3], double t);
double vnorm(double v[3]);
double vnorm2(double v1[3],double v2[3]);
double vlnorm(int l, double v[l]);
double vlnorm2(int l, double v1[l], double v2[l]);
double vabs(double v[3]);
double vlabs(int l, double v[l]);
void vinv(double v[3]);
void vcopy(double vi[3], double vo[3]);
void vlcopy(int l, double vi[l], double vo[l]);
void vcross(double v1i[3],double v2i[3], double vr[3]);
double vcrossnorm(double[3],double[3], double[3]);
double vdot(double[3],double[3]);
double vldot(int l, double v1[l], double v2[l]);
int vequal(double v1[3],double v2[3], double t);
void vadd(double[3],double[3], double[3]);
void vladd(int l, double v1[l],double v2[l], double vr[l]);
void madd(double A[3][3], double B[3][3], double C[3][3]);
void mladd(int l, double A[l][l], double B[l][l], double C[l][l]);
void vsub(double[3],double[3], double[3]);
void vlsub(int l, double v1[l],double v2[l], double vr[l]);
void vscale(double,double[3], double[3]);
void vlscale(double s,int l, double v[l], double vr[l]);
void mscale(double s,double m[3][3], double mr[3][3]);
void mlscale(double s,int l, double m[l][l], double mr[l][l]);
void vproj_plane(double v[3], double plane[3], double proj[3]);
void vproj(double v[3], double u[3], double vo[3]);
void vlproj(int l, double v[l], double u[l], double vo[l]);
void vlprint(int l, double v[l]);
void vcomplement(double v1[3], double v2[3]);
double vangle(double[3],double[3]);
void vrotate(double theta, double v[3], double axis[3], double vr[3]);
void mrotate(double theta, double axis[3], double m[3][3]);
void vreflect(double v[3], double axis[3], double vr[3]);
void mreflect(double axis[3], double m[3][3]);
void mvmul(double v[3], double m[3][3], double r[3]);
void mvlmul(int r, int c, double M[r][c], double v[c], double vo[r]);
void mmmul(double A[3][3], double B[3][3], double C[3][3]);
void mmlmul(int rla, int cla, double A[rla][cla], int clb, double B[cla][clb], double C[rla][clb]);
void mmtlmul(int rla, int cla, double A[rla][cla], int rlb, double B[rlb][cla], double C[rla][rlb]);
void minv(double M[3][3], double I[3][3]);
double mdet(double M[3][3]);
void mcopy(double A[3][3], double B[3][3]);
void mlcopy(int l, double A[l][l], double B[l][l]);
void mtranspose(double A[3][3], double B[3][3]);
void mltranspose(int rl, int cl, double A[rl][cl], double B[cl][rl]);
double mltrace(int l, double M[l][l]);
int mequal(double A[3][3], double B[3][3], double t);
void malign(double v[3], double axis[3], double m[3][3]);
int ipow(int b, int e);
int mgs(int l, double M[l][l], double O[l][l], int n, double t);
void kron(int al, double A[al][al], int bl, double B[bl][bl], int cl, double C[cl][cl]);
void mlFilterSmall(int l, double A[l][l]);
void jacobi(double m[6], double e[3], double ev[3][3], double threshold);


#endif /* defined(__MSYM_LINALG_h) */
