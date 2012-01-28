//==========================================================
//
// Cell.cpp
//
// A component of the TubeGen program
//
// Generalized lattice cell.
//
// Created:		26.JAN.2002
// Last Mod:	14.FEB.2002: Addition of coordinate conversions
//

#include "Cell.h"
#include "CoreConst.h"

// Added for Avogadro:
#include <iostream>
#include <iomanip>
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#include <math.h>
#endif

//

	// =======================================================
	// * Default constructor
	Cell::Cell()
	{
		refs											= 1;  	//  so far, only one reference count
		
		a = b = c									= 4.0;	//  arbitrary 4 angstrom cell dimension
		alpha = beta = gamma			= 90.0;	//  make the cell cubic in nature
		
		this->GenerateCellVectors();
	}
	
//

	// =======================================================
	// * Secondary constructor
	Cell::Cell(
		double		in_a,
		double		in_b,
		double		in_c,
		double		in_alpha,
		double		in_beta,
		double		in_gamma
	)
	{
		refs		= 1;  	//  so far, only one reference count
		
		a 			= (in_a > 0.0)?(in_a):(4.0);
		b 			= (in_b > 0.0)?(in_b):(4.0);
		c 			= (in_c > 0.0)?(in_c):(4.0);
		
		alpha		= ((in_alpha > 0.0) && (in_alpha < 180.0))?(in_alpha):(90.0);
		beta		= ((in_beta > 0.0) && (in_beta < 180.0))?(in_beta):(90.0);
		gamma		= ((in_gamma > 0.0) && (in_gamma < 180.0))?(in_gamma):(90.0);
		
		this->GenerateCellVectors();
	}
	
//

	// =======================================================
	// * Makes a reference-copy of object
	Cell*
	Cell::Retain()
	{
		refs++;
		return this;
	}
	
//

	// =======================================================
	// * Release a reference to the object; when references
	//   drop to zero, we can dispose of the object.
	void
	Cell::Release()
	{
		if (--refs == 0)
		  delete this;
	}
	
//

	// =======================================================
	// * Set the length of the "a" axis
	void
	Cell::SetDimensionA(
		double		dim
	)
	{
		if (dim > 0.0) {
		  a = dim;
			this->GenerateCellVectors();
		}
	}
	
//

	// =======================================================
	// * Set the length of the "b" axis
	void
	Cell::SetDimensionB(
		double		dim
	)
	{
		if (dim > 0.0) {
		  b = dim;
			this->GenerateCellVectors();
		}
	}
	
//

	// =======================================================
	// * Set the length of the "c" axis
	void
	Cell::SetDimensionC(
		double		dim
	)
	{
		if (dim > 0.0) {
		  c = dim;
			this->GenerateCellVectors();
		}
	}
	
//

	// =======================================================
	// * Set the angle made by axes "b" and "c"
	void
	Cell::SetAngleAlpha(
		double		ang
	)
	{
		if ((ang > 0.0) && (ang < 180.0)) {
		  alpha = ang;
			this->GenerateCellVectors();
		}
	}
	
//

	// =======================================================
	// * Set the angle made by axes "a" and "c"
	void
	Cell::SetAngleBeta(
		double		ang
	)
	{
		if ((ang > 0.0) && (ang < 180.0)) {
		  beta = ang;
			this->GenerateCellVectors();
		}
	}
	
//

	// =======================================================
	// * Set the angle made by axes "a" and "b"
	void
	Cell::SetAngleGamma(
		double		ang
	)
	{
		if ((ang > 0.0) && (ang < 180.0)) {
		  gamma = ang;
			this->GenerateCellVectors();
		}
	}
	
//

	// =======================================================
	// * Return the vector which gives the translational
	//   symmetry of the cell in Cartesian space.
	TVector3D
	Cell::GetCellTranslationVector()
	{
		TVector3D v = { av[0].x + av[1].x + av[2].x ,
										av[0].y + av[1].y + av[2].y ,
										av[0].z + av[1].z + av[2].z };
		return v;
	}

//

	// =======================================================
	// * Fill the passed tensor reference with the cell's
	//   own metric tensor.
	void
	Cell::GetMetricTensorForCell(
		MetricTensor& 		tensor
	)
	{
		tensor = G;
	}

