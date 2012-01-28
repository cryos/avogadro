// ===============================================================================
// TubuleBasis.cpp                                           ©2001-2002, J.T. Frey
// ===============================================================================
// Written:   J.T. Frey, 07.26.2001
// Purpose:   Main program file for the TubeGen app.  This file contains the
//            routines which generate the atomic basis for a unit cell and the
//            k-point list for a unit cell.  Also contains the main program
//            which processes user commands.
//
// Last Mod:  20.FEB.2002: Switch to hexagonal unit cell; k-point generator is
//                         no longer necessary.
//            18.NOV.2002: Modifications to position generation loop [3.0.5]
//            03.DEC.2002: Cell expansion/contraction to preserve rolled C-C
//                         bond lengths added as an option [3.0.6]
//            09.JUL.2004: Support for PDB [3.1.2]
//            22.DEC.2004: Fixed WIEN cell parameters for cubic & planar
//

//  Include some of our own headers:
#include "TubuleBasis.h"
#include "CoreConst.h"
#include "Constants.h"
#include "ANSR.h"
#include "Bitmap.h"

// Added for Avogadro
#include <iostream>
#include <iomanip>
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#include <math.h>
#define strcasecmp _stricmp
#endif

//

struct ExportFormatRec {
  int     stringIndex;
  int     formatID;
};

#define __TubuleBasis_FormatSpecCount EFormat_Max

const char* __TubuleBasis_FormatStrings[__TubuleBasis_FormatSpecCount] = {
  "gaussian" ,
  "gaussian-pbc" ,
  "wien" ,
  "xyz" ,
  "pdb" ,
  "pdb-pbc" ,
  "pov" ,
  "bgf" ,
  "cif"
};

const char* __TubuleBasis_FileExtensions[__TubuleBasis_FormatSpecCount] = {
  "com" ,
  "com" ,
  "struct" ,
  "xyz" ,
  "pdb" ,
  "pdb" ,
  "povinc" ,
  "bgf" ,
  "cif"
};

struct ExportFormatRec __TubuleBasis_FormatSpecs[__TubuleBasis_FormatSpecCount] = {
  { 7 , EFormat_BGF },
  { 8 , EFormat_CIF },
  { 0 , EFormat_Gaussian },
  { 1 , EFormat_GaussianPBC },
  { 4 , EFormat_PDB },
  { 5 , EFormat_PDBPBC },
  { 6 , EFormat_POV },
  { 2 , EFormat_WIEN },
  { 3 , EFormat_XYZ }
};

//

int __TubuleBasis_FormatSpecBSearchDriver(const void* key,const void* value)
{
  return strcasecmp((const char*)key,__TubuleBasis_FormatStrings[((struct ExportFormatRec*)value)->stringIndex]);
}

void __TubuleBasis_FormatStringSummary()
{
  int     i = 0;
  for ( ; i < EFormat_Max - 1 ; i++ )
    printf("%s,",__TubuleBasis_FormatStrings[i]);
  printf("%s",__TubuleBasis_FormatStrings[i]);
}

inline int __TubuleBasis_FormatIDForString(const char* formatString)
{
  struct ExportFormatRec*   match = (ExportFormatRec*)
                                          bsearch(formatString,
                                              __TubuleBasis_FormatSpecs,
                                              __TubuleBasis_FormatSpecCount,
                                              sizeof(struct ExportFormatRec),
                                              __TubuleBasis_FormatSpecBSearchDriver
                                            );
  if (match)
    return match->formatID;
  return EFormat_Undefined;
}

inline const char* __TubuleBasis_StringForFormatID(int formatID)
{
  return __TubuleBasis_FormatStrings[formatID];
}

inline const char* __TubuleBasis_FileExtensionForFormatID(int formatID)
{
  return __TubuleBasis_FileExtensions[formatID];
}

//

  // ----------------------------------------------------------------------
  // * gcd
  // ----------------------------------------------------------------------
  // Updated: Jeff Frey, 03.19.2001
  // Purpose: Greatest common divisor of two integers.
  //
  // Last Mod:  n/a
  
  int gcd(
    int i,
    int j
  )
  {
    int m,n,r;
    
    (i < j) ? (m = j,n = i) : (m = i,n = j);
    
    if (n == 0)
      return m;
      
    while (r = m % n) {
      m = n;
      n = r;
    }
    
    return n;
  }
  
//
////
//

  TubuleBasis::TubuleBasis()
  {
		InitializeBasis(3,3);
  }

//

  TubuleBasis::TubuleBasis(
    int       nval,
    int       mval
  )
  {
		InitializeBasis(nval, mval);
  }
  
