//==========================================================
//
// CrystalCell.cpp
//
// A component of the TubeGen program
//
// Adds atomic basis to a lattice cell.  The basis is
// maintained as a dynamically-resizable array of
// TAtomicCoordinate structures.
//
// Created:		19.FEB.2002
// Last Mod:	22.DEC.2004: Modified fractional coordinate addition
//                         for more robust/efficient transformation
//                         into purely-fractional values.
//

#include "CrystalCell.h"
#include "ANSR.h"
#include "CoreConst.h"

// Added for Avogadro
#include <iomanip>
#ifdef _MSC_VER
// Inefficient for cases with complex x, but ok for uses here
#define trunc(x) (((x) < 0) ? ceil(x) : floor(x))
#endif

//

	#define DEFAULT_BASIS_SIZE		5
	#define DEFAULT_BASIS_GROW		5
	#define DEFAULT_SEARCH_RADIUS	1e-2

//

	// =======================================================
	// * Default constructor
	CrystalCell::CrystalCell()
		: Cell()
	{
		basisSize = DEFAULT_BASIS_SIZE;
		basisCount = 0;
		basis = (TAtomicCoordinate*)calloc(basisSize,sizeof(TAtomicCoordinate));
	}
	
//

	// =======================================================
	// * Alternate constructor; cell parameters only
	CrystalCell::CrystalCell(
		double in_a,double in_b,double in_c, 
		double in_alpha,double in_beta,double in_gamma
	)
		: Cell(in_a,in_b,in_c,in_alpha,in_beta,in_gamma)
	{
		basisSize = DEFAULT_BASIS_SIZE;
		basisCount = 0;
		basis = (TAtomicCoordinate*)calloc(basisSize,sizeof(TAtomicCoordinate));
	}
	
//

	// =======================================================
	// * Alternate constructor; cell parameters plus basis
	//   size hint.
	CrystalCell::CrystalCell(
		double in_a,double in_b,double in_c, 
		double in_alpha,double in_beta,double in_gamma,
		unsigned basisHint
	)
		: Cell(in_a,in_b,in_c,in_alpha,in_beta,in_gamma)
	{
		basisSize = basisHint;
		basisCount = 0;
		basis = (TAtomicCoordinate*)calloc(basisSize,sizeof(TAtomicCoordinate));
	}
	
//

	// =======================================================
	// * Alternate constructor; cell parameters plus basis.
	CrystalCell::CrystalCell(
		double in_a,double in_b,double in_c, 
		double in_alpha,double in_beta,double in_gamma,
		unsigned basisDim,TAtomicCoordinate* theBasis
	)
		: Cell(in_a,in_b,in_c,in_alpha,in_beta,in_gamma)
	{
		basisSize = basisDim;
		basisCount = basisDim;
		basis = (TAtomicCoordinate*)calloc(basisSize,sizeof(TAtomicCoordinate));
		for ( unsigned i = 0 ; i < basisDim ; i++ )
		  basis[i] = theBasis[i];
	}
	
//

	// =======================================================
	// * Retrieve an atomic coordinate from the basis
	TAtomicCoordinate
	CrystalCell::GetAtomicCoordinate(
		unsigned 		index
	)
	{
		if (index < basisCount)
		  return basis[index];
	}

//

	// =======================================================
	// * Retrieve a coordinate from the basis
	TPoint3D
	CrystalCell::GetCoordinate(
		unsigned 		index
	)
	{
		if (index < basisCount)
		  return basis[index].atomPosition;
	}

//
	
	// =======================================================
	// * Add a new atom to the basis, at the Cartesian
	//   coordinate specified.
	int
	CrystalCell::DidAddAtomAtCartesianPoint(
		unsigned			atomicNumber,
		TPoint3D			pt
	)
	{
		return DidAddAtomAtFractionalPoint(atomicNumber,CartesianToFractional(pt));
	}
	
