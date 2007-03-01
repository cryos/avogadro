/*
 *  (c) 2004 Iowa State University
 *      see the LICENSE file in the top level directory
 */

/*	еееееееееееееееееееееееееееееееееееееееее
	GamessInputData.cpp

	Class member functions related to GamessInputData
	Brett Bode - February 1996
	Changed InputeFileData uses to BufferFile calls 8-97
*/

#include "gamessinputdata.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#pragma segment IData

using namespace Avogadro;
	//GamessInputData functions
GamessInputData::GamessInputData(void) {
	//Always create Control, System, Basis, and Data groups
	Control = new GamessControlGroup;
	System = new GamessSystemGroup;
	Basis = new GamessBasisGroup;
	Data = new GamessDataGroup;
	StatPt = new GamessStatPtGroup;
	Guess = new GamessGuessGroup;
	SCF = NULL;
	MP2 = NULL;
	Hessian = NULL;
	DFT = NULL;
}
GamessInputData::GamessInputData(GamessInputData *Copy) {
	//Always create Control, System, Basis, and Data groups
	Control = new GamessControlGroup(Copy->Control);	//Create the new group and copy over the data
	System = new GamessSystemGroup(Copy->System);
	Basis = new GamessBasisGroup(Copy->Basis);
	Data = new GamessDataGroup(Copy->Data);
	StatPt = new GamessStatPtGroup(Copy->StatPt);
	if (Copy->Guess) Guess = new GamessGuessGroup(Copy->Guess);
	else Guess = NULL;
	if (Copy->SCF) SCF = new GamessSCFGroup(Copy->SCF);
	else SCF = NULL;
	if (Copy->MP2) MP2 = new GamessMP2Group(Copy->MP2);
	else MP2 = NULL;
	if (Copy->Hessian) Hessian = new GamessHessianGroup(Copy->Hessian);
	else Hessian = NULL;
	DFT = NULL;
	if (Copy->DFT) DFT = new GamessDFTGroup(Copy->DFT);
}
GamessInputData::~GamessInputData(void) {	//destructor
	if (Control) delete Control;	//simply delete all groups present
	if (System) delete System;
	if (Basis) delete Basis;
	if (Data) delete Data;
	if (Guess) delete Guess;
	if (SCF) delete SCF;
	if (MP2) delete MP2;
	if (Hessian) delete Hessian;
	if (StatPt) delete StatPt;
	if (DFT) delete DFT;
}
long GamessInputData::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessInputData::WriteToBuffer(BufferFile *Buffer) {
	long Pos, length=sizeof(GamessInputData);

	Pos = Buffer->Write((Ptr) &length, sizeof(long));
	Pos += Buffer->Write((Ptr) this, length);

	if (Control) {
		length = 1;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = Control->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += Control->WriteToBuffer(Buffer);
	}
	if (System) {
		length = 2;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = System->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += System->WriteToBuffer(Buffer);
	}
	if (Basis) {
		length = 3;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = Basis->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += Basis->WriteToBuffer(Buffer);
	}
	if (Data) {
		length = 4;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = Data->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += Data->WriteToBuffer(Buffer);
	}
	if (Guess) {
		length = 5;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = Guess->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += Guess->WriteToBuffer(Buffer);
	}
	if (SCF) {
		length = 6;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = SCF->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += SCF->WriteToBuffer(Buffer);
	}
	if (MP2) {
		length = 7;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = MP2->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += MP2->WriteToBuffer(Buffer);
	}
	if (Hessian) {
		length = 8;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = Hessian->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += Hessian->WriteToBuffer(Buffer);
	}
	if (StatPt) {
		length = 9;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = StatPt->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += StatPt->WriteToBuffer(Buffer);
	}
	if (DFT) {
		length = 10;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = DFT->GetSize(Buffer);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += DFT->WriteToBuffer(Buffer);
	}
		
	return Pos;
}
void GamessInputData::WriteXML(XMLElement * parent) const {
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_InputOptionsElement));
	
	if (Control) {
		Control->WriteXML(Ele);
	}
	if (System) {
		System->WriteXML(Ele);
	}
	if (Basis) {
		Basis->WriteXML(Ele);
	}
	if (Data) {
		Data->WriteXML(Ele);
	}
	if (Guess) {
		Guess->WriteXML(Ele);
	}
	if (SCF) {
		SCF->WriteXML(Ele);
	}
	if (MP2) {
		MP2->WriteXML(Ele);
	}
	if (Hessian) {
		Hessian->WriteXML(Ele);
	}
	if (StatPt) {
		StatPt->WriteXML(Ele);
	}
	if (DFT) {
		DFT->WriteXML(Ele);
	}
}
void GamessInputData::ReadXML(XMLElement * parent) {
	XMLElementList * ipxml = parent->getChildren();
	if (ipxml) {
		if (ipxml->length() > 0) {
			XMLElement * ipx = ipxml->item(0);
			if (ipx) {
				if (!strcmp(ipx->getName(),CML_convert(MMP_InputOptionsElement))) {
					XMLElementList * children = ipx->getChildren();
					if (children) {
						for (int i=0; i<children->length(); i++) {
							XMLElement * child = children->item(i);
							MMP_InputOptionsNS IOchild;
							if (CML_convert(child->getName(), IOchild)) {
								switch (IOchild) {
									case MMP_IOGamessControlGroupElement:
										if (Control == NULL) Control = new GamessControlGroup;
										if (Control) Control->ReadXML(child);
										break;
									case MMP_IOGamessSystemGroupElement:
										if (System == NULL) System = new GamessSystemGroup;
										if (System) System->ReadXML(child);
										break;
									case MMP_IOGamessBasisGroupElement:
										if (Basis == NULL) Basis = new GamessBasisGroup;
										if (Basis) Basis->ReadXML(child);
										break;
									case MMP_IOGamessDataGroupElement:
										if (Data == NULL) Data = new GamessDataGroup;
										if (Data) Data->ReadXML(child);
										break;
									case MMP_IOGamessGuessGroupElement:
										if (Guess == NULL) Guess = new GamessGuessGroup;
										if (Guess) Guess->ReadXML(child);
										break;
									case MMP_IOGamessSCFGroupElement:
										if (SCF == NULL) SCF = new GamessSCFGroup;
										if (SCF) SCF->ReadXML(child);
										break;
									case MMP_IOGamessMP2GroupElement:
										if (MP2 == NULL) MP2 = new GamessMP2Group;
										if (MP2) MP2->ReadXML(child);
										break;
									case MMP_IOGamessHessianGroupElement:
										if (Hessian == NULL) Hessian = new GamessHessianGroup;
										if (Hessian) Hessian->ReadXML(child);
										break;
									case MMP_IOGamessStatPtGroupElement:
										if (StatPt == NULL) StatPt = new GamessStatPtGroup;
										if (StatPt) StatPt->ReadXML(child);
										break;
									case MMP_IOGamessDFTGroupElement:
										if (DFT == NULL) DFT = new GamessDFTGroup;
										if (DFT) DFT->ReadXML(child);
										break;
								}
							}
						}
						delete children;
					}
				}
			}
		}
		delete ipxml;
	}
}

void GamessInputData::ReadFromBuffer(BufferFile *Buffer, long length) {
	long lPos, code, objectLength;

	lPos = Buffer->Read((Ptr) &objectLength, sizeof(long));
	lPos += Buffer->BufferSkip(objectLength);
		
	while (lPos<length) {
		lPos += Buffer->Read((Ptr) &code, sizeof(long));
		lPos += Buffer->Read((Ptr) &objectLength, sizeof(long));
		long blockStart = Buffer->GetFilePos();
		switch (code) {
			case 1:	//control group
				if (Control==NULL) Control = new GamessControlGroup;
				if (Control) Control->ReadFromBuffer(Buffer, objectLength);
				else throw MemoryError();
			break;
			case 2: //system
				if (System == NULL) System = new GamessSystemGroup;
				if (System) {
					code = System->ReadFromBuffer(Buffer);
//					if (code != objectLength) throw DataError();
				} else throw MemoryError();
			break;
			case 3:
				if (Basis == NULL) Basis = new GamessBasisGroup;
				if (Basis) {
					code = Basis->ReadFromBuffer(Buffer);
//					if (code != objectLength) throw DataError();
				} else throw MemoryError();
			break;
			case 4:
				if (Data == NULL) Data = new GamessDataGroup;
				if (Data) Data->ReadFromBuffer(Buffer, objectLength);
				else throw MemoryError();
			break;
			case 5:
				if (Guess == NULL) Guess = new GamessGuessGroup;
				if (Guess) {
					code = Guess->ReadFromBuffer(Buffer);
	//				if (code != objectLength) throw DataError();
				} else throw MemoryError();
			break;
			case 6:
				if (SCF == NULL) SCF = new GamessSCFGroup;
				if (SCF) {
					code = SCF->ReadFromBuffer(Buffer);
//					if (code != objectLength) throw DataError();
				} else throw MemoryError();
			break;
			case 7:
				if (MP2 == NULL) MP2 = new GamessMP2Group;
				if (MP2) {
					code = MP2->ReadFromBuffer(Buffer);
//					if (code != objectLength) throw DataError();
				} else throw MemoryError();
			break;
			case 8:
				if (Hessian == NULL) Hessian = new GamessHessianGroup;
				if (Hessian) {
					code = Hessian->ReadFromBuffer(Buffer);
//					if (code != objectLength) throw DataError();
				} else throw MemoryError();
			break;
			case 9:
				if (StatPt == NULL) StatPt = new GamessStatPtGroup;
				if (StatPt) {
					code = StatPt->ReadFromBuffer(Buffer);
//					if (code != objectLength) throw DataError();
				} else throw MemoryError();
			break;
			case 10:
				if (DFT == NULL) DFT = new GamessDFTGroup;
				if (DFT) {
					code = DFT->ReadFromBuffer(Buffer);
//					if (code != objectLength) throw DataError();
				} else throw MemoryError();
			break;
			default:
				Buffer->BufferSkip(objectLength);	//unknown group just skip it
		}
		Buffer->SetFilePos(blockStart+objectLength);
		lPos += objectLength;
	}
}

#pragma segment Control
	//GamessControlGroup functions