//

	// =======================================================
	// * Convert a cartesian coordinate to it's fractional
	//   counterpart within this cell.
	TPoint3D
	Cell::CartesianToFractional(
		TPoint3D		p
	)
	{
		TPoint3D		p_f = { 0.0 , 0.0 , 0.0 };
		
		p_f.x = Vector3D_Dot(&p,&bv[0]);
		p_f.y = Vector3D_Dot(&p,&bv[1]);
		p_f.z = Vector3D_Dot(&p,&bv[2]);
		
		Point3D_Rezero(&p_f,1e-10);
		
		return p_f;
	}
	
//

	// =======================================================
	// * Convert a fractional coordinate within this cell
	//   to it's cartesian counterpart.
	TPoint3D
	Cell::FractionalToCartesian(
		TPoint3D		p
	)
	{
		TPoint3D		p_f = { 0.0 , 0.0 , 0.0 };
		
		Vector3D_Scalar(&av[0],p.x,&p_f);
		Vector3D_ScaledSum(&p_f,p.y,&av[1],&p_f);
		Vector3D_ScaledSum(&p_f,p.z,&av[2],&p_f);
		
		Point3D_Rezero(&p_f,1e-10);
		
		return p_f;
	}
	
//

	// =======================================================
	// * Calculate the Cartesian-space distance between two
	//   fractional points
	double
	Cell::CartesianDistanceBetweenFractionalPoints(
		TPoint3D 		p1,
		TPoint3D 		p2
	)
	{
		TVector3D		v;
		
		Vector3D_Diff(&p1,&p2,&v);
		return sqrt((v.x * G.t[0] + v.y * G.t[1] + v.z * G.t[3]) * v.x + \
								(v.x * G.t[1] + v.y * G.t[2] + v.z * G.t[4]) * v.y + \
								(v.x * G.t[3] + v.y * G.t[4] + v.z * G.t[5]) * v.z);
	}
	
//

	// =======================================================
	// * Display the cell parameters in human-readable form
	void
	Cell::print(
		ostream&		os
	)
	{
    unsigned		i;
    ios_base::fmtflags  savedFlags = os.flags();
    
    os.setf(ios::fixed);
    os << "Cell { " << endl;
    
    os << "             (a,b,c) =   ";
    
    os << setprecision(6) << setw(12) << a << " , ";
    os << setprecision(6) << setw(12) << b << " , ";
    os << setprecision(6) << setw(12) << c << endl;
    
    os << "  (alpha,beta,gamma) =   ";
    os << setprecision(6) << setw(12) << alpha << " , ";
    os << setprecision(6) << setw(12) << beta << " , ";
    os << setprecision(6) << setw(12) << gamma << endl;
    
    for ( i = 0 ; i < 3 ; i++ ) {
      os << "                  a" << i+1 << " = < ";
      os << setprecision(6) << setw(12) << av[i].x << " , ";
      os << setprecision(6) << setw(12) << av[i].y << " , ";
      os << setprecision(6) << setw(12) << av[i].z << " >" << endl;
    }
    
    os << "        volume, real =   " << setprecision(6) << setw(12) << volume << endl;
    
    for ( i = 0 ; i < 3 ; i++ ) {
      os << "                  b" << i+1 << " = < ";
      os << setprecision(6) << setw(12) << bv[i].x << " , ";
      os << setprecision(6) << setw(12) << bv[i].y << " , ";
      os << setprecision(6) << setw(12) << bv[i].z << " >" << endl;
    }

    os << "  volume, reciprocal =   " << setprecision(6) << setw(12) << 1.0 / volume << endl;
    
    os << "                       [ " << setprecision(6) << setw(12) << G.t[0];
    os << "                               ]" << endl;
    os << "   metric tensor (g) = [ " << setprecision(6) << setw(12) << G.t[1] << "   ";
    os << setprecision(6) << setw(12) << G.t[2] << "                ]" << endl;
    os << "                       [ " << setprecision(6) << setw(12) << G.t[3] << "   ";
    os << setprecision(6) << setw(12) << G.t[4] << "   ";
    os << setprecision(6) << setw(12) << G.t[5] << " ]" << endl;
    
    os << "}" << endl;
    
    os.setf(savedFlags);
	}
	