//
	
	// =======================================================
	// * Add a new atom to the basis, at the fractional
	//   coordinate specified.  The fractional coordinate is
	//   forced to be within the cell; the coordinate is also
	//   checked with the basis to make sure there is not an
	//   atom already in the position.
	int
	CrystalCell::DidAddAtomAtFractionalPoint(
		unsigned			atomicNumber,
		TPoint3D			pt
	)
	{
    /*
		//  If we're close enough to 0, then make the coordinate zero.
    //  Otherwise if 0 > x,y,z > 1 keep adding/subtracting 1 to
    //  get within that range:
    if (fabs(pt.x) < 1e-4) {
      pt.x = 0.0;
		} else if (pt.x > 0.0) {
		  while (pt.x >= 1.0) {
		    pt.x -= 1.0;
				if (fabs(pt.x) < 1e-4)
				  pt.x = 0.0;
			}
		} else {
		  while (pt.x < 0.0) {
			  pt.x += 1.0;
				if (fabs(pt.x) < 1e-4)
				  pt.x = 0.0;
			}
		}
		if (fabs(pt.y) < 1e-4) {
      pt.y = 0.0;
		} else if (pt.y > 0.0) {
		  while (pt.y >= 1.0) {
		    pt.y -= 1.0;
				if (fabs(pt.y) < 1e-4)
				  pt.y = 0.0;
			}
		} else {
		  while (pt.y < 0.0) {
			  pt.y += 1.0;
				if (fabs(pt.y) < 1e-4)
				  pt.y = 0.0;
			}
		}
		if (fabs(pt.z) < 1e-4) {
      pt.z = 0.0;
		} else if (pt.z > 0.0) {
		  while (pt.z >= 1.0) {
		    pt.z -= 1.0;
				if (fabs(pt.z) < 1e-4)
				  pt.z = 0.0;
			}
		} else {
		  while (pt.z < 0.0) {
			  pt.z += 1.0;
				if (fabs(pt.z) < 1e-4)
				  pt.z = 0.0;
			}
		}
    */
    
    //  Remove any integer portion of the coordinates so that
    //  they're truly fractional; if we're close enough to zero
    //  then just go with zero, too:
    if (fabs(pt.x -= trunc(pt.x)) < 1e-4)
      pt.x = 0.0;
    else if (pt.x < 0.0)
      pt.x += 1.0;
      
    if (fabs(pt.y -= trunc(pt.y)) < 1e-4)
      pt.y = 0.0;
    else if (pt.y < 0.0)
      pt.y += 1.0;
      
    if (fabs(pt.z -= trunc(pt.z)) < 1e-4)
      pt.z = 0.0;
    else if (pt.z < 0.0)
      pt.z += 1.0;
		
		//  Check to see if anything else is there:
		if (PositionIsUnoccupied(pt)) {
			//  If the number of basis atoms matches the size of the array
			//  we need to increase the size of our array of basis atoms:
			if (basisCount == basisSize)
			  if (!DidResizeBasisArray(basisSize + DEFAULT_BASIS_GROW))
				  return 0;
			basis[basisCount].atomicNumber = atomicNumber;
			basis[basisCount++].atomPosition = pt;
			return 1;
		}
		return 0;
	}
	
//

	// =======================================================
	// * Display the crystal cell parameters in human-readable
	//   form
	void
	CrystalCell::print(
		ostream&		os
	)
	{
    ios_base::fmtflags    savedFlags = os.flags();
    
    os << "CrystalCell { basisSize=" << basisSize << " basisCount=" << basisCount << endl;
    os.setf(ios::fixed);
    for ( unsigned i = 0 ; i < basisCount ; i++ ) {
      os.setf(ios::left);
      os << setw(3) << basis[i].atomicNumber << ' ';
      os.unsetf(ios::left);
      os << setprecision(6) << setw(10) << basis[i].atomPosition.x << ' ';
      os << setprecision(6) << setw(10) << basis[i].atomPosition.y << ' ';
      os << setprecision(6) << setw(10) << basis[i].atomPosition.z << endl;
    }
    os.unsetf(ios::fixed);
    //  Ask our superclass to print its info, too:
    Cell::print(os);
    os << "}\n";
    os.setf(savedFlags);
	}
	
//

	// =======================================================
	// * Resize the cell; does not attempt to keep the atomic
	//   coordinates in-place (use the padding method for that
	//   operation)
	void
	CrystalCell::ResizeCell(
		double		in_a,
		double		in_b,
		double		in_c
	)
	{
		SetDimensionA(in_a);
		SetDimensionB(in_b);
		SetDimensionC(in_c);
	}
	