GamessControlGroup::GamessControlGroup(void) {
	ExeType = NULL;
	Options=0;
	InitControlPaneData();
	InitProgPaneData();
	NPrint=ITol=ICut=0;
}
GamessControlGroup::GamessControlGroup(GamessControlGroup *Copy) {
	if (Copy == NULL) return;
	*this = *Copy;
	ExeType = NULL;
	if (Copy->ExeType) {
		ExeType = new char[1+strlen(Copy->ExeType)];
		if (ExeType) strcpy(ExeType, Copy->ExeType);
	}
}
GamessControlGroup::~GamessControlGroup(void) {
	if (ExeType) delete [] ExeType;
}
void GamessControlGroup::InitControlPaneData(void) {
	if (ExeType) {
		delete [] ExeType;
		ExeType = NULL;
	}
	SCFType=GAMESSDefaultSCFType;
	MPLevelCIType=MaxIt=Charge=Multiplicity=0;
	Local=GAMESS_No_Localization;
	RunType=InvalidRunType;
	CCType = CC_None;
}
void GamessControlGroup::InitProgPaneData(void) {
	Friend=Friend_None;
	SetMolPlot(false);
	SetPlotOrb(false);
	SetAIMPAC(false);
	SetRPAC(false);
}
GAMESS_SCFType GamessControlGroup::SetSCFType(GAMESS_SCFType NewSCFType) {
	if ((NewSCFType >= GAMESSDefaultSCFType)||(NewSCFType<NumGAMESSSCFTypes))
		SCFType = NewSCFType;
	return SCFType;
}
const char * GamessControlGroup::GAMESSSCFTypeToText(GAMESS_SCFType t) {
	switch (t) {
		case GAMESS_RHF:
			return "RHF";
		case GAMESS_UHF:
			return "UHF";
		case GAMESS_ROHF:
			return "ROHF";
		case GAMESS_GVB:
			return "GVB";
		case GAMESS_MCSCF:
			return "MCSCF";
		case GAMESS_NO_SCF:
			return "NONE";
	}
	return "invalid";
}
GAMESS_SCFType GamessControlGroup::SetSCFType(const char *SCFText) {
	GAMESS_SCFType temp = GAMESS_Invalid_SCFType;
	for (int i=1; i<=NumGAMESSSCFTypes; i++) {
		if (!strcasecmp(SCFText, GAMESSSCFTypeToText((GAMESS_SCFType) i))) {
			temp = (GAMESS_SCFType) i;
			break;
		}
	}
	if (temp != GAMESS_Invalid_SCFType) SCFType = temp;
	return temp;
}
long GamessControlGroup::SetMPLevel(short NewMPLevel) {
	if ((NewMPLevel!=0)&&(NewMPLevel!=2)) return -1;

	MPLevelCIType = (MPLevelCIType & 0xFFF0) + NewMPLevel;
	return (MPLevelCIType & 0x0F);
}
short GamessControlGroup::GetMPLevel(void) const {	//return the appropriate MP value based on SCF and Run types
	short result=-1;
		//MP2 energy and gradients are available for RHF, UHF and ROHF
	if ((SCFType >= GAMESSDefaultSCFType)&&(SCFType <= GAMESS_ROHF)) result = (MPLevelCIType & 0x0F);
		//MrMP2 energies are also available
	else if ((SCFType==GAMESS_MCSCF)&&
		(RunType!=GradientRun)&&(RunType!=HessianRun)&&(RunType!=OptimizeRun)&&
		(RunType!=SadPointRun)&&(RunType!=IRCRun)&&(RunType!=GradExtrRun)&&(RunType!=DRCRun)) result=(MPLevelCIType & 0x0F);
	if (MPLevelCIType & 0xF0) result = -1;	//deactivate MP2 when CI is requested
	if (GetCCType() != CC_None) result = -1;
	return result;
}
CIRunType GamessControlGroup::SetCIType(CIRunType NewVal) {
	MPLevelCIType = (MPLevelCIType&0x0F) + (NewVal<<4);
	return (CIRunType)(MPLevelCIType & 0xF0);
}
CIRunType GamessControlGroup::GetCIType(void) const {
	short result = ((MPLevelCIType & 0xF0)>>4);
	if (GetSCFType() == 2) result = 0;
	return (CIRunType) result;
};
TypeOfRun GamessControlGroup::SetRunType(const TypeOfRun & NewRunType) {
	if ((NewRunType<=0)||(NewRunType>NumGAMESSRunTypes)) return InvalidRunType;
	
	RunType = NewRunType;
	return RunType;
}
TypeOfRun GamessControlGroup::SetRunType(const char *RunText) {
	TypeOfRun NewType = InvalidRunType;

	for (int i=1; i<NumGAMESSRunTypes; i++) {
		const char * test = GetGAMESSRunText((TypeOfRun)i);
		if (-1<LocateKeyWord(RunText, test, strlen(test), 9)) {
			NewType = (TypeOfRun)i;
			break;
		}
	}

	if (NewType<=0) return InvalidRunType;
	
	RunType = NewType;
	return RunType;
}
const char * GamessControlGroup::GetGAMESSRunText(const TypeOfRun & r) {
	switch (r) {
		case Energy:
			return "ENERGY";
		case GradientRun:
			return "GRADIENT";
		case HessianRun:
			return "HESSIAN";
		case OptimizeRun:
			return "OPTIMIZE";
		case TrudgeRun:
			return "TRUDGE";
		case SadPointRun:
			return "SADPOINT";
		case IRCRun:
			return "IRC";
		case GradExtrRun:
			return "GRADEXTR";
		case DRCRun:
			return "DRC";
		case SurfaceRun:
			return "SURFACE";
		case PropRun:
			return "PROP";
		case MorokumaRun:
			return "MOROKUMA";
		case TransitnRun:
			return "TRANSITN";
		case SpinOrbitRun:
			return "SPINORBT";
		case FFieldRun:
			return "FFIELD";
		case TDHFRun:
			return "TDHF";
		case GLOBOPRun:
			return "GLOBOP";
		case VSCFRun:
			return "VSCF";
		case OptFMORun:
			return "OPTFMO";
		case RamanRun:
			return "RAMAN";
		case NMRRun:
			return "NMR";
		case MakeEFPRun:
			return "MAKEFP";
		default:
			return "unknown";
	}
}
const char * GamessControlGroup::GetGAMESSCCType(const CCRunType & r) {
	switch (r) {
		case CC_None:
			return "NONE";
		case CC_LCCD:
			return "LCCD";
		case CC_CCD:
			return "CCD";
		case CC_CCSD:
			return "CCSD";
		case CC_CCSDT:
			return "CCSD(T)";
		case CC_RCC:
			return "R-CC";
		case CC_CRCC:
			return "CR-CC";
		case CC_EOMCCSD:
			return "EOM-CCSD";
		case CC_CREOM:
			return "CR-EOM";
		default:
			return "unknown";
	}
}
CCRunType GamessControlGroup::SetCCType(CCRunType n) {
	CCType = n;
	return CCType;
}
CCRunType GamessControlGroup::SetCCType(const char * n) {
	CCRunType NewType = CC_None;
	
	for (int i=1; i<NumCCTypes; i++) {
		const char * test = GetGAMESSCCType((CCRunType)i);
		if (-1<LocateKeyWord(n, test, strlen(test), 8)) {
			NewType = (CCRunType)i;
			break;
		}
	}
	
	if (NewType<=0) return CC_None;
	
	CCType = NewType;
	return CCType;
}
CCRunType GamessControlGroup::GetCCType(void) const {
	CCRunType result = CCType;
	
	if (GetSCFType() > 1) result = CC_None;
	if (GetCIType() > 0) result = CC_None;
	return result;
}
short GamessControlGroup::GetExeType(void) {
	if ((ExeType==NULL)||(0<=LocateKeyWord(ExeType, "RUN", 3,3))) return 0;	//Normal run
	if (0<=LocateKeyWord(ExeType, "CHECK", 5,5)) return 1;
	if (0<=LocateKeyWord(ExeType, "DEBUG", 5,5)) return 2;
	return 3;
}
short GamessControlGroup::SetExeType(const char *ExeText) {
	if (ExeText==NULL) return 0;
	long nchar = strlen(ExeText);
	if (ExeType) {
		delete [] ExeType;
		ExeType = NULL;
	}
	ExeType = new char[nchar+1];
	strcpy(ExeType, ExeText);
	return nchar;
}
short GamessControlGroup::SetExeType(short NewType) {
	if ((NewType < 0)||(NewType > 2)) return -1;
	if (ExeType) {
		delete [] ExeType;
		ExeType = NULL;
	}
	if (NewType==1) {
		ExeType = new char[6];
		strcpy(ExeType, "CHECK");
	} else if (NewType == 2) {
		ExeType = new char[6];
		strcpy(ExeType, "DEBUG");
	}
	return NewType;
}
CIRunType GamessControlGroup::SetCIType(const char * CIText) {
	CIRunType newType = CI_None;
	if (-1<FindKeyWord(CIText, "GUGA", 4)) newType = CI_GUGA;
	else if (-1<FindKeyWord(CIText, "ALDET", 5)) newType = CI_ALDET;
	else if (-1<FindKeyWord(CIText, "ORMAS", 5)) newType = CI_ORMAS;
	else if (-1<FindKeyWord(CIText, "CIS", 3)) newType = CI_CIS;
	else if (-1<FindKeyWord(CIText, "FSOCI", 5)) newType = CI_FSOCI;
	else if (-1<FindKeyWord(CIText, "GENCI", 5)) newType = CI_GENCI;
	return SetCIType(newType);
}
const char * GamessControlGroup::GetCIType(const CIRunType & citype) const {
	switch (citype) {
		default:
			return "NONE";
		case CI_GUGA:
			return "GUGA";
		case CI_ALDET:
			return "ALDET";
		case CI_ORMAS:
			return "ORMAS";
		case CI_CIS:
			return "CIS";
		case CI_FSOCI:
			return "FSOCI";
		case CI_GENCI:
			return "GENCI";
	}
}
CIRunType GamessControlGroup::GetCIType(char * outText) const {
	CIRunType temp = GetCIType();
	if (outText != NULL) {
		strcpy(outText, GetCIType(temp));
	}
	return temp;
}
short GamessControlGroup::SetMaxIt(short NewVal) {
	if (NewVal>=0) MaxIt = NewVal;
	return MaxIt;
}
GAMESS_Localization GamessControlGroup::SetLocal(GAMESS_Localization NewVal) {
	if ((NewVal>=GAMESS_No_Localization)&&(NewVal<NumGAMESSLocalizations)) Local = NewVal;
	return Local;
}
GAMESS_Localization GamessControlGroup::SetLocal(const char * t) {
	GAMESS_Localization temp = Invalid_Localization;
	for (int i=0; i<NumGAMESSLocalizations; i++) {
		if (!strcasecmp(t, GAMESSLocalizationToText((GAMESS_Localization) i))) {
			temp = (GAMESS_Localization) i;
			break;
		}
	}
	if (temp != Invalid_Localization) Local = temp;
	return temp;
}
const char * GamessControlGroup::GAMESSLocalizationToText(GAMESS_Localization t) {
	switch (t) {
		case GAMESS_No_Localization:
			return "NONE";
		case GAMESS_BOYS_Localization:
			return "BOYS";
		case GAMESS_RUEDNBRG_Localization:
			return "RUEDNBRG";
		case GAMESS_POP_Localization:
			return "POP";
	}
	return "invalid";
}
const char * GamessControlGroup::GetFriendText(FriendType f) {
	switch (f) {
		case Friend_HONDO:
			return "HONDO";
		case Friend_MELDF:
			return "MELDF";
		case Friend_GAMESSUK:
			return "GAMESSUK";
		case Friend_GAUSSIAN:
			return "GAUSSIAN";
		case Friend_ALL:
			return "ALL";
	}
	return "invalid";	//Getting to here indicates a bad value
}
FriendType GamessControlGroup::TextToFriend(const char * c) {
	FriendType result = Friend_None;
	for (int i=0; i<NumFriendTypes; i++) {
		if (!strcasecmp(c, GetFriendText((FriendType) i))) {
			result = (FriendType) i;
			break;
		}
	}
	return result;
}
FriendType GamessControlGroup::SetFriend(FriendType NewValue) {
	if ((NewValue >= Friend_None)&&(NewValue < NumFriendTypes)) Friend = NewValue;
	return (FriendType)Friend;
}
FriendType GamessControlGroup::SetFriend(const char * c) {
	return SetFriend(TextToFriend(c));
}
short GamessControlGroup::SetCharge(short NewCharge) {
	Charge = NewCharge;
	return Charge;
}
short GamessControlGroup::SetMultiplicity(short NewMult) {
	Multiplicity = NewMult;
	return Multiplicity;
}
bool GamessControlGroup::SetMolPlot(bool State) {
	if (Options & 1) Options -= 1;
	if (State) Options += 1;
	return ((Options & 1)?true:false);
}
bool GamessControlGroup::SetPlotOrb(bool State) {
	if (Options & (1<<1)) Options -= (1<<1);
	if (State) Options += (1<<1);
	return ((Options & (1<<1))?true:false);
}
bool GamessControlGroup::SetAIMPAC(bool State) {
	if (Options & (1<<2)) Options -= (1<<2);
	if (State) Options += (1<<2);
	return ((Options & (1<<2))?true:false);
}
bool GamessControlGroup::SetRPAC(bool State) {
	if (Options & (1<<3)) Options -= (1<<3);
	if (State) Options += (1<<3);
	return ((Options & (1<<3))?true:false);
}
bool GamessControlGroup::SetIntType(bool State) {
	if (Options & (1<<5)) Options -= (1<<5);
	if (State) Options += (1<<5);
	return ((Options & (1<<5))?true:false);
}
bool GamessControlGroup::SetNormF(bool State) {
	if (Options & (1<<6)) Options -= (1<<6);
	if (State) Options += (1<<6);
	return ((Options & (1<<6))?true:false);
}
bool GamessControlGroup::UseDFT(bool State) {
	if (Options & (1<<4)) Options -= (1<<4);
	if (State) Options += (1<<4);
	return (UseDFT());
}
bool GamessControlGroup::UseDFT(void) const {
	bool result = false;
	result = ((Options & (1<<4))?true:false);
	if (GetSCFType() > 3) result = false;
	if (GetMPLevel() > 0) result = false;
	if (GetCIType() > 0) result = false;
	if (GetCCType() != CC_None) result = false;
	return result;
}
bool GamessControlGroup::SetNormP(bool State) {
	if (Options & (1<<7)) Options -= (1<<7);
	if (State) Options += (1<<7);
	return GetNormP();
}
long GamessControlGroup::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessControlGroup::WriteToBuffer(BufferFile *Buffer) {
	long length = 28;

	long Pos = Buffer->Write((Ptr) &length, sizeof(long));
	Pos += Buffer->Write((Ptr) ExeType, sizeof (char *));
	Pos += Buffer->Write((Ptr) &SCFType, sizeof(short));
	Pos += Buffer->Write((Ptr) &MPLevelCIType, sizeof(short));
	Pos += Buffer->Write((Ptr) &RunType, sizeof(short));
	Pos += Buffer->Write((Ptr) &MaxIt, sizeof(short));
	Pos += Buffer->Write((Ptr) &Charge, sizeof(short));
	Pos += Buffer->Write((Ptr) &Multiplicity, sizeof(short));
	Pos += Buffer->Write((Ptr) &Local, sizeof(short));
	Pos += Buffer->Write((Ptr) &Friend, sizeof(short));
	Pos += Buffer->Write((Ptr) &NPrint, sizeof(short));
	Pos += Buffer->Write((Ptr) &ITol, sizeof(short));
	Pos += Buffer->Write((Ptr) &ICut, sizeof(short));
	Pos += Buffer->Write((Ptr) &Options, sizeof(char));
	length = 0;
	Pos += Buffer->Write((Ptr) &length, sizeof(char));	//padding byte for compatibility

	if (ExeType) {
		length = 1;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = 1 + strlen(ExeType);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += Buffer->Write((Ptr) ExeType, length);
	}
	if (CCType != CC_None) {
		length = 2;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = sizeof(short);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += Buffer->Write((Ptr) &CCType, length);
	}
	length = 99;
	Pos += Buffer->Write((Ptr) &length, sizeof(long));
	return Pos;
}
long GamessControlGroup::ReadFromBuffer(BufferFile *Buffer, long length) {
	long mylength, pos, code;

	pos = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength == 28) {
		pos += Buffer->Read((Ptr) &ExeType, sizeof(char *));
		pos += Buffer->Read((Ptr) &SCFType, sizeof(short));
		pos += Buffer->Read((Ptr) &MPLevelCIType, sizeof(short));
		pos += Buffer->Read((Ptr) &RunType, sizeof(short));
		pos += Buffer->Read((Ptr) &MaxIt, sizeof(short));
		pos += Buffer->Read((Ptr) &Charge, sizeof(short));
		pos += Buffer->Read((Ptr) &Multiplicity, sizeof(short));
		pos += Buffer->Read((Ptr) &Local, sizeof(short));
		pos += Buffer->Read((Ptr) &Friend, sizeof(short));
		pos += Buffer->Read((Ptr) &NPrint, sizeof(short));
		pos += Buffer->Read((Ptr) &ITol, sizeof(short));
		pos += Buffer->Read((Ptr) &ICut, sizeof(short));
		pos += Buffer->Read((Ptr) &Options, sizeof(char));
		pos += Buffer->Read((Ptr) &mylength, sizeof(char));//padding byte
	} else {
		return pos;
	}
	ExeType = NULL;
	while (pos < length) {
		//otherwise look for the exetype
		pos += Buffer->Read((Ptr) &code, sizeof(long));
		switch (code) {
			case 1:
				pos += Buffer->Read((Ptr) &mylength, sizeof(long));
				ExeType = new char[mylength];
				if (ExeType)
					pos += Buffer->Read(ExeType, mylength);
				else throw MemoryError();
				break;
			case 2:
				pos += Buffer->Read((Ptr) &mylength, sizeof(long));
				pos += Buffer->Read((Ptr) &CCType, sizeof(short));
				break;
		}
	}
	return pos;
}
void GamessControlGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IOCGSCFType:
					{
						const char * v = child->getValue();
						if (v) SetSCFType(v);
					}
						break;
					case MMP_IOCGRunType:
					{
						const char * v = child->getValue();
						if (v) SetRunType(v);
					}
						break;
					case MMP_IOCGExeType:
					{
						const char * v = child->getValue();
						if (v) SetExeType(v);
					}
						break;
					case MMP_IOCGMPLevel:
					{
						const char * v = child->getValue();
						if (v) {
							int mpl;
							sscanf(v, "%d", &mpl);
							SetMPLevel(mpl);
						}
					}
						break;
					case MMP_IOCGCIType:
					{
						const char * v = child->getValue();
						if (v) SetCIType(v);
					}
						break;
					case MMP_IOCGCCType:
					{
						const char * v = child->getValue();
						if (v) SetCCType(v);
					}
						break;
					case MMP_IOCGMaxIterations:
					{
						const char * v = child->getValue();
						if (v) {
							int mit;
							sscanf(v, "%d", &mit);
							SetMaxIt(mit);
						}
					}
						break;
					case MMP_IOCGCharge:
					{
						const char * v = child->getValue();
						if (v) {
							int chg;
							sscanf(v, "%d", &chg);
							SetCharge(chg);
						}
					}
						break;
					case MMP_IOCGMultiplicity:
					{
						const char * v = child->getValue();
						if (v) {
							int mul;
							sscanf(v, "%d", &mul);
							SetMultiplicity(mul);
						}
					}
						break;
					case MMP_IOCGLocalization:
					{
						const char * v = child->getValue();
						if (v) SetLocal(v);
					}
						break;
					case MMP_IOCGFriend:
					{
						const char * v = child->getValue();
						if (v) SetFriend(v);
					}
						break;
					case MMP_IOCGPrintOption:
					{
						const char * v = child->getValue();
						if (v) {
							int np;
							sscanf(v, "%d", &np);
							NPrint = np;
						}
					}
						break;
					case MMP_IOCGTolerance:
					{
						const char * v = child->getValue();
						if (v) {
							int it;
							sscanf(v, "%d", &it);
							ITol = it;
						}
					}
						break;
					case MMP_IOCGCutoff:
					{
						const char * v = child->getValue();
						if (v) {
							int it;
							sscanf(v, "%d", &it);
							ICut = it;
						}
					}
						break;
					case MMP_IOCGMolPlt:
						if (child->getBoolValue(tb))
							SetMolPlot(tb);
						break;
					case MMP_IOCGPlotOrb:
						if (child->getBoolValue(tb))
							SetPlotOrb(tb);
						break;
					case MMP_IOCGAIMPac:
						if (child->getBoolValue(tb))
							SetAIMPAC(tb);
						break;
					case MMP_IOCGRPac:
						if (child->getBoolValue(tb))
							SetRPAC(tb);
						break;
					case MMP_IOCGDFTActive:
						if (child->getBoolValue(tb))
							UseDFT(tb);
						break;
					case MMP_IOCGIntType:
						if (child->getBoolValue(tb))
							SetIntType(tb);
						break;
					case MMP_IOCGNormF:
						if (child->getBoolValue(tb))
							SetNormF(tb);
						break;
					case MMP_IOCGNormP:
						if (child->getBoolValue(tb))
							SetNormP(tb);
						break;
				}
			}
		}
		delete children;
	}
}
void GamessControlGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessControlGroupElement));
	if (SCFType)
		Ele->addChildElement(CML_convert(MMP_IOCGSCFType), GetSCFTypeText());
	if (ExeType) Ele->addChildElement(CML_convert(MMP_IOCGExeType), ExeType);
	Ele->addChildElement(CML_convert(MMP_IOCGCIType), GetCIType(GetCIType()));
	Ele->addChildElement(CML_convert(MMP_IOCGRunType), GetGAMESSRunText(GetRunType()));
	snprintf(line, kMaxLineLength, "%d", GetMPLevel());
	Ele->addChildElement(CML_convert(MMP_IOCGMPLevel), line);
	if (GetCCType() != CC_None)
		Ele->addChildElement(CML_convert(MMP_IOCGCCType), GetGAMESSCCType(GetCCType()));
	if (MaxIt) {
		snprintf(line, kMaxLineLength, "%d", MaxIt);
		Ele->addChildElement(CML_convert(MMP_IOCGMaxIterations), line);
	}
	if (Charge) {
		snprintf(line, kMaxLineLength, "%d", Charge);
		Ele->addChildElement(CML_convert(MMP_IOCGCharge), line);
	}
	if (Multiplicity) {
		snprintf(line, kMaxLineLength, "%d", Multiplicity);
		Ele->addChildElement(CML_convert(MMP_IOCGMultiplicity), line);
	}
	if (Local) {
		Ele->addChildElement(CML_convert(MMP_IOCGLocalization), GetLocalText());
	}
	if (Friend) {	//punchs out input to other programs, disables exetype (forces check run)
		Ele->addChildElement(CML_convert(MMP_IOCGFriend), GetFriendText());
	}
	if (NPrint) {
		snprintf(line, kMaxLineLength, "%d", NPrint);
		Ele->addChildElement(CML_convert(MMP_IOCGPrintOption), line);
	}
	if (ITol) {
		snprintf(line, kMaxLineLength, "%d", ITol);
		Ele->addChildElement(CML_convert(MMP_IOCGTolerance), line);
	}
	if (ICut) {
		snprintf(line, kMaxLineLength, "%d", ICut);
		Ele->addChildElement(CML_convert(MMP_IOCGCutoff), line);
	}
	if (GetMolPlot()) Ele->addChildElement(CML_convert(MMP_IOCGMolPlt), trueXML);
	if (GetPlotOrb()) Ele->addChildElement(CML_convert(MMP_IOCGPlotOrb), trueXML);
	if (GetAIMPAC()) Ele->addChildElement(CML_convert(MMP_IOCGAIMPac), trueXML);
	if (GetRPAC()) Ele->addChildElement(CML_convert(MMP_IOCGRPac), trueXML);
	if (UseDFT()) Ele->addChildElement(CML_convert(MMP_IOCGDFTActive), trueXML);
	if (GetIntType()) Ele->addChildElement(CML_convert(MMP_IOCGIntType), trueXML);
	if (GetNormF()) Ele->addChildElement(CML_convert(MMP_IOCGNormF), trueXML);
	if (GetNormP()) Ele->addChildElement(CML_convert(MMP_IOCGNormP), trueXML);
}
void GamessControlGroup::WriteToFile(BufferFile *File, GamessInputData *IData, long NumElectrons) {
	char	Out[133], textVal[133];

		//Punch the group label
	File->WriteLine(" $CONTRL ", false);
		//punch the SCF type and Run type
	if (SCFType) {
		sprintf(Out,"SCFTYP=%s ",GetSCFTypeText());
		File->WriteLine(Out, false);
	} else {	//Punch out the default RHF/ROHF wavefunction
		if (NumElectrons & 1) sprintf(Out, "SCFTYP=ROHF ");
		else sprintf(Out, "SCFTYP=RHF ");
		File->WriteLine(Out, false);
	}
	sprintf(Out,"RUNTYP=%s ", GetGAMESSRunText(GetRunType()));
	File->WriteLine(Out, false);
	if ((ExeType)&&(!Friend)) {	//punch out ExeType if it is other than run
		sprintf(Out, "EXETYP=%s ", ExeType);
		File->WriteLine(Out, false);
	}
	if (GetMPLevel() > 0) {	//Write out MP level only if > zero
		sprintf(Out,"MPLEVL=2 ");
		File->WriteLine(Out, false);
	}
	if (GetCIType() || (GetSCFType() == 6)) {	//punch CIType if CI requested
		GetCIType(textVal);
		sprintf(Out, "CITYP=%s ", textVal);
		File->WriteLine(Out, false);
	}
	if (GetCCType() != CC_None) {
		sprintf(Out, "CCTYP=%s ", GetGAMESSCCType(CCType));
		File->WriteLine(Out, false);
	}

	if (IData->DFT) {
		if (UseDFT()) {
			sprintf(Out, "DFTTYP=%s ", IData->DFT->GetFunctionalText());
			File->WriteLine(Out, false);
		}
	}
	
	if (MaxIt) {	//Punch Maxit if non-default value
		sprintf(Out, "MAXIT=%d ",MaxIt);
		File->WriteLine(Out, false);
	}
	if (Charge) {
		sprintf(Out, "ICHARG=%d ", Charge);
		File->WriteLine(Out, false);
	}
	if (Multiplicity) {
		sprintf(Out, "MULT=%d ", Multiplicity);
		File->WriteLine(Out, false);
	} else if (NumElectrons & 1) {	//for odd electron systems punch out a default doublet
		sprintf(Out, "MULT=2 ");
		File->WriteLine(Out, false);
	}
	if (Local) {
		sprintf(Out, "LOCAL=%s ", GetLocalText());
 		File->WriteLine(Out, false);
	}
	if (IData->Basis) {
		if (IData->Basis->GetECPPotential()) {
			sprintf(Out, "ECP=%s ",IData->Basis->GetECPPotentialText());
	 		File->WriteLine(Out, false);
		}
	}
	if (IData->Data) {
		if (IData->Data->GetCoordType()) {
			sprintf(Out, "COORD=%s ", IData->Data->GetCoordText());
	 		File->WriteLine(Out, false);
		}
		if (IData->Data->GetUnits()) {
			sprintf(Out, "UNITS=BOHR ");
	 		File->WriteLine(Out, false);
		}
		if (IData->Data->GetNumZVar()) {
			sprintf(Out, "NZVAR=%d ",IData->Data->GetNumZVar());
	 		File->WriteLine(Out, false);
		}
		if (!IData->Data->GetUseSym()) {
			sprintf(Out, "NOSYM=1 ");
	 		File->WriteLine(Out, false);
		}
	}
	if (Friend) {	//punchs out input to other programs, disables exetype (forces check run)
		sprintf(Out, "FRIEND=%s ", GetFriendText());
 		File->WriteLine(Out, false);
	}
	if (GetMolPlot()) {
		sprintf(Out, "MOLPLT=.TRUE. ");
 		File->WriteLine(Out, false);
	}
	if (GetPlotOrb()) {
		sprintf(Out, "PLTORB=.TRUE. ");
 		File->WriteLine(Out, false);
	}
	if ((1!=GetExeType())&&(Friend==0)) {
		if (GetAIMPAC()) {
			sprintf(Out, "AIMPAC=.TRUE. ");
	 		File->WriteLine(Out, false);
		}
		if (GetRPAC()) {
			sprintf(Out, "RPAC=.TRUE. ");
	 		File->WriteLine(Out, false);
		}
	}

	File->WriteLine("$END", true);
}
void GamessControlGroup::RevertControlPane(GamessControlGroup *OldData) {
	RunType = OldData->RunType;
	SCFType = OldData->SCFType;
	SetMPLevel(OldData->GetMPLevel());
	UseDFT(OldData->UseDFT());
	SetCIType(OldData->GetCIType());
	SetCCType(OldData->GetCCType());
	MaxIt = OldData->MaxIt;
	if (ExeType) {
		delete [] ExeType;
		ExeType = NULL;
	}
	SetExeType(OldData->ExeType);
	Local = OldData->Local;
	Charge = OldData->Charge;
	Multiplicity = OldData->Multiplicity;
}
void GamessControlGroup::RevertProgPane(GamessControlGroup *OldData) {
	SetMolPlot(OldData->GetMolPlot());
	SetPlotOrb(OldData->GetPlotOrb());
	SetAIMPAC(OldData->GetAIMPAC());
	SetRPAC(OldData->GetRPAC());
	SetFriend(OldData->GetFriend());
}
#pragma mark GamessSystemGroup
		//GamessSystemGroup member functions
