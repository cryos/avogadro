/* ===============================================================================
// Core3DMath.c                                              ©1999-2001, J.T. Frey
// ===============================================================================
// Written:		J.T. Frey, 03/11/2001
// Purpose:		Defines a specialized library for three-dimensional
//				applications and their specialized features.
//
// Last Mod:	n/a
*/

#include "Core3DMath.h"
#include "CoreConst.h"

// Added for Avogadro:
#include <float.h>

	/* G L O B A L S */
	
	const TPoint3D			gOrigin		= { 0.0 , 0.0 , 0.0 };
	const TVector3D			gXAxis		= { 1.0 , 0.0 , 0.0 };
	const TVector3D			gYAxis		= { 0.0 , 1.0 , 0.0 };
	const TVector3D			gZAxis		= { 0.0 , 0.0 , 1.0 };
	
	const TXMatrix			gXIdentity	= { { 1.0 , 0.0 , 0.0 , 0.0 , \
																				0.0 , 1.0 , 0.0 , 0.0 , \
																				0.0 , 0.0 , 1.0 , 0.0 , \
																				0.0 , 0.0 , 0.0 , 1.0 } };
/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_Zero
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Make the point be the origin (0,0,0) 
	//				
	//
	// Last Mod:	n/a
	*/

	void
	Point3D_Zero(
		TPoint3D*			p
	)
	{
		Point3D_Splat(p,0.0);
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Zero
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Make the vector be a null vector, <0,0,0>
	//				
	//
	// Last Mod:	n/a
	*/

	void
	Vector3D_Zero(
		TVector3D*		v
	)
	{
		Vector3D_Splat(v,0.0);
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_Identity
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Make the quaternion be identity <1,0,0,0>
	//				
	//
	// Last Mod:	n/a
	*/

	void
	Quaternion_Identity(
		TQuaternion*			q
	)
	{
		if (q) {
		  q->w = 1.0;
		  q->x = 0.0;
		  q->y = 0.0;
		  q->z = 0.0;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_Identity
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Make the matrix be identity.
	//				
	//
	// Last Mod:	n/a
	*/

	void
	XMatrix_Identity(
		TXMatrix*			m
	)
	{
		if (m)
		  memcpy(m,&gXIdentity,sizeof(TXMatrix));
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_Splat
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Fill a 3D point with the given value. 
	//				
	//
	// Last Mod:	n/a
	*/

	void
	Point3D_Splat(
		TPoint3D*			p,
		double				s
	)
	{
		if (p) {
		  p->x = s;
		  p->y = s;
		  p->z = s;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Splat
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Fill a 3D vector with the given value. 
	//				
	//
	// Last Mod:	n/a
	*/

	void
	Vector3D_Splat(
		TVector3D*		v,
		double			s
	)
	{
		if (v) {
		  v->x = s;
		  v->y = s;
		  v->z = s;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_Rezero
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Clean up a 3D point by looking for values less than
	//				the specified minimum.
	//				
	//
	// Last Mod:	n/a
	*/

	void
	Point3D_Rezero(
		TPoint3D*			p,
		double				zeroAt
	)
	{
		if (p) {
		  if (fabs(p->x) <= zeroAt) p->x = 0.0;
		  if (fabs(p->y) <= zeroAt) p->y = 0.0;
		  if (fabs(p->z) <= zeroAt) p->z = 0.0;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Rezero
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Clean up a vector by looking for values less than
	//				the specified minimum.
	//				
	//
	// Last Mod:	n/a
	*/

	void
	Vector3D_Rezero(
		TVector3D*		v,
		double			zeroAt
	)
	{
		if (v) {
		  if (fabs(v->x) <= zeroAt) v->x = 0.0;
		  if (fabs(v->y) <= zeroAt) v->y = 0.0;
		  if (fabs(v->z) <= zeroAt) v->z = 0.0;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_Rezero
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Clean up a quaternion by looking for values less than
	//				the specified minimum.
	//				
	//
	// Last Mod:	n/a
	*/

	void
	Quaternion_Rezero(
		TQuaternion*	q,
		double			zeroAt
	)
	{
		if (q) {
		  if (fabs(q->w) <= zeroAt) q->w = 0.0;
		  if (fabs(q->x) <= zeroAt) q->x = 0.0;
		  if (fabs(q->y) <= zeroAt) q->y = 0.0;
		  if (fabs(q->z) <= zeroAt) q->z = 0.0;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_Rezero
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Clean up a transformation matrix by looking for values
	//				less than the specified minimum.
	//				
	//
	// Last Mod:	n/a
	*/

	void
	XMatrix_Rezero(
		TXMatrix*		m,
		double			zeroAt
	)
	{
		long			i;
		double			*mat;
		
		if (m) {
		  mat = m->matrix;
		  for ( i = 0 ; i < 16 ; i++ ) {
		    if (fabs(*mat) <= zeroAt)
			  *mat = 0.0;
			mat++;
		  }
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Dot
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Dot-product of two 3D vectors.
	//
	// Last Mod:	n/a
	*/

	double
	Vector3D_Dot(
		const TVector3D*		v1,
		const TVector3D*		v2
	)
	{
		double		tmp = 0.0;
		if ((v1) && (v2)) {
		  tmp = (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
			if (fabs(tmp) < DBL_EPSILON)
			  tmp = 0.0;
		}
		return tmp;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Cross
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Cross-product of two 3D vectors.
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_Cross(
		const TVector3D*		v1,
		const TVector3D*		v2,
		TVector3D*				r
	)
	{
		TVector3D		tmpV;
		TVector3D*		V = ((r == v1) || (r == v2)) ? &tmpV : r;
		
		if ((r == NULL) || (v1 == NULL) || (v2 == NULL))
		  return NULL;
		
		V->x = ((fabs(V->x = v1->y * v2->z - v1->z * v2->y) >= kZeroFloat)?(V->x):(0.0));
		V->y = ((fabs(V->y = v1->z * v2->x - v1->x * v2->z) >= kZeroFloat)?(V->y):(0.0));
		V->z = ((fabs(V->z = v1->x * v2->y - v1->y * v2->x) >= kZeroFloat)?(V->z):(0.0));
		
		if (V == &tmpV)
		  *r = tmpV;
		
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Magnitude
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Length of a 3D vector.
	//
	// Last Mod:	n/a
	*/

	double
	Vector3D_Magnitude(
		const TVector3D*		v
	)
	{
		if (v)
		  return sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
		return 0.0;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Normalize
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Normalize a 3D vector.
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_Normalize(
		const TVector3D*		v,
		TVector3D*				r
	)
	{
		double		norm;
		
		if ((r == NULL) || (v == NULL))
		  return NULL;
		  
		norm = sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
		
		if (norm >= kZeroFloat) {
		  norm = 1 / norm;
		  r->x = v->x * norm;
		  r->y = v->y * norm;
		  r->z = v->z * norm;
		} else
		  r->x = r->y = r->z = 0.0;
		  
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Negate
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Negate a 3D vector.
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_Negate(
		const TVector3D*		v,
		TVector3D*				r
	)
	{
		if ((r == NULL) || (v == NULL))
		  return NULL;
		
		r->x = -v->x;
		r->y = -v->y;
		r->z = -v->z;
		  
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Scalar
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Scale a 3D vector.
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_Scalar(
		const TVector3D*		v,
		double					s,
		TVector3D*				r
	)
	{
		if ((r == NULL) || (v == NULL))
		  return NULL;
		
		r->x = s * v->x;
		r->y = s * v->y;
		r->z = s * v->z;
		  
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Sum
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Sum two 3D vectors.
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_Sum(
		const TVector3D*		v1,
		const TVector3D*		v2,
		TVector3D*				r
	)
	{
		if ((r == NULL) || (v1 == NULL) || (v2 == NULL))
		  return NULL;
		
		r->x = v1->x + v2->x;
		r->y = v1->y + v2->y;
		r->z = v1->z + v2->z;
		  
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_ScaledSum
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Sum a scaled 3D vector and another vector:
	//                      __     __
	//					r = v1 + s v2
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_ScaledSum(
		const TVector3D*		v1,
		double					s,
		const TVector3D*		v2,
		TVector3D*				r
	)
	{
		if ((r == NULL) || (v1 == NULL) || (v2 == NULL))
		  return NULL;
		
		r->x = v1->x + s * v2->x;
		r->y = v1->y + s * v2->y;
		r->z = v1->z + s * v2->z;
		  
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_Diff
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Subtract one 3D vector from another 3D vector.
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_Diff(
		const TVector3D*		v1,
		const TVector3D*		v2,
		TVector3D*				r
	)
	{
		if ((r == NULL) || (v1 == NULL) || (v2 == NULL))
		  return NULL;
		
		r->x = v1->x - v2->x;
		r->y = v1->y - v2->y;
		r->z = v1->z - v2->z;
		  
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_ScaledDiff
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Subtract a scaled 3D vector from another vector:
	//                      __     __
	//					r = v1 + s v2
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_ScaledDiff(
		const TVector3D*		v1,
		double					s,
		const TVector3D*		v2,
		TVector3D*				r
	)
	{
		if ((r == NULL) || (v1 == NULL) || (v2 == NULL))
		  return NULL;
		  
		r->x = v1->x - s * v2->x;
		r->y = v1->y - s * v2->y;
		r->z = v1->z - s * v2->z;
		
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_TransformQuaternion
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Transform a vector using a unit quaternion.
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_TransformQuaternion(
		const TVector3D*	v,
		const TQuaternion*	q,
		TVector3D*			r
	)
	{
		TQuaternion 		temp;
		
		if ((r == NULL) || (v == NULL) || (q == NULL))
		  return NULL;
		  
		/* Multiply qinv * v  */
		temp.w = q->x * v->x + q->y * v->y + q->z * v->z;
		temp.x = q->w * v->x + (q->y * v->z - q->z * v->y);
		temp.y = q->w * v->y - (q->x * v->z - q->z * v->x);
		temp.z = q->w * v->z + (q->x * v->y - q->y * v->x);
	  
		/* Multiply temp * q  and poke into the return spot in the process:  */
		r->x = temp.w * q->x + temp.x * q->w - (temp.y * q->z - temp.z * q->y);
		r->y = temp.w * q->y + temp.y * q->w + (temp.x * q->z - temp.z * q->x);
		r->z = temp.w * q->z + temp.z * q->w - (temp.x * q->y - temp.y * q->x);
		
		return r;
	}

/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_TransformMatrix
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Transform a vector using a transformation matrix.
	//
	// Last Mod:	n/a
	*/

	TVector3D*
	Vector3D_TransformMatrix(
		const TVector3D*	v,
		const TXMatrix*		m,
		TVector3D*			r
	)
	{
		#define				X		v->x
		#define				Y		v->y
		#define				Z		v->z
		#define				M(I,J)	m->matrix[J + I * 4]
		
		TVector3D	 		tmpV;
		TVector3D*			V = (r == v) ? &tmpV : r;
		
		if ((r == NULL) || (v == NULL) || (m == NULL))
		  return NULL;
		  
		/*  Row vector (point) times matrix:  */
		V->x = M(0,0)*X + M(1,0)*Y + M(2,0)*Z + M(3,0);
		V->y = M(0,1)*X + M(1,1)*Y + M(2,1)*Z + M(3,1);
		V->z = M(0,2)*X + M(1,2)*Y + M(2,2)*Z + M(3,2);
		
		if (V == &tmpV)
		  *r = tmpV;
		
		return r;
		
		#undef X
		#undef Y
		#undef Z
		#undef M
	}

/**/

	/* ----------------------------------------------------------------------
	//		* Vector3D_WriteToStream
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Print the vector.
	//
	// Last Mod:	n/a
	*/

	void
	Vector3D_WriteToStream(
		const TVector3D*	v,
		FILE*				stream
	)
	{
		if (v)
		  fprintf(stream,"< %lg %lg %lg >",v->x,v->y,v->z);
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_Distance
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Distance between two points (thanx to Pythagoras).
	//
	// Last Mod:	n/a
	*/

	double
	Point3D_Distance(
		const TPoint3D*		p1,
		const TPoint3D*		p2
	)
	{
		double		dx,dy,dz;
		
		if ((p1 == NULL) || (p2 == NULL))
		  return 0.0;
		  
		dx = p1->x - p2->x; dy = p1->y - p2->y; dz = p1->z - p2->z;
		return sqrt(dx * dx + dy * dy + dz * dz);
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_VectorTransform
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Translate a point in space using a vector.
	//
	// Last Mod:	n/a
	*/

	TPoint3D*
	Point3D_VectorTransform(
		const TPoint3D*		p,
		const TVector3D*	v,
		TPoint3D*			r
	)
	{
		if ((r == NULL) || (p == NULL) || (v == NULL))
		  return NULL;
		  
		r->x = p->x + v->x;
		r->y = p->y + v->y;
		r->z = p->z + v->z;
		
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_ScaledVectorTransform
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Translate a point in space using a vector which is
	//				scaled by some factor 's'.
	//
	// Last Mod:	n/a
	*/

	TPoint3D*
	Point3D_ScaledVectorTransform(
		const TPoint3D*		p,
		double				s,
		const TVector3D*	v,
		TPoint3D*			r
	)
	{
		if ((r == NULL) || (v == NULL) || (p == NULL))
		  return NULL;
		  
		r->x = p->x + s * v->x;
		r->y = p->y + s * v->y;
		r->z = p->z + s * v->z;
		
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_TransformQuaternion
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Transform a point using a unit quaternion.
	//
	// Last Mod:	n/a
	*/

	TPoint3D*
	Point3D_TransformQuaternion(
		const TPoint3D*		p,
		const TQuaternion*	q,
		TPoint3D*			r
	)
	{
		TQuaternion 		temp;
		
		if ((r == NULL) || (p == NULL) || (q == NULL))
		  return NULL;
		  
		/* Multiply qinv * p  */
		temp.w = q->x * p->x + q->y * p->y + q->z * p->z;
		temp.x = q->w * p->x + (q->y * p->z - q->z * p->y);
		temp.y = q->w * p->y - (q->x * p->z - q->z * p->x);
		temp.z = q->w * p->z + (q->x * p->y - q->y * p->x);
		
		/* Multiply temp * q  and poke into the return spot in the process:  */
		r->x = temp.w * q->x + temp.x * q->w - (temp.y * q->z - temp.z * q->y);
		r->y = temp.w * q->y + temp.y * q->w + (temp.x * q->z - temp.z * q->x);
		r->z = temp.w * q->z + temp.z * q->w - (temp.x * q->y - temp.y * q->x);
		
		return r;
	}

/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_TransformMatrix
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Transform a point using a transformation matrix.
	//
	// Last Mod:	n/a
	*/

	TPoint3D*
	Point3D_TransformMatrix(
		const TPoint3D*		p,
		const TXMatrix*		m,
		TPoint3D*			r
	)
	{
		#define				X		p->x
		#define				Y		p->y
		#define				Z		p->z
		#define				M(I,J)	m->matrix[J + I * 4]
		
		TPoint3D	 		tmpP;
		TPoint3D*			P = (r == p) ? &tmpP : r;
		
		if ((r == NULL) || (p == NULL) || (m == NULL))
		  return NULL;
		  
		/*  Row vector (point) times matrix:  */
		P->x = M(0,0)*X + M(1,0)*Y + M(2,0)*Z + M(3,0);
		P->y = M(0,1)*X + M(1,1)*Y + M(2,1)*Z + M(3,1);
		P->z = M(0,2)*X + M(1,2)*Y + M(2,2)*Z + M(3,2);
		
		if (P == &tmpP)
		  *r = tmpP;
		
		return r;
		
		#undef X
		#undef Y
		#undef Z
		#undef M
	}

/**/

	/* ----------------------------------------------------------------------
	//		* Point3D_WriteToStream
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Print the point.
	//
	// Last Mod:	n/a
	*/

	void
	Point3D_WriteToStream(
		const TPoint3D*		p,
		FILE*				stream
	)
	{
		if (p)
		  fprintf(stream,"( %lg %lg %lg )",p->x,p->y,p->z);
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_Multiply
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Reverse-order multiplication of quaternions.
	//
	// Last Mod:	n/a
	*/

	TQuaternion*
	Quaternion_Multiply(
		const TQuaternion*	q1,
		const TQuaternion*	q2,
		TQuaternion*		r
	)
	{
		TQuaternion			tmpQ;
		TQuaternion*		Q = ((r == q1) || (r == q2)) ? &tmpQ : r;
		
		if ((r == NULL) || (q1 == NULL) || (q2 == NULL))
		  return NULL;
		
		Q->w = q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z;
		Q->x = q1->w*q2->x + q1->x*q2->w - q1->y*q2->z + q1->z*q2->y;
		Q->y = q1->w*q2->y + q1->y*q2->w - q1->z*q2->x + q1->x*q2->z;
		Q->z = q1->w*q2->z + q1->z*q2->w - q1->x*q2->y + q1->y*q2->x;
		
		if (Q == &tmpQ)
		  *r = tmpQ;
		
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_Invert
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Invert a quaternion.
	//
	// Last Mod:	n/a
	*/

	TQuaternion*
	Quaternion_Invert(
		const TQuaternion*	q,
		TQuaternion*		r
	)
	{
		if ((r == NULL) || (q == NULL))
		  return NULL;
		  
		r->w =  q->w;
		r->x = -q->x;
		r->y = -q->y;
		r->z = -q->z;
		
		return r;
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_SetRotateAroundXAxis
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Set the fields of a quaternion such that it represents a
	//				rotation around the x-axis.
	//
	// Last Mod:	n/a
	*/

	void
	Quaternion_SetRotateAroundXAxis(
		TQuaternion*		q,
		double				theta
	)
	{
		double		phi = 0.5 * theta;
		
		if (q) {
		  q->w = cos(phi);
		  q->x = sin(phi);
		  q->y = 0.0;
		  q->z = 0.0;
          
          if (fabs(q->w) < DBL_EPSILON) q->w = 0.0;
          if (fabs(q->x) < DBL_EPSILON) q->x = 0.0;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_SetRotateAroundYAxis
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Set the fields of a quaternion such that it represents a
	//				rotation around the y-axis.
	//
	// Last Mod:	n/a
	*/

	void
	Quaternion_SetRotateAroundYAxis(
		TQuaternion*		q,
		double				theta
	)
	{
		double		phi = 0.5 * theta;
		
		if (q) {
		  q->w = cos(phi);
		  q->x = 0.0;
		  q->y = sin(phi);
		  q->z = 0.0;
          
          if (fabs(q->w) < DBL_EPSILON) q->w = 0.0;
          if (fabs(q->y) < DBL_EPSILON) q->y = 0.0;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_SetRotateAroundZAxis
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Set the fields of a quaternion such that it represents a
	//				rotation around the z-axis.
	//
	// Last Mod:	n/a
	*/

	void
	Quaternion_SetRotateAroundZAxis(
		TQuaternion*		q,
		double				theta
	)
	{
		double		phi = 0.5 * theta;
		
		if (q) {
		  q->w = cos(phi);
		  q->x = 0.0;
		  q->y = 0.0;
		  q->z = sin(phi);
          
          if (fabs(q->w) < DBL_EPSILON) q->w = 0.0;
          if (fabs(q->z) < DBL_EPSILON) q->z = 0.0;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_SetRotateAroundAxis
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Set the fields of a quaternion such that it represents a
	//				rotation around an arbitrary axis.
	//
	// Last Mod:	n/a
	*/

	void
	Quaternion_SetRotateAroundAxis(
		TQuaternion*		q,
		const TVector3D*	v,
		double				theta
	)
	{
		double		phi = 0.5 * theta;
		double		sinePhi = sin(phi);
		double		cosinePhi = cos(phi);
		double		norm = sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
		
        if (fabs(cosinePhi) < DBL_EPSILON) cosinePhi = 0.0;
        if (fabs(sinePhi) < DBL_EPSILON) sinePhi = 0.0;
        
		if ((q) && (v)) {
		  q->w = cosinePhi;
		  q->x = v->x / norm * sinePhi;
		  q->y = v->y / norm * sinePhi;
		  q->z = v->z / norm * sinePhi;
		}
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_SetRotateVectorToVector
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Set the fields of a quaternion such that it represents a
	//				rotation of one vector into another.
	//
	// Last Mod:	n/a
	*/

	void
	Quaternion_SetRotateVectorToVector(
		TQuaternion*		q,
		const TVector3D*	vFrom,
		const TVector3D*	vTo
	)
	{
		#define				EPSILON				0.00001
		
		TVector3D			normTov1v2,v1Normal,v2Normal;
		double				cosine_sqrd,len;
		
		if ((q == NULL) || (vFrom == NULL) || (vTo == NULL))
		  return;
		
		/* Normalize the input parameters - this is not required by the spec,
		   but the QD3D implementation accepts unnormalised inputs.
		   [jtf] For greatest speed, I'll normalize on my own.
		*/
		len = 1 / sqrt(vFrom->x * vFrom->x + vFrom->y * vFrom->y + vFrom->z * vFrom->z);
		v1Normal.x = vFrom->x * len;
		v1Normal.y = vFrom->y * len;
		v1Normal.z = vFrom->z * len;
		
		len = 1 / sqrt(vTo->x * vTo->x + vTo->y * vTo->y + vTo->z * vTo->z);
		v2Normal.x = vTo->x * len;
		v2Normal.y = vTo->y * len;
		v2Normal.z = vTo->z * len;

		/* Check for vFrom, vTo equality */
		if ((fabs(v1Normal.x - v2Normal.x) < EPSILON) && \
			(fabs(v1Normal.y - v2Normal.y) < EPSILON) && \
			(fabs(v1Normal.z - v2Normal.z) < EPSILON))
		{
			q->w = 1;
			q->x = q->y = q->z = 0;
			return;
		}

		/* Check for vectors in the same line but opposite direction
		   the cross would be 0,0,0 what is not what we want
		*/
		if ((fabs(v1Normal.x + v2Normal.x) < EPSILON) && \
			(fabs(v1Normal.y + v2Normal.y) < EPSILON) && \
			(fabs(v1Normal.z + v2Normal.z) < EPSILON))
		{
			/* this vector is orthogonal to both of the vectors */
			normTov1v2.x = -v1Normal.y;
			normTov1v2.y = -v1Normal.z;
			normTov1v2.z =  v1Normal.x;
			len = 1;
		}
		else {
			normTov1v2.x = v1Normal.y * v2Normal.z - v2Normal.y * v1Normal.z;
			normTov1v2.y = v1Normal.z * v2Normal.x - v2Normal.z * v1Normal.x;
			normTov1v2.z = v1Normal.x * v2Normal.y - v2Normal.x * v1Normal.y;
			len = sqrt(normTov1v2.x * normTov1v2.x + normTov1v2.y * normTov1v2.y + normTov1v2.z * normTov1v2.z);
		}
		
		
		/* A unit quaternion exists as a scalar representing rotation about an
		   arbitrary axis -- w = cos (theta / 2) -- and the 3-vector describing
		   the axis.
		  
		   w = cos ( theta / 2 ) = sqrt( (v1.v2 + 1)/2 )
		   
		   Note:  J.B. initially raised the question of whether or not the eqn.
		   		  should actually be:
		  
		   			w = cos ( theta / 2 ) = ± sqrt( (v1.v2 + 1)/2 )
		   
		   		  It's a good point, but the answer is this:  it depends upon the
		   		  range of theta.  Since the dot product only generates values in
		   		  the range of [-1,1], and the inverse cosine has a domain of 
		   		  [0,Pi] for this range, we can assume that our theta ranges from
		   		  [0,Pi].  As such, the largest half-angle that would appear for
		   		  w is (Pi/2), so the true range of our cos(theta/2) term is [0,Pi/2].
		   		  In this range, all cosines are positive, so we can stick with the
		   		  first definition of w.  The handedness of the rotation is handled
		   		  by the orientation of the rotational axis.  Rotation about the
		   		  +y-axis by theta is NOT the same as rotation about the -y-axis by
		   		  theta.
		*/
		cosine_sqrd = (v1Normal.x * v2Normal.x + v1Normal.y * v2Normal.y + v1Normal.z * v2Normal.z + 1) / 2;
		
		/* The axis about which we rotate is restricted to be of length
		   sin (theta / 2); luckily, via trig identity:
		
		   sin (theta / 2) = sqrt( 1 - (cos (theta / 2))^2 )
		   sin (theta / 2) = sqrt( 1 - cosine_sqrd )
		
		   At the same time we must first normalize the normal vector; so we
		   simply compute the entire scalar which the normal vector would be
		   multiplied by to get (1) normalized and (2) length sin(theta/2):
		
		   scalar factor = sin(theta/2) / length of normal vector
		*/
		len = sqrt( 1 - cosine_sqrd ) / len;
		
		/* Set the quaternion; we want the square root of cosine_sqrd and each
		   term of the normal vector must be multiplied by the scalar factor
		   which was just computed:
		*/
		q->w = sqrt(cosine_sqrd);
		q->x = normTov1v2.x * len;
		q->y = normTov1v2.y * len;
		q->z = normTov1v2.z * len;
        
        Quaternion_Rezero(q,DBL_EPSILON);
		
		#undef EPSILON
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* Quaternion_WriteToStream
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Print the quaternion.
	//
	// Last Mod:	n/a
	*/

	void
	Quaternion_WriteToStream(
		const TQuaternion*	q,
		FILE*				stream
	)
	{
		if (q)
		  fprintf(stream,"< %lg %lg %lg %lg >",q->w,q->x,q->y,q->z);
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_Multiply
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Concatenates (multiplies) one transformation matrix by
	//				another.
	//
	// Last Mod:	n/a
	*/

	TXMatrix*
	XMatrix_Multiply(
		const TXMatrix*		m1,
		const TXMatrix*		m2,
		TXMatrix*			r
	)
	{
		#define		M1(I,J)		m1->matrix[J + I * 4]
		#define		M2(I,J)		m2->matrix[J + I * 4]
		#define		R(I,J)		M->matrix[J + I * 4]
		
		TXMatrix			tmpM;
		TXMatrix*			M = ((r == m1) || (r == m2)) ? &tmpM : r;
		
		if ((r == NULL) || (m1 == NULL) || (m2 == NULL))
		  return r;
		
		R(0,0) = M1(0,0)*M2(0,0) + M1(0,1)*M2(1,0) + M1(0,2)*M2(2,0) + M1(0,3)*M2(3,0);
		R(0,1) = M1(0,0)*M2(0,1) + M1(0,1)*M2(1,1) + M1(0,2)*M2(2,1) + M1(0,3)*M2(3,1);
		R(0,2) = M1(0,0)*M2(0,2) + M1(0,1)*M2(1,2) + M1(0,2)*M2(2,2) + M1(0,3)*M2(3,2);
		R(0,3) = M1(0,0)*M2(0,3) + M1(0,1)*M2(1,3) + M1(0,2)*M2(2,3) + M1(0,3)*M2(3,3);
		
		R(1,0) = M1(1,0)*M2(0,0) + M1(1,1)*M2(1,0) + M1(1,2)*M2(2,0) + M1(1,3)*M2(3,0);
		R(1,1) = M1(1,0)*M2(0,1) + M1(1,1)*M2(1,1) + M1(1,2)*M2(2,1) + M1(1,3)*M2(3,1);
		R(1,2) = M1(1,0)*M2(0,2) + M1(1,1)*M2(1,2) + M1(1,2)*M2(2,2) + M1(1,3)*M2(3,2);
		R(1,3) = M1(1,0)*M2(0,3) + M1(1,1)*M2(1,3) + M1(1,2)*M2(2,3) + M1(1,3)*M2(3,3);
		
		R(2,0) = M1(2,0)*M2(0,0) + M1(2,1)*M2(1,0) + M1(2,2)*M2(2,0) + M1(2,3)*M2(3,0);
		R(2,1) = M1(2,0)*M2(0,1) + M1(2,1)*M2(1,1) + M1(2,2)*M2(2,1) + M1(2,3)*M2(3,1);
		R(2,2) = M1(2,0)*M2(0,2) + M1(2,1)*M2(1,2) + M1(2,2)*M2(2,2) + M1(2,3)*M2(3,2);
		R(2,3) = M1(2,0)*M2(0,3) + M1(2,1)*M2(1,3) + M1(2,2)*M2(2,3) + M1(2,3)*M2(3,3);
		
		R(3,0) = M1(3,0)*M2(0,0) + M1(3,1)*M2(1,0) + M1(3,2)*M2(2,0) + M1(3,3)*M2(3,0);
		R(3,1) = M1(3,0)*M2(0,1) + M1(3,1)*M2(1,1) + M1(3,2)*M2(2,1) + M1(3,3)*M2(3,1);
		R(3,2) = M1(3,0)*M2(0,2) + M1(3,1)*M2(1,2) + M1(3,2)*M2(2,2) + M1(3,3)*M2(3,2);
		R(3,3) = M1(3,0)*M2(0,3) + M1(3,1)*M2(1,3) + M1(3,2)*M2(2,3) + M1(3,3)*M2(3,3);
		
		if (M == &tmpM)
		  *r = tmpM;
		
		return r;
		
		#undef M1
		#undef M2
		#undef R
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_UniformScaleXMatrix
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Scales a tranformation matrix without resetting the
	//				factors that are already there.
	//
	// Last Mod:	n/a
	*/

	void
	XMatrix_UniformScaleXMatrix(
		TXMatrix*			m,
		double				s
	)
	{
		#define M(I,J)  m->matrix[J + I * 4]
		
		if (m) {
		  M(0,0) *= s;
		  M(1,1) *= s;
		  M(2,2) *= s;
		}
		
		#undef M
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_TranslateXMatrix
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Modifies the translation factors in a transformation
	//				matrix.
	//
	// Last Mod:	n/a
	*/

	void
	XMatrix_TranslateXMatrix(
		TXMatrix*			m,
		double				dx,
		double				dy,
		double				dz
	)
	{
		#define M(I,J)  m->matrix[J + I * 4]
		
		if (m) {
		  M(3,0) = dx;
		  M(3,1) = dy;
		  M(3,2) = dz;
		}
		
		#undef M
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_SetRotateAroundXAxis
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Set a rotation matrix to represent rotation about the
	//				x-axis.
	//
	// Last Mod:	n/a
	*/

	void
	XMatrix_SetRotateAroundXAxis(
		TXMatrix*			m,
		double				theta
	)
	{
		#define M(I,J)  m->matrix[J + I * 4]
		
		double		cosTheta = cos(theta);
		double		sinTheta = sin(theta);
		
        if (fabs(cosTheta) < DBL_EPSILON) cosTheta = 0.0;
        if (fabs(sinTheta) < DBL_EPSILON) sinTheta = 0.0;
		if (m) {
		  memset(m,0,sizeof(TXMatrix));
		  
		  M(0,0) = 1.0;
		  
		  M(1,1) = cosTheta;
		  M(1,2) = sinTheta;
		  
		  M(2,1) = -sinTheta;
		  M(2,2) = cosTheta;
		  
		  M(3,3) = 1.0;
		}
		
		#undef M
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_SetRotateAroundYAxis
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Set a rotation matrix to represent rotation about the
	//				y-axis.
	//
	// Last Mod:	n/a
	*/

	void
	XMatrix_SetRotateAroundYAxis(
		TXMatrix*			m,
		double				theta
	)
	{
		#define M(I,J)  m->matrix[J + I * 4]
		
		double		cosTheta = cos(theta);
		double		sinTheta = sin(theta);
		
        if (fabs(cosTheta) < DBL_EPSILON) cosTheta = 0.0;
        if (fabs(sinTheta) < DBL_EPSILON) sinTheta = 0.0;
		if (m) {
		  memset(m,0,sizeof(TXMatrix));
		  
		  M(0,0) = cosTheta;
		  M(0,2) = sinTheta;
		  
		  M(1,1) = 1.0;
		  
		  M(2,0) = -sinTheta;
		  M(2,2) = cosTheta;
		  
		  M(3,3) = 1.0;
		}
		
		#undef M
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_SetRotateAroundZAxis
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Set a rotation matrix to represent rotation about the
	//				z-axis.
	//
	// Last Mod:	n/a
	*/

	void
	XMatrix_SetRotateAroundZAxis(
		TXMatrix*			m,
		double				theta
	)
	{
		#define M(I,J)  m->matrix[J + I * 4]
		
		double		cosTheta = cos(theta);
		double		sinTheta = sin(theta);
		
        if (fabs(cosTheta) < DBL_EPSILON) cosTheta = 0.0;
        if (fabs(sinTheta) < DBL_EPSILON) sinTheta = 0.0;
		if (m) {
		  memset(m,0,sizeof(TXMatrix));
		  
		  M(0,0) = cosTheta;
		  M(0,1) = sinTheta;
		  
		  M(1,0) = -sinTheta;
		  M(1,1) = cosTheta;
		  
		  M(2,2) = 1.0;
		  
		  M(3,3) = 1.0;
		}
		
		#undef M
	}
	
/**/

	/* ----------------------------------------------------------------------
	//		* XMatrix_WriteToStream
	// ----------------------------------------------------------------------
	// Updated:		Jeff Frey, 12/19/2001
	// Purpose:		Print the transformation matrix.
	//
	// Last Mod:	n/a
	*/

	void
	XMatrix_WriteToStream(
		const TXMatrix*		m,
		FILE*				stream
	)
	{
		long			i,j;
		double*			mat;
		
		if (m) {
		  mat = (double*)(m->matrix);
		  
		  for ( i = 0 ; i < 4 ; i++ ) {
		    if (i == 0)
			  fprintf(stream,"[[ ");
			else
			  fprintf(stream," [ ");
		    for ( j = 0 ; j < 4 ; j++ , mat++ )
			  fprintf(stream,"%lg ",*mat);
			if (i == 3)
			  fprintf(stream,"]]");
			else
			  fprintf(stream,"]\n");
		  }
		}
	}
	
/**/