//

  CrystalCell*
  TubuleBasis::GenerateCrystalCell()
  {
    CrystalCell*      cell = NULL;
    double            a,b,c;
    double            lenV;
    int               i,j,k;
    int               iMax,jMax,iMin,jMin;
    TVector3D         v;
    TVector3D         center;
    TPoint3D          p,p1;
    
    //  Display lattice-type information:
    if (oVerbose) {
      int     N;
      
      switch (oLattice) {
        case ELattice_Cubic:
          printf("  Producing rolled, cubic nanotube lattice.\n\n");
          break;
          
        case ELattice_Planar:
          printf("  Producing planar nanotube lattice.\n\n");
          break;
          
        default:
          printf("  Producing rolled, hexagonal nanotube lattice.\n\n");
          break;
      }
      //  Calculate the number of hexagonal sub-cells:
      N = 2 * (m * m + n * n + n * m) / dR;
      
      if (N > 1)
        printf("  Lattice consists of %d hexagonal sub-cells.\n",N);
      else
        printf("  Lattice consists of a single hexagonal sub-cell.\n");
    }
    
    //  Create the crystal cell:
    switch (oLattice) {
      case ELattice_Cubic:
      case ELattice_Hexagonal: {
        a = 2.0 * (r + gutter.x);
        b = 2.0 * (r + gutter.y);
        c = lenT + 2.0 * gutter.z;
        if (oLattice == ELattice_Hexagonal)
          cell = new CrystalCell(a,b,c,90.0,90.0,120.0);
        else
          cell = new CrystalCell(a,b,c,90.0,90.0,90.0);
        center = cell->GetRealBasisVector1();
        v = cell->GetRealBasisVector2();
        Vector3D_Scalar(&center,0.5,&center);
        Vector3D_ScaledSum(&center,0.5,&v,&center);
        break;
      }
      
      case ELattice_Planar: {
        a = lenCh + 2.0 * gutter.x;
        b = lenCh + 2.0 * gutter.y;
        c = lenT + 2.0 * gutter.z;
        cell = new CrystalCell(a,b,c,90.0,90.0,90.0);
        break;
      }
    }
    
    //  Begin generating coordinates:
    iMin = (nprime < 0)?(nprime):(0);
    iMin = (iMin < n)?(iMin):(n);
    iMax = ((n + nprime) > n)?(n + nprime):(n);
    iMax = (iMax > nprime)?(iMax):(nprime);
    
    jMin = (-mprime < 0)?(-mprime):(0);
    jMin = (jMin < m)?(jMin):(m);
    jMax = ((m - mprime) > m)?(m - mprime):(m);
    jMax = (jMax > -mprime)?(jMax):(-mprime);
    for ( i = iMin ; i <= iMax ; i++ ) {
      for ( j = jMin ; j <= jMax ; j++) {
        //  And finally, we loop over the two atoms in the
        //  hexagonal graphite basis, giving us
        //  i(a1) + j(a2)   and   i(a1) + j(a2) + <C-C,0,0>
        for ( k = 0 ; k < 2 ; k++ ) {
        
          //  Construct i(a1) + j(a2):
          v.x = (double)i * a1.x + (double)j * a2.x;
          v.y = (double)i * a1.y + (double)j * a2.y;
          v.z = 0.0;
          
          //  Second time through we add a C-C bond displacement:
          if (k == 1)
            v.x += bondLen * bondLenScale;
          Vector3D_Rezero(&v,FLT_EPSILON);
          
          lenV = Vector3D_Magnitude(&v);
          
          //  Check v; if it's a zero vector that's really easy; otherwise
          //  we need to project onto Ch and T to get fractional coordinates
          //  along those axes.
          p.y = 0.5;
          if (lenV < FLT_EPSILON) {
            p.x = p.z = 0.0;
          } else {
            p.x = Vector3D_Dot(&v,&Ch_R); // / (lenCh * lenCh);
            p.z = Vector3D_Dot(&v,&T_R); // / (lenT * lenT);
            if (fabs(p.x) < FLT_EPSILON) p.x = 0.0;
            if (fabs(p.z) < FLT_EPSILON) p.z = 0.0;
          }
          //  If point "p" is within [0,1) in x and z, we have a point:
          if ((p.x < 1.0) && (p.x >= 0.0) && (p.z < 1.0) && (p.z >= 0.0)) {
            //  Check if we're too close to 1.0:
            if ((1.0 - p.x > FLT_EPSILON) && (1.0 - p.z > FLT_EPSILON)) {
              //  Recalculate in terms of Ch and Tprime:
              p.x = Vector3D_Dot(&v,&Ch) / (lenCh * lenCh);
              p.z = Vector3D_Dot(&v,&Tprime) / (h * h);
              //  This is the rolled- vs. flat-specific stuff:
              if ((oLattice == ELattice_Cubic) || (oLattice == ELattice_Hexagonal)) {
                //  theta = 2(pi) times displacement along chiral vector:
                double    theta = k2Pi * p.x;
                
                //  Redefine the point as a polar coordinate in xy-plane:
                p.x = r * cos(theta) + center.x;
                p.y = r * sin(theta) + center.y;
                p.z *= h;
                cell->DidAddAtomAtCartesianPoint(element[k],p);
              } else {
                p.x = (p.x * lenCh + gutter.x) / a;
                p.z = (p.z * h + gutter.z) / c;
                cell->DidAddAtomAtFractionalPoint(element[k],p);
              }
            }
          }
        }
      }
    }
    
    if (oVerbose) {
      if (cell)
        printf("  Cell generation complete.  %d basis points defined.\n",cell->GetBasisCount());
      else
        printf("  Cell generation failed!\n");
    }
    
    return cell;
  }
  
//

  void
  TubuleBasis::WriteInputFile(
    ostream&            os
  )
  {
    CrystalCell*  theCell = GenerateCrystalCell();
    
    if (theCell) {
      WriteInputFile(os,theCell);
      delete theCell;
    }
  }