long GamessSystemGroup::SetTimeLimit(long NewTime) {
	if (NewTime >= 0) TimeLimit = NewTime;
	return TimeLimit;
}
const char * MemoryUnitToText(const MemoryUnit & mu) {
	switch (mu) {
		case wordsUnit:
			return "words";
		case bytesUnit:
			return "bytes";
		case megaWordsUnit:
			return "Mwords";
		case megaBytesUnit:
			return "MB";
		case gigaWordsUnit:
			return "Gwords";
		case gigaBytesUnit:
			return "GB";
	}
	return "invalid";
}
bool TextToMemoryUnit(const char * t, MemoryUnit & mu) {
	if (!t || !*t) return false;
	for (int i = (int) wordsUnit; i != (int) NumberMemoryUnits; ++i) {
		if (strcmp(t, MemoryUnitToText((MemoryUnit) i)) == 0) {
			mu = (MemoryUnit) i;
			return true;
		}
	}
	return false;
}
const char * TimeUnitToText(const TimeUnit & tu) {
	switch (tu) {
		case secondUnit:
			return "sec";
		case minuteUnit:
			return "min";
		case hourUnit:
			return "hr";
		case dayUnit:
			return "days";
		case weekUnit:
			return "weeks";
		case yearUnit:
			return "years";
		case milleniaUnit:
			return "millenia";
	}
	return "invalid";
}
bool TextToTimeUnit(const char * t, TimeUnit & tu) {
	if (!t || !*t) return false;
	for (int i = (int) secondUnit; i != (int) NumberTimeUnits; ++i) {
		if (strcmp(t, TimeUnitToText((TimeUnit) i)) == 0) {
			tu = (TimeUnit) i;
			return true;
		}
	}
	return false;
}
TimeUnit GamessSystemGroup::SetTimeUnits(TimeUnit NewUnits) {
	if ((NewUnits >= secondUnit)&&(NewUnits<NumberTimeUnits)) TimeUnits = NewUnits;
	return TimeUnits;
}
float GamessSystemGroup::GetConvertedTime(void) const {
	float result, factor=1.0;

	if (TimeLimit) result = TimeLimit;
	else result = 525600.0;

	switch (TimeUnits) {
		case milleniaUnit:
			factor = 1.0/1000.0;
		case yearUnit:
			factor *= 1/52.0;
		case weekUnit:
			factor *= 1/7.0;
		case dayUnit:
			factor *= 1/24.0;
		case hourUnit:
			factor *= 1/60.0;
		case minuteUnit:
		break;
		case secondUnit:
			factor = 60.0;
		break;
	}
	result *= factor;
	return result;
}
long GamessSystemGroup::SetConvertedTime(float NewTime) {
	long	result, factor = 1;

	switch (TimeUnits) {
		case milleniaUnit:
			factor = 1000;
		case yearUnit:
			factor *= 52;
		case weekUnit:
			factor *= 7;
		case dayUnit:
			factor *= 24;
		case hourUnit:
			factor *= 60;
		case minuteUnit:
			result = (long)(NewTime * factor);
		break;
		case secondUnit:
			result = (long)(NewTime/60.0);
		break;
	}
	if (result >= 0) TimeLimit = result;
	return TimeLimit;
}
double GamessSystemGroup::SetMemory(double NewMemory) {
	if (NewMemory > 0.0) Memory = NewMemory;
	return Memory;
}
MemoryUnit GamessSystemGroup::SetMemUnits(MemoryUnit NewUnits) {
	if ((NewUnits>=wordsUnit)&&(NewUnits<NumberMemoryUnits)) MemUnits = NewUnits;
	return MemUnits;
}
double GamessSystemGroup::GetConvertedMem(void) const {
	double result, factor=1.0;

	if (Memory) result = Memory;
	else result = 1000000;

	switch (MemUnits) {
		case bytesUnit:
			factor = 8.0;
		break;
		case megaWordsUnit:
			factor = 1.0/1000000.0;
		break;
		case megaBytesUnit:
			factor = 8.0/(1024*1024);
		break;
	}
	result *= factor;
	return result;
}
double GamessSystemGroup::SetConvertedMem(double NewMem) {
	double	result, factor = 1;

	switch (MemUnits) {
		case megaBytesUnit:
			factor *= 1024*1024;
		case bytesUnit:
			result = (long)(factor*NewMem/8.0);
		break;
		case megaWordsUnit:
			factor *= 1000000;
		case wordsUnit:
			result = (long)(factor*NewMem);
		break;
	}
	if (result >= 0) Memory = result;
	return Memory;
}
double GamessSystemGroup::SetMemDDI(double NewMemory) {
	if (NewMemory >= 0.0) MemDDI = NewMemory;
	return Memory;
}
MemoryUnit GamessSystemGroup::SetMemDDIUnits(MemoryUnit NewUnits) {
	if ((NewUnits>=megaWordsUnit)&&(NewUnits<NumberMemoryUnits)) MemDDIUnits = NewUnits;
	return MemDDIUnits;
}
double GamessSystemGroup::GetConvertedMemDDI(void) const {
	double result, factor=1.0;
	
	result = MemDDI;	//memDDI is stored in MW
	
	switch (MemDDIUnits) {
		case megaBytesUnit:
			factor = 8.0;
			break;
		case gigaWordsUnit:
			factor = 1.0/1000.0;
			break;
		case gigaBytesUnit:
			factor = 8.0/(1000.0);
			break;
	}
	result *= factor;
	return result;
}
double GamessSystemGroup::SetConvertedMemDDI(double NewMem) {
	double	result, factor = 1;
	
	switch (MemDDIUnits) {
		case megaBytesUnit:
			factor = 1.0/8.0;
			break;
		case gigaWordsUnit:
			factor = 1000.0;
			break;
		case gigaBytesUnit:
			factor = 1000.0/8.0;
			break;
	}
	result = NewMem*factor;
	if (result >= 0) MemDDI = result;
	return MemDDI;
}

