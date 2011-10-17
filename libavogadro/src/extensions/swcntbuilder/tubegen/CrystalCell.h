//==========================================================
//
// CrystalCell.h
//
// A component of the TubeGen program
//
// Adds atomic basis to a lattice cell.
//
// Created:		19.FEB.2002
// Last Mod:	05.Feb.2003:	Removed all ostream::form calls and moved to purely
//													C++ stream operators.
//

#ifndef __CRYSTALCELL__
#define __CRYSTALCELL__

// Removed for Avogadro:
// #include <stdcxx-includes.h>
//
// Replaced with:

#include "Cell.h"

using namespace std;

//

	enum {
	  kCrystalCellPadFromCenter,
		kCrystalCellPadFromOrigin
	};

//

	enum {
		kCrystalCellPropogateStandard,
	  kCrystalCellPropogateCentered
	};

//

	typedef struct _TAtomicCoordinate {
		unsigned			atomicNumber;
		TPoint3D			atomPosition;
	} TAtomicCoordinate;

//

	class CrystalCell : public Cell {
		public:
			unsigned						basisCount;
			unsigned						basisSize;
			TAtomicCoordinate*	basis;
		
		public:
								CrystalCell();
								CrystalCell(double in_a,double in_b,double in_c, 
										 double in_alpha,double in_beta,double in_gamma);
								CrystalCell(double in_a,double in_b,double in_c, 
										 double in_alpha,double in_beta,double in_gamma,
										 unsigned basisHint);
								CrystalCell(double in_a,double in_b,double in_c, 
										 double in_alpha,double in_beta,double in_gamma,
										 unsigned basisDim,TAtomicCoordinate* theBasis);
			
			unsigned	GetBasisCount() { return basisCount; };
      TAtomicCoordinate GetAtomicCoordinate(unsigned index);
			TPoint3D	GetCoordinate(unsigned index);
			
			//  Add an atom to the atomic basis:
			int				DidAddAtomAtCartesianPoint(unsigned atomicNumber,TPoint3D pt);
			int				DidAddAtomAtFractionalPoint(unsigned atomicNumber,TPoint3D pt);
			
			//  Write cell information to an ostream:
			void			print(ostream& os);
			
			//  Change the size of the cell:
			void			ResizeCell(double in_a,double in_b,double in_c);
			void			AddPaddingToCell(double da,double db,double dc,unsigned opts);
			
			//  Atomic basis transformations:
			void			ApplyCartesianTranslation(TVector3D xfrm);
			void			ApplyFractionalTranslation(TVector3D xfrm);
			
			//  Propogate through space:
			void			Propogate(unsigned i,unsigned j,unsigned k,ostream& os,unsigned opts = kCrystalCellPropogateStandard);
			bool      Propogate(unsigned i,unsigned j,unsigned k,TAtomicCoordinate** coords,unsigned* length,unsigned opts = kCrystalCellPropogateStandard);
      
		private:
			int				DidResizeBasisArray(unsigned newSize);
			
			int				PositionIsUnoccupied(TPoint3D pt);
			int				PositionIsUnoccupied(TPoint3D pt,double radius);
	};

#endif //__CRYSTALCELL__