//

  void
  TubuleBasis::WriteInputFile(
    ostream&          os,
    CrystalCell*      theCell
  )
  {
    ANSRDB*           periodicTable = ANSRDB::DefaultANSRDB();
    
    if (theCell) {
      ios_base::fmtflags    savedFlags = os.flags();
      
      os.setf(ios::fixed);
      switch (oFormat) {
      
        case EFormat_GaussianPBC: {
          TVector3D     bv;
          
          os << "# PBEPBE/6-31g" << endl << endl;
          os << "(" << n << ',' << m << ") Nanotube" << endl;
          os << "TubeGen 3.3, J T Frey, University of Delaware" << endl << endl;
          os << " 0    1" << endl;
          theCell->Propogate(1,1,1,os,kCrystalCellPropogateStandard);
          //  For Gaussian 03 we have periodic boundary conditions; we'll just write out our
          //  cell vectors at the end of all the atom positions:
          bv = theCell->GetRealBasisVector1();
          os.setf(ios::left);
           os << "  " << setw(3) << "TV" << "  ";
          os.unsetf(ios::left);
          os << setprecision(6) << setw(12) << bv.x << ' ';
          os << setprecision(6) << setw(12) << bv.y << ' ';
          os << setprecision(6) << setw(12) << bv.z << endl;
          bv = theCell->GetRealBasisVector2();
          os.setf(ios::left);
           os << "  " << setw(3) << "TV" << "  ";
          os.unsetf(ios::left);
          os << setprecision(6) << setw(12) << bv.x << ' ';
          os << setprecision(6) << setw(12) << bv.y << ' ';
          os << setprecision(6) << setw(12) << bv.z << endl;
          bv = theCell->GetRealBasisVector3();
          os.setf(ios::left);
           os << "  " << setw(3) << "TV" << "  ";
          os.unsetf(ios::left);
          os << setprecision(6) << setw(12) << bv.x << ' ';
          os << setprecision(6) << setw(12) << bv.y << ' ';
          os << setprecision(6) << setw(12) << bv.z << endl;
          os << endl;
          break;
        }
        
        case EFormat_CIF: {
          os << "data_nanotube" << endl << endl;
          os << "_audit_creation_method       '";
          os << '(' << n << ',' << m << ") Nanotube -- TubeGen 3.3, J T Frey, University of Delaware'" << endl << endl;
          
          os << "_cell_length_a     " << setprecision(4) << setw(10) << theCell->GetDimensionA() << endl;
          os << "_cell_length_b     " << setprecision(4) << setw(10) << theCell->GetDimensionB() << endl;
          os << "_cell_length_c     " << setprecision(4) << setw(10) << theCell->GetDimensionC() << endl;
          os << "_cell_angle_alpha  " << setprecision(2) << setw(8) << 90. << endl;
          os << "_cell_angle_beta   " << setprecision(2) << setw(8) << 90. << endl;
          switch (oLattice) {
            case ELattice_Hexagonal:
              os << "_cell_angle_gamma  " << setprecision(2) << setw(8) << 120. << endl << endl;
              break;
            case ELattice_Planar:
            case ELattice_Cubic:
              os << "_cell_angle_gamma  " << setprecision(2) << setw(8) << 90. << endl << endl;
              break;
          }
          
          os << "_symmetry_space_group_name_H-M   'P 1'" << endl;
          os << "_symmetry_Int_Tables_number       1" << endl << endl;
          
          os << "loop_" << endl;
          os << "_atom_site_label" << endl;
          os << "_atom_site_fract_x" << endl;
          os << "_atom_site_fract_y" << endl;
          os << "_atom_site_fract_z" << endl;
          
          for ( unsigned i = 0 ; i < theCell->GetBasisCount() ; i++ ) {
            TAtomicCoordinate   coord = theCell->GetAtomicCoordinate(i);
            TElementSymbol      sym = periodicTable->LookupSymbolForNumber(coord.atomicNumber);
            
            if (sym != kANSRInvalidSymbol)
              os << setw(6) << left << (char*)&sym << right;
            else
              os << setw(6) << left << coord.atomicNumber << right;
              
            os << setprecision(4) << setw(10) << coord.atomPosition.x;
            os << setprecision(4) << setw(10) << coord.atomPosition.y;
            os << setprecision(4) << setw(10) << coord.atomPosition.z << endl;
          }
          os << endl << endl;
          break;
        }
           
        case EFormat_WIEN: {
          os << '(' << n << ',' << m << ") Nanotube -- TubeGen 3.3, J T Frey, University of Delaware" << endl;
          switch (oLattice) {
            case ELattice_Hexagonal: {
              os << "H   LATTICE,NONEQUIV. ATOMS: 1" << endl;
              os << "MODE OF CALC=RELA" << endl;
              os << setprecision(6) << setw(10) << theCell->GetDimensionA() / ANGSTROM_PER_BOHR;
              os << setprecision(6) << setw(10) << theCell->GetDimensionB() / ANGSTROM_PER_BOHR;
              os << setprecision(6) << setw(10) << theCell->GetDimensionC() / ANGSTROM_PER_BOHR;
              os << setprecision(6) << setw(10) << 90.0;
              os << setprecision(6) << setw(10) << 90.0;
              os << setprecision(6) << setw(10) << 120.0 << endl;
              break;
            }
            
            case ELattice_Planar:
            case ELattice_Cubic: {
              os << "P   LATTICE,NONEQUIV. ATOMS: 1" << endl;
              os << "MODE OF CALC=RELA" << endl;
              os << setprecision(6) << setw(10) << theCell->GetDimensionA() / ANGSTROM_PER_BOHR;
              os << setprecision(6) << setw(10) << theCell->GetDimensionB() / ANGSTROM_PER_BOHR;
              os << setprecision(6) << setw(10) << theCell->GetDimensionC() / ANGSTROM_PER_BOHR;
              os << setprecision(6) << setw(10) << 90.0;
              os << setprecision(6) << setw(10) << 90.0;
              os << setprecision(6) << setw(10) << 90.0 << endl;
              break;
            }
          }
          for ( unsigned i = 0 ; i < theCell->GetBasisCount() ; i++ ) {
            TPoint3D    pt = theCell->GetCoordinate(i);
            if (i == 0)
              os << "ATOM= ";
            else
              os << "      ";
            os << "-1: X=";
            os << setprecision(8) << setw(10) << pt.x;
            os << " Y=" << setprecision(8) << setw(10) << pt.y;
            os << " Z=" << setprecision(8) << setw(10) << pt.z << endl;
            if (i == 0) {
              os.setf(ios::left);
              os << "          MULT=" << setw(4) << theCell->GetBasisCount() << "        ISPLIT= 8" << endl;
              os.setf(ios::right);
            }
          }
          os << "C          NPT=  781  R0=0.00010000 RMT=    1.3000   Z:  6.0" << endl;
          os << "                     1.0000000 0.0000000 0.0000000" << endl;
          os << "                     0.0000000 1.0000000 0.0000000" << endl;
          os << "                     0.0000000 0.0000000 1.0000000" << endl;
          os << "   0 SYMMETRY OPERATIONS:" << endl;
          break;
        }
        
        case EFormat_Gaussian: {
          os << "# PBEPBE/6-31g" << endl << endl;
          os << "(" << n << ',' << m << ") Nanotube";
          if ((replicate[0] > 1) || (replicate[1] > 1) || (replicate[2] > 1))
            os << " (" << replicate[0] << ',' << replicate[1] << ',' << replicate[2] << ") replication";
          os << endl << "TubeGen 3.3, J T Frey, University of Delaware" << endl << endl;
          os << " 0    1" << endl;
          theCell->Propogate(replicate[0],replicate[1],replicate[2],os,kCrystalCellPropogateCentered);
          os << endl;
          break;
        }
        
        case EFormat_XYZ: {
          os << theCell->GetBasisCount() * replicate[0] * replicate[1] * replicate[2] << endl;
          os << "(" << n << ',' << m << ") Nanotube";
          if ((replicate[0] > 1) || (replicate[1] > 1) || (replicate[2] > 1))
            os << " (" << replicate[0] << ',' << replicate[1] << ',' << replicate[2] << ") replication";
          os << " -- TubeGen 3.3, J T Frey, University of Delaware" << endl;
          theCell->Propogate(replicate[0],replicate[1],replicate[2],os,kCrystalCellPropogateCentered);
          os << endl;
          break;
        }
        
        case EFormat_PDB:
        case EFormat_PDBPBC: {
          //  Starting in version 3.2 I have a routine to propogate the system
          //  to an array; this allows me to find connectivity for PDBs and also
          //  makes the "propogate to temp file then reread to reformat" scheme
          //  unnecessary (thank god!!)
          TAtomicCoordinate*    positions = NULL;
          unsigned              posCount = 0;
          bool                  state;
          
          //  Get the propogated cell:
          if (oFormat == EFormat_PDB)
            state = theCell->Propogate(replicate[0],replicate[1],replicate[2],&positions,&posCount,kCrystalCellPropogateCentered);
          else
            state = theCell->Propogate(1,1,1,&positions,&posCount,kCrystalCellPropogateCentered);
          
          if (state) {
            //  We'll use an array of variable-size bit vectors:
            BitmapRef           *connectivity = (BitmapRef*)alloca(posCount * sizeof(BitmapRef));
            
            if ( ! connectivity ) {
              fprintf(stderr, "ERROR:  unable to allocate connectivity array\n");
              exit(1);
            }
            
            //  Zero-out the connectivity array:
            memset(connectivity,0,sizeof(BitmapRef) * posCount);
            
            //  Write the header:
            os << "HEADER    PROTEIN" << endl;
            os << "TITLE     (" << n << ',' << m << ") Nanotube";
            if ((replicate[0] > 1) || (replicate[1] > 1) || (replicate[2] > 1))
              os << " (" << replicate[0] << ',' << replicate[1] << ',' << replicate[2] << ") replication";
            os << endl << "AUTHOR    TubeGen 3.3, J T Frey, University of Delaware" << endl;
            os << "EXPDTA    THEORETICAL MODEL" << endl;
            
            //  Write-out the crystal cell when necessary:
            if (oFormat == EFormat_PDBPBC) {
              os << "CRYST1";
              os << setprecision(3) << setw(9) << theCell->GetDimensionA();
              os << setprecision(3) << setw(9) << theCell->GetDimensionB();
              os << setprecision(3) << setw(9) << theCell->GetDimensionC();
              switch (oLattice) {
                case ELattice_Hexagonal:
                  os << setprecision(2) << setw(7) << 90.0;
                  os << setprecision(2) << setw(7) << 90.0;
                  os << setprecision(2) << setw(7) << 120.0;
                  break;
                
                case ELattice_Planar:
                case ELattice_Cubic:
                  os << setprecision(2) << setw(7) << 90.0;
                  os << setprecision(2) << setw(7) << 90.0;
                  os << setprecision(2) << setw(7) << 90.0;
                  break;
              }
              os << " P 1          1" << endl;
              
              TVector3D     recipVec = theCell->GetReciprocalBasisVector1();
              
              os << "SCALE1    " << setprecision(6) << setw(10) << recipVec.x;
              os << setprecision(6) << setw(10) << recipVec.y;
              os << setprecision(6) << setw(10) << recipVec.z;
              os << "     " << setprecision(5) << setw(10) << 0.0 << endl;
              
              recipVec = theCell->GetReciprocalBasisVector2();
              os << "SCALE2    " << setprecision(6) << setw(10) << recipVec.x;
              os << setprecision(6) << setw(10) << recipVec.y;
              os << setprecision(6) << setw(10) << recipVec.z;
              os << "     " << setprecision(5) << setw(10) << 0.0 << endl;
              
              recipVec = theCell->GetReciprocalBasisVector3();
              os << "SCALE3    " << setprecision(6) << setw(10) << recipVec.x;
              os << setprecision(6) << setw(10) << recipVec.y;
              os << setprecision(6) << setw(10) << recipVec.z;
              os << "     " << setprecision(5) << setw(10) << 0.0 << endl;
            }
            
            //  Now we just need to keep doing the records in PDB format.  As
            //  we go we figure out the connectivity, too:
            TAtomicCoordinate*    p1 = positions;
            unsigned              count1 = 0;
            
            while (count1 < posCount) {
              unsigned            count2 = count1 + 1;
              TAtomicCoordinate*  p2 = positions + count2;
              TElementSymbol      symbol = periodicTable->LookupSymbolForNumber(p1->atomicNumber);
              
              //  Write the ATOM record to the stream first of all:
              os << setw(6) << "ATOM  ";
              os << setw(5) << (count1 + 1);
              if (symbol == kANSRInvalidSymbol)
                os << setw(3) << p1->atomicNumber << "   UNK     1    ";
              else
                os << setw(3) << (char*)&symbol << "   UNK     1    ";
              os << setw(8) << setprecision(3) << p1->atomPosition.x;
              os << setw(8) << setprecision(3) << p1->atomPosition.y;
              os << setw(8) << setprecision(3) << p1->atomPosition.z;
              os << "  1.00  0.00" << endl;
              
              //  Now let's play some more connectivity games:
              if (count1 < posCount - 1) {
                BitmapRef     aBitmap = connectivity[count1];
                
                while (count2 < posCount) {
                  //  How far is positions[count2] from positions[count1]?
                  //  If we're no more than 19% larger than the official bond length:
                  if (Point3D_Distance(&p1->atomPosition,&p2->atomPosition) <=  1.1 * bondLen) {
                    //  Do we need to create a bitmap?
                    if (!aBitmap) {
                      if (!(aBitmap = BitmapCreateWithCapacity(posCount)))
                        break;
                    }
                    BitmapFlipBit(aBitmap,count2);
                      
                    //  *sigh* PDB seems to like to have redundant connectivity
                    //  data, so let's do the other, as well:
                    BitmapRef otherBitmap = connectivity[count2];
                    
                    if (!otherBitmap) {
                      if (!(otherBitmap = BitmapCreateWithCapacity(posCount)))
                        break;
                      connectivity[count2] = otherBitmap;
                    }
                    BitmapFlipBit(otherBitmap,count1);
                  }
                  count2++;
                  p2++;
                }
                connectivity[count1] = aBitmap;
              }
              count1++;
              p1++; 
            }
            
            //  Now examine the connectivity data:
            unsigned    conectCount = 0;
            
            count1 = 0;
            while (count1 < posCount) {
              BitmapRef   theConnections = connectivity[count1];
              
              //printf(":: ");
              //BitmapSummarizeToStream(stdout,theConnections);
              //printf("\n");
              if (theConnections) {
                unsigned        count2 = BitmapGetNextBitWithValue(theConnections,0,1);
                
                if (count2 != ((unsigned)-1)) {
                  unsigned      bitCount = 0;
                  
                  while (count2 < posCount) {
                    if (BitmapGetBit(theConnections,count2++)) {
                      if ((bitCount % 4) == 0) {
                        if (bitCount)
                          os << endl;
                        conectCount++;
                        os << "CONECT" << setw(5) << count1 + 1 << setw(5) << count2;
                      } else
                        os << setw(5) << count2;
                      bitCount++;
                    }
                  }
                  os << endl;
                }
                BitmapDestroy(theConnections);
              }
              count1++;
            }
            
            //  Bookkeeping stuff:
            os << "MASTER        0    0    0    0    0    0    0";
            if (oFormat == EFormat_PDB)
              os << "    0";
            else
              os << "    3";
            os << setw(5) << posCount;
            os << "    0";
            os << setw(5) << conectCount;
            os << "    0" << endl;
            os << "END   " << endl;
          }
            
          break;
        }
        
        case EFormat_BGF: {
          //  MSI BIOGRAF format:
          TAtomicCoordinate*    positions = NULL;
          unsigned              posCount = 0;
          
          //  Get the propogated cell:
          if (theCell->Propogate(replicate[0],replicate[1],replicate[2],&positions,&posCount,kCrystalCellPropogateCentered)) {
            //  We'll use an array of variable-size bit vectors:
            BitmapRef           *connectivity = (BitmapRef*)alloca(posCount * sizeof(BitmapRef));
            
            if ( ! connectivity ) {
              fprintf(stderr, "ERROR:  unable to allocate connectivity array\n");
              exit(1);
            }
            
            //  Zero-out the connectivity array:
            memset(connectivity,0,sizeof(BitmapRef) * posCount);
            
            //  Write the header:
            os << "BIOGRF" << setw(5) << 300 << endl;
            os << "DESCRP Nanotube" << endl;
            os << "REMARK (" << n << ',' << m << ") Nanotube -- TubeGen 3.3, J T Frey, University of Delaware" << endl;
            os << "FORMAT ATOM   (a6,1x,i5,1x,a5,1x,a3,1x,a1,1x,a5,3f10.5,1x,a5,i3,i2,1x,f8.5)" << endl;
            
            //  Now we just need to keep doing the records in PDB format.  As
            //  we go we figure out the connectivity, too:
            TAtomicCoordinate*    p1 = positions;
            unsigned              count1 = 0;
            
            while (count1 < posCount) {
              unsigned            count2 = count1 + 1;
              TAtomicCoordinate*  p2 = positions + count2;
              TElementSymbol      symbol = periodicTable->LookupSymbolForNumber(p1->atomicNumber);
              
              //  Write the ATOM record to the stream first of all:
              os << "ATOM   ";
              os << setw(5) << (count1 + 1) << ' ';
              if (symbol == kANSRInvalidSymbol)
                os << setw(5) << left << p1->atomicNumber << right << " RES A   444";
              else
                os << setw(5) << left << (char*)&symbol << right << " RES A   444";
              os << setw(10) << setprecision(5) << p1->atomPosition.x;
              os << setw(10) << setprecision(5) << p1->atomPosition.y;
              os << setw(10) << setprecision(5) << p1->atomPosition.z;
              os << " CA     1 0  0.00000" << endl;
              
              //  Now let's play some more connectivity games:
              if (count1 < posCount - 1) {
                BitmapRef     aBitmap = connectivity[count1];
                
                while (count2 < posCount) {
                  //  How far is positions[count2] from positions[count1]?
                  //  If we're no more than 19% larger than the official bond length:
                  if (Point3D_Distance(&p1->atomPosition,&p2->atomPosition) <=  1.1 * bondLen) {
                    //  Do we need to create a bitmap?
                    if (!aBitmap) {
                      if (!(aBitmap = BitmapCreateWithCapacity(posCount)))
                        break;
                    }
                    BitmapFlipBit(aBitmap,count2);
                      
                    //  *sigh* PDB seems to like to have redundant connectivity
                    //  data, so let's do the other, as well:
                    BitmapRef otherBitmap = connectivity[count2];
                    
                    if (!otherBitmap) {
                      if (!(otherBitmap = BitmapCreateWithCapacity(posCount)))
                        break;
                      connectivity[count2] = otherBitmap;
                    }
                    BitmapFlipBit(otherBitmap,count1);
                  }
                  count2++;
                  p2++;
                }
                connectivity[count1] = aBitmap;
              }
              count1++;
              p1++; 
            }
            
            //  Now examine the connectivity data:
            unsigned    conectCount = 0;
            
            os << "FORMAT CONECT (a6,12i6)" << endl;
            count1 = 0;
            while (count1 < posCount) {
              BitmapRef   theConnections = connectivity[count1];
              
              if (theConnections) {
                unsigned        count2 = BitmapGetNextBitWithValue(theConnections,0,1);
                
                if (count2 != ((unsigned)-1)) {
                  unsigned      bitCount = 0,altBitCount = 0;
                  
                  while (count2 < posCount) {
                    if (BitmapGetBit(theConnections,count2++)) {
                      if ((bitCount % 12) == 0) {
                        if (bitCount)
                          os << endl;
                        conectCount++;
                        os << "CONECT" << setw(6) << count1 + 1 << setw(6) << count2;
                      } else
                        os << setw(6) << count2;
                      bitCount++;
                    }
                  }
                  os << endl;
                  while (altBitCount < bitCount) {
                    if ((altBitCount % 12) == 0) {
                      if (altBitCount)
                        os << endl;
                      os << "ORDER " << setw(6) << count1 + 1 << setw(6) << 1;
                    } else
                      os << setw(6) << 1;
                    altBitCount++;
                  }
                  os << endl;
                }
                BitmapDestroy(theConnections);
              }
              count1++;
            }
            
            //  Done:
            os << "END" << endl;
          }
            
          break;
        }
        
        case EFormat_POV: {
          //  Starting in version 3.2 we'll output POV objects!
          TAtomicCoordinate*    positions = NULL;
          unsigned              posCount = 0;
          
          //  Get the propogated cell:
          if (theCell->Propogate(replicate[0],replicate[1],replicate[2],&positions,&posCount,kCrystalCellPropogateCentered)) {
            
            //  Now we just need to keep doing the records in PDB format.  As
            //  we go we figure out the connectivity, too:
            TAtomicCoordinate*    p1 = positions;
            unsigned              count1 = 0;
            
            os << "// (" << n << ',' << m << ") Nanotube";
            if ((replicate[0] > 1) || (replicate[1] > 1) || (replicate[2] > 1))
              os << " (" << replicate[0] << ',' << replicate[1] << ',' << replicate[2] << ") replication";
            os << endl << "// TubeGen 3.3, J T Frey, University of Delaware" << endl << endl;
            
            os << "#declare Nanotube = union {" << endl;
            while (count1 < posCount) {
              unsigned            count2 = count1 + 1;
              TAtomicCoordinate*  p2 = positions + count2;
              TElementSymbol      symbol = periodicTable->LookupSymbolForNumber(p1->atomicNumber);
              
              //  Write the sphere object to the stream first of all:
              os << "  sphere { // C_" << count1 + 1 << endl;
              os << "    <";
              os << setw(8) << setprecision(3) << p1->atomPosition.x << " , ";
              os << setw(8) << setprecision(3) << p1->atomPosition.y << " , ";
              os << setw(8) << setprecision(3) << p1->atomPosition.z << " > , C_Radius" << endl;
              os << "    texture { finish { DefaultFinish } pigment { C_Color } }" << endl;
              os << "  }" << endl;
              
              //  Now let's play the connectivity game:
              if (count1 + 1 < posCount) {
                while (count2 < posCount) {
                  //  How far is positions[count2] from positions[count1]?
                  if (Point3D_Distance(&p1->atomPosition,&p2->atomPosition) <=  1.2 * bondLen) {
                    os << "  cylinder { // C_" << count1 + 1 << ":C_" << count2 + 1 << endl;
                    os << "    <";
                    os << setw(8) << setprecision(3) << p1->atomPosition.x << " , ";
                    os << setw(8) << setprecision(3) << p1->atomPosition.y << " , ";
                    os << setw(8) << setprecision(3) << p1->atomPosition.z << " >," << endl;
                    os << "    <";
                    os << setw(8) << setprecision(3) << p2->atomPosition.x << " , ";
                    os << setw(8) << setprecision(3) << p2->atomPosition.y << " , ";
                    os << setw(8) << setprecision(3) << p2->atomPosition.z << " >," << endl;
                    os << "    BondRadius" << endl << "    open" << endl;
                    os << "    texture { finish { DefaultFinish } pigment { C_Color }}" << endl;
                    os << "  }" << endl;
                  }
                  count2++;
                  p2++;
                }
              }
              count1++;
              p1++; 
            }
            os << "} // Nanotube" << endl;
          }
            
          break;
        }
      }
      os.setf(savedFlags);
    }
  }