char GamessSystemGroup::SetDiag(char NewMethod) {
	if ((NewMethod>=0)&&(NewMethod<4)) KDiag = NewMethod;
	return KDiag;
}
bool GamessSystemGroup::SetCoreFlag(bool State) {
	if (Flags & 1) Flags --;
	if (State) Flags ++;
	return GetCoreFlag();
}
bool GamessSystemGroup::SetBalanceType(bool Type) {
	if (Flags & 2) Flags -= 2;
	if (Type) Flags += 2;
	return GetBalanceType();
}
bool GamessSystemGroup::SetXDR(bool State) {
	if (Flags & 4) Flags -= 4;
	if (State) Flags += 4;
	return GetXDR();
}
bool GamessSystemGroup::SetParallel(bool State) {
	if (Flags & 8) Flags -= 8;
	if (State) Flags += 8;
	return GetParallel();
}
GamessSystemGroup::GamessSystemGroup(void) {
	InitData();
}
GamessSystemGroup::GamessSystemGroup(GamessSystemGroup *Copy) {
	if (Copy) *this=*Copy;
}
void GamessSystemGroup::InitData(void) {
	TimeLimit = 0;
	Memory = 0.0;
	MemDDI = 0.0;
	KDiag = 0;
	TimeUnits = minuteUnit;
	MemUnits = wordsUnit;
	MemDDIUnits = megaWordsUnit;
	Flags = 0;
}
long GamessSystemGroup::GetSize(BufferFile *Buffer) {
	bool cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessSystemGroup::WriteToBuffer(BufferFile *Buffer) {
	long Pos, length = sizeof(GamessSystemGroup);

	Pos = Buffer->Write((Ptr) &length, sizeof(long));
	Pos += Buffer->Write((Ptr) this, length);
	return Pos;
}
void GamessSystemGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessSystemGroupElement));
	if (TimeLimit) {
		snprintf(line, kMaxLineLength, "%f", GetConvertedTime());
		XMLElement * t = Ele->addChildElement(CML_convert(MMP_IOSGTimeLimit), line);
		t->addAttribute(CML_convert(MMP_IOSGTimeUnits), TimeUnitToText(TimeUnits));
	}
	if (Memory) {
		snprintf(line, kMaxLineLength, "%lf", GetConvertedMem());
		XMLElement * t = Ele->addChildElement(CML_convert(MMP_IOSGMemory), line);
		t->addAttribute(CML_convert(MMP_IOSGMemoryUnits), MemoryUnitToText(MemUnits));
	}
	if (MemDDI) {
		snprintf(line, kMaxLineLength, "%lf", GetConvertedMemDDI());
		XMLElement * t = Ele->addChildElement(CML_convert(MMP_IOSGMemDDI), line);
		t->addAttribute(CML_convert(MMP_IOSGMemoryUnits), MemoryUnitToText(MemDDIUnits));
	}
	if (KDiag) {
		snprintf(line, kMaxLineLength, "%d", KDiag);
		Ele->addChildElement(CML_convert(MMP_IOSGKDiag), line);
	}
	if (GetParallel()) Ele->addChildElement(CML_convert(MMP_IOSGParallel), trueXML);
	if (GetCoreFlag()) Ele->addChildElement(CML_convert(MMP_IOSGCoreFlag), trueXML);
	if (GetBalanceType()) Ele->addChildElement(CML_convert(MMP_IOSGBalanceType), trueXML);
	if (GetXDR()) Ele->addChildElement(CML_convert(MMP_IOSGXDR), trueXML);
}
void GamessSystemGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IOSGTimeLimit:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							const char * u = child->getAttributeValue(CML_convert(MMP_IOSGTimeUnits));
							if (u) {
								TimeUnit t;
								if (TextToTimeUnit(u, t)) SetTimeUnits(t);
							}
							SetConvertedTime(temp);
						}
					}
						break;
					case MMP_IOSGMemory:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							const char * u = child->getAttributeValue(CML_convert(MMP_IOSGMemoryUnits));
							if (u) {
								MemoryUnit t;
								if (TextToMemoryUnit(u, t)) SetMemUnits(t);
							}
							SetConvertedMem(temp);
						}
					}
						break;
					case MMP_IOSGKDiag:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetDiag(temp);
						}
					}
						break;
					case MMP_IOSGCoreFlag:
						if (child->getBoolValue(tb))
							SetCoreFlag(tb);
						break;
					case MMP_IOSGBalanceType:
						if (child->getBoolValue(tb))
							SetBalanceType(tb);
						break;
					case MMP_IOSGXDR:
						if (child->getBoolValue(tb))
							SetXDR(tb);
						break;
					case MMP_IOSGMemDDI:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							const char * u = child->getAttributeValue(CML_convert(MMP_IOSGMemoryUnits));
							if (u) {
								MemoryUnit t;
								if (TextToMemoryUnit(u, t)) SetMemDDIUnits(t);
							}
							SetConvertedMemDDI(temp);
						}
					}
						break;
					case MMP_IOSGParallel:
						if (child->getBoolValue(tb))
							SetParallel(tb);
						break;
				}
			}
		}
	}
}
long GamessSystemGroup::ReadFromBuffer(BufferFile *Buffer) {
	long mylength, size;

	size = Buffer->Read((Ptr) &mylength, sizeof(long));
//	if (mylength != sizeof(GamessSystemGroup)) return size;
	if (mylength != 12) return size;
//	size += Buffer->Read((Ptr) this, mylength);
	size += Buffer->Read((Ptr) &TimeLimit, sizeof(long));
	size += Buffer->Read((Ptr) &mylength, sizeof(long));
	Memory = mylength;
	size += Buffer->Read(&KDiag, sizeof(char));
	char temp;
	size += Buffer->Read(&temp, sizeof(char));
	SetTimeUnits((TimeUnit) temp);
	size += Buffer->Read(&temp, sizeof(char));
	SetMemUnits((MemoryUnit) temp);
	size += Buffer->Read(&Flags, sizeof(char));
	return size;
}
void GamessSystemGroup::WriteToFile(BufferFile *File) {
	long	test;
	char	Out[133];

		//Punch the group label
	File->WriteLine(" $SYSTEM ", false);
		//Time limit
	test = TimeLimit;
	if (test==0) test = 600;
	sprintf(Out,"TIMLIM=%ld ",test);
	File->WriteLine(Out, false);
		//Memory
	if (Memory) {
		sprintf(Out, "MEMORY=%ld ", (long)Memory);
		File->WriteLine(Out, false);
	}
	if (MemDDI) {
		sprintf(Out, "MEMDDI=%ld ", (long)MemDDI);
		File->WriteLine(Out, false);
	}	//PARALL
	if (GetParallel()) {
		sprintf(Out, "PARALL=.TRUE. ");
		File->WriteLine(Out, false);
	}	//diag method
	if (KDiag) {
		sprintf(Out, "KDIAG=%d ", KDiag);
		File->WriteLine(Out, false);
	}	//core flag
	if (GetCoreFlag()) {
		sprintf(Out, "COREFL=.TRUE. ");
		File->WriteLine(Out, false);
	}	//Balance type
	if (GetBalanceType()) {
		sprintf(Out, "BALTYP=NXTVAL ");
		File->WriteLine(Out, false);
	}	//XDR
	if (GetXDR()) {
		sprintf(Out, "XDR=.TRUE. ");
		File->WriteLine(Out, false);
	}
	File->WriteLine("$END", true);
}
#pragma mark GamessBasisGroup
		//GamessBasisGroup member functions
GamessBasisGroup::GamessBasisGroup(void) {
	InitData();
}
GamessBasisGroup::GamessBasisGroup(GamessBasisGroup *Copy) {
	if (Copy) {
		*this = *Copy;
	}
}
void GamessBasisGroup::InitData(void) {
	Split2[0]=Split2[1]=0.0;
	Split3[0]=Split3[1]=Split3[2]=0.0;
	Basis=NumGauss=NumHeavyFuncs=NumPFuncs=ECPPotential=0;
	Polar = GAMESS_BS_No_Polarization;
	Flags = 0;
}
const char * GamessBasisGroup::GAMESSBasisSetToText(GAMESS_BasisSet bs) {
	switch (bs) {
		case GAMESS_BS_MINI:
			return "MINI";
		case GAMESS_BS_MIDI:
			return "MIDI";
		case GAMESS_BS_STO:
			return "STO";
		case GAMESS_BS_N21:
			return "N21";
		case GAMESS_BS_N31:
			return "N31";
		case GAMESS_BS_N311:
			return "N311";
		case GAMESS_BS_DZV:
			return "DZV";
		case GAMESS_BS_DH:
			return "DH";
		case GAMESS_BS_BC:
			return "BC";
		case GAMESS_BS_TZV:
			return "TZV";
		case GAMESS_BS_MC:
			return "MC";
		case GAMESS_BS_SBK:
			return "SBK";
		case GAMESS_BS_HW:
			return "HW";
		case GAMESS_BS_MNDO:
			return "MNDO";
		case GAMESS_BS_AM1:
			return "AM1";
		case GAMESS_BS_PM3:
			return "PM3";
	}
	return "invalid";
}
short GamessBasisGroup::SetBasis(const char *BasisText) {
	short NewBasis = -1;

	for (int i=GAMESS_BS_None; i<NumGAMESSBasisSetsItem; i++) {
		if (!strcasecmp(BasisText, GAMESSBasisSetToText((GAMESS_BasisSet)i))) {
			NewBasis = i;
			break;
		}
	}
	if (NewBasis<0) return -1;

	Basis = NewBasis;
	return Basis;
}
short GamessBasisGroup::SetBasis(short NewBasis) {
	if ((NewBasis<-1)||(NewBasis>16)) return -1;
	
	Basis = NewBasis;
	return Basis;
}
const char * GamessBasisGroup::GetBasisText(void) const {
	short temp = Basis;
	if (temp <= 0) temp = 1;

	return GAMESSBasisSetToText((GAMESS_BasisSet) temp);
}
short GamessBasisGroup::GetBasis(void) const {
	return Basis;
}
short GamessBasisGroup::SetNumGauss(short NewNumGauss) {
	if ((NewNumGauss<0)||(NewNumGauss>6)) return -1;
	if ((Basis==4)&&(NewNumGauss!=3)&&(NewNumGauss!=6)) return -1;
	if ((Basis==5)&&(NewNumGauss<4)) return -1;
	if ((Basis==6)&&(NewNumGauss!=6)) return -1;

	NumGauss = NewNumGauss;
	return NumGauss;
}
short GamessBasisGroup::GetNumGauss(void) const {
	return NumGauss;
}
short GamessBasisGroup::SetNumDFuncs(short NewNum) {
	if (NewNum > 3) return -1;

	NumHeavyFuncs = NewNum + (NumHeavyFuncs & 0xF0);
	return (NumHeavyFuncs & 0x0F);
}
short GamessBasisGroup::GetNumDFuncs(void) const {
	return (NumHeavyFuncs & 0x0F);
}
short GamessBasisGroup::SetNumFFuncs(short NewNum) {
	if (NewNum > 3) return -1;

	NumHeavyFuncs = (NewNum<<4) + (NumHeavyFuncs & 0x0F);
	return ((NumHeavyFuncs & 0xF0)>>4);
}
short GamessBasisGroup::GetNumFFuncs(void) const {
	return ((NumHeavyFuncs & 0xF0)>>4);
}
short GamessBasisGroup::SetNumPFuncs(short NewNum) {
	if (NewNum > 3) return -1;

	NumPFuncs = NewNum;
	return NumPFuncs;
}
short GamessBasisGroup::GetNumPFuncs(void) const {
	return NumPFuncs;
}
short GamessBasisGroup::SetDiffuseSP(bool state) {
	if (state && (!(Flags & 1))) Flags += 1;
	else if (!state && (Flags & 1)) Flags -= 1;

	return state;
}
short GamessBasisGroup::SetDiffuseS(bool state) {
	if (state && (!(Flags & 2))) Flags += 2;
	else if (!state && (Flags & 2)) Flags -= 2;

	return state;
}
GAMESS_BS_Polarization GamessBasisGroup::SetPolar(GAMESS_BS_Polarization NewPolar) {
	if ((NewPolar>=GAMESS_BS_No_Polarization)||(NewPolar<NumGAMESSBSPolarItems)) {
		Polar = NewPolar;
	}
	return Polar;
}
GAMESS_BS_Polarization GamessBasisGroup::SetPolar(const char *PolarText) {
	GAMESS_BS_Polarization NewPolar = GAMESS_BS_Invalid_Polar;

	for (int i=GAMESS_BS_No_Polarization; i<NumGAMESSBSPolarItems; i++) {
		if (!strcasecmp(PolarText, PolarToText((GAMESS_BS_Polarization)i))) {
			NewPolar = (GAMESS_BS_Polarization) i;
			break;
		}
	}
	if (NewPolar>=0) Polar = NewPolar;
	return NewPolar;
}
const char * GamessBasisGroup::PolarToText(GAMESS_BS_Polarization p) {
	switch (p) {
		case GAMESS_BS_No_Polarization:
			return "none";
		case GAMESS_BS_Pople_Polar:
			return "POPLE";
		case GAMESS_BS_PopN311_Polar:
			return "POPN311";
		case GAMESS_BS_Dunning_Polar:
			return "DUNNING";
		case GAMESS_BS_Huzinaga_Polar:
			return "HUZINAGA";
		case GAMESS_BS_Hondo7_Polar:
			return "HONDO7";
	}
	return "invalid";
}
const char * GamessBasisGroup::GAMESSECPToText(GAMESS_BS_ECPotential p) {
	switch (p) {
		case GAMESS_BS_ECP_None:
			return "NONE";
		case GAMESS_BS_ECP_Read:
			return "READ";
		case GAMESS_BS_ECP_SBK:
			return "SBK";
		case GAMESS_BS_ECP_HW:
			return "HW";
	}
	return "invalid";
}
GAMESS_BS_ECPotential GamessBasisGroup::SetECPPotential(const char *ECPText) {
	GAMESS_BS_ECPotential NewPot = GAMESS_BS_Invalid_ECP;
	
	for (int i=GAMESS_BS_ECP_None; i<NumGAMESSBSECPItems; i++) {
		if (!strcasecmp(ECPText, GAMESSECPToText((GAMESS_BS_ECPotential)i))) {
			NewPot = (GAMESS_BS_ECPotential) i;
			break;
		}
	}
	if (NewPot>=0) ECPPotential = NewPot;
	return NewPot;
}
short GamessBasisGroup::GetECPPotential(void) const {
	short value = ECPPotential;
	if (value == 0) {
		if (Basis == 12) value = 2;
		if (Basis == 13) value = 3;
	}
	return value;
}
const char * GamessBasisGroup::GetECPPotentialText(void) const {
	short value = ECPPotential;
	if (value == 0) {
		if (Basis == 12) value = 2;
		if (Basis == 13) value = 3;
	}
	return GAMESSECPToText((GAMESS_BS_ECPotential) value);
}
short GamessBasisGroup::SetECPPotential(short NewType) {
	if ((NewType<0)||(NewType>3)) return -1;
	ECPPotential = NewType;
	return ECPPotential;
}
long GamessBasisGroup::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessBasisGroup::WriteToBuffer(BufferFile *Buffer) {
	long Pos, length = sizeof(GamessBasisGroup);

	Pos = Buffer->Write((Ptr) &length, sizeof(long));
	Pos += Buffer->Write((Ptr) this, length);
	return Pos;
}
long GamessBasisGroup::ReadFromBuffer(BufferFile *Buffer) {
	long mylength, size;

	size = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != sizeof(GamessBasisGroup)) return size;
	size += Buffer->Read((Ptr) this, mylength);
	return size;
}
void GamessBasisGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessBasisGroupElement));
	if (GetBasis() != 0) {
		Ele->addChildElement(CML_convert(MMP_IOBGBasisSet), GetBasisText());
	}
	if (NumGauss) {
		snprintf(line, kMaxLineLength, "%d", NumGauss);
		Ele->addChildElement(CML_convert(MMP_IOBGNumGauss), line);
	}
	if (GetNumDFuncs()) {
		snprintf(line, kMaxLineLength, "%d", GetNumDFuncs());
		Ele->addChildElement(CML_convert(MMP_IOBGNumDFuncs), line);
	}
	if (GetNumFFuncs()) {
		snprintf(line, kMaxLineLength, "%d", GetNumFFuncs());
		Ele->addChildElement(CML_convert(MMP_IOBGNumFFuncs), line);
	}
	if (GetNumPFuncs()) {
		snprintf(line, kMaxLineLength, "%d", GetNumPFuncs());
		Ele->addChildElement(CML_convert(MMP_IOBGNumPFuncs), line);
	}
	if (GetPolar() != 0) {
		Ele->addChildElement(CML_convert(MMP_IOBGPolar), GetPolarText());
	}
	if (GetECPPotential() != 0) {
		Ele->addChildElement(CML_convert(MMP_IOBGECPPotential), GetECPPotentialText());
	}
	if (GetDiffuseSP()) Ele->addChildElement(CML_convert(MMP_IOBGDiffuseSP), trueXML);
	if (GetDiffuseS()) Ele->addChildElement(CML_convert(MMP_IOBGDiffuseS), trueXML);
	if (CheckBasis()) Ele->addChildElement(CML_convert(MMP_IOBGDisableBS), trueXML);
}
void GamessBasisGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IOBGBasisSet:
					{
						const char * v = child->getValue();
						if (v) SetBasis(v);
					}
						break;
					case MMP_IOBGNumGauss:
					{
						long temp;
						if (child->getLongValue(temp)) SetNumGauss(temp);
					}
						break;
					case MMP_IOBGNumDFuncs:
					{
						long temp;
						if (child->getLongValue(temp)) SetNumDFuncs(temp);
					}
						break;
					case MMP_IOBGNumFFuncs:
					{
						long temp;
						if (child->getLongValue(temp)) SetNumFFuncs(temp);
					}
						break;
					case MMP_IOBGNumPFuncs:
					{
						long temp;
						if (child->getLongValue(temp)) SetNumPFuncs(temp);
					}
						break;
					case MMP_IOBGPolar:
					{
						const char * v = child->getValue();
						if (v) SetPolar(v);
					}
						break;
					case MMP_IOBGECPPotential:
					{
						const char * v = child->getValue();
						if (v) SetECPPotential(v);
					}
						break;
					case MMP_IOBGDiffuseSP:
						if (child->getBoolValue(tb))
							SetDiffuseSP(tb);
						break;
					case MMP_IOBGDiffuseS:
						if (child->getBoolValue(tb))
							SetDiffuseS(tb);
						break;
					case MMP_IOBGDisableBS:
						if (child->getBoolValue(tb))
							CheckBasis(tb);
						break;
				}
			}
		}
	}
}
long GamessBasisGroup::WriteToFile(BufferFile *File, MoleculeData * lData) {
	char	Out[133];
		//if a general basis set is present don't punch the $Basis group
	if (lData->GetBasisSet() && (GetBasis() == 0)) return 1;
		//Punch the group label
	File->WriteLine(" $BASIS ", false);
		//Basis Set
	sprintf(Out,"GBASIS=%s ", GetBasisText());
	File->WriteLine(Out, false);
		//Number of Gaussians
	if (NumGauss) {
		sprintf(Out, "NGAUSS=%d ", NumGauss);
		File->WriteLine(Out, false);
	}	//number of heavy atom polarization functions
	if (GetNumDFuncs()) {
		sprintf(Out, "NDFUNC=%d ", GetNumDFuncs());
		File->WriteLine(Out, false);
	}	//number of heavy atom f type polarization functions
	if (GetNumFFuncs()) {
		sprintf(Out, "NFFUNC=%d ", GetNumFFuncs());
		File->WriteLine(Out, false);
	}	//number of light atom polarization functions
	if (NumPFuncs) {
		sprintf(Out, "NPFUNC=%d ", NumPFuncs);
		File->WriteLine(Out, false);
	}	//type of Polarization functions
	if ((Polar)&&((NumHeavyFuncs)||(NumPFuncs))) {
		sprintf(Out, "POLAR=%s ", GetPolarText());
		File->WriteLine(Out, false);
	}
	if (GetDiffuseSP()) {
		sprintf(Out, "DIFFSP=.TRUE. ");
		File->WriteLine(Out, false);
	}
	if (GetDiffuseS()) {
		sprintf(Out, "DIFFS=.TRUE. ");
		File->WriteLine(Out, false);
	}
	File->WriteLine("$END", true);
	return 0;
}
#pragma mark GamessDataGroup
		// Data Group member functions
