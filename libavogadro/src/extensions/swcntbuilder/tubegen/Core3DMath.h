/* ===============================================================================
// Core3DMath.h												 �1999-2001, J.T. Frey
// ===============================================================================
// Written:		J.T. Frey, 03/11/2001
// Purpose:		Defines a specialized library for three-dimensional
//				applications and their specialized features.
//
// Last Mod:	n/a
*/

#ifndef __CORE3DMATH__
#define __CORE3DMATH__

// Removed for Avogadro:
// #include <stdc-includes.h>
//
// Replaced with:
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif
	
	/*!
	 *  @struct
	 *		TVector3D
	 *  @discussion
	 *		Cartesian vector in basis {<1,0,0>;<0,1,0>;<0,0,1>}
	 *  @field x		coefficient to <1,0,0>
	 *  @field y		coefficient to <0,1,0>
	 *  @field z		coefficient to <0,0,1>
	 */
	typedef struct TVector3D {
		double		x;
		double		y;
		double		z;
	} TVector3D;

/**/

	/*!
	 *  @struct
	 *		TPoint3D
	 *  @discussion
	 *		Cartesian point, which is merely a special variant of a vector.
	 *  @field x		displacement along x-axis
	 *  @field y		displacement along y-axis
	 *  @field z		displacement along z-axis
	 */
	#define TPoint3D					TVector3D

/**/

	/*!
	 *  @struct
	 *		TXMatrix3D
	 *  @discussion
	 *		A 4x4 matrix which represents a transformation
	 *		in 3-space.  We actually don't even use the 
	 */
	typedef struct TXMatrix {
		double 		matrix[16];
	} TXMatrix;

/**/

	/*!
	 *  @struct
	 *		TQuaternion
	 *  @discussion
	 *		A 4D vector quantity.  The <x,y,z> portion of the
	 *		quantity can be considered a vector in 3-space
	 *		which has been normalized to |sin (theta/2)|.
	 *  @field w		cos (theta/2)
	 *  @field x		sin (theta/2) * <nx,0,0>
	 *  @field y		sin (theta/2) * <0,ny,0>
	 *  @field z		sin (theta/2) * <0,0,nz>
	 */
	typedef struct TQuaternion {
		double		w;
		double		x;
		double		y;
		double		z;
	} TQuaternion;

/**/

	extern const TPoint3D			gOrigin;
	extern const TVector3D			gXAxis;
	extern const TVector3D			gYAxis;
	extern const TVector3D			gZAxis;

/**/

	/*  House-keeping ops:  */
	#define				Point3D_Set(P,X,Y,Z)			{ (P)->x = X ; (P)->y = Y ; (P)->z = Z; }
	#define				Vector3D_Set(V,X,Y,Z)			{ (V)->x = X ; (V)->y = Y ; (V)->z = Z; }
	#define				Quaternion_Set(Q,W,X,Y,Z)		{ (Q)->w = W ; (Q)->x = X ; (Q)->y = Y ;(Q)->z = Z; }
	void				Point3D_Zero					(TPoint3D*);
	void				Vector3D_Zero					(TVector3D*);
	void				Quaternion_Identity				(TQuaternion*);
	void				XMatrix_Identity				(TXMatrix*);
	void				Point3D_Splat					(TPoint3D*,double);
	void				Vector3D_Splat					(TVector3D*,double);
	void				Point3D_Rezero					(TPoint3D*,double);
	void				Vector3D_Rezero					(TVector3D*,double);
	void				Quaternion_Rezero				(TQuaternion*,double);
	void				XMatrix_Rezero					(TXMatrix*,double);
	
	/*  Math ops:  */
	
	/* 3D  V E C T O R  */
	double				Vector3D_Dot					(const TVector3D*,const TVector3D*);
	TVector3D*			Vector3D_Cross					(const TVector3D*,const TVector3D*,TVector3D*);
	double				Vector3D_Magnitude				(const TVector3D*);
	TVector3D*			Vector3D_Normalize				(const TVector3D*,TVector3D*);
	TVector3D*			Vector3D_Negate					(const TVector3D*,TVector3D*);
	TVector3D*			Vector3D_Scalar					(const TVector3D*,double,TVector3D*);
	TVector3D*			Vector3D_Sum					(const TVector3D*,const TVector3D*,TVector3D*);
	TVector3D*			Vector3D_ScaledSum				(const TVector3D*,double,const TVector3D*,TVector3D*);
	TVector3D*			Vector3D_Diff					(const TVector3D*,const TVector3D*,TVector3D*);
	TVector3D*			Vector3D_ScaledDiff				(const TVector3D*,double,const TVector3D*,TVector3D*);
	TVector3D*			Vector3D_TransformQuaternion	(const TVector3D*,const TQuaternion*,TVector3D*);
	TVector3D*			Vector3D_TransformMatrix		(const TVector3D*,const TXMatrix*,TVector3D*);
	void				Vector3D_WriteToStream			(const TVector3D*,FILE*);

	/* 3D  P O I N T  */
	double				Point3D_Distance				(const TPoint3D*,const TPoint3D*);
	TPoint3D*			Point3D_VectorTransform			(const TPoint3D*,const TVector3D*,TPoint3D*);
	TPoint3D*			Point3D_ScaledVectorTransform	(const TPoint3D*,double,const TVector3D*,TPoint3D*);
	TPoint3D*			Point3D_TransformQuaternion		(const TPoint3D*,const TQuaternion*,TPoint3D*);
	TPoint3D*			Point3D_TransformMatrix			(const TPoint3D*,const TXMatrix*,TPoint3D*);
	void				Point3D_WriteToStream			(const TPoint3D*,FILE*);
	
	/* Q U A T E R N I O N  */
	TQuaternion*		Quaternion_Multiply				(const TQuaternion*,const TQuaternion*,TQuaternion*);
	TQuaternion*		Quaternion_Invert				(const TQuaternion*,TQuaternion*);
	void				Quaternion_SetRotateAroundXAxis	(TQuaternion*,double);
	void				Quaternion_SetRotateAroundYAxis	(TQuaternion*,double);
	void				Quaternion_SetRotateAroundZAxis	(TQuaternion*,double);
	void				Quaternion_SetRotateAroundAxis	(TQuaternion*,const TVector3D*,double);
	void				Quaternion_SetRotateVectorToVector(TQuaternion*,const TVector3D*,const TVector3D*);
	void				Quaternion_WriteToStream		(const TQuaternion*,FILE*);
	
	/* X M A T R I X  */
	TXMatrix*			XMatrix_Multiply				(const TXMatrix*,const TXMatrix*,TXMatrix*);
	void				XMatrix_UniformScaleXMatrix		(TXMatrix*,double);
	void				XMatrix_TranslateXMatrix		(TXMatrix*,double,double,double);
	void				XMatrix_SetRotateAroundXAxis	(TXMatrix*,double);
	void				XMatrix_SetRotateAroundYAxis	(TXMatrix*,double);
	void				XMatrix_SetRotateAroundZAxis	(TXMatrix*,double);
	void				XMatrix_WriteToStream			(const TXMatrix*,FILE*);

#if defined(__cplusplus)
}
#endif

#endif
