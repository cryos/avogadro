//==========================================================
//
// graphite.cpp
//
// Graphite lattice propogator
//
// Uses the CrystalCell class to generate a cell for
// turbostratic graphite.  The primitive cell is then
// used to construct the (i,j,k) super-cell (output as
// a set of Cartesian coordinated).
//
// Created:		19.FEB.2002
// Last Mod:	n/a
//

#include "ANSR.h"
#include "CrystalCell.h"

	//  a = b = 2.456 angstrom
	#define			GRAPHITE_AB					2.456
	//  c = 6.694 angstrom
	#define			GRAPHITE_C					6.694
	
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
		//  Our hexagonal unit cell:
		CrystalCell		theCell(GRAPHITE_AB,GRAPHITE_AB,GRAPHITE_C,90.0,90.0,120.0);
		
		//  These are the fractional coordinates for turbostratic
		//  graphite:
		TPoint3D			p1 = { 0.0       , 0.0       , 0.25 };
		TPoint3D			p2 = { 1.0 / 3.0 , 2.0 / 3.0 , 0.25 };
		TPoint3D			p3 = { 0.0       , 0.0       , 0.75 };
		TPoint3D			p4 = { 2.0 / 3.0 , 1.0 / 3.0 , 0.75 };
		
		//  For propogation count:
		unsigned			i,j,k;
		int						got;
		
		//  Get command line stuff:
		if (argc < 2) {
		  printf("  usage:  graphite i,j,k\n");
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
		theCell.DidAddAtomAtFractionalPoint(6,p1);
		theCell.DidAddAtomAtFractionalPoint(6,p2);
		theCell.DidAddAtomAtFractionalPoint(6,p3);
		theCell.DidAddAtomAtFractionalPoint(6,p4);
		
		//  Generate the coordinates:
		theCell.Propogate(i,j,k,cout,kCrystalCellPropogateCentered);
	}