//

	// =======================================================
	// * Add padding to the cell.  This amounts to enlarging
	//   the size of the cell but keeping the atomic basis
	//   fixed their relative distance from one another.
	void
	CrystalCell::AddPaddingToCell(
		double		da,
		double		db,
		double		dc,
		unsigned	opts
	)
	{
		double		newa = a + da;
		double		newb = b + db;
		double		newc = c + dc;
		double		aRatio = a / newa;
		double		bRatio = b / newb;
		double		cRatio = c / newc;
		unsigned	i;
		TVector3D	xlate = { 0.0 , 0.0 , 0.0 };
		
		//  Check to see if any of the coordinates will now
		//  be outside the cell:
		for ( i = 0 ; i < basisCount ; i++ ) {
		  if ((aRatio * basis[i].atomPosition.x >= 1.0) ||
					(bRatio * basis[i].atomPosition.y >= 1.0) ||
					(cRatio * basis[i].atomPosition.z >= 1.0))
			  return;
		}
		//  If we're adding padding to all edges, then define
		//  a translational vector that we can add to each
		//  point to do the centering:
		if (opts == kCrystalCellPadFromCenter) {
			xlate.x = (0.5 * da / newa);
			xlate.y = (0.5 * db / newb);
			xlate.z = (0.5 * dc / newc);
		  for ( i = 0 ; i < basisCount ; i++ ) {
			  basis[i].atomPosition.x = basis[i].atomPosition.x * aRatio + xlate.x;
			  basis[i].atomPosition.y = basis[i].atomPosition.y * bRatio + xlate.y;
			  basis[i].atomPosition.z = basis[i].atomPosition.z * cRatio + xlate.z;
			}
		} else {
			//  Grow relative to the origin:
		  for ( i = 0 ; i < basisCount ; i++ ) {
			  basis[i].atomPosition.x *= aRatio;
			  basis[i].atomPosition.y *= bRatio;
			  basis[i].atomPosition.z *= cRatio;
			}
		}
		SetDimensionA(newa);
		SetDimensionB(newb);
		SetDimensionC(newc);
	}
	
//

	// =======================================================
	// * Translate all points in the atomic basis; the
	//   translation is given as a Cartesian shift.
	void
	CrystalCell::ApplyCartesianTranslation(
		TVector3D			xfrm
	)
	{
		ApplyFractionalTranslation(CartesianToFractional(xfrm));
	}
	
//

	// =======================================================
	// * Translate all points in the atomic basis; the
	//   translation is given as a fractional coordinate
	//   shift.
	void
	CrystalCell::ApplyFractionalTranslation(
		TVector3D			xfrm
	)
	{
		for ( unsigned i = 0 ; i < basisCount ; i++ ) {
			TPoint3D		p;
			
			Vector3D_Sum(&basis[i].atomPosition,&xfrm,&p);
			Point3D_Rezero(&p,1e-6);
			
			if (p.x >= 1.0)
			  while (p.x >= 1.0) p.x -= 1.0;
			else
			  while (p.x < 0.0) p.x += 1.0;
			
			if (p.y >= 1.0)
			  while (p.y >= 1.0) p.y -= 1.0;
			else
			  while (p.y < 0.0) p.y += 1.0;
			
			if (p.z >= 1.0)
			  while (p.z >= 1.0) p.z -= 1.0;
			else
			  while (p.z < 0.0) p.z += 1.0;
			
			Point3D_Rezero(&p,1e-6);
			basis[i].atomPosition = p;
		}
	}
	
//

	// =======================================================
	// * Generate a listing of cartesian coordinates for a
	//   cell propogated through (i,j,k) translations.  The
	//   coordinates are centered on the origin.
	void
	CrystalCell::Propogate(
		unsigned		i,
		unsigned		j,
		unsigned		k,
		ostream&		os,
		unsigned		opts
	)
	{
    TVector3D             xform = { 0.0 , 0.0 , 0.0 };
    TPoint3D              pt;
    unsigned              li,lj,lk,bb;
    ios_base::fmtflags    savedFlags = os.flags();
    ANSRDB*               periodicTable = ANSRDB::DefaultANSRDB();
    
    //  Vector-transform to be used on each point such
    //  that we center the generated lattice at the
    //  origin:
    if (opts ==  kCrystalCellPropogateCentered) {
      Vector3D_ScaledSum(&xform,(double)i,&av[0],&xform);
      Vector3D_ScaledSum(&xform,(double)j,&av[1],&xform);
      Vector3D_ScaledSum(&xform,(double)k,&av[2],&xform);
      Vector3D_Scalar(&xform,-0.5,&xform);
    }
    
    //  Simply loop and generate points:
    os.setf(ios::fixed);
    for ( li = 0 ; li < i ; li++ ) {
      for ( lj = 0 ; lj < j ; lj++ ) {
        for ( lk = 0 ; lk < k ; lk++ ) {
          for ( bb = 0 ; bb < basisCount ; bb++ ) {
            pt = basis[bb].atomPosition;
            if (li) pt.x += (double)li;
            if (lj) pt.y += (double)lj;
            if (lk) pt.z += (double)lk;
            pt = FractionalToCartesian(pt);
            Vector3D_Sum(&pt,&xform,&pt);
            
            TElementSymbol		symbol = periodicTable->LookupSymbolForNumber(basis[bb].atomicNumber);
            
            if (symbol == kANSRInvalidSymbol) {
              os.setf(ios::left);
              os << "  " << setw(3) << basis[bb].atomicNumber << "  ";
              os.unsetf(ios::left);
              os << setprecision(6) << setw(12) << pt.x << ' ';
              os << setprecision(6) << setw(12) << pt.y << ' ';
              os << setprecision(6) << setw(12) << pt.z << endl;
            } else {
              os.setf(ios::left);
              os << "  " << setw(3) << (char*)&symbol << "  ";
              os.unsetf(ios::left);
              os << setprecision(6) << setw(12) << pt.x << ' ';
              os << setprecision(6) << setw(12) << pt.y << ' ';
              os << setprecision(6) << setw(12) << pt.z << endl;
            }
          }
        }
      }
    }
    os.setf(savedFlags);
	}
  