//
// PRIVATE methods
//

	// =======================================================
	// * From the cell dimensions and angles, generate the
	//   three basis vectors for it
	void
	Cell::GenerateCellVectors()
	{
		//  The a axis is easy; project the "a" dimension
		//  along the x-axis:
		av[0].x = a; av[0].y = 0.0; av[0].z = 0.0;
		
		//  The b axis is fairly easy, too; we keep it in
		//  the xy-plane and simply rotate through gamma
		//  degrees:
		double		gammaRad = gamma * kDegreesToRadians;
		double		cosGamma = cos(gammaRad);
		double		sinGamma = sin(gammaRad);
		av[1].x = b * cosGamma; av[1].y = b * sinGamma; av[1].z = 0.0;
		
		//  Check for underflows:
		Vector3D_Rezero(&av[1],1e-10);
		
		//  The c axis is the toughy.  We need to rotate
		//  relative to the two previously-defined vectors:
		double		alphaRad = alpha * kDegreesToRadians;
		double		betaRad = beta * kDegreesToRadians;
		double		cosAlpha = cos(alphaRad);
		double		cosBeta = cos(betaRad);
		double		sinBeta = sin(betaRad);
		double		sinPhi;
		//  (1) Rotate away from a1 (on the x-axis) by beta:
		av[2].x = c * cosBeta;
		av[2].y = 0.0;
		av[2].z = c * sinBeta;
		//  (2) Do a constrained rotation around the x-axis such that
		//      the angle between a3 and a2 becomes alpha:
		//
		//        c = [[|c| cos(beta)][0][|c| sin(beta)]]
		//				a3 = [[1 0 0][0 cos(phi) -sin(phi)][0 sin(phi) cos(phi)]].c = Phi.c
		//        a2(t).Phi.c = |b||c| cos(alpha)
		//
		//      Given the construction of a1 and a2 thus far, the equation
		//      is soluble and we find the angle phi and can perform the
		//      rotation in question.
		sinPhi = (cosGamma * cosBeta - cosAlpha) / (sinGamma * sinBeta);
		av[2].y = av[2].z * -sinPhi;
		av[2].z = av[2].z * sqrt( 1.0 - sinPhi * sinPhi );
		
		//printf(" Consistency Check:  acos ( a.b ) = %lf\n",kRadiansToDegrees * acos( Vector3D_Dot(&av[0],&av[1]) / (a * b) ));
		//printf("                     acos ( a.c ) = %lf\n",kRadiansToDegrees * acos( Vector3D_Dot(&av[0],&av[2]) / (a * c) ));
		//printf("                     acos ( b.c ) = %lf\n",kRadiansToDegrees * acos( Vector3D_Dot(&av[1],&av[2]) / (b * c) ));
		
		//  Check for underflows:
		Vector3D_Rezero(&av[2],1e-10);
		
		//  That gives us the real-space basis; construct the reciprocal
		//  space basis:
		Vector3D_Cross(&av[1],&av[2],&bv[0]);
		Vector3D_Cross(&av[2],&av[0],&bv[1]);
		Vector3D_Cross(&av[0],&av[1],&bv[2]);
		//  Each must be divided by the volume, too:
		volume = Vector3D_Dot(&av[0],&bv[0]);
		
		double			recipVol = 1.0 / volume;
		
		Vector3D_Scalar(&bv[0],recipVol,&bv[0]);
		Vector3D_Scalar(&bv[1],recipVol,&bv[1]);
		Vector3D_Scalar(&bv[2],recipVol,&bv[2]);
		
		//  Check for underflows:
		Vector3D_Rezero(&bv[0],1e-10);
		Vector3D_Rezero(&bv[1],1e-10);
		Vector3D_Rezero(&bv[2],1e-10);
		
		//  Create our metric:
		int		idx = 0;
		for ( int i = 0 ; i < 3 ; i++ )
		  for ( int j = 0 ; j <= i ; j++ )
		    G.t[idx++] = Vector3D_Dot(&av[i],&av[j]);
	}
	
//