//

  int
  TubuleBasis::Get_n()
  {
    return n;
  }
  
  void
  TubuleBasis::Set_n(int newn)
  {
    if ((newn != n) && (newn >= 0)) {
      n = newn;
      
      CalculateGraphiticBasisVectors();
      CalculateTranslationalIndices();
      OptimizeGammas();
    }
  }
  
//

  int
  TubuleBasis::Get_m()
  {
    return m;
  }
  
  void
  TubuleBasis::Set_m(int newm)
  {
    if ((newm != m) && (newm >= 0)) {
      m = newm;
      
      CalculateGraphiticBasisVectors();
      CalculateTranslationalIndices();
      OptimizeGammas();
    }
  }
  
//
  
  void
  TubuleBasis::Set_nandm(int newn,int newm)
  {
    if ((newn == n) && (newm == m))
      return;
    
    if ((newn >= 0) && (newm >= 0)) {
      n = newn;
      m = newm;
      
      CalculateGraphiticBasisVectors();
      CalculateTranslationalIndices();
      OptimizeGammas();
    }
  }
  
//

  unsigned
  TubuleBasis::Get_element(
    unsigned    which
  )
  {
    if (which < 2)
      return element[which];
    return 0;
  }

//

  void
  TubuleBasis::Set_element(
    unsigned    which,
    unsigned    atomicNumber
  )
  {
    if (which < 2)
      element[which] = atomicNumber;
  }