//

  bool
  CrystalCell::Propogate(
    unsigned i,unsigned j,unsigned k,
    TAtomicCoordinate** coords,
    unsigned*           length,
    unsigned            opts
  )
  {
    TAtomicCoordinate   *array = *coords,*aPtr;
    bool                hadArray = (array != NULL);
    unsigned            count = *length,total = 0;
    TVector3D           xform = { 0.0 , 0.0 , 0.0 };
    TPoint3D            pt;
    unsigned            li,lj,lk,bb;
    
    //  Vector-transform to be used on each point such
    //  that we center the generated lattice at the
    //  origin:
    if (opts ==  kCrystalCellPropogateCentered) {
      Vector3D_ScaledSum(&xform,(double)i,&av[0],&xform);
      Vector3D_ScaledSum(&xform,(double)j,&av[1],&xform);
      Vector3D_ScaledSum(&xform,(double)k,&av[2],&xform);
      Vector3D_Scalar(&xform,-0.5,&xform);
    }
    
    //  Get an array allocated:
    if (!hadArray) {
      count = i * j * k * basisCount;
      array = (TAtomicCoordinate*)(calloc(count,sizeof(TAtomicCoordinate)));
      if (!array)
        return false;
    }
    aPtr = array;
    
    //  Simply loop and generate points:
    for ( li = 0 ; li < i && count > 0 ; li++ ) {
      for ( lj = 0 ; lj < j && count > 0 ; lj++ ) {
        for ( lk = 0 ; lk < k && count > 0 ; lk++ ) {
          for ( bb = 0 ; bb < basisCount && count > 0 ; bb++ ) {
            aPtr->atomicNumber = basis[bb].atomicNumber;
            pt = basis[bb].atomPosition;
            if (li) pt.x += (double)li;
            if (lj) pt.y += (double)lj;
            if (lk) pt.z += (double)lk;
            pt = FractionalToCartesian(pt);
            Vector3D_Sum(&pt,&xform,&aPtr->atomPosition);
            total++;
            aPtr++;
            count--;
          }
        }
      }
    }
    
    //  Prepare for return:
    if (!hadArray)
      *coords = array;
    *length = total;
    return true;
  }
	
//
// PRIVATE methods
//
	
	// =======================================================
	// * Resize the array of atomic coordinates; returns zero
	//   if realloc fails, 1 if realloc succeeds.
	int
	CrystalCell::DidResizeBasisArray(
		unsigned		newSize
	)
	{
		TAtomicCoordinate*		newBasis = NULL;
		
		if (newBasis = (TAtomicCoordinate*)realloc(basis,sizeof(TAtomicCoordinate) * newSize)) {
			basis = newBasis;
			basisSize = newSize;
			return 1;
		}
		return 0;
	}
	
//

	// =======================================================
	// * Determine if the point is already occupied within the
	//   atomic basis.  Uses a default search-radius around
	//   the new position.
	int
	CrystalCell::PositionIsUnoccupied(
		TPoint3D		pt
	)
	{
		return PositionIsUnoccupied(pt,DEFAULT_SEARCH_RADIUS);
	}
	
//

	// =======================================================
	// * Determine if the point is already occupied within the
	//   atomic basis using the supplied search radius.
  //
  // 14 Dec 2006 [JTF] Was using a simple fractional distance
  //                   test, but this fails for large crystal
  //                   cells (cell size inc, fractional distance
  //                   dec).  Using the Cell class' Cartesian
  //                   distance between fraction points method
  //                   now.
	int
	CrystalCell::PositionIsUnoccupied(
		TPoint3D		pt,
		double			radius
	)
	{
		for ( unsigned i = 0 ; i < basisCount ; i++ )
		  if (CartesianDistanceBetweenFractionalPoints(pt,basis[i].atomPosition) < radius)
			  return 0;
		return 1;
	}

//
