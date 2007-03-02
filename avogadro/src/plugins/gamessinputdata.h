/**********************************************************************
  GamessInputData - GAMESS Input Data Class

  Copyright (C) 2004 by Iowa State University
  Copyright (C) 2006 by Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on WxMacMolPlt
  For more information, see <http://www.scl.ameslab.gov/~brett/MacMolPlt/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef __GAMESSINPUTDATA_H
#define __GAMESSINPUTDATA_H

#include <iostream>
#include <avogadro/primitives.h>

using namespace std;
namespace Avogadro {

  enum TypeOfRun {
    InvalidRunType=0,
    Energy=1,
    GradientRun=2,
    HessianRun,
    OptimizeRun=4,
    TrudgeRun,
    SadPointRun,
    IRCRun=7,
    GradExtrRun,
    DRCRun=9,
    SurfaceRun,
    PropRun,
    MorokumaRun,
    TransitnRun,
    SpinOrbitRun,
    FFieldRun,
    TDHFRun,
    GLOBOPRun=17,
    VSCFRun,
    OptFMORun,
    RamanRun,
    NMRRun,
    MakeEFPRun,

    NumGAMESSRunTypes
  };

  enum GAMESS_SCFType {
    GAMESS_Invalid_SCFType=-1,
    GAMESSDefaultSCFType=0,
    GAMESS_RHF,
    GAMESS_UHF,
    GAMESS_ROHF,
    GAMESS_GVB,
    GAMESS_MCSCF,
    GAMESS_NO_SCF,

    NumGAMESSSCFTypes
  };

  enum CCRunType {
    CC_None=0,
    CC_LCCD,
    CC_CCD,
    CC_CCSD,
    CC_CCSDT,
    CC_RCC,
    CC_CRCC,
    CC_EOMCCSD,
    CC_CREOM,

    NumCCTypes
  };

  enum CIRunType {
    CI_None=0,
    CI_GUGA,
    CI_ALDET,
    CI_ORMAS,
    CI_CIS,
    CI_FSOCI,
    CI_GENCI,

    NumCITypes
  };

  class GamessInputData;

  enum FriendType {
    Friend_invalid=-1,
    Friend_None=0,
    Friend_HONDO,
    Friend_MELDF,
    Friend_GAMESSUK,
    Friend_GAUSSIAN,
    Friend_ALL,

    NumFriendTypes
  };

  enum GAMESS_Localization {
    Invalid_Localization=-1,
    GAMESS_No_Localization=0,
    GAMESS_BOYS_Localization,
    GAMESS_RUEDNBRG_Localization,
    GAMESS_POP_Localization,

    NumGAMESSLocalizations
  };

  class GamessControlGroup {
    private:
      char		*ExeType;
      GAMESS_SCFType	SCFType;
      short		MPLevelCIType;	//first 4 bits = MP level, 2nd 4 bits CI Type
      TypeOfRun	RunType;
      short		MaxIt;
      short		Charge;
      short		Multiplicity;
      GAMESS_Localization	Local;
      FriendType	Friend;
      short		NPrint;
      short		ITol;
      short		ICut;
      CCRunType	CCType;
      char		Options;		//1 bit for several options
      //bit 1		MolPlot;
      //bit 2		PlotOrb;
      //bit 3		AIMPAC;
      //bit 4		RPAC;
      //bit 5		DFT active;
      //bit 6		IntType;
      //bit 7		NormF;
      //bit 8		NormP;
    public:		//Public data access functions
      //other member functions
      GamessControlGroup(void);
      GamessControlGroup(GamessControlGroup *Copy);
      ~GamessControlGroup(void);

      GAMESS_SCFType SetSCFType(GAMESS_SCFType NewSCFType);
      GAMESS_SCFType SetSCFType(const char *SCFText);
      const char * GetSCFTypeText(void) const {return GAMESSSCFTypeToText(SCFType);};
      static const char * GAMESSSCFTypeToText(GAMESS_SCFType t);
      inline GAMESS_SCFType GetSCFType(void) const {return SCFType;};
      long SetMPLevel(short NewMPLevel);
      short GetMPLevel(void) const;
      CIRunType GetCIType(void) const;
      CIRunType GetCIType(char * outText) const;
      const char * GetCIType(const CIRunType &) const;
      CIRunType SetCIType(CIRunType NewVal);
      CIRunType SetCIType(const char * CIText);
      TypeOfRun SetRunType(const TypeOfRun & NewRunType);
      TypeOfRun SetRunType(const char *RunText);
      static const char * GetGAMESSRunText(const TypeOfRun & r);
      inline TypeOfRun GetRunType(void) const {return RunType;};
      short GetExeType(void);
      short GetExeType(unsigned char ExeText[256]);
      short SetExeType(const char *ExeText);
      short SetExeType(short NewType);
      short SetMaxIt(short NumIt);
      inline short GetMaxIt(void) {return MaxIt;};
      short SetCharge(short charge);
      inline short GetCharge(void) {return Charge;};
      short SetMultiplicity(short NewMult);
      inline short GetMultiplicity(void) {return Multiplicity;};
      GAMESS_Localization SetLocal(const char *LocalText);
      GAMESS_Localization SetLocal(GAMESS_Localization NewLocal);
      inline GAMESS_Localization GetLocal(void) const {return Local;};
      static const char * GAMESSLocalizationToText(GAMESS_Localization t);
      const char * GetLocalText(void) const {return GAMESSLocalizationToText(Local);};
      FriendType SetFriend(const char *FriendText);
      FriendType SetFriend(FriendType NewFriend);
      static const char * GetFriendText(FriendType f);
      static FriendType TextToFriend(const char * c);
      inline FriendType GetFriend(void) const {return Friend;};
      inline const char * GetFriendText(void) const {return GetFriendText(Friend);};
      bool SetMolPlot(bool State);
      inline bool GetMolPlot(void) const {return ((Options & 1)?true:false);};
      bool SetPlotOrb(bool State);
      inline bool GetPlotOrb(void) const {return ((Options & (1<<1))?true:false);};
      bool SetAIMPAC(bool State);
      inline bool GetAIMPAC(void) const {return ((Options & (1<<2))?true:false);};
      bool SetRPAC(bool State);
      inline bool GetRPAC(void) const {return ((Options & (1<<3))?true:false);};
      bool UseDFT(bool state);
      bool UseDFT(void) const;
      bool SetIntType(bool State);
      inline bool GetIntType(void) const {return ((Options & (1<<5))?true:false);};
      bool SetNormF(bool State);
      inline bool GetNormF(void) const {return ((Options & (1<<6))?true:false);};
      bool SetNormP(bool State);
      inline bool GetNormP(void) const {return ((Options & (1<<7))?true:false);};
      CCRunType GetCCType(void) const;
      static const char * GetGAMESSCCType(const CCRunType & );
      CCRunType SetCCType(CCRunType n);
      CCRunType SetCCType(const char * n);

      void InitControlPaneData(void);
      void InitProgPaneData(void);
      
//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer, long length);
      void WriteToFile(ostream &File, GamessInputData *IData, long NumElectrons);
      void RevertControlPane(GamessControlGroup *OrgData);
      void RevertProgPane(GamessControlGroup *OrgData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };

  enum MemoryUnit {
    wordsUnit=1,
    bytesUnit,
    megaWordsUnit,
    megaBytesUnit,
    gigaWordsUnit,
    gigaBytesUnit,

    NumberMemoryUnits
  };
  const char * MemoryUnitToText(const MemoryUnit & mu);
  bool TextToMemoryUnit(const char * t, MemoryUnit & mu);

  enum TimeUnit {
    secondUnit=1,
    minuteUnit,
    hourUnit,
    dayUnit,
    weekUnit,
    yearUnit,
    milleniaUnit,

    NumberTimeUnits
  };
  const char * TimeUnitToText(const TimeUnit & tu);
  bool TextToTimeUnit(const char * t, TimeUnit & tu);

  class GamessSystemGroup {
    private:
      long		TimeLimit; //This is always stored as minutes
      double		Memory; //Always stored as words
      double		MemDDI;	//stored as megawords
      char		KDiag;
      TimeUnit	TimeUnits;
      MemoryUnit	MemUnits;
      MemoryUnit	MemDDIUnits;
      char		Flags;		//One bit flags
      //bit 1		CoreFlag;
      //bit 2		BalanceType; set to true for LOOP
      //bit 3		XDR;
      //bit 4		PARALL;
    public:
      GamessSystemGroup(void);
      GamessSystemGroup(GamessSystemGroup *Copy);

      inline long GetTimeLimit(void) const {return TimeLimit;};
      long SetTimeLimit(long NewTime);
      inline TimeUnit GetTimeUnits(void) const {return TimeUnits;};
      TimeUnit SetTimeUnits(TimeUnit NewUnits);
      float GetConvertedTime(void) const;
      long SetConvertedTime(float NewTime);
      inline double GetMemory(void) const {return Memory;};
      double SetMemory(double NewMemory);
      inline MemoryUnit GetMemUnits(void) const {return MemUnits;};
      MemoryUnit SetMemUnits(MemoryUnit NewUnits);
      double GetConvertedMem(void) const;
      double SetConvertedMem(double NewMem);
      inline MemoryUnit GetMemDDIUnits(void) const {return MemDDIUnits;};
      inline double GetMemDDI(void) const {return MemDDI;};
      double SetMemDDI(double NewMemDDI);
      double GetConvertedMemDDI(void) const;
      double SetConvertedMemDDI(double NewMem);
      MemoryUnit SetMemDDIUnits(MemoryUnit NewUnits);
      inline char GetDiag(void) const {return KDiag;};
      char SetDiag(char NewMethod);
      bool GetCoreFlag(void) const {return ((Flags & 1)?true:false);};
      bool SetCoreFlag(bool State);
      bool GetBalanceType(void) const {return ((Flags & 2)?true:false);};
      bool SetBalanceType(bool Type);
      bool GetXDR(void) const {return ((Flags & 4)?true:false);};
      bool SetXDR(bool State);
      inline bool GetParallel(void) const {return ((Flags & 8)?true:false);};
      bool SetParallel(bool State);

      void InitData(void);
//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer);
      void WriteToFile(ostream &File);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
//       void ReadSystemOptions(BufferFile * Buffer);
  };
  enum GAMESS_BasisSet {
    GAMESS_BS_None=0,
    GAMESS_BS_MINI,
    GAMESS_BS_MIDI,
    GAMESS_BS_STO,
    GAMESS_BS_N21,
    GAMESS_BS_N31,
    GAMESS_BS_N311,
    GAMESS_BS_DZV,
    GAMESS_BS_DH,
    GAMESS_BS_BC,
    GAMESS_BS_TZV,
    GAMESS_BS_MC,
    GAMESS_BS_SBK,
    GAMESS_BS_HW,
    GAMESS_BS_MNDO,
    GAMESS_BS_AM1,
    GAMESS_BS_PM3,

    NumGAMESSBasisSetsItem
  };
  enum GAMESS_BS_Polarization {
    GAMESS_BS_Invalid_Polar=-1,
    GAMESS_BS_No_Polarization=0,
    GAMESS_BS_Pople_Polar,
    GAMESS_BS_PopN311_Polar,
    GAMESS_BS_Dunning_Polar,
    GAMESS_BS_Huzinaga_Polar,
    GAMESS_BS_Hondo7_Polar,

    NumGAMESSBSPolarItems
  };
  enum GAMESS_BS_ECPotential {
    GAMESS_BS_Invalid_ECP=-1,
    GAMESS_BS_ECP_None=0,
    GAMESS_BS_ECP_Read,
    GAMESS_BS_ECP_SBK,
    GAMESS_BS_ECP_HW,

    NumGAMESSBSECPItems
  };
  class GamessBasisGroup {
    private:
      float		Split2[2];
      float		Split3[3];
      short		Basis;
      short		NumGauss;
      short		NumHeavyFuncs;	//8 bits for #D funcs, 8 bits for # f funcs
      short		NumPFuncs;
      GAMESS_BS_Polarization		Polar;
      short		ECPPotential;	//Potential type for ECP calculations (in $CONTRL)
      char		Flags;		//boolean type options
      //bit 1		DiffuseSP;
      //bit 2		DiffuseS;
      //bit 3		disables builtin basis types based on current atom set
    public:		//Member functions
      //other member functions
      GamessBasisGroup(void);
      GamessBasisGroup(GamessBasisGroup *Copy);

      short SetBasis(const char *text);
      short SetBasis(short NewBasis);
      const char * GetBasisText(void) const;
      short GetBasis(void) const;
      static const char * GAMESSBasisSetToText(GAMESS_BasisSet bs);
      short SetNumGauss(short NewNumGauss);
      short GetNumGauss(void) const;
      short SetNumDFuncs(short NewNum);
      short GetNumDFuncs(void) const;
      short SetNumFFuncs(short NewNum);
      short GetNumFFuncs(void) const;
      short SetNumPFuncs(short NewNum);
      short GetNumPFuncs(void) const;
      short SetDiffuseSP(bool state);
      bool GetDiffuseSP(void) const {return ((Flags&1)?true:false);};
      short SetDiffuseS(bool state);
      bool GetDiffuseS(void) const {return ((Flags&2)?true:false);};
      GAMESS_BS_Polarization SetPolar(GAMESS_BS_Polarization NewPolar);
      GAMESS_BS_Polarization SetPolar(const char *text);
      static const char * PolarToText(GAMESS_BS_Polarization p);
      inline const char * GetPolarText(void) const {return PolarToText(Polar);};
      inline GAMESS_BS_Polarization GetPolar(void) const {return Polar;};
      static const char * GAMESSECPToText(GAMESS_BS_ECPotential p);
      short GetECPPotential(void) const;
      const char * GetECPPotentialText(void) const;
      GAMESS_BS_ECPotential SetECPPotential(const char * ECPText);
      short SetECPPotential(short NewType);
      inline bool CheckBasis(void) const {return ((Flags & 4)?true:false);};
      inline void CheckBasis(bool state) {Flags = (Flags& 0xFB) + (state?4:0);};
      void InitData(void);
//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer);
      long WriteToFile(ostream &File, GamessInputData * iData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };

  enum CoordinateType {
    invalidCoordinateType=0,
    UniqueCoordType,
    HINTCoordType,
    CartesianCoordType,
    ZMTCoordType,
    ZMTMPCCoordType,

    NumberCoordinateTypes
  };
  enum GAMESSPointGroup {
    invalidPGroup=0,
    GAMESS_C1,
    GAMESS_CS,
    GAMESS_CI,
    GAMESS_CNH,
    GAMESS_CNV,
    GAMESS_CN,
    GAMESS_S2N,
    GAMESS_DND,
    GAMESS_DNH,
    GAMESS_DN,
    GAMESS_TD,
    GAMESS_TH,
    GAMESS_T,
    GAMESS_OH,
    GAMESS_O,

    NumberGAMESSPointGroups
  };
  class GamessDataGroup {
    private:
      char		*Title;
      short		Coord;
      short		NumZVar;
      char		PointGroup;
      char		PGroupOrder;
      char		Options;
      //bit 1 units
      //bit 2 NoSym
    public:		//data access functions
      //other member functions
      GamessDataGroup(void);
      GamessDataGroup(GamessDataGroup *Copy);
      ~GamessDataGroup(void);

      short SetPointGroup(GAMESSPointGroup NewPGroup);
      short SetPointGroup(char *GroupText);
      static const char * GetGAMESSPointGroupText(GAMESSPointGroup p);
      const char * GetPointGroupText(void) const {return GetGAMESSPointGroupText((GAMESSPointGroup) PointGroup);};
      inline GAMESSPointGroup GetPointGroup(void) const {return (GAMESSPointGroup)PointGroup;};
      short SetPointGroupOrder(short NewOrder);
      inline short GetPointGroupOrder(void) const {return PGroupOrder;};
      short SetTitle(const char *NewTitle, long length=-1);
      const char * GetTitle(void) const;
      CoordinateType GetCoordType(void) const;
      static const char * GetCoordTypeText(CoordinateType t);
      const char * GetCoordText(void) const {return GetCoordTypeText((CoordinateType) Coord);};
      CoordinateType SetCoordType(const char *CoordText);
      CoordinateType SetCoordType(CoordinateType NewType);
      bool GetUnits(void) const {return ((Options&1)?true:false);};
      bool SetUnits(bool NewType);
      bool SetUseSym(bool State);
      bool GetUseSym(void) const {return ((Options&2)?true:false);};
      short GetNumZVar(void) const {return NumZVar;};
      short SetNumZVar(short NewNum);
      void InitData(void);
//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       void ReadFromBuffer(BufferFile *Buffer, long length);
      void WriteToFile(ostream &File, Molecule * MainData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };
  typedef enum TypeOfGuess {
    invalidGuessType=0,
    HUCKELGuessType=1,
    HCOREGuessType,
    MOREADGuessType,
    MOSAVEDGuessType,
    SkipGuessType,

    NumberGuessTypes
  } TypeOfGuess;
  class GamessGuessGroup {
    private:
      float		MOTolZ;
      float		MOTolEquil;
      long		*IOrder;
      long		*JOrder;
      long		NumOrbs;
      short		VecSource;
      short		GuessType;
      char		Options;
      //bit 1 Print initial guess PRTMO
      //bit 2 reordering switch
      //bit 3 orbital mixing switch
    public:		//data access functions
      GamessGuessGroup(void);
      GamessGuessGroup(GamessGuessGroup *Copy);
      //	~GuessGroup(void);	//not needed until iorder and jorder are used

      inline short GetGuess(void) const {return GuessType;};
      const char * GetGuessText(void) const;
      short SetGuess(const char *GuessText);
      inline short SetGuess(short NewGuess) {if ((NewGuess<0)&&(NewGuess>5)) return -1; GuessType = NewGuess; return GetGuess();};
      inline short GetVecSource(void) const {return VecSource;};
      inline void SetVecSource(short NewVal) {if (NewVal>0) VecSource = NewVal;};
      inline long GetNumOrbs(void) const {return NumOrbs;};
      inline long SetNumOrbs(long NewNum) {if (NewNum >= 0) NumOrbs = NewNum; return NumOrbs;};
      inline bool GetPrintMO(void) const {return (Options & 1);};
      inline void SetPrintMO(bool State) {Options = (Options & 0xFE) + (State ? 1 : 0);};
      inline bool GetNOrder(void) const {return ((Options & 2)?true:false);};
      inline void SetNOrder(bool State) {Options = (Options & 0xFD) + (State ? 2 : 0);};
      inline bool GetMix(void) const {return ((Options & 4)?true:false);};
      inline void SetMix(bool State) {Options = (Options & 0xFB) + (State ? 4 : 0);};
      static const char * ConvertGuessType(const int & type);
      //other member functions
      void InitData(void);
//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer);
      void WriteToFile(ostream &File, GamessInputData *IData);
//       void WriteVecGroup(BufferFile *File, Molecule * lData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };
  class GamessSCFGroup {
    private:
      float	SOGTolerance;		//SOSCF threshold
      float	EnErrThresh;		//DIIS error threshold
      float	DEMCutoff;			//DEM limit
      float	DampCutoff;			//Damping limit
      short	ConvCriteria;		//Convergance cutoff 10^(-n)
      short	MaxDIISEq;			//Max size of the DIIS linear equations
      short	MVOCharge;			//Modified Virtual Orbital Charge
      char	Punch;				//Orbital printout flags
      char	Options1;			//Main option flags
      //bit 1 Direct SCF
      //bit 2 Fock matrix diff
      //bit 3 UHF natural orbitals
      char	ConverganceFlags;	//flags for each converger
      // 1 extrapolation
      // 2 damping
      // 3 level shifting
      // 4 restriction
      // 5 DIIS
      // 6 Second order SCF
      // 7 direct energy min.
    public:	//member functions
      GamessSCFGroup(void);
      GamessSCFGroup(GamessSCFGroup *Copy);

      bool GetDirectSCF(void) const {return ((Options1 & 1)?true:false);};
      bool SetDirectSCF(bool State);
      bool GetFockDiff(void) const {return ((Options1 & 2)?true:false);};
      bool SetFockDiff(bool State);
      bool GetUHFNO(void) const {return ((Options1 & 4)?true:false);};
      bool SetUHFNO(bool State);
      short GetConvergance(void) const {return ConvCriteria;};
      short SetConvergance(short NewConv);
      void InitData(void);
      //no destructor for now
//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer);
      void WriteToFile(ostream &File, GamessInputData *IData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };
  class GamessMP2Group {
    private:
      double		CutOff;	//double needed to hold very small cutoffs
      long		NumCoreElectrons;
      long		Memory;
      char		Method;
      char		AOInts;
      char		LMOMP2;
      bool		MP2Prop;
    public:
      GamessMP2Group(void);
      GamessMP2Group(GamessMP2Group *Copy);
      void InitData(void);

      inline float GetIntCutoff(void) const {return CutOff;};
      float SetIntCutoff(float NewCutOff);
      inline long GetNumCoreElectrons(void) const {return NumCoreElectrons;};
      long SetNumCoreElectrons(long NewNum);
      inline long GetMemory(void) const {return Memory;};
      long SetMemory(long NewMem);
      inline char GetMethod(void) const {return Method;};
      char SetMethod(char NewMethod);
      inline char GetAOIntMethod(void) const {return AOInts;};
      const char * GetAOIntMethodText(void) const;
      char SetAOIntMethod(char NewMethod);
      void SetAOIntMethod(const char * t);
      bool GetLMOMP2(void) const;
      bool SetLMOMP2(bool State);
      bool GetMP2Prop(void) const {return MP2Prop;};
      void SetMP2Prop(bool state) {MP2Prop = state;};

//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer);
      void WriteToFile(ostream &File, GamessInputData *IData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };
  class GamessHessianGroup {
    private:
      float	DisplacementSize;
      float	FrequencyScaleFactor;
      long	BitOptions;	//Bit 1: method
      //Bit 2: double difference
      //Bit 3: purify
      //Bit 4: print internal FC's
      //Bit 5: vibrational analysis
    public:
      GamessHessianGroup(void) {InitData();};
      GamessHessianGroup(GamessHessianGroup *Copy) {*this = *Copy;};
      void InitData(void);

      inline float GetDisplacementSize(void) const {return DisplacementSize;};
      inline float SetDisplacementSize(float NewVal) {if (NewVal > 0.0) DisplacementSize = NewVal; return DisplacementSize;};
      inline float GetFreqScale(void) const {return FrequencyScaleFactor;};
      inline float SetFreqScale(float NewVal) {if (NewVal > 0.0) FrequencyScaleFactor = NewVal; return FrequencyScaleFactor;};
      inline bool GetAnalyticMethod(void) const {return (BitOptions & 1);};
      inline void SetAnalyticMethod(bool NewVal) {if (BitOptions & 1) BitOptions--; if (NewVal) {BitOptions++; if (BitOptions & 32) BitOptions -= 32;}};
      inline bool GetDoubleDiff(void) const {return ((BitOptions & 2)?true:false);};
      inline void SetDoubleDiff(bool NewVal) {if (BitOptions & 2) BitOptions-=2; if (NewVal) BitOptions += 2;};
      inline bool GetPurify(void) const {return ((BitOptions & 4)?true:false);};
      inline void SetPurify(bool NewVal) {if (BitOptions & 4) BitOptions -= 4; if (NewVal) BitOptions += 4;};
      inline bool GetPrintFC(void) const {return ((BitOptions & 8)?true:false);};
      inline void SetPrintFC(bool NewVal) {if (BitOptions & 8) BitOptions -= 8; if (NewVal) BitOptions += 8;};
      inline bool GetVibAnalysis(void) const {return ((BitOptions & 16)?true:false);};
      inline void SetVibAnalysis(bool NewVal) {if (BitOptions & 16) BitOptions -= 16; if (NewVal) BitOptions += 16;};

//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer);
      void WriteToFile(ostream &File, GamessInputData *IData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };
  enum DFTFunctionalsGrid {
    invalidDFTGrid=0,
    DFT_Grid_Slater,
    DFT_Grid_Becke,
    DFT_Grid_VWN,
    DFT_Grid_LYP,
    DFT_Grid_SVWN,
    DFT_Grid_BVWN,
    DFT_Grid_BLYP,
    DFT_Grid_B3LYP,
    DFT_Grid_GILL,
    DFT_Grid_PBE,
    DFT_Grid_OP,
    DFT_Grid_SLYP,
    DFT_Grid_SOP,
    DFT_Grid_BOP,
    DFT_Grid_GVWN,
    DFT_Grid_GLYP,
    DFT_Grid_GOP,
    DFT_Grid_PBEVWN,
    DFT_Grid_PBELYP,
    DFT_Grid_PBEOP,
    DFT_Grid_BHHLYP,

    NumberGRIDDFTFuncs
  };
  enum DFTFunctionalsGridFree {
    invalidDFTGridFreeType=0,
    DFT_GridFree_Slater,
    DFT_GridFree_Becke,
    DFT_GridFree_VWN,
    DFT_GridFree_LYP,
    DFT_GridFree_SVWN,
    DFT_GridFree_BVWN,
    DFT_GridFree_BLYP,
    DFT_GridFree_B3LYP,
    DFT_GridFree_XALPHA,
    DFT_GridFree_Depristo,
    DFT_GridFree_CAMA,
    DFT_GridFree_HALF,
    DFT_GridFree_PWLOC,
    DFT_GridFree_BPWLOC,
    DFT_GridFree_CAMB,
    DFT_GridFree_XVWN,
    DFT_GridFree_XPWLOC,
    DFT_GridFree_SPWLOC,
    DFT_GridFree_WIGNER,
    DFT_GridFree_WS,
    DFT_GridFree_WIGEXP,

    NumberDFTGridFreeFuncs
  };
  class GamessDFTGroup {
    private:
      float	GridSwitch;
      float	Threshold;
      short	Functional;
      short	NumRadialGrids;
      short	NumThetaGrids;
      short	NumPhiGrids;
      short	NumRadialGridsInit;
      short	NumThetaGridsInit;
      short	NumPhiGridsInit;
      char	BitFlags;
    public:
      GamessDFTGroup(void) {InitData();};
      GamessDFTGroup(GamessDFTGroup *Copy) {*this = *Copy;};
      void InitData(void);

      bool MethodGrid(void) const {return ((BitFlags & 1) != 0);};
      void SetMethodGrid(bool state) {BitFlags = (BitFlags & 0xE) + (state ? 1 : 0);};
      bool GetAuxFunctions(void) const {return ((BitFlags & 2) != 0);};
      void SetAuxFunctions(bool state) {BitFlags = (BitFlags & 0xD) + (state ? 2 : 0);};
      bool GetThree(void) const {return ((BitFlags & 4) != 0);};
      void SetThree(bool state) {BitFlags = (BitFlags & 0xB) + (state ? 4 : 0);};
      inline short GetFunctional(void) const {return Functional;};
      const char * GetFunctionalText(void) const;
      short SetFunctional(short newvalue);
      static const char * GetDFTGridFuncText(DFTFunctionalsGrid d);
      static const char * GetDFTGridFreeFuncText(DFTFunctionalsGridFree d);

//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer);
      void WriteToFile(ostream &File, GamessInputData *IData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };
  class GamessStatPtGroup {
    private:
      float		OptConvergance;
      float		InitTrustRadius;
      float		MaxTrustRadius;
      float		MinTrustRadius;
      float		StatJumpSize;
      long		ModeFollow;
      long		BitOptions;	//1: radius update; 2: Stat. Pt.; 3-5 Hess method; 6: print orbs every iter.
      short		method;
      short		MaxSteps;
      short		nRecalcHess;
    public:
      GamessStatPtGroup(void) {InitData();};
      GamessStatPtGroup(GamessStatPtGroup *Copy) {*this = *Copy;};
      void InitData(void);

      inline float GetOptConvergance(void) const {return OptConvergance;};
      inline void SetOptConvergance(float NewVal) {if (NewVal>0.0) OptConvergance = NewVal;};
      inline float GetInitRadius(void) const {return InitTrustRadius;};
      inline void SetInitRadius(float NewVal) {if (NewVal>0.0) InitTrustRadius = NewVal;};
      inline float GetMaxRadius(void) const {return MaxTrustRadius;};
      inline void SetMaxRadius(float NewVal) {if (NewVal>0.0) MaxTrustRadius = NewVal;};
      inline float GetMinRadius(void) const {return MinTrustRadius;};
      inline void SetMinRadius(float NewVal) {if (NewVal>0.0) MinTrustRadius = NewVal;};
      inline float GetStatJump(void) const {return StatJumpSize;};
      inline void SetStatJump(float NewVal) {if (NewVal>0.0) StatJumpSize = NewVal;};
      inline long GetModeFollow(void) const {return ModeFollow;};
      inline void SetModeFollow(long NewVal) {if (NewVal>0) ModeFollow = 1;};
      inline short GetMethod(void) const {return method;};
      inline void SetMethod(short NewVal) {if ((NewVal>0)&&(NewVal<=5)) method = NewVal;};
      inline short GetMaxSteps(void) const {return MaxSteps;};
      inline void SetMaxSteps(short NewVal) {if (NewVal>0) MaxSteps = NewVal;};
      inline short GetHessRecalcInterval(void) const {return nRecalcHess;};
      inline void SetHessRecalcInterval(short NewVal) {if (NewVal>=0) nRecalcHess = NewVal;};
      inline bool GetRadiusUpdate(void) const {return (BitOptions & 1);};
      inline void SetRadiusUpdate(bool NewVal) {BitOptions = (BitOptions & 0xFE) + (NewVal ? 1 : 0);};
      inline bool GetStatPoint(void) const {return ((BitOptions & 2) != 0);};
      inline void SetStatPoint(bool NewVal) {BitOptions = (BitOptions & 0xFD) + (NewVal ? 2 : 0);};
      inline short GetHessMethod(void) const {return ((BitOptions & 28) >> 2);};
      inline void SetHessMethod(short NewVal) {if ((NewVal>=1)&&(NewVal<=3)) BitOptions = (BitOptions & 0xE3) + (NewVal << 2);};
      inline bool AlwaysPrintOrbs(void) const {return ((BitOptions & 32) != 0);};
      inline void SetAlwaysPrintOrbs(bool NewVal) {BitOptions = (BitOptions & 0xDF) + (NewVal ? 32 : 0);};

//       long GetSize(BufferFile *Buffer);
//       long WriteToBuffer(BufferFile *Buffer);
//       long ReadFromBuffer(BufferFile *Buffer);
      void WriteToFile(ostream &File, GamessInputData *IData);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };

  class GamessInputData {
    public:
      //! Constructors
      GamessInputData(Molecule *molecule);
      GamessInputData(GamessInputData *Copy);

      //! Deconstructors
      ~GamessInputData(void);

      Molecule              *m_molecule;
      GamessControlGroup	*Control;
      GamessSystemGroup		*System;
      GamessBasisGroup		*Basis;
      GamessDataGroup		*Data;
      GamessGuessGroup		*Guess;
      GamessSCFGroup		*SCF;
      GamessMP2Group		*MP2;
      GamessHessianGroup	*Hessian;
      GamessStatPtGroup		*StatPt;
      GamessDFTGroup		*DFT;

//       long GetSize(BufferFile *Buffer);	//returns total size of the Input group and all subgroups
//       long WriteToBuffer(BufferFile *Buffer);	//Pack Input data into the provided buffer
      long WriteInputFile(ostream &buffer);	//Write out an input file for another program (GAMESS etc)

      long GetNumElectrons() const;
      void SetMolecule(Molecule *molecule) { m_molecule = molecule; }

//       void ReadFromBuffer(BufferFile *Buffer, long length);
//       void WriteXML(XMLElement * parent) const;
//       void ReadXML(XMLElement * parent);
  };

}

#endif