//

  int
  TubuleBasis::Get_nprime()
  {
    return nprime;
  }
  
//

  int
  TubuleBasis::Get_mprime()
  {
    return mprime;
  }
  
//
  
  double
  TubuleBasis::Get_bond()
  {
    return bondLen;
  }
  
  void
  TubuleBasis::Set_bond(double bondLength)
  {
    if (bondLength > 1.0) {
      bondLen = bondLength;
      
      CalculateGraphiticBasisVectors();
      CalculateTubuleCellVectors();
      OptimizeGammas();
    }
  }
  
//

  void
  TubuleBasis::Get_a1(TVector3D* v)
  {
    if (v)
      *v = a1;
  }
  
//

  void
  TubuleBasis::Get_a2(TVector3D* v)
  {
    if (v)
      *v = a2;
  }
  
//

  void
  TubuleBasis::Get_Ch(TVector3D* v)
  {
    if (v)
      *v = Ch;
  }
  
//

  void
  TubuleBasis::Get_T(TVector3D* v)
  {
    if (v)
      *v = T;
  }
  
//

  double
  TubuleBasis::Get_gutter(int which)
  {
    switch (which) {
      case EDirection_X:
        return gutter.x;
      case EDirection_Y:
        return gutter.y;
      case EDirection_Z:
        return gutter.z;
    }
    return HUGE_VAL;
  }

  void
  TubuleBasis::Set_gutter(int which,double val)
  {
    if (val >= 0.0) {
      switch (which) {
        case EDirection_X:
          gutter.x = val;
          break;
        case EDirection_Y:
          gutter.y = val;
          break;
        case EDirection_Z:
          gutter.z = val;
          break;
      }
    }
  }
  
