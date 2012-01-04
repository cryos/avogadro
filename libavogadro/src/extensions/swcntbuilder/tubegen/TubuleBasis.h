// ===============================================================================
// TubuleBasis.h                                             �2001-2002, J.T. Frey
// ===============================================================================
// Written:		J.T. Frey, 12.07.2002
// Purpose:		Class that encapsulates the process of generating atomic positions
//						in the nanotube lattice.
//
// Last Mod:	05.Feb.2003:	Removed all ostream::form calls and moved to purely
//													C++ stream operators.

#ifndef __TUBULEBASIS__
#define __TUBULEBASIS__

// Removed for Avogadro:
// #include <stdcxx-includes.h>
//
// Replaced with:

#include "Core3DMath.h"
#include "CrystalCell.h"

using namespace std;

	enum {
	  EUnits_Angstrom			= 0,
		EUnits_Bohr
	};
	
	enum {
    EFormat_Undefined   = -1,
	  EFormat_Gaussian		= 0,
    EFormat_GaussianPBC,
		EFormat_WIEN,
		EFormat_XYZ,
    EFormat_PDB,
    EFormat_PDBPBC,
    EFormat_POV,
    EFormat_BGF,
    EFormat_CIF,
    EFormat_Max
	};
	
	enum {
	  ELattice_Hexagonal	= 0,
		ELattice_Cubic,
		ELattice_Planar
	};
	
	//  Standard bond length (C-C), in Angstrom
	#define STD_BOND                    1.421
	
	//  Standard gutter length, in Angstrom
	#define STD_GUTTER_LENGTH						1.6735
	
	//  Standard convergence values for relaxation of tubule:
	#define STD_RADIUS_CONVERGENCE			1e-15
	#define STD_ERROR_CONVERGENCE				1e-15
	#define STD_GAMMA_CONVERGENCE				1e-15

//

	enum {
	  EDirection_X,
		EDirection_Y,
		EDirection_Z
	};

//

	class TubuleBasis {
		public:
													TubuleBasis();
													TubuleBasis(int,int);
													
			CrystalCell*				GenerateCrystalCell();
			
			void								WriteInputFile(ostream&);
			void								WriteInputFile(ostream&,CrystalCell*);
			
			int									Get_n();
			void								Set_n(int);
			int									Get_m();
			void								Set_m(int);
			void								Set_nandm(int,int);
			int									Get_nprime();
			int									Get_mprime();
      
      unsigned            Get_element(unsigned which);
      void                Set_element(unsigned which,unsigned atomicNumber);
			
			double							Get_bond();
			void								Set_bond(double bondLength);
			
			void								Get_a1(TVector3D*);
			void								Get_a2(TVector3D*);
			
			void								Get_Ch(TVector3D*);
			void								Get_T(TVector3D*);
			
			double							Get_gutter(int);
			void								Set_gutter(int,double);
			
			int									Get_replication(int);
			void								Set_replication(int,int);
			
			int									GetOption_Units();
			void								SetOption_Units(int);
			double							GetUnitConversionFactor();
			
			int									GetOption_Format();
			void								SetOption_Format(int);
      bool                SetOption_Format(const char*);
      const char*         StringForFormat();
      const char*         FileExtensionsForFormat();
      static void         SummarizeFormatStrings()
                            {
                              void __TubuleBasis_FormatStringSummary(void);
                              
                              __TubuleBasis_FormatStringSummary();
                            }
			
			int									GetOption_Lattice();
			void								SetOption_Lattice(int);
			
			bool								GetOption_Verbose();
			void								SetOption_Verbose(bool);
			
			bool								GetOption_RelaxTubule();
			void								SetOption_RelaxTubule(bool);
			
			double							GetOption_RadiusConv();
			void								SetOption_RadiusConv(double);
			
			double							GetOption_ErrorConv();
			void								SetOption_ErrorConv(double);
			
			double							GetOption_GammaConv();
			void								SetOption_GammaConv(double);
	
		private:
			//  Member data:
			double              bondLen,bondLenScale;
			TVector3D           a1,a2,gutter;
			int                 replicate[3];
			int                 n,m,nprime,mprime,d,dR;
			TVector3D           Ch,T,Tprime;
			TVector3D           Ch_R,T_R;
			double              lenCh,lenT,r,h;
      unsigned            element[2];
			//  Options:
			int                 oUnits;
			int                 oFormat;
			int                 oLattice;
			bool                oVerbose;
			bool                oRelaxTubule;
			bool                oNonOrthProj;
			double              oRadiusConv;
			double              oErrorConv;
			double              oGammaConv;
			double              cUnitConversion;
			//  For relaxation routines:
			TVector3D           optvec[3];
			double              gamma[3];
			double              lenvec[3];
			
			void								InitializeBasis(int, int);
			
			void								CalculateGraphiticBasisVectors();
			void								CalculateTranslationalIndices();
			void								CalculateTubuleCellVectors();
			
			double							CalculateError(int,double);
			double							CalculateErrorDerivative(int,double);
			double							CalculateTheta(int);
			void								OptimizeGammas();
	};

#endif
