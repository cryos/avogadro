//==========================================================
//
// diamond.cpp
//
// Diamond lattice propogator
//
// Uses the CrystalCell class to generate a cell for
// diamond-like structures.  The primitive cell is then
// used to construct the (i,j,k) super-cell (output as
// a set of Cartesian coordinated).
//
// Created:		21.FEB.2002
// Last Mod:	n/a
//

#include "ANSR.h"
#include "CrystalCell.h"
	
	//  Real diamond
	//  a = b = c = 3.566 angstrom
	#define			DIAMOND_ABC					3.566
	//  Silicon
	//  a = b = c = 5.43 angstrom
	#define			SILICON_ABC					5.43
	//  Germanium
	//  a = b = c = 5.66 angstrom
	#define			GERMANIUM_ABC				5.66
	//  Si, Ge info from:
	//			http://hyperphysics.phy-astr.gsu.edu/hbase/solids/sili2.html#c1

//

	//  We use an ANSRDB initialized from /usr/share/periodic.table
	//  which, if the file isn't there, will just be the default
	//  anyway:
	ANSRDB					gPeriodicTable("/usr/share/periodic.table");
	
//

	int
	main(
		int		argc,
		char*	argv[]
	)
	{
		CrystalCell*		theCell;
		unsigned				atomicNumber = 6;
		
		if (argc > 2) {
			if (strcmp(argv[2],"Si") == 0) {
			  atomicNumber = 14;
			  theCell = new CrystalCell(SILICON_ABC,SILICON_ABC,SILICON_ABC,90.0,90.0,90.0);
			} else if (strcmp(argv[2],"Ge") == 0) {
			  atomicNumber = 32;
			  theCell = new CrystalCell(GERMANIUM_ABC,GERMANIUM_ABC,GERMANIUM_ABC,90.0,90.0,90.0);
			} else
			  theCell = new CrystalCell(DIAMOND_ABC,DIAMOND_ABC,DIAMOND_ABC,90.0,90.0,90.0);
		} else
			theCell = new CrystalCell(DIAMOND_ABC,DIAMOND_ABC,DIAMOND_ABC,90.0,90.0,90.0);
			
		//  These are the fractional coordinates for a diamond
		//  lattice:
		TPoint3D			p1 = { 0.0       , 0.0       , 1.0 / 8.0 };
		TPoint3D			p2 = { 0.5       , 0.5       , 1.0 / 8.0 };
		TPoint3D			p3 = { 3.0 / 4.0 , 1.0 / 4.0 , 3.0 / 8.0 };
		TPoint3D			p4 = { 1.0 / 4.0 , 3.0 / 4.0 , 3.0 / 8.0 };
		TPoint3D			p5 = { 0.5       , 0.0       , 5.0 / 8.0 };
		TPoint3D			p6 = { 0.0       , 0.5       , 5.0 / 8.0 };
		TPoint3D			p7 = { 1.0 / 4.0 , 1.0 / 4.0 , 7.0 / 8.0 };
		TPoint3D			p8 = { 3.0 / 4.0 , 3.0 / 4.0 , 7.0 / 8.0 };
		
		//  For propogation count:
		unsigned			i,j,k;
		int						got;
		
		//  Get command line stuff:
		if (argc < 2) {
		  printf("  usage:  diamond i,j,k {Si,Ge}\n");
			return -1;
		}
		got = sscanf(argv[1],"%d,%d,%d",&i,&j,&k);
		if (got < 3)
		  k = 1;
		if (got < 2)
		  j = 1;
		if (got < 1)
			i = 1;
		
		//  Add the atomic basis to the crystal cell:
		theCell->DidAddAtomAtFractionalPoint(atomicNumber,p1);
		theCell->DidAddAtomAtFractionalPoint(atomicNumber,p2);
		theCell->DidAddAtomAtFractionalPoint(atomicNumber,p3);
		theCell->DidAddAtomAtFractionalPoint(atomicNumber,p4);
		theCell->DidAddAtomAtFractionalPoint(atomicNumber,p5);
		theCell->DidAddAtomAtFractionalPoint(atomicNumber,p6);
		theCell->DidAddAtomAtFractionalPoint(atomicNumber,p7);
		theCell->DidAddAtomAtFractionalPoint(atomicNumber,p8);
		
		//  Generate the coordinates:
		theCell->Propogate(i,j,k,cout,kCrystalCellPropogateCentered);
	}