//

  int
  TubuleBasis::Get_replication(int which)
  {
    switch (which) {
      case EDirection_X:
        return replicate[0];
      case EDirection_Y:
        return replicate[1];
      case EDirection_Z:
        return replicate[2];
    }
    return 0;
  }

  void
  TubuleBasis::Set_replication(int which,int val)
  {
    if (val > 0) {
      switch (which) {
        case EDirection_X:
          replicate[0] = val;
          break;
        case EDirection_Y:
          replicate[1] = val;
          break;
        case EDirection_Z:
          replicate[2] = val;
          break;
      }
    }
  }
  
//

  int
  TubuleBasis::GetOption_Units()
  {
    return oUnits;
  }
  
  void
  TubuleBasis::SetOption_Units(int val)
  {
    switch (val) {
      case EUnits_Angstrom:
        oUnits = val;
        cUnitConversion = 1.0;
        break;
      case EUnits_Bohr:
        oUnits = val;
        cUnitConversion = 1.0 / ANGSTROM_PER_BOHR;
        break;
    }
  }
  
  double
  TubuleBasis::GetUnitConversionFactor()
  {
    return cUnitConversion;
  }
  
//

  int
  TubuleBasis::GetOption_Format()
  {
    return oFormat;
  }
  
  void
  TubuleBasis::SetOption_Format(int val)
  {
    if ((val >= EFormat_Gaussian) && (val < EFormat_Max))
      oFormat = val;
  }
  
//

  bool
  TubuleBasis::SetOption_Format(
    const char* formatString
  )
  {
    int   formatID = __TubuleBasis_FormatIDForString(formatString);
    
    if (formatID != EFormat_Undefined) {
      oFormat = formatID;
      return true;
    }
    return false;
  }
  
//

  const char*
  TubuleBasis::StringForFormat()
  {
    return __TubuleBasis_StringForFormatID(oFormat);
  }

//

  const char*
  TubuleBasis::FileExtensionsForFormat()
  {
    return __TubuleBasis_FileExtensionForFormatID(oFormat);
  }

//

  int
  TubuleBasis::GetOption_Lattice()
  {
    return oLattice;
  }
  
  void
  TubuleBasis::SetOption_Lattice(int val)
  {
    if (val == oLattice)
      return;
      
    switch (val) {
      case ELattice_Hexagonal:
      case ELattice_Cubic:
        if (oLattice == ELattice_Planar)
          OptimizeGammas();
        oLattice = val;
        break;
      
      case ELattice_Planar:
        CalculateGraphiticBasisVectors();
        CalculateTubuleCellVectors();
        oLattice = val;
        break;
    }
  }
  
//

  bool
  TubuleBasis::GetOption_Verbose()
  {
    return oVerbose;
  }
  
  void
  TubuleBasis::SetOption_Verbose(bool val)
  {
    oVerbose = val;
  }
  
//
  
  bool
  TubuleBasis::GetOption_RelaxTubule()
  {
    return oRelaxTubule;
  }
  
  void
  TubuleBasis::SetOption_RelaxTubule(bool val)
  {
    if (val != oRelaxTubule) {
      oRelaxTubule = val;
      
      CalculateGraphiticBasisVectors();
      CalculateTubuleCellVectors();
      OptimizeGammas();
    }
  }
  
//
  
  double
  TubuleBasis::GetOption_RadiusConv()
  {
    return oRadiusConv;
  }
  
  void
  TubuleBasis::SetOption_RadiusConv(double val)
  {
    oRadiusConv = fabs(val);
  }
  
//
  
  double
  TubuleBasis::GetOption_ErrorConv()
  {
    return oErrorConv;
  }
  
  void
  TubuleBasis::SetOption_ErrorConv(double val)
  {
    oErrorConv = fabs(val);
  }
  
//
  
  double
  TubuleBasis::GetOption_GammaConv()
  {
    return oGammaConv;
  }
  
  void
  TubuleBasis::SetOption_GammaConv(double val)
  {
    oGammaConv = fabs(val);
  }
  
//
////
//

	void
	TubuleBasis::InitializeBasis(
		int			nval,
		int			mval
	)
	{
    n               = nval;
    m               = mval;
    bondLen         = STD_BOND;
    bondLenScale    = 1.0;
    
    //  Start out with carbon for both:
    element[0] = element[1] = 6;
    
    //  Default options:
    oUnits          = EUnits_Angstrom;
    oFormat         = EFormat_Gaussian;
    oLattice        = ELattice_Hexagonal;
    oVerbose        = false;
    oRelaxTubule    = true;
    oNonOrthProj    = false;
    oRadiusConv     = STD_RADIUS_CONVERGENCE;
    oErrorConv      = STD_ERROR_CONVERGENCE;
    oGammaConv      = STD_GAMMA_CONVERGENCE;
    cUnitConversion = 1.0;
    
    //  Define the graphitic basis, a1 and a2:
    CalculateGraphiticBasisVectors();
    
    //  Calculate nprime and mprime (which gets Ch and T in the process):
    CalculateTranslationalIndices();
    OptimizeGammas();
    
    //  Default gutters and replication counts:
    Vector3D_Set(&gutter,STD_GUTTER_LENGTH,STD_GUTTER_LENGTH,0.0);
    replicate[0] = replicate[1] = replicate[2] = 1;
	}