GamessDataGroup::GamessDataGroup(void) {
	InitData();
}
GamessDataGroup::GamessDataGroup(GamessDataGroup *Copy) {
	if (Copy) {
		*this = *Copy;
		Title = NULL;
		if (Copy->Title) {
			Title = new char[1+strlen(Copy->Title)];
			if (Title) strcpy(Title, Copy->Title);
		}
	}
}
GamessDataGroup::~GamessDataGroup(void) {
	if (Title) delete [] Title;
}
void GamessDataGroup::InitData(void) {
	Title = 0;
	Coord = NumZVar = 0;
	PointGroup = 1;
	PGroupOrder = Options = 0;
	SetUseSym(true);
}
short GamessDataGroup::SetPointGroup(GAMESSPointGroup NewPGroup) {
	if ((NewPGroup<invalidPGroup)||(NewPGroup>NumberGAMESSPointGroups)) return -1;

	PointGroup = NewPGroup;
	return PointGroup;
}
const char * GamessDataGroup::GetGAMESSPointGroupText(GAMESSPointGroup p) {
	switch (p) {
		case GAMESS_C1:
			return "C1";
		case GAMESS_CS:
			return "CS";
		case GAMESS_CI:
			return "CI";
		case GAMESS_CNH:
			return "CNH";
		case GAMESS_CNV:
			return "CNV";
		case GAMESS_CN:
			return "CN";
		case GAMESS_S2N:
			return "S2N";
		case GAMESS_DND:
			return "DND";
		case GAMESS_DNH:
			return "DNH";
		case GAMESS_DN:
			return "DN";
		case GAMESS_TD:
			return "TD";
		case GAMESS_TH:
			return "TH";
		case GAMESS_T:
			return "T";
		case GAMESS_OH:
			return "OH";
		case GAMESS_O:
			return "O";
	}
	return "invalid";
}
short GamessDataGroup::SetPointGroup(char *GroupText) {
	GAMESSPointGroup NewPGroup=invalidPGroup;
	
	if (GroupText[0] == 'S') {
		PGroupOrder = GroupText[2] - 48;
		GroupText[2]='N';
	} else {
		int i=0;
		while (GroupText[i]&&(GroupText[i]!=' ')) {
			if (isdigit(GroupText[i])&&(GroupText[i]!='1')) {
				PGroupOrder = GroupText[i] - 48;	//single digit coverted to decimal digit
				GroupText[i]='N';
			}
			i++;
		}
	}

	for (int i=1; i<NumberGAMESSPointGroups; i++) {
		if (strcmp(GroupText, GetGAMESSPointGroupText((GAMESSPointGroup) i))==0) {
			NewPGroup = (GAMESSPointGroup) i;
			break;
		}
	}
	if (NewPGroup<=invalidPGroup) return invalidPGroup;

	if (NewPGroup<0) return -1;

	PointGroup = NewPGroup;
	return PointGroup;
}
short GamessDataGroup::SetPointGroupOrder(short NewOrder) {
	if (NewOrder > 0) PGroupOrder = NewOrder;
	return PGroupOrder;
}
short GamessDataGroup::SetTitle(const char *NewTitle, long length) {
	if (Title) delete Title;
	Title = NULL;

	if (length == -1) length = strlen(NewTitle);

		long TitleStart=0, TitleEnd=length-1, i, j;
		//Strip blanks of both ends of title
	while ((NewTitle[TitleStart] <= ' ')&&(TitleStart<length)) TitleStart ++;
	while ((NewTitle[TitleEnd] <= ' ')&&(TitleEnd>0)) TitleEnd --;
	length = TitleEnd - TitleStart + 1;

	if (length <= 0) return 0;
	if (length > 132) return -1;	//Title card is limited to one line

	Title = new char[length + 1];
	if (Title == NULL) throw MemoryError();
	j=0;
	for (i=TitleStart; i<=TitleEnd; i++) {
		if ((NewTitle[i] == '\n')||(NewTitle[i] == '\r')) {
			Title[j] = 0;
			break;
		}
		Title[j] = NewTitle[i];
		j++;
	}
	Title[j]=0;
	return j;
}
const char * GamessDataGroup::GetTitle(void) const {
	return Title;
}
CoordinateType GamessDataGroup::GetCoordType(void) const {
	return (CoordinateType) Coord;
}
const char * GamessDataGroup::GetCoordTypeText(CoordinateType t) {
	switch (t) {
		case UniqueCoordType:
			return "UNIQUE";
		case HINTCoordType:
			return "HINT";
		case CartesianCoordType:
			return "CART";
		case ZMTCoordType:
			return "ZMT";
		case ZMTMPCCoordType:
			return "ZMTMPC";
	}
	return "invalid";
}
CoordinateType GamessDataGroup::SetCoordType(const char * CoordText) {
	CoordinateType NewCoord = invalidCoordinateType;
	for (int i=1; i<NumberCoordinateTypes; i++) {
		if (strcmp(CoordText, GetCoordTypeText((CoordinateType) i))==0) {
			NewCoord = (CoordinateType) i;
			break;
		}
	}
	if (NewCoord<=invalidCoordinateType) return invalidCoordinateType;
	Coord = NewCoord;
	return (CoordinateType) Coord;
}
CoordinateType GamessDataGroup::SetCoordType(CoordinateType NewType) {
	if ((NewType<UniqueCoordType)&&(NewType>NumberCoordinateTypes)) return invalidCoordinateType;
	Coord = NewType;
	return (CoordinateType) Coord;
}
bool GamessDataGroup::SetUnits(bool NewType) {
	if (Options & 1) Options -= 1;
	if (NewType) Options += 1;
	return GetUnits();
}
bool GamessDataGroup::SetUseSym(bool State) {
	if (Options & (1<<1)) Options -= (1<<1);
	if (State) Options += (1<<1);
	return GetUseSym();
}
short GamessDataGroup::SetNumZVar(short NewNum) {
	if (NewNum<0) return -1;	//bad number
	NumZVar = NewNum;
	return NumZVar;
}
long GamessDataGroup::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessDataGroup::WriteToBuffer(BufferFile *Buffer) {
	long Pos, length = sizeof(GamessDataGroup);

	Pos = Buffer->Write((Ptr) &length, sizeof(long));
	Pos += Buffer->Write((Ptr) this, length);

	if (Title) {
		length = 1;
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		length = 1 + strlen(Title);
		Pos += Buffer->Write((Ptr) &length, sizeof(long));
		Pos += Buffer->Write((Ptr) Title, length);
	}
	return Pos;
}
void GamessDataGroup::ReadFromBuffer(BufferFile *Buffer, long length) {
	long mylength, pos, code;

	pos = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != sizeof(GamessDataGroup)) return;
	pos += Buffer->Read((Ptr) this, mylength);
	if (pos < length) {
		pos += Buffer->Read((Ptr) &code,sizeof(long));
		if (code == 1) {	//Title card
			pos += Buffer->Read((Ptr) &mylength, sizeof(long));
			Title = new char[mylength];
			if (Title) {
				pos += Buffer->Read(Title, mylength);
			} else throw MemoryError();
		}
	}
}
void GamessDataGroup::WriteToFile(BufferFile *File, MoleculeData * MainData, WinPrefs * Prefs, long BasisTest) {
	char	Out[133];

	Frame * cFrame = MainData->GetCurrentFramePtr();
	BasisSet * lBasis = MainData->GetBasisSet();
	BasisTest = BasisTest && lBasis;	//Make sure there really is a basis set defined
//	if (BasisTest) File->WriteLine(" $CONTRL NORMP=1 $END", true);
		//Punch the group label
	File->WriteLine(" $DATA ", true);
		//title
	if (Title == NULL) File->WriteLine("Title goes here", true);
	else File->WriteLine(Title, true);
		//Point Group
	if ((PointGroup>GAMESS_CI)&&(PointGroup<GAMESS_TD)) {
		sprintf(Out, "%s %d", GetPointGroupText(), PGroupOrder);
	} else sprintf(Out, "%s", GetPointGroupText());
	File->WriteLine(Out, true);
	if ((PointGroup!=0)&&(PointGroup!=1)) File->WriteLine("", true);
		//coordinates
	if (Coord == ZMTCoordType) {	//"normal" style z-matrix
		Internals * IntCoords = MainData->GetInternalCoordinates();
		if (IntCoords) IntCoords->WriteCoordinatesToFile(File, MainData, Prefs);
	} else if (Coord == ZMTMPCCoordType) {
		Internals * IntCoords = MainData->GetInternalCoordinates();
		if (IntCoords) IntCoords->WriteMPCZMatCoordinatesToFile(File, MainData, Prefs);
	} else {
		for (int iatom=0; iatom<cFrame->NumAtoms; iatom++) {
			Str255 AtomLabel;
			Prefs->GetAtomLabel(cFrame->Atoms[iatom].GetType()-1, AtomLabel);
			AtomLabel[AtomLabel[0]+1] = 0;
			sprintf(Out, "%s   %5.1f  %10.5f  %10.5f  %10.5f",
				(char *) &(AtomLabel[1]), (float) (cFrame->Atoms[iatom].GetType()), 
				cFrame->Atoms[iatom].Position.x, cFrame->Atoms[iatom].Position.y,
				cFrame->Atoms[iatom].Position.z);
			File->WriteLine(Out, true);
			if (BasisTest) lBasis->WriteBasis(File, iatom);
		}
	}
	
	File->WriteLine(" $END", true);
	if (NumZVar) {	//punch out the current connectivity in a $ZMAT group
		Internals * IntCoords = MainData->GetInternalCoordinates();
		if (IntCoords) IntCoords->WriteZMATToFile(File);
	}
}
void GamessDataGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessDataGroupElement));
	if (Title) Ele->addChildElement(CML_convert(MMP_IODGTitle), Title);
	if (PointGroup) {
		snprintf(line, kMaxLineLength, "%d", PointGroup);
		XMLElement * t = Ele->addChildElement(CML_convert(MMP_IODGPointGroup), line);
		if (PGroupOrder) {
			snprintf(line, kMaxLineLength, "%d", PGroupOrder);
			t->addAttribute(CML_convert(MMP_IODGPointGroupOrder), line);
		}
	}
	if (Coord) Ele->addChildElement(CML_convert(MMP_IODGCoordType), GetCoordTypeText((CoordinateType) Coord));
	if (GetNumZVar()) {
		snprintf(line, kMaxLineLength, "%d", GetNumZVar());
		Ele->addChildElement(CML_convert(MMP_IODGNumZVars), line);
	}
	if (GetUnits()) Ele->addChildElement(CML_convert(MMP_IODGUnits), trueXML);
	if (GetUseSym()) Ele->addChildElement(CML_convert(MMP_IODGNoSymFlag), trueXML);
}
void GamessDataGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IODGTitle:
					{
						const char * v = child->getValue();
						if (v) SetTitle(v);
					}
						break;
					case MMP_IODGPointGroup:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetPointGroup((GAMESSPointGroup)temp);
							if (child->getAttributeValue(CML_convert(MMP_IODGPointGroupOrder), temp)) 
								SetPointGroupOrder(temp);
						}
					}
						break;
					case MMP_IODGCoordType:
					{
						const char * v = child->getValue();
						if (v) SetCoordType(v);
					}
						break;
					case MMP_IODGNumZVars:
					{
						long temp;
						if (child->getLongValue(temp)) SetNumZVar(temp);
					}
						break;
					case MMP_IODGUnits:
						if (child->getBoolValue(tb))
							SetUnits(tb);
						break;
					case MMP_IODGNoSymFlag:
						if (child->getBoolValue(tb))
							SetUseSym(tb);
						break;
				}
			}
		}
	}
}
#pragma mark GamessGuessGroup
	//Guess Group functions
