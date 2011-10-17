//==========================================================
//
// fcc.cpp
//
// FCC lattice propogator
//
// Uses the CrystalCell class to generate a cell for
// FCC structures.  The primitive cell is then
// used to construct the (i,j,k) super-cell (output as
// a set of Cartesian coordinated).
//
// Created:		17.Sep.2005
// Last Mod:	n/a
//

#include "ANSR.h"
#include "CrystalCell.h"
	
	//  Silver
	//  a = b = c = 4.0782 angstrom
	#define			SILVER_ABC          4.0782
	//  Gold
	//  a = b = c = 4.08 angstrom
	#define			GOLD_ABC            4.0782
	//  Lattice info from:
	//			http://www.webelements.com/

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
		CrystalCell*		theCell = NULL;
		unsigned				atomicNumber = 79;
		
		if (argc > 2) {
			if (strcmp(argv[2],"Au") == 0) {
			  atomicNumber = 79;
			  theCell = new CrystalCell(GOLD_ABC,GOLD_ABC,GOLD_ABC,90.0,90.0,90.0);
			} else if (strcmp(argv[2],"Ag") == 0) {
			  atomicNumber = 47;
			  theCell = new CrystalCell(SILVER_ABC,SILVER_ABC,SILVER_ABC,90.0,90.0,90.0);
			}
		}
    if (!theCell) {
      atomicNumber = 79;
			theCell = new CrystalCell(GOLD_ABC,GOLD_ABC,GOLD_ABC,90.0,90.0,90.0);
    }
			
		//  These are the fractional coordinates for an FCC
		//  lattice:
		TPoint3D			p1 = { 0.0 , 0.0 , 0.0 };
		TPoint3D			p2 = { 0.5 , 0.5 , 0.0 };
		TPoint3D			p3 = { 0.5 , 0.0 , 0.5 };
		TPoint3D			p4 = { 0.0 , 0.5 , 0.5 };
		
		//  For propogation count:
		unsigned			i,j,k;
		int						got;
		
		//  Get command line stuff:
		if (argc < 2) {
		  printf("  usage:  fcc i,j,k {Au,Ag}\n");
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
		
		//  Generate the coordinates:
		theCell->Propogate(i,j,k,cout,kCrystalCellPropogateCentered);
	}