//

  void
  TubuleBasis::CalculateGraphiticBasisVectors()
  {
    double    x,y;
    
    if (oVerbose)
      printf(" --- Construction of Graphitic Atomic-Basis Vectors ---------------------------\n");
    
    x = 1.5 * bondLen;
    y = 0.5 * sqrt(3.0) * bondLen;
    Vector3D_Set(&a1,x,y,0.0);
    Vector3D_Set(&a2,x,-y,0.0);
    
    if (oVerbose) {
      printf("  Nearest neighbor bonding distance as:  %lg\n",bondLen * cUnitConversion);
      printf("    a1 = < %lg , %lg >\n",a1.x * cUnitConversion,a1.y * cUnitConversion);
      printf("    a2 = < %lg , %lg >\n",a2.x * cUnitConversion,a2.y * cUnitConversion);
      printf(" ------------------------------------------------------------------------------\n\n");
    }
  }

//

  void
  TubuleBasis::CalculateTranslationalIndices()
  {
    //  Calculate d and dR:
    d = gcd(n,m);
    if (((n - m) % (3 * d)) == 0)
      dR = 3 * d;
    else
      dR = d;
    
    //  Calculate nprime and mprime:
    nprime = (2 * m + n) / dR;
    mprime = (2 * n + m) / dR;
    
    CalculateTubuleCellVectors();
  }

//

  void
  TubuleBasis::CalculateTubuleCellVectors()
  {
    double      oneOverV;
    
    if (oVerbose) {
      printf(" --- Construction of Chiral/Tubule Translation Vectors ------------------------\n");
      printf("  n = (%3d) and m = (%3d):\n",n,m);
      printf("  n'= (%3d) and m'= (%3d):\n",nprime,mprime);
    }
    
    //  Calculate Ch:
    Vector3D_Scalar(&a1,(double)n,&Ch);
    Vector3D_ScaledSum(&Ch,(double)m,&a2,&Ch);
    Vector3D_Rezero(&Ch,FLT_EPSILON);
    //  Store the length of Ch:
    lenCh = Vector3D_Magnitude(&Ch);
    if (oVerbose) {
      double    Ch_proj_a1 = Vector3D_Dot(&a1,&Ch) / ( Vector3D_Magnitude(&a1) * lenCh );
      
      if (Ch_proj_a1 < 0.)
        Ch_proj_a1 = Vector3D_Dot(&a2,&Ch) / ( Vector3D_Magnitude(&a2) * lenCh );
      if (fabs(Ch_proj_a1 - 1.) < 1e-3)
        Ch_proj_a1 = 0;
      else
        Ch_proj_a1 = acos(Ch_proj_a1);
      printf("  Chiral vector Ch constructed as %d(a1) + %d(a2):\n",n,m);
      printf("    Ch = < %lg , %lg >, |Ch| = %lg\n",Ch.x * cUnitConversion, \
                                                    Ch.y * cUnitConversion, \
                                                    lenCh * cUnitConversion);
      printf("  Chiral angle is %lg degrees\n",Ch_proj_a1 * kRadiansToDegrees);
    }
    
    //  Calculate T:
    Vector3D_Scalar(&a1,(double)nprime,&T);
    Vector3D_ScaledSum(&T,(double)(-mprime),&a2,&T);
    Vector3D_Rezero(&T,FLT_EPSILON);
    //  Store the length of T:
    lenT = Vector3D_Magnitude(&T);
    if (oVerbose) {
      printf("  Tubule translation vector T constructed as %d(a1) - %d(a2):\n",nprime,mprime);
      printf("    T = < %lg , %lg >, |T| = %lg\n",T.x * cUnitConversion, \
                                                  T.y * cUnitConversion,  \
                                                  lenT * cUnitConversion);
    }
    
    //  Calculate r:
    r = 0.5 * M_1_PI * lenCh;
    if (oVerbose)
      printf("  Tubule radius: %g\n",r * cUnitConversion);
    
    //  Calculate Tprime and h; project Ch out of T and then find length:
    oneOverV = Vector3D_Dot(&T,&Ch) / (lenCh * lenCh);
    Vector3D_ScaledSum(&T,-oneOverV,&Ch,&Tprime);
    Vector3D_Rezero(&Tprime,FLT_EPSILON);
    h = Vector3D_Magnitude(&Tprime);
    if (oVerbose)
      printf("  Tubule height: %g\n",h * cUnitConversion);
    
    //  Calculate the two basis vectors for the reciprocal space of
    //  Ch and T (used for projections).  The third vector we use is
    //  just z, since both Ch and T are in the xy-plane.
    //
    //  V    = Ch . (T x z)
    //  Ch_R = (T x z) / V
    //  T_R  = (z x Ch) / V
    //  z_R  = < 0 , 0 , -1 >
    //
    oneOverV = 1.0 / ( T.y * Ch.x - T.x * Ch.y );  //  1 / V
    Vector3D_Set(&Ch_R,oneOverV * T.y,-oneOverV * T.x,0.0);
    Vector3D_Set(&T_R,-oneOverV * Ch.y,oneOverV * Ch.x,0.0);
    
    if (oVerbose)
      printf(" ------------------------------------------------------------------------------\n\n");
  }
  
//

  double
  TubuleBasis::CalculateError(
    int                 vec,
    double              theta
  )
  {
    double      accum;
    double      tmp1,tmp2;
    
    tmp1 = lenvec[vec];
    accum = tmp1 * tmp1;      //  lenA[i]^2
    
    tmp1 = r;
    tmp1 *= tmp1;                         //  r^2
    tmp1 *= 2.0;                          //  2 r^2
    tmp2 = 1.0 - cos(theta);              //  1 - cos(theta)
    accum -= tmp1 * tmp2;     //  lenA[i]^2 - 2 r^2 (1 - cos(theta))
    
    tmp1 = gamma[vec];
    tmp1 *= tmp1;                         //  gamma[i]^2
    tmp2 = Vector3D_Dot(&optvec[vec],&Tprime) / h;
    tmp2 *= tmp2;
    accum -= tmp1 * tmp2;     //  lenA[i]^2 - 2 r^2 (1 - cos(theta)) - gamma[i]^2 (<a[i],T>/|T|)^2
    
    return accum;
  }
  