//This function is here to provide a default value before returning the string
const char * GamessGuessGroup::GetGuessText(void) const {
	short value = GetGuess();
	if (value == 0) value = 1;

	return ConvertGuessType(value);
}
short GamessGuessGroup::SetGuess(const char * GuessText) {
	short NewGuess = -1;

	for (int i=1; i<NumberGuessTypes; i++) {
		const char * val = ConvertGuessType(i);
		if (-1<LocateKeyWord(GuessText, val, strlen(val), 7)) {
			NewGuess = i;
			break;
		}
	}
	if (NewGuess<0) return -1;
	NewGuess = SetGuess(NewGuess);
	return NewGuess;
}
const char * GamessGuessGroup::ConvertGuessType(const int & type) {
	switch (type) {
		case HUCKELGuessType:
			return "HUCKEL";
		case HCOREGuessType:
			return "HCORE";
		case MOREADGuessType:
			return "MOREAD";
		case MOSAVEDGuessType:
			return "MOSAVED";
		case SkipGuessType:
			return "SKIP";	//By hand later?
		default:
			return "invalid";
	}
	return NULL;
}
GamessGuessGroup::GuessGroup(void) {
	InitData();
}
GamessGuessGroup::GuessGroup(GuessGroup *Copy) {	//copy constructor
	if (Copy) {
		*this = *Copy;
		IOrder = JOrder = NULL;
			//check and copy I & J order here
	}
}
void GamessGuessGroup::InitData(void) {
	MOTolZ = MOTolEquil = 0.0;
	IOrder = JOrder = NULL;
	NumOrbs = 0;
	VecSource = 0;
	GuessType = 0;
	Options = 0;
}
long GamessGuessGroup::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessGuessGroup::WriteToBuffer(BufferFile *Buffer) {
	long total, code, length = sizeof(GamessGuessGroup);

	code = 1;
	total = Buffer->Write((Ptr) &code, sizeof(long));
	length = 2*sizeof(float) + sizeof(long) + 2*sizeof(short) + sizeof(char);
	total += Buffer->Write((Ptr) &length, sizeof(long));
	total += Buffer->Write((Ptr) &MOTolZ, sizeof(float));
	total += Buffer->Write((Ptr) &MOTolEquil, sizeof(float));
	total += Buffer->Write((Ptr) &NumOrbs, sizeof(long));
	total += Buffer->Write((Ptr) &VecSource, sizeof(short));
	total += Buffer->Write((Ptr) &GuessType, sizeof(short));
	total += Buffer->Write((Ptr) &Options, sizeof(char));

		//copy iorder and jorder too

	return total;
}
long GamessGuessGroup::ReadFromBuffer(BufferFile *Buffer) {
	long mylength, size;

	size = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != 1) return size;
	size += Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != (2*sizeof(float) + sizeof(long) + 2*sizeof(short) + sizeof(char))) return size;
	
	size += Buffer->Read((Ptr) &MOTolZ, sizeof(float));
	size += Buffer->Read((Ptr) &MOTolEquil, sizeof(float));
	size += Buffer->Read((Ptr) &NumOrbs, sizeof(long));
	size += Buffer->Read((Ptr) &VecSource, sizeof(short));
	size += Buffer->Read((Ptr) &GuessType, sizeof(short));
	size += Buffer->Read((Ptr) &Options, sizeof(char));
	return size;
}
void GamessGuessGroup::WriteToFile(BufferFile *File, GamessInputData *IData, MoleculeData * MainData) {
	long	test=false;
	char	Out[133];

	Frame * lFrame = MainData->GetCurrentFramePtr();
		//first determine wether or not the Guess group needs to be punched
	if (GetGuess()) test = true;
	if (GetPrintMO()) test = true;
	if (GetMix()&&IData->Control->GetMultiplicity()&&
		(IData->Control->GetSCFType()==2)) test = true;

	if (!test) return;

		//Punch the group label
	File->WriteLine(" $GUESS ", false);
		//Guess Type
	if (GetGuess()) {
		sprintf(Out,"GUESS=%s ", GetGuessText());
		File->WriteLine(Out, false);
	}
		//NumOrbs
	if (GetGuess()==3) {
		long nOrbs = GetNumOrbs();
		if (!nOrbs) {	//Make a guess if the guess comes from local orbs
			short tempVec = GetVecSource();
			const std::vector<OrbitalRec *> * Orbs = lFrame->GetOrbitalSetVector();
			if (Orbs->size() > 0) {
				if ((tempVec<=0)||(tempVec>Orbs->size() + 2)) tempVec = 2;
				if (tempVec > 1) {
					OrbitalRec * OrbSet = (*Orbs)[tempVec-2];
					nOrbs = OrbSet->getNumOccupiedAlphaOrbitals();
					if (nOrbs <= 0) nOrbs = OrbSet->getNumAlphaOrbitals();
				}
			}
		}
		sprintf(Out, "NORB=%d ", nOrbs);
		File->WriteLine(Out, false);
	}	//PrintMO
	if (GetPrintMO()) {
		sprintf(Out, "PRTMO=.TRUE. ");
		File->WriteLine(Out, false);
	}	//Mix
	if (GetMix()&&((IData->Control->GetMultiplicity()==1)||
			(IData->Control->GetMultiplicity()==0))&&(IData->Control->GetSCFType()==2)) {
		sprintf(Out, "MIX=.TRUE. ");
		File->WriteLine(Out, false);
	}
	File->WriteLine("$END", true);
}
void GamessGuessGroup::WriteVecGroup(BufferFile *File, MoleculeData * lData) {
		//prepare to punch out $Vec information if Guess=MORead
	if (GetGuess() == 3) {
		Frame * lFrame = lData->GetCurrentFramePtr();
		BasisSet * lBasis = lData->GetBasisSet();
		long NumBasisFuncs = lBasis->GetNumBasisFuncs(false);
		short tempVec = GetVecSource();
		const std::vector<OrbitalRec *> * Orbs = lFrame->GetOrbitalSetVector();
		if ((tempVec != 1)&&(Orbs->size() > 0)) {
			if ((tempVec<=0)||(tempVec>Orbs->size() + 2)) tempVec = 2;
			if (tempVec > 1) {
				OrbitalRec * OrbSet = (*Orbs)[tempVec-2];
				long nOrbs = GetNumOrbs();
				if (nOrbs <= 0) {	//Setup the default value for the orbital count
					nOrbs = OrbSet->getNumOccupiedAlphaOrbitals();
					if (nOrbs <= 0) nOrbs = OrbSet->getNumAlphaOrbitals();
				}
				OrbSet->WriteVecGroup(File, NumBasisFuncs, nOrbs);
			}
		} else {
			File->WriteLine("You must provide a $VEC group here!", true);
		}
	}
}
void GamessGuessGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessGuessGroupElement));
	if (GuessType) Ele->addChildElement(CML_convert(MMP_IOGGGuessType), ConvertGuessType(GuessType));
	if (NumOrbs) {
		snprintf(line, kMaxLineLength, "%d", NumOrbs);
		Ele->addChildElement(CML_convert(MMP_IOGGNumOrbs), line);
	}
	if (VecSource) {
		snprintf(line, kMaxLineLength, "%d", VecSource);
		Ele->addChildElement(CML_convert(MMP_IOGGVecSource), line);
	}
	if (GetPrintMO()) Ele->addChildElement(CML_convert(MMP_IOGGPrintMO), trueXML);
	if (GetNOrder()) Ele->addChildElement(CML_convert(MMP_IOGGOrbReorder), trueXML);
	if (GetMix()) Ele->addChildElement(CML_convert(MMP_IOGGOrbMix), trueXML);
	if (MOTolZ) {
		snprintf(line, kMaxLineLength, "%f", MOTolZ);
		Ele->addChildElement(CML_convert(MMP_IOGGMOTolZ), line);
	}
	if (MOTolEquil) {
		snprintf(line, kMaxLineLength, "%f", MOTolEquil);
		Ele->addChildElement(CML_convert(MMP_IOGGMOTolEquil), line);
	}
}
void GamessGuessGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IOGGGuessType:
					{
						const char * v = child->getValue();
						if (v) SetGuess(v);
					}
						break;
					case MMP_IOGGNumOrbs:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetNumOrbs(temp);
						}
					}
						break;
					case MMP_IOGGVecSource:
					{
						long temp;
						if (child->getLongValue(temp)) {
							VecSource = temp;
						}
					}
						break;
					case MMP_IOGGPrintMO:
						if (child->getBoolValue(tb))
							SetPrintMO(tb);
						break;
					case MMP_IOGGOrbReorder:
						if (child->getBoolValue(tb))
							SetNOrder(tb);
						break;
					case MMP_IOGGOrbMix:
						if (child->getBoolValue(tb))
							SetMix(tb);
						break;
					case MMP_IOGGMOTolZ:
					{
						double temp;
						if (child->getDoubleValue(temp)) MOTolZ = temp;
					}
						break;
					case MMP_IOGGMOTolEquil:
					{
						double temp;
						if (child->getDoubleValue(temp)) MOTolEquil = temp;
					}
						break;
				}
			}
		}
	}
}
#pragma mark GamessSCFGroup
GamessSCFGroup::SCFGroup(void) {
	InitData();
}
GamessSCFGroup::SCFGroup(SCFGroup *Copy) {
	if (Copy)
		*this = *Copy;
	else
		GamessSCFGroup();
}
void GamessSCFGroup::InitData(void) {
	SOGTolerance = EnErrThresh = DEMCutoff = DampCutoff = 0.0;
	ConvCriteria = MaxDIISEq = MVOCharge = 0;
	Punch = Options1 = ConverganceFlags = 0;
		//default Direct SCF to true. This is not the GAMESS default
		//but is better in most cases.
	SetDirectSCF(true);
	SetFockDiff(true);
}
bool GamessSCFGroup::SetDirectSCF(bool State) {
	if (Options1 & 1) Options1--;
	if (State) Options1 ++;
	return GetDirectSCF();
}
bool GamessSCFGroup::SetFockDiff(bool State) {
	if (Options1 & 2) Options1 -= 2;
	if (State) Options1 += 2;
	return GetFockDiff();
}
bool GamessSCFGroup::SetUHFNO(bool State) {
	if (Options1 & 4) Options1 -= 4;
	if (State) Options1 += 4;
	return GetUHFNO();
}
short GamessSCFGroup::SetConvergance(short NewConv) {
	if (NewConv > 0) ConvCriteria = NewConv;
	return ConvCriteria;
}
long GamessSCFGroup::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessSCFGroup::WriteToBuffer(BufferFile *Buffer) {
	long	pos, length = sizeof(GamessSCFGroup);

	pos = Buffer->Write((Ptr) &length, sizeof(long));
	pos += Buffer->Write((Ptr) this, length);
	return pos;
}
long GamessSCFGroup::ReadFromBuffer(BufferFile *Buffer) {
	long mylength, size;

	size = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != sizeof(GamessSCFGroup)) return size;
	size += Buffer->Read((Ptr) this, mylength);
	return size;
}
void GamessSCFGroup::WriteToFile(BufferFile *File, GamessInputData *IData) {
	long	test=false;
	char	Out[133];

		//first determine wether or not the SCF group needs to be punched
	if (IData->Control->GetSCFType() > 4) return;	//not relavent to the selected SCF type
	if (ConvCriteria > 0) test = true;
	if (GetDirectSCF()) test = true;
	
	if (!test) return;

		//Punch the group label
	File->WriteLine(" $SCF ", false);
		//Direct SCF
	if (GetDirectSCF()) {
		sprintf(Out,"DIRSCF=.TRUE. ");
		File->WriteLine(Out, false);
		if (!GetFockDiff()) {	//Fock Differencing requires direct SCF
			sprintf(Out,"FDIFF=.FALSE. ");
			File->WriteLine(Out, false);
		}
	}
		//convergance
	if (ConvCriteria > 0) {
		sprintf(Out, "NCONV=%d ", ConvCriteria);
		File->WriteLine(Out, false);
	}	//UHF Natural Orbitals
	if (GetUHFNO()) {
		sprintf(Out, "UHFNOS=.TRUE. ");
		File->WriteLine(Out, false);
	}

	File->WriteLine("$END", true);
}
void GamessSCFGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessSCFGroupElement));
	if (GetConvergance()) {
		snprintf(line, kMaxLineLength, "%d", GetConvergance());
		Ele->addChildElement(CML_convert(MMP_IOSGConvCriteria), line);
	}
	if (GetDirectSCF()) Ele->addChildElement(CML_convert(MMP_IOSGDirectSCF), trueXML);
	if (GetFockDiff()) Ele->addChildElement(CML_convert(MMP_IOSGFockDiff), trueXML);
	if (GetUHFNO()) Ele->addChildElement(CML_convert(MMP_IOSGUHFNauralOrbitals), trueXML);
}
void GamessSCFGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IOSGConvCriteria:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetConvergance(temp);
						}
					}
						break;
					case MMP_IOSGDirectSCF:
						if (child->getBoolValue(tb))
							SetDirectSCF(tb);
						break;
					case MMP_IOSGFockDiff:
						if (child->getBoolValue(tb))
							SetFockDiff(tb);
						break;
					case MMP_IOSGUHFNauralOrbitals:
						if (child->getBoolValue(tb))
							SetUHFNO(tb);
						break;
				}
			}
		}
	}
}
#pragma mark GamessMP2Group
GamessMP2Group::MP2Group(void) {
	InitData();
}
GamessMP2Group::MP2Group(MP2Group *Copy) {
	*this = *Copy;
}
void GamessMP2Group::InitData(void) {
	CutOff = 0.0;
	NumCoreElectrons = -1;
	Memory = 0;
	Method = AOInts = LMOMP2 = 0;
	MP2Prop = false;
}
float GamessMP2Group::SetIntCutoff(float NewCutoff) {
	if (NewCutoff > 0.0) CutOff = NewCutoff;
	return CutOff;
}
long GamessMP2Group::SetNumCoreElectrons(long NewNum) {
	if (NewNum>=0) NumCoreElectrons = NewNum;
	return NumCoreElectrons;
}
long GamessMP2Group::SetMemory(long NewMem) {
	if (NewMem >= 0) Memory = NewMem;
	return Memory;
}
char GamessMP2Group::SetMethod(char NewMethod) {
	if ((NewMethod==2)||(NewMethod==3)) Method = NewMethod;
	return Method;
}
const char * GamessMP2Group::GetAOIntMethodText(void) const {
	if (AOInts == 0) return NULL;
	if (AOInts == 1) return "DUP";
	return "DIST";
}
void GamessMP2Group::SetAOIntMethod(const char * t) {
	if (!t) return;
	if (!strcmp(t, "DUP")) AOInts = 1;
	else if (!strcmp(t, "DIST")) AOInts = 2;
}
char GamessMP2Group::SetAOIntMethod(char NewMethod) {
	if ((NewMethod == 1)||(NewMethod == 2)) AOInts = NewMethod;
	return AOInts;
}
bool GamessMP2Group::GetLMOMP2(void) const {
	if (LMOMP2) return true;
	return false;
}
bool GamessMP2Group::SetLMOMP2(bool State) {
	if (State) LMOMP2 = true;
	else LMOMP2 = false;
	return LMOMP2;
}
long GamessMP2Group::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessMP2Group::WriteToBuffer(BufferFile *Buffer) {
	long	pos, length=sizeof(GamessMP2Group);

	pos = Buffer->Write((Ptr) &length, sizeof(long));
	pos += Buffer->Write((Ptr) this, length);
	return pos;
}
long GamessMP2Group::ReadFromBuffer(BufferFile *Buffer) {
	long mylength, size;

	size = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != sizeof(GamessMP2Group)) return size;
	size += Buffer->Read((Ptr) this, mylength);
	return size;
}
void GamessMP2Group::WriteToFile(BufferFile *File, GamessInputData *IData) {
	long	test=false;
	char	Out[133];

		//first determine wether or not the MP2 group needs to be punched
	if (IData->Control->GetMPLevel() != 2) return;	//Don't punch if MP2 isn't active
	if ((NumCoreElectrons>=0)||Memory||Method||AOInts) test = true;
	if (GetLMOMP2()) test = true;
	if (CutOff > 0.0) test = true;
	
	if (!test) return;

		//Punch the group label
	File->WriteLine(" $MP2 ", false);
		//core electrons
	if (NumCoreElectrons >= 0) {
		sprintf(Out,"NACORE=%ld ", NumCoreElectrons);
		File->WriteLine(Out, false);
		if (IData->Control->GetSCFType() == GAMESS_UHF) {
			sprintf(Out,"NBCORE=%ld ", NumCoreElectrons);
			File->WriteLine(Out, false);
		}
	}
	//MP2Prop
	if ((IData->Control->GetRunType() <= Energy) && GetMP2Prop()) {
		sprintf(Out, "MP2PRP=.TRUE. ");
		File->WriteLine(Out, false);
	}
		//LMOMP2
	if (GetLMOMP2()) {
		sprintf(Out, "LMOMP2=.TRUE. ");
		File->WriteLine(Out, false);
	}	//Memory
	if (Memory) {
		sprintf(Out, "NWORD=%ld ",Memory);
		File->WriteLine(Out, false);
	}	//CutOff
	if (CutOff > 0.0) {
		sprintf(Out, "CUTOFF=%.2e ", CutOff);
		File->WriteLine(Out, false);
	}	//Method
	if (Method) {
		sprintf(Out, "METHOD=%d ", Method);
		File->WriteLine(Out, false);
	}	//AO storage
	if (AOInts) {
		sprintf(Out, "AOINTS=%s ", GetAOIntMethodText());
		File->WriteLine(Out, false);
	}

	File->WriteLine("$END", true);
}
void GamessMP2Group::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessMP2GroupElement));
	if (NumCoreElectrons) {
		snprintf(line, kMaxLineLength, "%d", NumCoreElectrons);
		Ele->addChildElement(CML_convert(MMP_IOMGNumCoreElectrons), line);
	}
	if (CutOff > 0.0) {
		snprintf(line, kMaxLineLength, "%lf", CutOff);
		Ele->addChildElement(CML_convert(MMP_IOMGCutOff), line);
	}
	if (Memory) {
		snprintf(line, kMaxLineLength, "%d", Memory);
		Ele->addChildElement(CML_convert(MMP_IOMGMemory), line);
	}
	if (Method) {
		snprintf(line, kMaxLineLength, "%d", Method);
		Ele->addChildElement(CML_convert(MMP_IOMGTransMethod), line);
	}
	if (AOInts) Ele->addChildElement(CML_convert(MMP_IOMGAOInts), GetAOIntMethodText());
	if (GetLMOMP2()) Ele->addChildElement(CML_convert(MMP_IOMGLMOMP2), trueXML);
	if (GetMP2Prop()) Ele->addChildElement(CML_convert(MMP_IOMP2MP2PRP), trueXML);
}
void GamessMP2Group::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				switch (item) {
					case MMP_IOMGNumCoreElectrons:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetNumCoreElectrons(temp);
						}
					}
						break;
					case MMP_IOMGCutOff:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							SetIntCutoff(temp);
						}
					}
						break;
					case MMP_IOMGMemory:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetMemory(temp);
						}
					}
						break;
					case MMP_IOMGTransMethod:
					{
						long temp;
						if (child->getLongValue(temp)) {
							Method = temp;
						}
					}
						break;
					case MMP_IOMGAOInts:
					{
						const char * v = child->getValue();
						if (v) {
							SetAOIntMethod(v);
						}
					}
						break;
					case MMP_IOMGLMOMP2:
					{
						bool tb;
						if (child->getBoolValue(tb))
							SetLMOMP2(tb);
					}
						break;
					case MMP_IOMP2MP2PRP:
					{
						bool tb;
						if (child->getBoolValue(tb))
							SetMP2Prop(tb);
					}
						break;
				}
			}
		}
	}
}
#pragma mark GamessHessianGroup
void GamessHessianGroup::InitData(void) {
	DisplacementSize = 0.01;
	FrequencyScaleFactor = 1.0;
	BitOptions = 17;	//bit 1 + bit 5
}
long GamessHessianGroup::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessHessianGroup::ReadFromBuffer(BufferFile *Buffer) {
	long mylength, size;

	size = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != sizeof(GamessHessianGroup)) return size;
	size += Buffer->Read((Ptr) this, mylength);
	return size;
}
long GamessHessianGroup::WriteToBuffer(BufferFile *Buffer) {
	long	pos, length=sizeof(GamessHessianGroup);

	pos = Buffer->Write((Ptr) &length, sizeof(long));
	pos += Buffer->Write((Ptr) this, length);
	return pos;
}
void GamessHessianGroup::WriteToFile(BufferFile *File, GamessInputData *IData) {
	Boolean	method=false;
	char	Out[133];

		//first determine wether or not the hessian group needs to be punched
		//punch for hessians and optimize/sadpoint runs using Hess=Calc
	if (IData->Control->GetRunType() == 3) method = true;
	else if ((IData->Control->GetRunType() == 4)||(IData->Control->GetRunType() == 6)) {
		if (IData->StatPt) {
			if (IData->StatPt->GetHessMethod() == 3) method = true;
		}
	}
	if (!method) return;

	bool AnalyticPoss = (((IData->Control->GetSCFType() == 1)||(IData->Control->GetSCFType() == 3)||
						  (IData->Control->GetSCFType() == 4)||(IData->Control->GetSCFType() == 0))&&
						 (IData->Control->GetMPLevel() == 0));
	method = GetAnalyticMethod() && AnalyticPoss;
		//Punch the group label
	File->WriteLine(" $FORCE ", false);
		//Method
	if (method) File->WriteLine("METHOD=ANALYTIC ", false);
	else File->WriteLine("METHOD=SEMINUM ", false);
	if (!method) {
			//NVIB
		if (GetDoubleDiff()) {
			File->WriteLine("NVIB=2 ", false);
		}	//Vib Size
		if (DisplacementSize != 0.01) {
			sprintf(Out, "VIBSIZ=%f ", DisplacementSize);
			File->WriteLine(Out, false);
		}
	}	//Purify
	if (GetPurify()) {
		File->WriteLine("PURIFY=.TRUE. ", false);
	}	//Print internal FC's
	if (GetPrintFC()) {
		File->WriteLine("PRTIFC=.TRUE. ", false);
	}	//vib analysis
	if (GetVibAnalysis()) {
		File->WriteLine("VIBANL=.TRUE. ", false);
		if (FrequencyScaleFactor != 1.0) {
			sprintf(Out, "SCLFAC=%f ", FrequencyScaleFactor);
			File->WriteLine(Out, false);
		}
	} else File->WriteLine("VIBANL=.FALSE. ", false);

	File->WriteLine("$END", true);
}
void GamessHessianGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessHessianGroupElement));
	if (DisplacementSize > 0.0) {
		snprintf(line, kMaxLineLength, "%f", DisplacementSize);
		Ele->addChildElement(CML_convert(MMP_IOHGDisplacementSize), line);
	}
	if (FrequencyScaleFactor > 0.0) {
		snprintf(line, kMaxLineLength, "%lf", FrequencyScaleFactor);
		Ele->addChildElement(CML_convert(MMP_IOHGFrequencyScaleFactor), line);
	}
	if (GetAnalyticMethod()) Ele->addChildElement(CML_convert(MMP_IOHGMethod), "analytic");
	else Ele->addChildElement(CML_convert(MMP_IOHGMethod), "numeric");
	if (GetPurify()) Ele->addChildElement(CML_convert(MMP_IOHGPurify), trueXML);
	if (GetPrintFC()) Ele->addChildElement(CML_convert(MMP_IOHGInternalFC), trueXML);
	if (GetVibAnalysis()) Ele->addChildElement(CML_convert(MMP_IOHGVibAnalysis), trueXML);
}
void GamessHessianGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IOHGDisplacementSize:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							SetDisplacementSize(temp);
						}
					}
						break;
					case MMP_IOHGFrequencyScaleFactor:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							SetFreqScale(temp);
						}
					}
						break;
					case MMP_IOHGMethod:
					{
						const char * v = child->getValue();
						if (v) {
							if (!strcasecmp(v, "analytic")) SetAnalyticMethod(true);
							else if (!strcasecmp(v, "numeric")) SetAnalyticMethod(false);
						}
					}
						break;
					case MMP_IOHGPurify:
						if (child->getBoolValue(tb))
							SetPurify(tb);
						break;
					case MMP_IOHGInternalFC:
						if (child->getBoolValue(tb))
							SetPrintFC(tb);
						break;
					case MMP_IOHGVibAnalysis:
						if (child->getBoolValue(tb))
							SetVibAnalysis(tb);
						break;
				}
			}
		}
	}
}
#pragma mark GamessDFTGroup
void GamessDFTGroup::InitData(void) {
	GridSwitch = 3.0e-4;
	Threshold = 1.0e-4;
	Functional = 0;
	NumRadialGrids = 96;
	NumThetaGrids = 12;
	NumPhiGrids = 24;
	NumRadialGridsInit = 24;
	NumThetaGridsInit = 8;
	NumPhiGridsInit = 16;
	BitFlags = 0;
	SetAuxFunctions(true);
	SetMethodGrid(true);
}
long GamessDFTGroup::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessDFTGroup::WriteToBuffer(BufferFile *Buffer) {
	long	pos, length=2*sizeof(float)+7*sizeof(short)+sizeof(char);

	pos = Buffer->Write((Ptr) &length, sizeof(long));
	pos += Buffer->Write((Ptr) &GridSwitch, sizeof(float));
	pos += Buffer->Write((Ptr) &Threshold, sizeof(float));
	pos += Buffer->Write((Ptr) &Functional, sizeof(short));
	pos += Buffer->Write((Ptr) &NumRadialGrids, sizeof(short));
	pos += Buffer->Write((Ptr) &NumThetaGrids, sizeof(short));
	pos += Buffer->Write((Ptr) &NumPhiGrids, sizeof(short));
	pos += Buffer->Write((Ptr) &NumRadialGridsInit, sizeof(short));
	pos += Buffer->Write((Ptr) &NumThetaGridsInit, sizeof(short));
	pos += Buffer->Write((Ptr) &NumPhiGridsInit, sizeof(short));
	pos += Buffer->Write((Ptr) &BitFlags, sizeof(char));
	return pos;
}
long GamessDFTGroup::ReadFromBuffer(BufferFile *Buffer) {
	long mylength, size, length = 2*sizeof(float)+7*sizeof(short)+sizeof(char);

	size = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != length) return size;
	size += Buffer->Read((Ptr) this, mylength);
	size += Buffer->Read((Ptr) &GridSwitch, sizeof(float));
	size += Buffer->Read((Ptr) &Threshold, sizeof(float));
	size += Buffer->Read((Ptr) &Functional, sizeof(short));
	size += Buffer->Read((Ptr) &NumRadialGrids, sizeof(short));
	size += Buffer->Read((Ptr) &NumThetaGrids, sizeof(short));
	size += Buffer->Read((Ptr) &NumPhiGrids, sizeof(short));
	size += Buffer->Read((Ptr) &NumRadialGridsInit, sizeof(short));
	size += Buffer->Read((Ptr) &NumThetaGridsInit, sizeof(short));
	size += Buffer->Read((Ptr) &NumPhiGridsInit, sizeof(short));
	size += Buffer->Read((Ptr) &BitFlags, sizeof(char));
	return size;
}
void GamessDFTGroup::WriteToFile(BufferFile *File, GamessInputData *IData) {
	char	Out[kMaxLineLength];

	short SCFType = IData->Control->GetSCFType();
		//first determine wether or not the DFT group needs to be punched
	if ((SCFType > 3)|| !IData->Control->UseDFT()) return;//only punch for HF runtypes (RHF, ROHF, UHF)
	if (MethodGrid()) return; //Only need this group for gridfree method currently
		//Punch the group label
	File->WriteLine(" $DFT ", false);
		//Write out the funtional, and any other optional parameters
		//Method
	if (!MethodGrid()) {	//punch method if it needs to be grid-free
		sprintf(Out, "METHOD=GRIDFREE ");
		File->WriteLine(Out, false);
	}

	File->WriteLine("$END", true);
}
const char * GamessDFTGroup::GetDFTGridFuncText(DFTFunctionalsGrid type) {
	switch (type) {
		case DFT_Grid_Slater:
			return "SLATER";
		case DFT_Grid_Becke:
			return "BECKE";
		case DFT_Grid_VWN:
			return "VWN";
		case DFT_Grid_LYP:
			return "LYP";
		case DFT_Grid_SVWN:
			return "SVWN";
		case DFT_Grid_BVWN:
			return "BVWN";
		case DFT_Grid_BLYP:
			return "BLYP";
		case DFT_Grid_B3LYP:
			return "B3LYP";
		case DFT_Grid_GILL:
			return "GILL";
		case DFT_Grid_PBE:
			return "PBE";
		case DFT_Grid_OP:
			return "OP";
		case DFT_Grid_SLYP:
			return "SLYP";
		case DFT_Grid_SOP:
			return "SOP";
		case DFT_Grid_BOP:
			return "BOP";
		case DFT_Grid_GVWN:
			return "GVWN";
		case DFT_Grid_GLYP:
			return "GLYP";
		case DFT_Grid_GOP:
			return "GOP";
		case DFT_Grid_PBEVWN:
			return "PBEVWN";
		case DFT_Grid_PBELYP:
			return "PBELYP";
		case DFT_Grid_PBEOP:
			return "PBEOP";
		case DFT_Grid_BHHLYP:
			return "BHHLYP";
	}
	return "invalid";
}
const char * GamessDFTGroup::GetDFTGridFreeFuncText(DFTFunctionalsGridFree type) {
	switch (type) {
		case DFT_GridFree_Slater:
			return "SLATER";
		case DFT_GridFree_Becke:
			return "BECKE";
		case DFT_GridFree_VWN:
			return "VWN";
		case DFT_GridFree_LYP:
			return "LYP";
		case DFT_GridFree_SVWN:
			return "SVWN";
		case DFT_GridFree_BVWN:
			return "BVWN";
		case DFT_GridFree_BLYP:
			return "BLYP";
		case DFT_GridFree_B3LYP:
			return "B3LYP";
		case DFT_GridFree_XALPHA:
			return "XALPHA";
		case DFT_GridFree_Depristo:
			return "DEPRISTO";
		case DFT_GridFree_CAMA:
			return "CAMA";
		case DFT_GridFree_HALF:
			return "HALF";
		case DFT_GridFree_PWLOC:
			return "PWLOC";
		case DFT_GridFree_BPWLOC:
			return "BPWLOC";
		case DFT_GridFree_CAMB:
			return "CAMB";
		case DFT_GridFree_XVWN:
			return "XVWN";
		case DFT_GridFree_XPWLOC:
			return "XPWLOC";
		case DFT_GridFree_SPWLOC:
			return "SPWLOC";
		case DFT_GridFree_WIGNER:
			return "WIGNER";
		case DFT_GridFree_WS:
			return "WS";
		case DFT_GridFree_WIGEXP:
			return "WIGEXP";
	}
	return "invalid";
}

