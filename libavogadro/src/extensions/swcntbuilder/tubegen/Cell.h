//==========================================================
//
// Cell.h
//
// A component of the TubeGen program
//
// Generalized lattice cell.
//
// Created:		26.JAN.2002
// Last Mod:	14.FEB.2002:  Addition of coordinate conversions
//						05.Feb.2003:	Removed all ostream::form calls and moved to purely
//													C++ stream operators.
//

#ifndef __CELL__
#define __CELL__

// Removed for Avogadro:
// #include <stdcxx-includes.h>
//
// Replaced with:
#include <iostream>

#include "Core3DMath.h"

using namespace std;

//

	//  The metric tensor will be symmetric, so we store
	//  just half of it (6 elements at 8 bytes/element
	//  leaves us on 32- and 64-bit boundaries, cool!):
	//
	//  Element indexes:
	//   [[ 0  1  3 ]
	//	  [ 1  2  4 ]
	//    [ 3  4  5 ]]
	//
	typedef struct { double t[6]; } MetricTensor;

//

	class Cell {
		public:
			int						refs;								//  basic reference copy counting scheme
		
			double				a,b,c;							//  basis vector lengths in Cartesian space
			double				alpha,beta,gamma;		//  basis vector angles in Cartesian space
			TVector3D			av[3];							//  basis vectors in real space
			TVector3D			bv[3];							//  basis vectors in reciprocal space
			double				volume;							//	Cartesian space volume of cell
			
			MetricTensor	G;									//  Metric for our "space"
		
		public:
								Cell();
								Cell(double in_a,double in_b,double in_c, 
										 double in_alpha,double in_beta,double in_gamma);
								
			Cell*			Retain();
			void			Release();
			
			//  Cell dimension accessors:
			double		GetDimensionA() { return a;};
			void			SetDimensionA(double dim);
			double		GetDimensionB() { return b;};
			void			SetDimensionB(double dim);
			double		GetDimensionC() { return c;};
			void			SetDimensionC(double dim);
			
			//  Cell-edge angle accessors:
			double		GetAngleAlpha() { return alpha;};
			void			SetAngleAlpha(double ang);
			double		GetAngleBeta() { return beta;};
			void			SetAngleBeta(double ang);
			double		GetAngleGamma() { return gamma;};
			void			SetAngleGamma(double ang);
			
			//  Get the cell vectors:
			TVector3D	GetRealBasisVector1() { return av[0]; };
			TVector3D GetRealBasisVector2() { return av[1]; };
			TVector3D GetRealBasisVector3() { return av[2]; };
			TVector3D	GetReciprocalBasisVector1() { return bv[0]; };
			TVector3D GetReciprocalBasisVector2() { return bv[1]; };
			TVector3D GetReciprocalBasisVector3() { return bv[2]; };
			
			//  The translational vector for the cell
			TVector3D GetCellTranslationVector();
			
			//  Get the metric tensor for the cell:
			void GetMetricTensorForCell(MetricTensor& tensor);
			
			//  Cell volume:
			double GetRealSpaceVolume() { return volume; };
			double GetReciprocalSpaceVolume() { return (1.0 / volume); };
			
			//  Point conversions:
			TPoint3D CartesianToFractional(TPoint3D p);
			TPoint3D FractionalToCartesian(TPoint3D p);
			
			//  Distance conversion:
			double CartesianDistanceBetweenFractionalPoints(TPoint3D p1,TPoint3D p2);
			
			//  Write cell information to an ostream:
			void print(ostream& os);
			
		private:
			
			void			GenerateCellVectors();
	};

#endif //__CELL__