//

  double
  TubuleBasis::CalculateErrorDerivative(
    int                 vec,
    double              theta
  )
  {
    double      accum;
    double      tmp1,tmp2,tmp3;
    
    tmp1 = r;
    tmp1 *= tmp1;                         //  r^2
    tmp1 *= 8.0 * M_PI;                   //  8 pi r^2
    tmp1 *= sin(theta);                   //  8 pi r^2 sin(theta)
    tmp3 = 1.0 / lenCh;
    tmp3 *= tmp3;                         //  1 / |Ch|^2
    tmp2 = Vector3D_Dot(&optvec[vec],&Ch) * tmp3;
    accum = -tmp1 * tmp2;
    
    tmp1 = 4.0 * gamma[vec];
    tmp2 = Vector3D_Dot(&optvec[vec],&Tprime) / h;
    tmp2 *= tmp2;
    accum -= tmp1 * tmp2;
    
    accum *= CalculateError(vec,theta);
    
    return accum;
  }

//

  double
  TubuleBasis::CalculateTheta(
    int                 vec
  )
  {
    double        tmp1 = 2.0 * M_PI * gamma[vec];
    double        tmp2 = Vector3D_Dot(&optvec[vec],&Ch);
    double        tmp3 = 1.0 / lenCh;
    
    tmp3 *= tmp3;
    return tmp1 * tmp2 * tmp3;
  }

//

  void
  TubuleBasis::OptimizeGammas()
  {
    int                 rIters = 0;
    double              delta_r = 0.0,oldr = r,delta_h = 0.0,oldh = h,ChDotT;
    bool                savedVerbosity = oVerbose;
    
    if ((oRelaxTubule) && (oLattice != ELattice_Planar)) {
      if (oVerbose) {
        printf(" --- Relaxing tubule to appropriate bond lengths ------------------------------\n");
        printf("                                                  delta-radius          %1.0lg\n",oRadiusConv);
        printf("                                               scaling-factors          %1.0lg\n",oGammaConv);
        printf("                                                error-function          %1.0lg\n",oErrorConv);
        printf("  ===========================================================================\n");
        printf("                                                                   Iterations\n");
        printf("  %-4s %-12s %-12s %-32s %3s %3s %3s\n","Iter","    delta-h","    delta-r","    Gammas"," a1"," a2"," a3");
        printf("  ===========================================================================\n");
      }
      
      //  We don't wanna see a butt-load of printed info
      //  each time we reset the chiral/translational vectors,
      //  so we turn off printing after remembering its
      //  current state:
      oVerbose = false;
        
      gamma[0] = gamma[1] = gamma[2] = 1.0;
      
      //  Setup the vectors that we'll be optimizing:
      Vector3D_Set(&optvec[0],bondLen,0.0,0.0);
      optvec[1] = a1;
      Vector3D_Diff(&optvec[1],&optvec[0],&optvec[1]);
      optvec[2] = a2;
      Vector3D_Diff(&optvec[2],&optvec[0],&optvec[2]);
      Vector3D_Rezero(&optvec[1],FLT_EPSILON);
      Vector3D_Rezero(&optvec[2],FLT_EPSILON);
    
      lenvec[0] = bondLen;
      lenvec[1] = Vector3D_Magnitude(&optvec[1]);
      lenvec[2] = Vector3D_Magnitude(&optvec[2]);
      
      do {
        int               vec;
        
        if (savedVerbosity) {
          if (rIters)          
            printf("  %-4d %12.4e %12.4le %10.6f %10.6f %10.6f ",rIters,delta_h,delta_r,gamma[0],gamma[1],gamma[2]);
          else
            printf("  %-4d                           %10.6f %10.6f %10.6f ",rIters,gamma[0],gamma[1],gamma[2]);
        }
        
        rIters++;
        
        //  Optimize gamma[i]:
        for ( vec = 0 ; vec < 3 ; vec++ ) {
          double          theta,error,dgamma = 1.0;
          int             gIters = 0;
          
          theta = CalculateTheta(vec);
          error = CalculateError(vec,theta);
          error *= error;
          while ((error > oErrorConv) && (fabs(dgamma) > oGammaConv)) {
            dgamma = error / CalculateErrorDerivative(vec,theta);
            gamma[vec] -= 0.5 * dgamma;
            theta = CalculateTheta(vec);
            error = CalculateError(vec,theta);
            error *= error;
            gIters++;
          }
          if (savedVerbosity)
            printf("%3d ",gIters);
        }
        if (savedVerbosity)
          printf("\n");
        
        //  Calculate new a1 and a2 and bondLenScale:
        bondLenScale = gamma[0];
        Vector3D_Set(&a1,bondLen * bondLenScale,0.0,0.0);
        Vector3D_ScaledSum(&a1,gamma[1],&optvec[1],&a1);
        Vector3D_Rezero(&a1,FLT_EPSILON);
        Vector3D_Set(&a2,bondLen * bondLenScale,0.0,0.0);
        Vector3D_ScaledSum(&a2,gamma[2],&optvec[2],&a2);
        Vector3D_Rezero(&a2,FLT_EPSILON);
        
        //  Calculate new Ch, T, h, and r:
        delta_r = r;
        delta_h = h;
        CalculateTubuleCellVectors();
        delta_r = r - delta_r;
        delta_h = h - delta_h;
      } while ((fabs(delta_r) > oRadiusConv) || ((fabs(delta_h) > oRadiusConv)));
      
      //  Check for orthogonality:
      ChDotT = Vector3D_Dot(&T,&Ch);
      if (fabs(ChDotT) > FLT_EPSILON)
        oNonOrthProj = true;
      else
        oNonOrthProj = false;
      
      //  Restore verbosity:
      oVerbose = savedVerbosity;
      
      //  Print information if necessary:
      if (oVerbose) {
        if (rIters)          
          printf("  %-4d %12.4e %12.4e %10.6f %10.6f %10.6f\n",rIters,delta_h,delta_r,gamma[0],gamma[1],gamma[2]);
        printf("  ===========================================================================\n");
        printf("  Convergence reached");
        if (rIters > 0) {
          printf(" in %d cycle",rIters);
          if (rIters > 1)
            printf("s");
        } else {
          printf(" immediately.");
        }
        printf("\n  New graphitic basis:\n");
        printf("    a1 = < %lg , %lg >\n",a1.x * cUnitConversion,a1.y * cUnitConversion);
        printf("    a2 = < %lg , %lg >\n",a2.x * cUnitConversion,a2.y * cUnitConversion);
        printf("    cc-bond = %lg\n",bondLen * bondLenScale * cUnitConversion);
        printf("  New chiral/tubule translation vectors:\n");
        printf("    Ch = < %lg , %lg >, |Ch| = %lg\n",Ch.x * cUnitConversion, \
                                                      Ch.y * cUnitConversion, \
                                                      lenCh * cUnitConversion);
        printf("    T = < %lg , %lg >, |T| = %lg\n",T.x * cUnitConversion, \
                                                    T.y * cUnitConversion,  \
                                                    lenT * cUnitConversion);
        printf("  Tubule radius: %lg     [total delta-r of %lg]\n",r * cUnitConversion,(r - oldr) * cUnitConversion);
        printf("  Tubule height: %lg     [total delta-h of %lg]\n",h * cUnitConversion,(h - oldh) * cUnitConversion);
        printf("  Angle between Ch and T:  %lg degrees\n",kRadiansToDegrees * acos( ChDotT ) );
        printf(" ------------------------------------------------------------------------------\n\n");
      }
    }
  }
  
//