const char * GamessDFTGroup::GetFunctionalText(void) const {
	short temp = Functional;
	if (temp <= 0) temp = 1;
	if (MethodGrid()) {
		return GetDFTGridFuncText((DFTFunctionalsGrid) temp);
	} else {	//Grid-free functional list is fairly different
		return GetDFTGridFreeFuncText((DFTFunctionalsGridFree) temp);
	}
	return NULL;
}
short GamessDFTGroup::SetFunctional(short newvalue) {
		//Probably need some checks here??
	Functional = newvalue;
	return Functional;
}
void GamessDFTGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessDFTGroupElement));
	snprintf(line, kMaxLineLength, "%f", GridSwitch);
	Ele->addChildElement(CML_convert(MMP_IODFTGGridSwitch), line);
	snprintf(line, kMaxLineLength, "%f", Threshold);
	Ele->addChildElement(CML_convert(MMP_IODFTThreshold), line);
	snprintf(line, kMaxLineLength, "%d", Functional);
	Ele->addChildElement(CML_convert(MMP_IODFTFunctional), line);
	snprintf(line, kMaxLineLength, "%d", NumRadialGrids);
	Ele->addChildElement(CML_convert(MMP_IODFTNumRadialGrids), line);
	snprintf(line, kMaxLineLength, "%d", NumThetaGrids);
	Ele->addChildElement(CML_convert(MMP_IODFTNumThetaGrids), line);
	snprintf(line, kMaxLineLength, "%d", NumPhiGrids);
	Ele->addChildElement(CML_convert(MMP_IODFTNumPhiGrids), line);
	snprintf(line, kMaxLineLength, "%d", NumRadialGridsInit);
	Ele->addChildElement(CML_convert(MMP_IODFTNumRadialGridsInit), line);
	snprintf(line, kMaxLineLength, "%d", NumThetaGridsInit);
	Ele->addChildElement(CML_convert(MMP_IODFTNumThetaGridsInit), line);
	snprintf(line, kMaxLineLength, "%d", NumPhiGridsInit);
	Ele->addChildElement(CML_convert(MMP_IODFTNumPhiGridsInit), line);
	if (MethodGrid()) Ele->addChildElement(CML_convert(MMP_IODFTGridMethod), trueXML);
	if (GetAuxFunctions()) Ele->addChildElement(CML_convert(MMP_IODFTGetAuxFunctions), trueXML);
	if (GetThree()) Ele->addChildElement(CML_convert(MMP_IODFTThree), trueXML);
}
void GamessDFTGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IODFTGGridSwitch:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							GridSwitch = temp;
						}
					}
						break;
					case MMP_IODFTThreshold:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							Threshold = temp;
						}
					}
						break;
					case MMP_IODFTFunctional:
					{
						long temp;
						if (child->getLongValue(temp)) {
							Functional = temp;
						}
					}
						break;
					case MMP_IODFTNumRadialGrids:
					{
						long temp;
						if (child->getLongValue(temp)) {
							NumRadialGrids = temp;
						}
					}
						break;
					case MMP_IODFTNumThetaGrids:
					{
						long temp;
						if (child->getLongValue(temp)) {
							NumThetaGrids = temp;
						}
					}
						break;
					case MMP_IODFTNumPhiGrids:
					{
						long temp;
						if (child->getLongValue(temp)) {
							NumPhiGrids = temp;
						}
					}
						break;
					case MMP_IODFTNumRadialGridsInit:
					{
						long temp;
						if (child->getLongValue(temp)) {
							NumRadialGridsInit = temp;
						}
					}
						break;
					case MMP_IODFTNumThetaGridsInit:
					{
						long temp;
						if (child->getLongValue(temp)) {
							NumThetaGridsInit = temp;
						}
					}
						break;
					case MMP_IODFTNumPhiGridsInit:
					{
						long temp;
						if (child->getLongValue(temp)) {
							NumPhiGridsInit = temp;
						}
					}
						break;
					case MMP_IODFTGridMethod:
						if (child->getBoolValue(tb))
							SetMethodGrid(tb);
						break;
					case MMP_IODFTGetAuxFunctions:
						if (child->getBoolValue(tb))
							SetAuxFunctions(tb);
						break;
					case MMP_IODFTThree:
						if (child->getBoolValue(tb))
							SetThree(tb);
						break;
				}
			}
		}
	}
}
#pragma mark GamessStatPtGroup
void GamessStatPtGroup::InitData(void) {
	OptConvergance = 0.0001;
	InitTrustRadius = 0.0;
	MaxTrustRadius = 0.0;
	MinTrustRadius = 0.05;
	StatJumpSize = 0.01;
	ModeFollow = 1;
	BitOptions = 0;
	method = 3;
	MaxSteps = 20;
	nRecalcHess = 0;
	SetRadiusUpdate(true);
}
long GamessStatPtGroup::GetSize(BufferFile *Buffer) {
	Boolean	cState = Buffer->GetOutput();
	Buffer->SetOutput(false);
	long size = WriteToBuffer(Buffer);
	Buffer->SetOutput(cState);
	return size;
}
long GamessStatPtGroup::WriteToBuffer(BufferFile *Buffer) {
	long	pos, length=sizeof(GamessStatPtGroup);

	pos = Buffer->Write((Ptr) &length, sizeof(long));
	pos += Buffer->Write((Ptr) this, length);
	return pos;
}
long GamessStatPtGroup::ReadFromBuffer(BufferFile *Buffer) {
	long mylength, size;

	size = Buffer->Read((Ptr) &mylength, sizeof(long));
	if (mylength != sizeof(GamessStatPtGroup)) return size;
	size += Buffer->Read((Ptr) this, mylength);
	return size;
}
void GamessStatPtGroup::WriteToFile(BufferFile *File, GamessInputData *IData) {
	char	Out[133];

	short runType = IData->Control->GetRunType();
		//first determine wether or not the statpt group needs to be punched
	if ((runType != 4)&&(runType != 6)) return;	//only punch for optimize and sadpoint runs

		//Punch the group label
	File->WriteLine(" $STATPT ", false);
		//write out the convergance criteria and number of steps by default, just
		//to remind the user of their values. Everything else is optional.
	sprintf(Out, "OPTTOL=%g ", GetOptConvergance());
	File->WriteLine(Out, false);
	sprintf(Out, "NSTEP=%d ", GetMaxSteps());
	File->WriteLine(Out, false);
		//Method
	if (GetMethod() != 3) {
		File->WriteLine("Method=", false);
		switch (GetMethod()) {
			case 1:
				File->WriteLine("NR ", false);
			break;
			case 2:
				File->WriteLine("RFO ", false);
			break;
			case 3:
				File->WriteLine("QA ", false);
			break;
			case 4:
				File->WriteLine("SCHLEGEL ", false);
			break;
			case 5:
				File->WriteLine("CONOPT ", false);
			break;
		}
	}	//DXMAX if non-default and method is not NR
	if ((GetInitRadius() != 0.0)&&(GetMethod() !=1)) {
		sprintf(Out, "DXMAX=%g ", GetInitRadius());
		File->WriteLine(Out, false);
	}
	if ((GetMethod()==2)||(GetMethod()==3)) {
		if (!GetRadiusUpdate()) File->WriteLine("TRUPD=.FALSE. ", false);
		if (GetMaxRadius() != 0.0) {
			sprintf(Out, "TRMAX=%g ", GetMaxRadius());
			File->WriteLine(Out, false);
		}
		if (fabs(GetMinRadius() - 0.05)>1e-5) {
			sprintf(Out, "TRMIN=%g ", GetMinRadius());
			File->WriteLine(Out, false);
		}
	}
	if ((runType == 6)&&(GetModeFollow() != 1)) {
		sprintf(Out, "IFOLOW=%d ", GetModeFollow());
		File->WriteLine(Out, false);
	}
	if (GetStatPoint()) {
		File->WriteLine("STPT=.TRUE. ", false);
		if (fabs(GetStatJump() - 0.01)>1e-5) {
			sprintf(Out, "STSTEP=%g ", GetStatJump());
			File->WriteLine(Out, false);
		}
	}
	if (GetHessMethod()) {
		File->WriteLine("HESS=", false);
		switch (GetHessMethod()) {
			case 1:
				File->WriteLine("GUESS ", false);
			break;
			case 2:
				File->WriteLine("READ ", false);
			break;
			case 3:
				File->WriteLine("CALC ", false);
			break;
		}
	}
	if (GetHessRecalcInterval()) {
		sprintf(Out, "IHREP=%d ", GetHessRecalcInterval());
		File->WriteLine(Out, false);
	}
	if (AlwaysPrintOrbs()) {
		File->WriteLine("NPRT=1 ", false);
	}

	File->WriteLine("$END", true);
}
void GamessStatPtGroup::WriteXML(XMLElement * parent) const {
	char line[kMaxLineLength];
	XMLElement * Ele = parent->addChildElement(CML_convert(MMP_IOGamessStatPtGroupElement));
	snprintf(line, kMaxLineLength, "%f", OptConvergance);
	Ele->addChildElement(CML_convert(MMP_IOSPGOptConvergance), line);
	snprintf(line, kMaxLineLength, "%f", InitTrustRadius);
	Ele->addChildElement(CML_convert(MMP_IOSPGInitTrustRadius), line);
	snprintf(line, kMaxLineLength, "%f", MaxTrustRadius);
	Ele->addChildElement(CML_convert(MMP_IOSPGMaxTrustRadius), line);
	snprintf(line, kMaxLineLength, "%f", MinTrustRadius);
	Ele->addChildElement(CML_convert(MMP_IOSPGMinTrustRadius), line);
	snprintf(line, kMaxLineLength, "%f", StatJumpSize);
	Ele->addChildElement(CML_convert(MMP_IOSPGStatJumpSize), line);
	snprintf(line, kMaxLineLength, "%ld", ModeFollow);
	Ele->addChildElement(CML_convert(MMP_IOSPGModeFollow), line);
	if (GetRadiusUpdate()) Ele->addChildElement(CML_convert(MMP_IOSPGRadiusUpdate), trueXML);
	if (GetStatPoint()) Ele->addChildElement(CML_convert(MMP_IOSPGStatPoint), trueXML);
	snprintf(line, kMaxLineLength, "%d", GetHessMethod());
	Ele->addChildElement(CML_convert(MMP_IOSPGHessMethod), line);
	snprintf(line, kMaxLineLength, "%d", method);
	Ele->addChildElement(CML_convert(MMP_IOSPGMethod), line);
	snprintf(line, kMaxLineLength, "%d", MaxSteps);
	Ele->addChildElement(CML_convert(MMP_IOSPGMaxSteps), line);
	snprintf(line, kMaxLineLength, "%d", nRecalcHess);
	Ele->addChildElement(CML_convert(MMP_IOSPGnRecalcHess), line);
	if (GetStatPoint()) Ele->addChildElement(CML_convert(MMP_IOSPGAlwaysPrintOrbs), trueXML);
}
void GamessStatPtGroup::ReadXML(XMLElement * parent) {
	XMLElementList * children = parent->getChildren();
	if (children) {
		for (int i=0; i<children->length(); i++) {
			XMLElement * child = children->item(i);
			MMP_IOGamessControlGroupNS item;
			if (child && CML_convert(child->getName(), item)) {
				bool tb;
				switch (item) {
					case MMP_IOSPGOptConvergance:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							SetOptConvergance(temp);
						}
					}
						break;
					case MMP_IOSPGInitTrustRadius:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							SetInitRadius(temp);
						}
					}
						break;
					case MMP_IOSPGMaxTrustRadius:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							SetMaxRadius(temp);
						}
					}
						break;
					case MMP_IOSPGMinTrustRadius:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							SetMinRadius(temp);
						}
					}
						break;
					case MMP_IOSPGStatJumpSize:
					{
						double temp;
						if (child->getDoubleValue(temp)) {
							SetStatJump(temp);
						}
					}
						break;
					case MMP_IOSPGModeFollow:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetModeFollow(temp);
						}
					}
						break;
					case MMP_IOSPGRadiusUpdate:
						if (child->getBoolValue(tb))
							SetRadiusUpdate(tb);
						break;
					case MMP_IOSPGStatPoint:
						if (child->getBoolValue(tb))
							SetStatPoint(tb);
						break;
					case MMP_IOSPGHessMethod:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetHessMethod(temp);
						}
					}
						break;
					case MMP_IOSPGMethod:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetMethod(temp);
						}
					}
						break;
					case MMP_IOSPGMaxSteps:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetMaxSteps(temp);
						}
					}
						break;
					case MMP_IOSPGnRecalcHess:
					{
						long temp;
						if (child->getLongValue(temp)) {
							SetHessRecalcInterval(temp);
						}
					}
						break;
					case MMP_IOSPGAlwaysPrintOrbs:
						if (child->getBoolValue(tb))
							SetAlwaysPrintOrbs(tb);
						break;
				}
			}
		}
	}
}

long MoleculeData::GetNumElectrons(void) const {
	long result=cFrame->GetNumElectrons();
	if (InputOptions && InputOptions->Control) result -= InputOptions->Control->GetCharge();
	return result;
}
short MoleculeData::GetMultiplicity(void) const {
	short result=1;
	if (InputOptions && InputOptions->Control) result = InputOptions->Control->GetMultiplicity();
	return result;
}
GamessInputData * MoleculeData::GetGamessInputData(void) {
	if (!InputOptions) InputOptions = new GamessInputData;
	if (!InputOptions) throw MemoryError();
	return InputOptions;
}
GamessInputData * MoleculeData::SetGamessInputData(GamessInputData * NewData) {
	if (InputOptions) delete InputOptions;
	InputOptions = new GamessInputData(NewData);
	return InputOptions;
}
// // void MoleculeData::WriteInputFile(MolDisplayWin * owner) {
// // 	if (InputOptions) {
// // 		InputOptions->WriteInputFile(this, owner);
// // 	}
// // }
// void MOPacInternals::WriteZMATToFile(BufferFile * File) {
// 	char	Out[133];
// 
// 	File->WriteLine(" $ZMAT IZMAT(1)=", false);
// 	for (long i=3; i<Count; i+=3) {
// 		if (i>9) File->WriteLine(", ", false);
// 		sprintf(Out, "1,%d,%d, ", (i+3)/3, ConnectionAtoms[i]+1);
// 		File->WriteLine(Out, false);
// 		if (i>3) {
// 			sprintf(Out, "2,%d,%d,%d, ", (i+3)/3, ConnectionAtoms[i]+1, ConnectionAtoms[i+1]+1);
// 			File->WriteLine(Out, false);
// 			if (i>6) {
// 				sprintf(Out, "3,%d,%d,%d,%d", (i+3)/3, ConnectionAtoms[i]+1,
// 					ConnectionAtoms[i+1]+1, ConnectionAtoms[i+2]+1);
// 				File->WriteLine(Out, false);
// 			}
// 		}
// 	}
// 	File->WriteLine(" $END", true);
// }
// void MOPacInternals::WriteCoordinatesToFile(BufferFile * File, MoleculeData * MainData, WinPrefs * Prefs) {
// 	UpdateAtoms(MainData);	//First make sure the connectivity and values are up to date
// 	CartesiansToInternals(MainData);
// 		char	Out[133];
// 		Str255	AtomLabel;
// 		Frame *	cFrame = MainData->GetCurrentFramePtr();
// 
// 	for (int iatom=0; iatom<cFrame->NumAtoms; iatom++) {
// 		Prefs->GetAtomLabel(cFrame->Atoms[iatom].GetType()-1, AtomLabel);
// 		AtomLabel[AtomLabel[0]+1] = 0;
// 		if (iatom==0) sprintf(Out, "%s", (char *) &(AtomLabel[1]));
// 		else if (iatom == 1)
// 			sprintf(Out, "%s  %d %10.5f", (char *) &(AtomLabel[1]),
// 				ConnectionAtoms[3*iatom]+1, Values[3*iatom]);
// 		else if (iatom == 2)
// 			sprintf(Out, "%s   %d %10.5f  %d %8.4f",
// 				(char *) &(AtomLabel[1]), ConnectionAtoms[3*iatom]+1, Values[3*iatom], 
// 				ConnectionAtoms[3*iatom+1]+1, Values[3*iatom+1]);
// 		else
// 			sprintf(Out, "%s   %d %10.5f  %d %8.4f  %d %8.4f",
// 				(char *) &(AtomLabel[1]), ConnectionAtoms[3*iatom]+1, Values[3*iatom], 
// 				ConnectionAtoms[3*iatom+1]+1, Values[3*iatom+1],
// 				ConnectionAtoms[3*iatom+2]+1, Values[3*iatom+2]);
// 		File->WriteLine(Out, true);
// 	}
// }
// //This if very similar to the prevous function, but the format is a little different
// void MOPacInternals::WriteMPCZMatCoordinatesToFile(BufferFile * File, MoleculeData * MainData, WinPrefs * Prefs) {
// 	UpdateAtoms(MainData);	//First make sure the connectivity and values are up to date
// 	CartesiansToInternals(MainData);
// 	char	Out[133];
// 	Str255	AtomLabel;
// 	Frame *	cFrame = MainData->GetCurrentFramePtr();
// 	
// 	for (int iatom=0; iatom<cFrame->NumAtoms; iatom++) {
// 		Prefs->GetAtomLabel(cFrame->Atoms[iatom].GetType()-1, AtomLabel);
// 		AtomLabel[AtomLabel[0]+1] = 0;
// 		if (iatom==0) sprintf(Out, "%s", (char *) &(AtomLabel[1]));
// 		else if (iatom == 1)
// 			sprintf(Out, "%s   %10.5f", (char *) &(AtomLabel[1]),
// 					Values[3*iatom]);
// 		else if (iatom == 2)
// 			sprintf(Out, "%s   %10.5f 0 %8.4f 0 %d %d",
// 					(char *) &(AtomLabel[1]), Values[3*iatom], 
// 					Values[3*iatom+1], ConnectionAtoms[3*iatom]+1, ConnectionAtoms[3*iatom+1]+1);
// 		else
// 			sprintf(Out, "%s   %10.5f 0 %8.4f 0 %8.4f 0 %d %d %d",
// 					(char *) &(AtomLabel[1]), Values[3*iatom], Values[3*iatom+1],
// 					Values[3*iatom+2], ConnectionAtoms[3*iatom]+1,
// 					ConnectionAtoms[3*iatom+1]+1, ConnectionAtoms[3*iatom+2]+1);
// 		File->WriteLine(Out, true);
// 	}
// }
// void OrbitalRec::WriteVecGroup(BufferFile * File, const long & NumBasisFuncs, const long & OrbCount) const {
// //First check for and write out the vec label
// 	if (Label) {	//The label should include any necessary linefeeds
// 		File->WriteLine(Label, true);
// 	}
// //Punch the Group title
// 	File->WriteLine(" $VEC", true);
// //write out the vectors using the GAMESS format (I2,I3,5E15.8)
// #ifdef WIN32
// 	//Visual studio defaults to using 3 digit exponent output which doesn't fit in the field
// 	//This function changes that behavior to 2 digits.
// 	_set_output_format(_TWO_DIGIT_EXPONENT);
// #endif
// 		long iline, nVec, nn, nOrbs, pOrb;
// 	if ((OrbCount > 0) && (OrbCount <= NumAlphaOrbs)) nOrbs = OrbCount;
// 	else nOrbs = NumAlphaOrbs;
// 		char	Line[kMaxLineLength];
// 		float * Vector=Vectors;
// 	for (int ipass=0; ipass<2; ipass++) {
// 		if (!Vector) {
// 			File->WriteLine("Error in Vectors request!", true);
// 			return;
// 		}
// 		nn = 0;	pOrb = 0;
// 		for (long i=0; i<nOrbs; i++) {
// 			iline = 1;	nVec = 0;
// 			pOrb++;
// 			if (pOrb>=100) pOrb -= 100;
// 			sprintf(Line, "%2d%3d", pOrb, iline);
// 			File->WriteLine(Line, false);
// 			for (long ivec=0; ivec<NumBasisFuncs; ivec++) {
// 				sprintf(Line, "%15.8E", Vector[nn]);
// 				nn++;
// 				File->WriteLine(Line, false);
// 				nVec++;
// 				if ((nVec>=5)&&(ivec+1<NumBasisFuncs)) {//wrap line and start the next line
// 					File->WriteLine("", true);
// 					iline ++;
// 					sprintf(Line, "%2d%3d", pOrb, iline);
// 					File->WriteLine(Line, false);
// 					nVec = 0;
// 				}
// 			}
// 			File->WriteLine("", true);
// 		}
// 		if (BaseWavefunction == UHF) {	//Repeat for beta set of orbitals for UHF wavefunctions
// 			Vector = VectorsB;
// 			if ((OrbCount > 0) && (OrbCount <= NumBetaOrbs)) nOrbs = OrbCount;
// 			else nOrbs = NumBetaOrbs;
// 		} else ipass++;
// 	}
// //finish off the group
// 	File->WriteLine(" $END", true);
// }
