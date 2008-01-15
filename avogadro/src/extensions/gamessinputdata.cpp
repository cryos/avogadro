/*
 *  (c) 2004 Iowa State University
 *      see the LICENSE file in the top level directory
 */

/* �����������������������������������������
    GamessInputData.cpp

    Class member functions related to GamessInputData
    Brett Bode - February 1996
    Changed InputeFileData uses to BufferFile calls 8-97
    */

#include "gamessinputdata.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

using namespace OpenBabel;

#define GAMESS_BUFF_LEN 180

namespace Avogadro
{
  long LocateKeyWord( const char *Buffer, const char * KeyWord, long length, long bytecount )
  {
    long test=0, pos=-1;

    while ( !test ) {
      for ( pos++; (( Buffer[pos] != KeyWord[0] )||( Buffer[pos+1] != KeyWord[1] ) )&&( pos<bytecount )&&
            ( Buffer[pos]!='\0' ); pos++ ) ;
      if ( pos>=bytecount ) return -1;
      if ( Buffer[pos]=='\0' ) return -1;
      test = 2;
      while (( Buffer[pos+test] == KeyWord[test] )&&( test<length ) ) test++;
      test = ( long ) test==length;
    }
    return pos;
  } /*LocateKeyWord*/

  long FindKeyWord( const char *buffer, const char keyin[], long numbyte )
  {
    long check;

    char * keyword = new char[numbyte+1];
    strncpy( keyword, keyin, numbyte );
    keyword[numbyte] = '\0';
    for ( check=0; check<numbyte; check++ )     /* Make sure the keyword is uppercase */
      if (( keyword[check]>96 ) && ( keyword[check]<123 ) ) keyword[check] -= 32;
    long pos=0;
    long result = -1;
    while ( buffer[pos] ) {
      check = 0;
      while ((( buffer[pos+check] == keyword[check] )||( buffer[pos+check]-32 == keyword[check] ) )&&
             ( check < numbyte ) ) check++;
      if ( check == numbyte ) {
        result = pos;
        break;
      }
      pos++;
    }
    delete [] keyword;
    return result;
  } /* FindKeyword */

  const char * MemoryUnitToText( const MemoryUnit & mu )
  {
    switch ( mu ) {
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
  bool TextToMemoryUnit( const char * t, MemoryUnit & mu )
  {
    if ( !t || !*t ) return false;
    for ( int i = ( int ) wordsUnit; i != ( int ) NumberMemoryUnits; ++i ) {
      if ( strcmp( t, MemoryUnitToText(( MemoryUnit ) i ) ) == 0 ) {
        mu = ( MemoryUnit ) i;
        return true;
      }
    }
    return false;
  }
  const char * TimeUnitToText( const TimeUnit & tu )
  {
    switch ( tu ) {
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
  bool TextToTimeUnit( const char * t, TimeUnit & tu )
  {
    if ( !t || !*t ) return false;
    for ( int i = ( int ) secondUnit; i != ( int ) NumberTimeUnits; ++i ) {
      if ( strcmp( t, TimeUnitToText(( TimeUnit ) i ) ) == 0 ) {
        tu = ( TimeUnit ) i;
        return true;
      }
    }
    return false;
  }
}

using namespace Avogadro;
//GamessInputData functions
#pragma segment IData
GamessInputData::GamessInputData( Molecule *molecule ) :
    m_molecule( molecule ),
    Control( new GamessControlGroup ),
    System( new GamessSystemGroup ),
    Basis( new GamessBasisGroup ),
    Data( new GamessDataGroup ),
    StatPt( new GamessStatPtGroup ),
    Guess( new GamessGuessGroup ),
    SCF( new GamessSCFGroup ),
    MP2( new GamessMP2Group ),
    Hessian( new GamessHessianGroup ),
    DFT( new GamessDFTGroup ),
    EFP( new GamessEFPData )
{
  //Always create Control, System, Basis, and Data groups
  //   m_molecule = molecule;
  //   Control = new GamessControlGroup;
  //   System = new GamessSystemGroup;
  //   Basis = new GamessBasisGroup;
  //   Data = new GamessDataGroup;
  //   StatPt = new GamessStatPtGroup;
  //   Guess = new GamessGuessGroup;
  //   SCF = NULL;
  //   MP2 = NULL;
  //   Hessian = NULL;
  //   DFT = NULL;
}

void GamessInputData::SetMolecule( Molecule *molecule )
{
  m_molecule = molecule;

  /*  FOR_ATOMS_OF_MOL(atom, m_molecule)
    {
      OBPairData *efpGroupID = 0;
      if(efpGroup = atom->GetData( "EFPGroupID" ))
      {
        std::string id = efpGroup->GetValue();
        std::string name = efpGroup
      }
    }*/
}

GamessInputData::GamessInputData( GamessInputData *Copy )
{
  m_molecule = Copy->m_molecule;
  //Always create Control, System, Basis, and Data groups
  Control = new GamessControlGroup( Copy->Control ); //Create the new group and copy over the data
  System = new GamessSystemGroup( Copy->System );
  Basis = new GamessBasisGroup( Copy->Basis );
  Data = new GamessDataGroup( Copy->Data );
  StatPt = new GamessStatPtGroup( Copy->StatPt );
  if ( Copy->Guess ) Guess = new GamessGuessGroup( Copy->Guess );
  else Guess = new GamessGuessGroup;
  if ( Copy->SCF ) SCF = new GamessSCFGroup( Copy->SCF );
  else SCF = new GamessSCFGroup;
  if ( Copy->MP2 ) MP2 = new GamessMP2Group( Copy->MP2 );
  else MP2 = new GamessMP2Group;
  if ( Copy->Hessian ) Hessian = new GamessHessianGroup( Copy->Hessian );
  else Hessian = new GamessHessianGroup;
  if ( Copy->DFT ) DFT = new GamessDFTGroup( Copy->DFT );
  else DFT = new GamessDFTGroup;
  EFP = new GamessEFPData();
}
GamessInputData::~GamessInputData( void )
{ //destructor
  if ( Control ) delete Control; //simply delete all groups present
  if ( System ) delete System;
  if ( Basis ) delete Basis;
  if ( Data ) delete Data;
  if ( Guess ) delete Guess;
  if ( SCF ) delete SCF;
  if ( MP2 ) delete MP2;
  if ( Hessian ) delete Hessian;
  if ( StatPt ) delete StatPt;
  if ( DFT ) delete DFT;
  if ( EFP ) delete EFP;
}

long GamessInputData::GetNumElectrons() const
{
  long numElectrons = 0;
  if ( m_molecule ) {
    FOR_ATOMS_OF_MOL( atom, m_molecule ) {
      numElectrons += atom->GetAtomicNum();
    }
  }
  return numElectrons;
}
//Prompt for a filename and then write out a valid input file for GAMESS
long GamessInputData::WriteInputFile( ostream &buffer )
{

  long BasisTest=0;
  buffer << "!   File created by the GAMESS Input Deck Generator Plugin for Avogadro" << endl;
  if ( Basis ) BasisTest = Basis->WriteToFile( buffer, this );
  if ( Control ) Control->WriteToFile( buffer, this, GetNumElectrons() );
  if ( DFT ) DFT->WriteToFile( buffer, this );
  if ( System ) System->WriteToFile( buffer );
  if ( Guess ) Guess->WriteToFile( buffer, this );
  if ( SCF ) SCF->WriteToFile( buffer, this );
  if ( MP2 ) MP2->WriteToFile( buffer, this );
  if ( StatPt ) StatPt->WriteToFile( buffer, this );
  if ( Hessian ) Hessian->WriteToFile( buffer, this );
  if ( Data ) Data->WriteToFile( buffer, this, m_molecule );
  //     if (Guess) Guess->WriteVecGroup(buffer, molecule);
  //   if(buffer) {
  //     delete buffer;
  //   }
  return 1;
}

#pragma segment EFP
//GamessEFPGroup functions
GamessEFPData::GamessEFPData() : m_qmCount( 0 ), m_efpCount( 0 )
{}

void GamessEFPGroup::GetCenterOfMass( Molecule *molecule, double &x, double &y, double &z )
{
  double sum[3];
  double mass = 0.0;

  sum[0] = 0.0;
  sum[1] = 0.0;
  sum[2] = 0.0;

  for ( std::vector<Atom *>::iterator i = atoms.begin(); i != atoms.end(); i++ ) {
    Atom *atom = *i;

    double m = atom->GetAtomicMass();

    mass += m;

    sum[0] += m * atom->x();
    sum[1] += m * atom->y();
    sum[2] += m * atom->z();
  }

  x = sum[0] / mass;
  y = sum[1] / mass;
  z = sum[2] / mass;

}

bool GamessEFPGroup::Contains(Atom *atom)
{
  for ( std::vector<Atom *>::iterator i = atoms.begin(); i != atoms.end(); i++ ) {
    if(atom == *i)
    {
      return true;
    }
  }
  return false;
}

int GamessEFPData::GetGroupCount()
{
  return m_efpCount + m_qmCount;
}

int GamessEFPData::GetGroupCount( GamessEFPGroup::Type type )
{
  if ( type == GamessEFPGroup::EFPType ) {
    return m_efpCount;
  } else if ( type == GamessEFPGroup::QMType ) {
    return m_qmCount;
  }

  return 0;
}

void GamessEFPData::AddGroup( GamessEFPGroup *group )
{
  m_groups.push_back( group );
  if ( group->type == GamessEFPGroup::EFPType ) {
    m_efpCount++;
  } else if ( group->type == GamessEFPGroup::QMType ) {
    m_qmCount++;
  }
}

void GamessEFPData::RemoveGroups( Atom *atom )
{
  std::vector<GamessEFPGroup *>::iterator iter;
  for ( iter = m_groups.begin(); iter != m_groups.end(); ) {
    if ( (*iter)->Contains(atom) ) {
      if( (*iter)->type == GamessEFPGroup::QMType )
      {
        m_qmCount--;
      }
      else if ( (*iter)->type == GamessEFPGroup::EFPType )
      {
        m_efpCount--;
      }
      delete (*iter);
      m_groups.erase( (iter) );
    }
    else
    {
      // if we remove it automatically increments
      iter++;
    }
  }
}

void GamessEFPData::RemoveGroup( GamessEFPGroup *group )
{
  std::vector<GamessEFPGroup *>::iterator iter;
  for ( iter = m_groups.begin(); iter != m_groups.end(); iter++ ) {
    if ( *iter == group ) {
      m_groups.erase( iter );
    }
  }
}

//GamessEFPData functions

#pragma segment Control
//GamessControlGroup functions
GamessControlGroup::GamessControlGroup( void )
{
  ExeType = NULL;
  Options=0;
  InitControlPaneData();
  InitProgPaneData();
  NPrint=ITol=ICut=0;
  RunType = Energy;
}
GamessControlGroup::GamessControlGroup( GamessControlGroup *Copy )
{
  if ( Copy == NULL ) return;
  *this = *Copy;
  ExeType = NULL;
  if ( Copy->ExeType ) {
    ExeType = new char[1+strlen( Copy->ExeType )];
    if ( ExeType ) strcpy( ExeType, Copy->ExeType );
  }
}
GamessControlGroup::~GamessControlGroup( void )
{
  if ( ExeType ) delete [] ExeType;
}
void GamessControlGroup::InitControlPaneData( void )
{
  if ( ExeType ) {
    delete [] ExeType;
    ExeType = NULL;
  }
  SCFType=GAMESSDefaultSCFType;
  MPLevelCIType=MaxIt=Charge=Multiplicity=0;
  Local=GAMESS_No_Localization;
  RunType=InvalidRunType;
  CCType = CC_None;
}
void GamessControlGroup::InitProgPaneData( void )
{
  Friend=Friend_None;
  SetMolPlot( false );
  SetPlotOrb( false );
  SetAIMPAC( false );
  SetRPAC( false );
}
GAMESS_SCFType GamessControlGroup::SetSCFType( GAMESS_SCFType NewSCFType )
{
  if (( NewSCFType >= GAMESSDefaultSCFType )||( NewSCFType<NumGAMESSSCFTypes ) )
    SCFType = NewSCFType;
  return SCFType;
}
const char * GamessControlGroup::GAMESSSCFTypeToText( GAMESS_SCFType t )
{
  switch ( t ) {
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
GAMESS_SCFType GamessControlGroup::SetSCFType( const char *SCFText )
{
  GAMESS_SCFType temp = GAMESS_Invalid_SCFType;
  for ( int i=1; i<=NumGAMESSSCFTypes; i++ ) {
    if ( !strcasecmp( SCFText, GAMESSSCFTypeToText(( GAMESS_SCFType ) i ) ) ) {
      temp = ( GAMESS_SCFType ) i;
      break;
    }
  }
  if ( temp != GAMESS_Invalid_SCFType ) SCFType = temp;
  return temp;
}
long GamessControlGroup::SetMPLevel( short NewMPLevel )
{
  if (( NewMPLevel!=0 )&&( NewMPLevel!=2 ) ) return -1;

  MPLevelCIType = ( MPLevelCIType & 0xFFF0 ) + NewMPLevel;
  return ( MPLevelCIType & 0x0F );
}
short GamessControlGroup::GetMPLevel( void ) const
{ //return the appropriate MP value based on SCF and Run types
  short result=-1;
  //MP2 energy and gradients are available for RHF, UHF and ROHF
  if (( SCFType >= GAMESSDefaultSCFType )&&( SCFType <= GAMESS_ROHF ) ) result = ( MPLevelCIType & 0x0F );
  //MrMP2 energies are also available
  else if (( SCFType==GAMESS_MCSCF )&&
           ( RunType!=GradientRun )&&( RunType!=HessianRun )&&( RunType!=OptimizeRun )&&
           ( RunType!=SadPointRun )&&( RunType!=IRCRun )&&( RunType!=GradExtrRun )&&( RunType!=DRCRun ) ) result=( MPLevelCIType & 0x0F );
  if ( MPLevelCIType & 0xF0 ) result = -1; //deactivate MP2 when CI is requested
  if ( GetCCType() != CC_None ) result = -1;
  return result;
}
CIRunType GamessControlGroup::SetCIType( CIRunType NewVal )
{
  MPLevelCIType = ( MPLevelCIType&0x0F ) + ( NewVal<<4 );
  return ( CIRunType )( MPLevelCIType & 0xF0 );
}
CIRunType GamessControlGroup::GetCIType( void ) const
{
  short result = (( MPLevelCIType & 0xF0 )>>4 );
  if ( GetSCFType() == 2 ) result = 0;
  return ( CIRunType ) result;
};
TypeOfRun GamessControlGroup::SetRunType( const TypeOfRun & NewRunType )
{
  if (( NewRunType<=0 )||( NewRunType>NumGAMESSRunTypes ) ) return InvalidRunType;

  RunType = NewRunType;
  return RunType;
}
TypeOfRun GamessControlGroup::SetRunType( const char *RunText )
{
  TypeOfRun NewType = InvalidRunType;

  for ( int i=1; i<NumGAMESSRunTypes; i++ ) {
    const char * test = GetGAMESSRunText(( TypeOfRun )i );
    if ( -1<LocateKeyWord( RunText, test, strlen( test ), 9 ) ) {
      NewType = ( TypeOfRun )i;
      break;
    }
  }

  if ( NewType<=0 ) return InvalidRunType;

  RunType = NewType;
  return RunType;
}
const char * GamessControlGroup::GetGAMESSRunText( const TypeOfRun & r )
{
  switch ( r ) {
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
const char * GamessControlGroup::GetGAMESSCCType( const CCRunType & r )
{
  switch ( r ) {
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
CCRunType GamessControlGroup::SetCCType( CCRunType n )
{
  CCType = n;
  return CCType;
}
CCRunType GamessControlGroup::SetCCType( const char * n )
{
  CCRunType NewType = CC_None;

  for ( int i=1; i<NumCCTypes; i++ ) {
    const char * test = GetGAMESSCCType(( CCRunType )i );
    if ( -1<LocateKeyWord( n, test, strlen( test ), 8 ) ) {
      NewType = ( CCRunType )i;
      break;
    }
  }

  if ( NewType<=0 ) return CC_None;

  CCType = NewType;
  return CCType;
}
CCRunType GamessControlGroup::GetCCType( void ) const
{
  CCRunType result = CCType;

  if ( GetSCFType() > 1 ) result = CC_None;
  if ( GetCIType() > 0 ) result = CC_None;
  return result;
}
short GamessControlGroup::GetExeType( void )
{
  if (( ExeType==NULL )||( 0<=LocateKeyWord( ExeType, "RUN", 3,3 ) ) ) return 0; //Normal run
  if ( 0<=LocateKeyWord( ExeType, "CHECK", 5,5 ) ) return 1;
  if ( 0<=LocateKeyWord( ExeType, "DEBUG", 5,5 ) ) return 2;
  return 3;
}
short GamessControlGroup::SetExeType( const char *ExeText )
{
  if ( ExeText==NULL ) return 0;
  long nchar = strlen( ExeText );
  if ( ExeType ) {
    delete [] ExeType;
    ExeType = NULL;
  }
  ExeType = new char[nchar+1];
  strcpy( ExeType, ExeText );
  return nchar;
}
short GamessControlGroup::SetExeType( short NewType )
{
  if (( NewType < 0 )||( NewType > 2 ) ) return -1;
  if ( ExeType ) {
    delete [] ExeType;
    ExeType = NULL;
  }
  if ( NewType==1 ) {
    ExeType = new char[6];
    strcpy( ExeType, "CHECK" );
  } else if ( NewType == 2 ) {
    ExeType = new char[6];
    strcpy( ExeType, "DEBUG" );
  }
  return NewType;
}
CIRunType GamessControlGroup::SetCIType( const char * CIText )
{
  CIRunType newType = CI_None;
  if ( -1<FindKeyWord( CIText, "GUGA", 4 ) ) newType = CI_GUGA;
  else if ( -1<FindKeyWord( CIText, "ALDET", 5 ) ) newType = CI_ALDET;
  else if ( -1<FindKeyWord( CIText, "ORMAS", 5 ) ) newType = CI_ORMAS;
  else if ( -1<FindKeyWord( CIText, "CIS", 3 ) ) newType = CI_CIS;
  else if ( -1<FindKeyWord( CIText, "FSOCI", 5 ) ) newType = CI_FSOCI;
  else if ( -1<FindKeyWord( CIText, "GENCI", 5 ) ) newType = CI_GENCI;
  return SetCIType( newType );
}
const char * GamessControlGroup::GetCIType( const CIRunType & citype ) const
{
  switch ( citype ) {
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
CIRunType GamessControlGroup::GetCIType( char * outText ) const
{
  CIRunType temp = GetCIType();
  if ( outText != NULL ) {
    strcpy( outText, GetCIType( temp ) );
  }
  return temp;
}
short GamessControlGroup::SetMaxIt( short NewVal )
{
  if ( NewVal>=0 ) MaxIt = NewVal;
  return MaxIt;
}
GAMESS_Localization GamessControlGroup::SetLocal( GAMESS_Localization NewVal )
{
  if (( NewVal>=GAMESS_No_Localization )&&( NewVal<NumGAMESSLocalizations ) ) Local = NewVal;
  return Local;
}
GAMESS_Localization GamessControlGroup::SetLocal( const char * t )
{
  GAMESS_Localization temp = Invalid_Localization;
  for ( int i=0; i<NumGAMESSLocalizations; i++ ) {
    if ( !strcasecmp( t, GAMESSLocalizationToText(( GAMESS_Localization ) i ) ) ) {
      temp = ( GAMESS_Localization ) i;
      break;
    }
  }
  if ( temp != Invalid_Localization ) Local = temp;
  return temp;
}
const char * GamessControlGroup::GAMESSLocalizationToText( GAMESS_Localization t )
{
  switch ( t ) {
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
const char * GamessControlGroup::GetFriendText( FriendType f )
{
  switch ( f ) {
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
  return "invalid"; //Getting to here indicates a bad value
}
FriendType GamessControlGroup::TextToFriend( const char * c )
{
  FriendType result = Friend_None;
  for ( int i=0; i<NumFriendTypes; i++ ) {
    if ( !strcasecmp( c, GetFriendText(( FriendType ) i ) ) ) {
      result = ( FriendType ) i;
      break;
    }
  }
  return result;
}
FriendType GamessControlGroup::SetFriend( FriendType NewValue )
{
  if (( NewValue >= Friend_None )&&( NewValue < NumFriendTypes ) ) Friend = NewValue;
  return ( FriendType )Friend;
}
FriendType GamessControlGroup::SetFriend( const char * c )
{
  return SetFriend( TextToFriend( c ) );
}
short GamessControlGroup::SetCharge( short NewCharge )
{
  Charge = NewCharge;
  return Charge;
}
short GamessControlGroup::SetMultiplicity( short NewMult )
{
  Multiplicity = NewMult;
  return Multiplicity;
}
bool GamessControlGroup::SetMolPlot( bool State )
{
  if ( Options & 1 ) Options -= 1;
  if ( State ) Options += 1;
  return (( Options & 1 )?true:false );
}
bool GamessControlGroup::SetPlotOrb( bool State )
{
  if ( Options & ( 1<<1 ) ) Options -= ( 1<<1 );
  if ( State ) Options += ( 1<<1 );
  return (( Options & ( 1<<1 ) )?true:false );
}
bool GamessControlGroup::SetAIMPAC( bool State )
{
  if ( Options & ( 1<<2 ) ) Options -= ( 1<<2 );
  if ( State ) Options += ( 1<<2 );
  return (( Options & ( 1<<2 ) )?true:false );
}
bool GamessControlGroup::SetRPAC( bool State )
{
  if ( Options & ( 1<<3 ) ) Options -= ( 1<<3 );
  if ( State ) Options += ( 1<<3 );
  return (( Options & ( 1<<3 ) )?true:false );
}
bool GamessControlGroup::SetIntType( bool State )
{
  if ( Options & ( 1<<5 ) ) Options -= ( 1<<5 );
  if ( State ) Options += ( 1<<5 );
  return (( Options & ( 1<<5 ) )?true:false );
}
bool GamessControlGroup::SetNormF( bool State )
{
  if ( Options & ( 1<<6 ) ) Options -= ( 1<<6 );
  if ( State ) Options += ( 1<<6 );
  return (( Options & ( 1<<6 ) )?true:false );
}
bool GamessControlGroup::UseDFT( bool State )
{
  if ( Options & ( 1<<4 ) ) Options -= ( 1<<4 );
  if ( State ) Options += ( 1<<4 );
  return ( UseDFT() );
}
bool GamessControlGroup::UseDFT( void ) const
{
  bool result = false;
  result = (( Options & ( 1<<4 ) )?true:false );
  if ( GetSCFType() > 3 ) result = false;
  if ( GetMPLevel() > 0 ) result = false;
  if ( GetCIType() > 0 ) result = false;
  if ( GetCCType() != CC_None ) result = false;
  return result;
}
bool GamessControlGroup::SetNormP( bool State )
{
  if ( Options & ( 1<<7 ) ) Options -= ( 1<<7 );
  if ( State ) Options += ( 1<<7 );
  return GetNormP();
}
void GamessControlGroup::WriteToFile( ostream &File, GamessInputData *IData, long NumElectrons )
{
  char Out[GAMESS_BUFF_LEN], textVal[GAMESS_BUFF_LEN];

  //Punch the group label
  File << " $CONTRL ";
  //punch the SCF type and Run type
  if ( SCFType ) {
    sprintf( Out,"SCFTYP=%s ",GetSCFTypeText() );
    File << Out;
  } else { //Punch out the default RHF/ROHF wavefunction
    if ( NumElectrons + GetCharge() & 1 ) sprintf( Out, "SCFTYP=ROHF " );
    else sprintf( Out, "SCFTYP=RHF " );
    File << Out;
  }
  if ( RunType ) {
    sprintf( Out,"RUNTYP=%s ", GetGAMESSRunText( GetRunType() ) );
    File << Out;
  }
  if (( ExeType )&&( !Friend ) ) { //punch out ExeType if it is other than run
    sprintf( Out, "EXETYP=%s ", ExeType );
    File << Out;
  }
  if ( GetMPLevel() > 0 ) { //Write out MP level only if > zero
    sprintf( Out,"MPLEVL=2 " );
    File << Out;
  }
  if ( GetCIType() || ( GetSCFType() == 6 ) ) { //punch CIType if CI requested
    GetCIType( textVal );
    sprintf( Out, "CITYP=%s ", textVal );
    File << Out;
  }
  if ( GetCCType() != CC_None ) {
    sprintf( Out, "CCTYP=%s ", GetGAMESSCCType( CCType ) );
    File << Out;
  }

  if ( IData->DFT ) {
    if ( UseDFT() ) {
      sprintf( Out, "DFTTYP=%s ", IData->DFT->GetFunctionalText() );
      File << Out;
    }
  }

  if ( MaxIt ) { //Punch Maxit if non-default value
    sprintf( Out, "MAXIT=%d ",MaxIt );
    File << Out;
  }
  if ( Charge ) {
    sprintf( Out, "ICHARG=%d ", Charge );
    File << Out;
  }
  if ( Multiplicity ) {
    sprintf( Out, "MULT=%d ", Multiplicity );
    File << Out;
  } else if ( NumElectrons & 1 ) { //for odd electron systems punch out a default doublet
    sprintf( Out, "MULT=2 " );
    File << Out;
  }
  if ( Local ) {
    sprintf( Out, "LOCAL=%s ", GetLocalText() );
    File << Out;
  }
  if ( IData->Basis ) {
    if ( IData->Basis->GetECPPotential() ) {
      sprintf( Out, "ECP=%s ",IData->Basis->GetECPPotentialText() );
      File << Out;
    }
  }
  if ( IData->Data ) {
    if ( IData->EFP->GetGroupCount( GamessEFPGroup::EFPType ) > 0 &&
         IData->EFP->GetGroupCount( GamessEFPGroup::QMType ) == 0 ) {
      sprintf( Out, "COORD=FRAGONLY " );
      File << Out;
    } else if ( IData->Data->GetCoordType() ) {
      sprintf( Out, "COORD=%s ", IData->Data->GetCoordText() );
      File << Out;
    }
    if ( IData->Data->GetUnits() ) {
      sprintf( Out, "UNITS=BOHR " );
      File << Out;
    }
    if ( IData->Data->GetNumZVar() ) {
      sprintf( Out, "NZVAR=%d ",IData->Data->GetNumZVar() );
      File << Out;
    }
    if ( !IData->Data->GetUseSym() ) {
      sprintf( Out, "NOSYM=1 " );
      File << Out;
    }
  }
  if ( Friend ) { //punchs out input to other programs, disables exetype (forces check run)
    sprintf( Out, "FRIEND=%s ", GetFriendText() );
    File << Out;
  }
  if ( GetMolPlot() ) {
    sprintf( Out, "MOLPLT=.TRUE. " );
    File << Out;
  }
  if ( GetPlotOrb() ) {
    sprintf( Out, "PLTORB=.TRUE. " );
    File << Out;
  }
  if (( 1!=GetExeType() )&&( Friend==0 ) ) {
    if ( GetAIMPAC() ) {
      sprintf( Out, "AIMPAC=.TRUE. " );
      File << Out;
    }
    if ( GetRPAC() ) {
      sprintf( Out, "RPAC=.TRUE. " );
      File << Out;
    }
  }

  File << "$END" << endl;
}
void GamessControlGroup::RevertControlPane( GamessControlGroup *OldData )
{
  RunType = OldData->RunType;
  SCFType = OldData->SCFType;
  SetMPLevel( OldData->GetMPLevel() );
  UseDFT( OldData->UseDFT() );
  SetCIType( OldData->GetCIType() );
  SetCCType( OldData->GetCCType() );
  MaxIt = OldData->MaxIt;
  if ( ExeType ) {
    delete [] ExeType;
    ExeType = NULL;
  }
  SetExeType( OldData->ExeType );
  Local = OldData->Local;
  Charge = OldData->Charge;
  Multiplicity = OldData->Multiplicity;
}
void GamessControlGroup::RevertProgPane( GamessControlGroup *OldData )
{
  SetMolPlot( OldData->GetMolPlot() );
  SetPlotOrb( OldData->GetPlotOrb() );
  SetAIMPAC( OldData->GetAIMPAC() );
  SetRPAC( OldData->GetRPAC() );
  SetFriend( OldData->GetFriend() );
}
#pragma mark GamessSystemGroup
//GamessSystemGroup member functions
long GamessSystemGroup::SetTimeLimit( long NewTime )
{
  if ( NewTime >= 0 ) TimeLimit = NewTime;
  return TimeLimit;
}
TimeUnit GamessSystemGroup::SetTimeUnits( TimeUnit NewUnits )
{
  if (( NewUnits >= secondUnit )&&( NewUnits<NumberTimeUnits ) ) TimeUnits = NewUnits;
  return TimeUnits;
}
float GamessSystemGroup::GetConvertedTime( void ) const
{
  float result, factor=1.0;

  if ( TimeLimit ) result = TimeLimit;
  else result = 525600.0;

  switch ( TimeUnits ) {
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
long GamessSystemGroup::SetConvertedTime( float NewTime )
{
  long result, factor = 1;

  switch ( TimeUnits ) {
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
      result = ( long )( NewTime * factor );
      break;
    case secondUnit:
      result = ( long )( NewTime/60.0 );
      break;
  }
  if ( result >= 0 ) TimeLimit = result;
  return TimeLimit;
}
double GamessSystemGroup::SetMemory( double NewMemory )
{
  if ( NewMemory > 0.0 ) Memory = NewMemory;
  return Memory;
}
MemoryUnit GamessSystemGroup::SetMemUnits( MemoryUnit NewUnits )
{
  if (( NewUnits>=wordsUnit )&&( NewUnits<NumberMemoryUnits ) ) MemUnits = NewUnits;
  return MemUnits;
}
double GamessSystemGroup::GetConvertedMem( void ) const
{
  double result, factor=1.0;

  result = Memory;
  // no defaults!  let GAMESS handle this
  //if (Memory) result = Memory;
  // else result = 1000000;

  switch ( MemUnits ) {
    case bytesUnit:
      factor = 8.0;
      break;
    case megaWordsUnit:
      factor = 1.0/1000000.0;
      break;
    case megaBytesUnit:
      factor = 8.0/( 1024*1024 );
      break;
  }
  result *= factor;
  return result;
}
double GamessSystemGroup::SetConvertedMem( double NewMem )
{
  double result, factor = 1;

  switch ( MemUnits ) {
    case megaBytesUnit:
      factor *= 1024*1024;
    case bytesUnit:
      result = ( long )( factor*NewMem/8.0 );
      break;
    case megaWordsUnit:
      factor *= 1000000;
    case wordsUnit:
      result = ( long )( factor*NewMem );
      break;
  }
  if ( result >= 0 ) Memory = result;
  return Memory;
}
double GamessSystemGroup::SetMemDDI( double NewMemory )
{
  if ( NewMemory >= 0.0 ) MemDDI = NewMemory;
  return Memory;
}
MemoryUnit GamessSystemGroup::SetMemDDIUnits( MemoryUnit NewUnits )
{
  if (( NewUnits>=megaWordsUnit )&&( NewUnits<NumberMemoryUnits ) ) MemDDIUnits = NewUnits;
  return MemDDIUnits;
}
double GamessSystemGroup::GetConvertedMemDDI( void ) const
{
  double result, factor=1.0;

  result = MemDDI; //memDDI is stored in MW

  switch ( MemDDIUnits ) {
    case megaBytesUnit:
      factor = 8.0;
      break;
    case gigaWordsUnit:
      factor = 1.0/1000.0;
      break;
    case gigaBytesUnit:
      factor = 8.0/( 1000.0 );
      break;
  }
  result *= factor;
  return result;
}
double GamessSystemGroup::SetConvertedMemDDI( double NewMem )
{
  double result, factor = 1;

  switch ( MemDDIUnits ) {
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
  if ( result >= 0 ) MemDDI = result;
  return MemDDI;
}

char GamessSystemGroup::SetDiag( char NewMethod )
{
  if (( NewMethod>=0 )&&( NewMethod<4 ) ) KDiag = NewMethod;
  return KDiag;
}
bool GamessSystemGroup::SetCoreFlag( bool State )
{
  if ( Flags & 1 ) Flags --;
  if ( State ) Flags ++;
  return GetCoreFlag();
}
bool GamessSystemGroup::SetBalanceType( bool Type )
{
  if ( Flags & 2 ) Flags -= 2;
  if ( Type ) Flags += 2;
  return GetBalanceType();
}
bool GamessSystemGroup::SetXDR( bool State )
{
  if ( Flags & 4 ) Flags -= 4;
  if ( State ) Flags += 4;
  return GetXDR();
}
bool GamessSystemGroup::SetParallel( bool State )
{
  if ( Flags & 8 ) Flags -= 8;
  if ( State ) Flags += 8;
  return GetParallel();
}
GamessSystemGroup::GamessSystemGroup( void )
{
  InitData();
}
GamessSystemGroup::GamessSystemGroup( GamessSystemGroup *Copy )
{
  if ( Copy ) *this=*Copy;
}
void GamessSystemGroup::InitData( void )
{
  TimeLimit = 600;
  Memory = 0.0;
  MemDDI = 0.0;
  KDiag = 0;
  // TimeUnits = minuteUnit;
  TimeUnits = hourUnit;
  // MemUnits = wordsUnit;
  MemUnits = megaBytesUnit;
  MemDDIUnits = megaWordsUnit;
  Flags = 0;
}
void GamessSystemGroup::WriteToFile( ostream &File )
{
  long test;
  char Out[GAMESS_BUFF_LEN];

  //Punch the group label
  File << " $SYSTEM ";
  //Time limit
  test = TimeLimit;
  if ( test==0 ) test = 600;
  sprintf( Out,"TIMLIM=%ld ",test );
  File << Out;
  //Memory
  if ( Memory ) {
    sprintf( Out, "MEMORY=%ld ", ( long )Memory );
    File << Out;
  }
  if ( MemDDI ) {
    sprintf( Out, "MEMDDI=%ld ", ( long )MemDDI );
    File << Out;
  } //PARALL
  if ( GetParallel() ) {
    sprintf( Out, "PARALL=.TRUE. " );
    File << Out;
  } //diag method
  if ( KDiag ) {
    sprintf( Out, "KDIAG=%d ", KDiag );
    File << Out;
  } //core flag
  if ( GetCoreFlag() ) {
    sprintf( Out, "COREFL=.TRUE. " );
    File << Out;
  } //Balance type
  if ( GetBalanceType() ) {
    sprintf( Out, "BALTYP=NXTVAL " );
    File << Out;
  } //XDR
  if ( GetXDR() ) {
    sprintf( Out, "XDR=.TRUE. " );
    File << Out;
  }
  File << "$END" << endl;
}
#pragma mark GamessBasisGroup
//GamessBasisGroup member functions
GamessBasisGroup::GamessBasisGroup( void )
{
  InitData();
}
GamessBasisGroup::GamessBasisGroup( GamessBasisGroup *Copy )
{
  if ( Copy ) {
    *this = *Copy;
  }
}
void GamessBasisGroup::InitData( void )
{
  Split2[0]=Split2[1]=0.0;
  Split3[0]=Split3[1]=Split3[2]=0.0;
  Basis=GAMESS_BS_STO;
  NumGauss=3;
  NumHeavyFuncs=NumPFuncs=ECPPotential=0;
  Polar = GAMESS_BS_No_Polarization;
  Flags = 0;
  WaterSolvate = false;
}
const char * GamessBasisGroup::GAMESSBasisSetToText( GAMESS_BasisSet bs )
{
  switch ( bs ) {
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
short GamessBasisGroup::SetBasis( const char *BasisText )
{
  short NewBasis = -1;

  for ( int i=GAMESS_BS_None; i<NumGAMESSBasisSetsItem; i++ ) {
    if ( !strcasecmp( BasisText, GAMESSBasisSetToText(( GAMESS_BasisSet )i ) ) ) {
      NewBasis = i;
      break;
    }
  }
  if ( NewBasis<0 ) return -1;

  Basis = NewBasis;
  return Basis;
}
short GamessBasisGroup::SetBasis( short NewBasis )
{
  if (( NewBasis<-1 )||( NewBasis>16 ) ) return -1;

  Basis = NewBasis;
  return Basis;
}
const char * GamessBasisGroup::GetBasisText( void ) const
{
  short temp = Basis;
  if ( temp <= 0 ) temp = 1;

  return GAMESSBasisSetToText(( GAMESS_BasisSet ) temp );
}
short GamessBasisGroup::GetBasis( void ) const
{
  return Basis;
}
short GamessBasisGroup::SetNumGauss( short NewNumGauss )
{
  if (( NewNumGauss<0 )||( NewNumGauss>6 ) ) return -1;
  if (( Basis==4 )&&( NewNumGauss!=3 )&&( NewNumGauss!=6 ) ) return -1;
  if (( Basis==5 )&&( NewNumGauss<4 ) ) return -1;
  if (( Basis==6 )&&( NewNumGauss!=6 ) ) return -1;

  NumGauss = NewNumGauss;
  return NumGauss;
}
short GamessBasisGroup::GetNumGauss( void ) const
{
  return NumGauss;
}
short GamessBasisGroup::SetNumDFuncs( short NewNum )
{
  if ( NewNum > 3 ) return -1;

  NumHeavyFuncs = NewNum + ( NumHeavyFuncs & 0xF0 );
  return ( NumHeavyFuncs & 0x0F );
}
short GamessBasisGroup::GetNumDFuncs( void ) const
{
  return ( NumHeavyFuncs & 0x0F );
}
short GamessBasisGroup::SetNumFFuncs( short NewNum )
{
  if ( NewNum > 3 ) return -1;

  NumHeavyFuncs = ( NewNum<<4 ) + ( NumHeavyFuncs & 0x0F );
  return (( NumHeavyFuncs & 0xF0 )>>4 );
}
short GamessBasisGroup::GetNumFFuncs( void ) const
{
  return (( NumHeavyFuncs & 0xF0 )>>4 );
}
short GamessBasisGroup::SetNumPFuncs( short NewNum )
{
  if ( NewNum > 3 ) return -1;

  NumPFuncs = NewNum;
  return NumPFuncs;
}
short GamessBasisGroup::GetNumPFuncs( void ) const
{
  return NumPFuncs;
}
short GamessBasisGroup::SetDiffuseSP( bool state )
{
  if ( state && ( !( Flags & 1 ) ) ) Flags += 1;
  else if ( !state && ( Flags & 1 ) ) Flags -= 1;

  return state;
}
short GamessBasisGroup::SetDiffuseS( bool state )
{
  if ( state && ( !( Flags & 2 ) ) ) Flags += 2;
  else if ( !state && ( Flags & 2 ) ) Flags -= 2;

  return state;
}
GAMESS_BS_Polarization GamessBasisGroup::SetPolar( GAMESS_BS_Polarization NewPolar )
{
  if (( NewPolar>=GAMESS_BS_No_Polarization )||( NewPolar<NumGAMESSBSPolarItems ) ) {
    Polar = NewPolar;
  }
  return Polar;
}
GAMESS_BS_Polarization GamessBasisGroup::SetPolar( const char *PolarText )
{
  GAMESS_BS_Polarization NewPolar = GAMESS_BS_Invalid_Polar;

  for ( int i=GAMESS_BS_No_Polarization; i<NumGAMESSBSPolarItems; i++ ) {
    if ( !strcasecmp( PolarText, PolarToText(( GAMESS_BS_Polarization )i ) ) ) {
      NewPolar = ( GAMESS_BS_Polarization ) i;
      break;
    }
  }
  if ( NewPolar>=0 ) Polar = NewPolar;
  return NewPolar;
}
const char * GamessBasisGroup::PolarToText( GAMESS_BS_Polarization p )
{
  switch ( p ) {
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
const char * GamessBasisGroup::GAMESSECPToText( GAMESS_BS_ECPotential p )
{
  switch ( p ) {
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
GAMESS_BS_ECPotential GamessBasisGroup::SetECPPotential( const char *ECPText )
{
  GAMESS_BS_ECPotential NewPot = GAMESS_BS_Invalid_ECP;

  for ( int i=GAMESS_BS_ECP_None; i<NumGAMESSBSECPItems; i++ ) {
    if ( !strcasecmp( ECPText, GAMESSECPToText(( GAMESS_BS_ECPotential )i ) ) ) {
      NewPot = ( GAMESS_BS_ECPotential ) i;
      break;
    }
  }
  if ( NewPot>=0 ) ECPPotential = NewPot;
  return NewPot;
}
short GamessBasisGroup::GetECPPotential( void ) const
{
  short value = ECPPotential;
  if ( value == 0 ) {
    if ( Basis == 12 ) value = 2;
    if ( Basis == 13 ) value = 3;
  }
  return value;
}
const char * GamessBasisGroup::GetECPPotentialText( void ) const
{
  short value = ECPPotential;
  if ( value == 0 ) {
    if ( Basis == 12 ) value = 2;
    if ( Basis == 13 ) value = 3;
  }
  return GAMESSECPToText(( GAMESS_BS_ECPotential ) value );
}
short GamessBasisGroup::SetECPPotential( short NewType )
{
  if (( NewType<0 )||( NewType>3 ) ) return -1;
  ECPPotential = NewType;
  return ECPPotential;
}
long GamessBasisGroup::WriteToFile( ostream &File, GamessInputData * iData )
{
  char Out[GAMESS_BUFF_LEN];
  //if a general basis set is present don't punch the $Basis group
  if ( !iData->Basis ) return 1;
  //Punch the group label
  File << " $BASIS ";
  //Basis Set
  sprintf( Out,"GBASIS=%s ", GetBasisText() );
  File << Out;
  //Number of Gaussians
  if ( NumGauss ) {
    sprintf( Out, "NGAUSS=%d ", NumGauss );
    File << Out;
  } //number of heavy atom polarization functions
  if ( GetNumDFuncs() ) {
    sprintf( Out, "NDFUNC=%d ", GetNumDFuncs() );
    File << Out;
  } //number of heavy atom f type polarization functions
  if ( GetNumFFuncs() ) {
    sprintf( Out, "NFFUNC=%d ", GetNumFFuncs() );
    File << Out;
  } //number of light atom polarization functions
  if ( NumPFuncs ) {
    sprintf( Out, "NPFUNC=%d ", NumPFuncs );
    File << Out;
  } //type of Polarization functions
  if (( Polar )&&(( NumHeavyFuncs )||( NumPFuncs ) ) ) {
    sprintf( Out, "POLAR=%s ", GetPolarText() );
    File << Out;
  }
  if ( GetDiffuseSP() ) {
    sprintf( Out, "DIFFSP=.TRUE. " );
    File << Out;
  }
  if ( GetDiffuseS() ) {
    sprintf( Out, "DIFFS=.TRUE. " );
    File << Out;
  }
  File << "$END" << endl;

  if ( WaterSolvate ) {
    File << " $PCM SOLVNT=WATER $END" << endl;
  }
  return 0;
}
#pragma mark GamessDataGroup
// Data Group member functions
GamessDataGroup::GamessDataGroup( void )
{
  InitData();
}
GamessDataGroup::GamessDataGroup( GamessDataGroup *Copy )
{
  if ( Copy ) {
    *this = *Copy;
    Title = NULL;
    if ( Copy->Title ) {
      Title = new char[1+strlen( Copy->Title )];
      if ( Title ) strcpy( Title, Copy->Title );
    }
  }
}
GamessDataGroup::~GamessDataGroup( void )
{
  if ( Title ) delete [] Title;
}
void GamessDataGroup::InitData( void )
{
  Title = 0;
  Coord = NumZVar = 0;
  PointGroup = 1;
  PGroupOrder = Options = 0;
  SetUseSym( true );
}
short GamessDataGroup::SetPointGroup( GAMESSPointGroup NewPGroup )
{
  if (( NewPGroup<invalidPGroup )||( NewPGroup>NumberGAMESSPointGroups ) ) return -1;

  PointGroup = NewPGroup;
  return PointGroup;
}
const char * GamessDataGroup::GetGAMESSPointGroupText( GAMESSPointGroup p )
{
  switch ( p ) {
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
short GamessDataGroup::SetPointGroup( char *GroupText )
{
  GAMESSPointGroup NewPGroup=invalidPGroup;

  if ( GroupText[0] == 'S' ) {
    PGroupOrder = GroupText[2] - 48;
    GroupText[2]='N';
  } else {
    int i=0;
    while ( GroupText[i]&&( GroupText[i]!=' ' ) ) {
      if ( isdigit( GroupText[i] )&&( GroupText[i]!='1' ) ) {
        PGroupOrder = GroupText[i] - 48; //single digit converted to decimal digit
        GroupText[i]='N';
      }
      i++;
    }
  }

  for ( int i=1; i<NumberGAMESSPointGroups; i++ ) {
    if ( strcmp( GroupText, GetGAMESSPointGroupText(( GAMESSPointGroup ) i ) )==0 ) {
      NewPGroup = ( GAMESSPointGroup ) i;
      break;
    }
  }
  if ( NewPGroup<=invalidPGroup ) return invalidPGroup;

  if ( NewPGroup<0 ) return -1;

  PointGroup = NewPGroup;
  return PointGroup;
}
short GamessDataGroup::SetPointGroupOrder( short NewOrder )
{
  if ( NewOrder > 0 ) PGroupOrder = NewOrder;
  return PGroupOrder;
}
short GamessDataGroup::SetTitle( const char *NewTitle, long length )
{
  if ( Title ) delete Title;
  Title = NULL;

  if ( length == -1 ) length = strlen( NewTitle );

  long TitleStart=0, TitleEnd=length-1, i, j;
  //Strip blanks of both ends of title
  while (( NewTitle[TitleStart] <= ' ' )&&( TitleStart<length ) ) TitleStart ++;
  while (( NewTitle[TitleEnd] <= ' ' )&&( TitleEnd>0 ) ) TitleEnd --;
  length = TitleEnd - TitleStart + 1;

  if ( length <= 0 ) return 0;
  if ( length > 132 ) return -1; //Title card is limited to one line

  Title = new char[length + 1];
  //if (Title == NULL) throw MemoryError();
  j=0;
  for ( i=TitleStart; i<=TitleEnd; i++ ) {
    if (( NewTitle[i] == '\n' )||( NewTitle[i] == '\r' ) ) {
      Title[j] = 0;
      break;
    }
    Title[j] = NewTitle[i];
    j++;
  }
  Title[j]=0;
  return j;
}
const char * GamessDataGroup::GetTitle( void ) const
{
  return Title;
}
CoordinateType GamessDataGroup::GetCoordType( void ) const
{
  return ( CoordinateType ) Coord;
}
const char * GamessDataGroup::GetCoordTypeText( CoordinateType t )
{
  switch ( t ) {
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
CoordinateType GamessDataGroup::SetCoordType( const char * CoordText )
{
  CoordinateType NewCoord = invalidCoordinateType;
  for ( int i=1; i<NumberCoordinateTypes; i++ ) {
    if ( strcmp( CoordText, GetCoordTypeText(( CoordinateType ) i ) )==0 ) {
      NewCoord = ( CoordinateType ) i;
      break;
    }
  }
  if ( NewCoord<=invalidCoordinateType ) return invalidCoordinateType;
  Coord = NewCoord;
  return ( CoordinateType ) Coord;
}
CoordinateType GamessDataGroup::SetCoordType( CoordinateType NewType )
{
  if (( NewType<UniqueCoordType )&&( NewType>NumberCoordinateTypes ) ) return invalidCoordinateType;
  Coord = NewType;
  return ( CoordinateType ) Coord;
}
bool GamessDataGroup::SetUnits( bool NewType )
{
  if ( Options & 1 ) Options -= 1;
  if ( NewType ) Options += 1;
  return GetUnits();
}
bool GamessDataGroup::SetUseSym( bool State )
{
  if ( Options & ( 1<<1 ) ) Options -= ( 1<<1 );
  if ( State ) Options += ( 1<<1 );
  return GetUseSym();
}
short GamessDataGroup::SetNumZVar( short NewNum )
{
  if ( NewNum<0 ) return -1; //bad number
  NumZVar = NewNum;
  return NumZVar;
}

void GamessDataGroup::WriteHeaderToFile( ostream &File )
{
  //Punch the group label
  File << endl << " $DATA " << endl;
  //title
  if ( Title == NULL ) {
    File << "Title" << endl;
  } else {
    File << Title << endl;
  }
}

void GamessDataGroup::WriteToFile( ostream &File, GamessInputData *IData, Molecule * molecule )
{
  char Out[GAMESS_BUFF_LEN];

  //   Frame * cFrame = molecule->GetCurrentFramePtr();
  //   BasisSet * lBasis = molecule->GetBasisSet();
  //   BasisTest = BasisTest && lBasis; //Make sure there really is a basis set defined
  // if (BasisTest) File << " $CONTRL NORMP=1 $END" << endl;
  //Point Group
  if (( PointGroup!=0 )&&( PointGroup!=1 ) ) File << "" << endl;
  //coordinates
  //   if (Coord == ZMTCoordType) { //"normal" style z-matrix
  //     Internals * IntCoords = molecule->GetInternalCoordinates();
  //     if (IntCoords) IntCoords->WriteCoordinatesToFile(File, molecule, Prefs);
  //   } else if (Coord == ZMTMPCCoordType) {
  //     Internals * IntCoords = molecule->GetInternalCoordinates();
  //     if (IntCoords) IntCoords->WriteMPCZMatCoordinatesToFile(File, molecule, Prefs);
  //   } else {
  if ( IData->EFP->GetGroupCount( GamessEFPGroup::QMType ) ||
       IData->EFP->GetGroupCount( GamessEFPGroup::EFPType ) == 0 ) {
    WriteHeaderToFile( File );
  }

  if (( PointGroup>GAMESS_CI )&&( PointGroup<GAMESS_TD ) ) {
    sprintf( Out, "%s %d", GetPointGroupText(), PGroupOrder );
  } else {
    sprintf( Out, "%s", GetPointGroupText() );
  }
  File << Out << endl;

  // we need the molecule!
  if ( !molecule ) { return; }

  if ( IData->EFP->GetGroupCount() ) {
    // write out EFP info
    for ( EFPGroupIter iter = IData->EFP->GetGroupBegin(); iter != IData->EFP->GetGroupEnd(); iter++ ) {
      if (( *iter )->type != GamessEFPGroup::QMType ) { continue; }

      for ( std::vector<Atom *>::iterator i = ( *iter )->atoms.begin(); i != ( *iter )->atoms.end(); i++ ) {
        Atom *atom = *i;

        char atomicNumber = atom ->GetAtomicNum();

        sprintf( Out, "%s   %5.1f  %10.5f  %10.5f  %10.5f",
                  etab.GetSymbol( atomicNumber ), ( float ) atomicNumber,
                  atom->GetX(), atom->GetY(), atom->GetZ() );
        File << Out << endl;
      }
    }

    if ( IData->EFP->GetGroupCount( GamessEFPGroup::QMType ) ) {
      File << " $END" << endl << endl;
    }

    File << " $EFRAG" << endl;

    for ( EFPGroupIter iter = IData->EFP->GetGroupBegin(); iter != IData->EFP->GetGroupEnd(); iter++ ) {
      if (( *iter )->type != GamessEFPGroup::EFPType ) { continue; }

      double com[3];

      ( *iter )->GetCenterOfMass( molecule, com[0], com[1], com[2] );

      Atom *atomIdx[3];
      int dist[3];

      for ( int i=0; i < 3; i++ ) {
        atomIdx[i] = 0;
        dist[i] = 0.0;
      }

      for ( std::vector<Atom *>::iterator idx = ( *iter )->atoms.begin(); idx != ( *iter )->atoms.end(); idx++ ) {
        Atom *atom = *idx;

        double atomPos[3];
        atomPos[0] = atom->GetX();
        atomPos[1] = atom->GetY();
        atomPos[2] = atom->GetZ();

        double d = distance( atomPos, com );

        for ( int i=0; i<3; i++ ) {
          if ( !atomIdx[i] ) {
            atomIdx[i] = *idx;
            dist[i] = d;
            break;
          }
          if ( d < dist[i] ) {
            //printf("insert at %d\n", i);

            for ( int j=2; j > i; j-- ) {
              //printf("atom[%d] becoming atom[%d]\n", j, j-1);
              atomIdx[j] = atomIdx[j-1];
              dist[j] = dist[j-1];
            }

            atomIdx[i] = *idx;
            dist[i] = d;
            break;
          }
        }
      }

      File << "FRAGNAME=" << ( *iter )->name << endl;

      for ( int i=0; i<3; i++ ) {
        if ( !atomIdx[i] ) { break; }

        char atomicNumber = atomIdx[i]->GetAtomicNum();

        sprintf( Out, "%s   %5.1f  %10.5f  %10.5f  %10.5f",
                  etab.GetSymbol( atomicNumber ), ( float ) atomicNumber,
                  atomIdx[i]->GetX(), atomIdx[i]->GetY(), atomIdx[i]->GetZ() );
        File << Out << endl;
      }

      /*      {
              OBAtom *atom = molecule->GetAtom(*idx);
              char atomicNumber = atom ->GetAtomicNum();

              if(atom)
              {
                sprintf(Out, "%s   %5.1f  %10.5f  %10.5f  %10.5f",
                        etab.GetSymbol(atomicNumber), (float) atomicNumber,
                                       atom->GetX(), atom->GetY(), atom->GetZ());
                File << Out << endl;
              }
            }*/
    }

    File << " $END" << endl << endl;

  } else {
    // write out normal molecule stuff
    FOR_ATOMS_OF_MOL( atom, molecule ) {
      int atomicNumber = atom->GetAtomicNum();
      sprintf( Out, "%s   %5.1f  %10.5f  %10.5f  %10.5f",
               etab.GetSymbol( atomicNumber ), ( float ) atomicNumber,
               atom->GetX(), atom->GetY(), atom->GetZ() );
      File << Out << endl;
      //       if (BasisTest) lBasis->WriteBasis(File, iatom);
    }

    File << " $END" << endl;
  }
//   }

//   if (NumZVar) { //punch out the current connectivity in a $ZMAT group
//     Internals * IntCoords = molecule->GetInternalCoordinates();
//     if (IntCoords) IntCoords->WriteZMATToFile(File);
//   }
}
#pragma mark GamessGuessGroup
//Guess Group functions
//This function is here to provide a default value before returning the string
const char * GamessGuessGroup::GetGuessText( void ) const
{
  short value = GetGuess();
  if ( value == 0 ) value = 1;

  return ConvertGuessType( value );
}
short GamessGuessGroup::SetGuess( const char * GuessText )
{
  short NewGuess = -1;

  for ( int i=1; i<NumberGuessTypes; i++ ) {
    const char * val = ConvertGuessType( i );
    if ( -1<LocateKeyWord( GuessText, val, strlen( val ), 7 ) ) {
      NewGuess = i;
      break;
    }
  }
  if ( NewGuess<0 ) return -1;
  NewGuess = SetGuess( NewGuess );
  return NewGuess;
}
const char * GamessGuessGroup::ConvertGuessType( const int & type )
{
  switch ( type ) {
    case HUCKELGuessType:
      return "HUCKEL";
    case HCOREGuessType:
      return "HCORE";
    case MOREADGuessType:
      return "MOREAD";
    case MOSAVEDGuessType:
      return "MOSAVED";
    case SkipGuessType:
      return "SKIP"; //By hand later?
    default:
      return "invalid";
  }
  return NULL;
}
GamessGuessGroup::GamessGuessGroup( void )
{
  InitData();
}
GamessGuessGroup::GamessGuessGroup( GamessGuessGroup *Copy )
{ //copy constructor
  if ( Copy ) {
    *this = *Copy;
    IOrder = JOrder = NULL;
    //check and copy I & J order here
  }
}
void GamessGuessGroup::InitData( void )
{
  MOTolZ = MOTolEquil = 0.0;
  IOrder = JOrder = NULL;
  NumOrbs = 0;
  VecSource = 0;
  GuessType = 0;
  Options = 0;
}
void GamessGuessGroup::WriteToFile( ostream &File, GamessInputData *IData )
{
  long test=false;
  char Out[GAMESS_BUFF_LEN];

  //   Frame * lFrame = MainData->GetCurrentFramePtr();
  //first determine wether or not the Guess group needs to be punched
  if ( GetGuess() ) test = true;
  if ( GetPrintMO() ) test = true;
  if ( GetMix()&&IData->Control->GetMultiplicity()&&
       ( IData->Control->GetSCFType()==2 ) ) test = true;

  if ( !test ) return;

  //Punch the group label
  File << " $GUESS ";
  //Guess Type
  if ( GetGuess() ) {
    sprintf( Out,"GUESS=%s ", GetGuessText() );
    File << Out;
  }
  //NumOrbs
  //FIXME help!  i need somebody
  if ( GetGuess()==3 ) {
    long nOrbs = GetNumOrbs();
    //     if (!nOrbs) { //Make a guess if the guess comes from local orbs
    //       short tempVec = GetVecSource();
    //       const std::vector<OrbitalRec *> * Orbs = lFrame->GetOrbitalSetVector();
    //       if (Orbs->size() > 0) {
    //         if ((tempVec<=0)||(tempVec>Orbs->size() + 2)) tempVec = 2;
    //         if (tempVec > 1) {
    //           OrbitalRec * OrbSet = (*Orbs)[tempVec-2];
    //           nOrbs = OrbSet->getNumOccupiedAlphaOrbitals();
    //           if (nOrbs <= 0) nOrbs = OrbSet->getNumAlphaOrbitals();
    //         }
    //       }
    //     }
    sprintf( Out, "NORB=%d ", nOrbs );
    File << Out;
  } //PrintMO
  if ( GetPrintMO() ) {
    sprintf( Out, "PRTMO=.TRUE. " );
    File << Out;
  } //Mix
  if ( GetMix()&&(( IData->Control->GetMultiplicity()==1 )||
                  ( IData->Control->GetMultiplicity()==0 ) )&&( IData->Control->GetSCFType()==2 ) ) {
    sprintf( Out, "MIX=.TRUE. " );
    File << Out;
  }
  File << "$END" << endl;
}

// void GamessGuessGroup::WriteVecGroup(BufferFile *File, Molecule * lData) {
//   //prepare to punch out $Vec information if Guess=MORead
//  if (GetGuess() == 3) {
//   Frame * lFrame = lData->GetCurrentFramePtr();
//   BasisSet * lBasis = lData->GetBasisSet();
//   long NumBasisFuncs = lBasis->GetNumBasisFuncs(false);
//   short tempVec = GetVecSource();
//   const std::vector<OrbitalRec *> * Orbs = lFrame->GetOrbitalSetVector();
//   if ((tempVec != 1)&&(Orbs->size() > 0)) {
//    if ((tempVec<=0)||(tempVec>Orbs->size() + 2)) tempVec = 2;
//    if (tempVec > 1) {
//     OrbitalRec * OrbSet = (*Orbs)[tempVec-2];
//     long nOrbs = GetNumOrbs();
//     if (nOrbs <= 0) { //Setup the default value for the orbital count
//      nOrbs = OrbSet->getNumOccupiedAlphaOrbitals();
//      if (nOrbs <= 0) nOrbs = OrbSet->getNumAlphaOrbitals();
//     }
//     OrbSet->WriteVecGroup(File, NumBasisFuncs, nOrbs);
//    }
//   } else {
//    File << "You must provide a $VEC group here!" << endl;
//   }
//  }
// }

#pragma mark GamessSCFGroup
GamessSCFGroup::GamessSCFGroup( void )
{
  InitData();
}
GamessSCFGroup::GamessSCFGroup( GamessSCFGroup *Copy )
{
  if ( Copy )
    *this = *Copy;
  else
    GamessSCFGroup();
}
void GamessSCFGroup::InitData( void )
{
  SOGTolerance = EnErrThresh = DEMCutoff = DampCutoff = 0.0;
  ConvCriteria = MaxDIISEq = MVOCharge = 0;
  Punch = Options1 = ConverganceFlags = 0;
  //default Direct SCF to true. This is not the GAMESS default
  //but is better in most cases.
  SetDirectSCF( false );
  SetFockDiff( true );
}
bool GamessSCFGroup::SetDirectSCF( bool State )
{
  if ( Options1 & 1 ) Options1--;
  if ( State ) Options1 ++;
  return GetDirectSCF();
}
bool GamessSCFGroup::SetFockDiff( bool State )
{
  if ( Options1 & 2 ) Options1 -= 2;
  if ( State ) Options1 += 2;
  return GetFockDiff();
}
bool GamessSCFGroup::SetUHFNO( bool State )
{
  if ( Options1 & 4 ) Options1 -= 4;
  if ( State ) Options1 += 4;
  return GetUHFNO();
}
short GamessSCFGroup::SetConvergance( short NewConv )
{
  if ( NewConv > 0 ) ConvCriteria = NewConv;
  return ConvCriteria;
}
void GamessSCFGroup::WriteToFile( ostream &File, GamessInputData *IData )
{
  long test=false;
  char Out[GAMESS_BUFF_LEN];

  //first determine wether or not the SCF group needs to be punched
  if ( IData->Control->GetSCFType() > 4 ) return; //not relavent to the selected SCF type
  if ( ConvCriteria > 0 ) test = true;
  if ( GetDirectSCF() ) test = true;

  if ( !test ) return;

  //Punch the group label
  File << " $SCF ";
  //Direct SCF
  if ( GetDirectSCF() ) {
    sprintf( Out,"DIRSCF=.TRUE. " );
    File << Out;
    if ( !GetFockDiff() && IData->Control->GetSCFType()<=3 ) { //Fock Differencing requires direct SCF
      sprintf( Out,"FDIFF=.FALSE. " );
      File << Out;
    }
  }
  //convergance
  if ( ConvCriteria > 0 ) {
    sprintf( Out, "NCONV=%d ", ConvCriteria );
    File << Out;
  } //UHF Natural Orbitals
  if ( GetUHFNO() ) {
    sprintf( Out, "UHFNOS=.TRUE. " );
    File << Out;
  }

  File << "$END" << endl;
}
#pragma mark GamessMP2Group
GamessMP2Group::GamessMP2Group( void )
{
  InitData();
}
GamessMP2Group::GamessMP2Group( GamessMP2Group *Copy )
{
  *this = *Copy;
}
void GamessMP2Group::InitData( void )
{
  CutOff = 0.0;
  NumCoreElectrons = -1;
  Memory = 0;
  Method = AOInts = LMOMP2 = 0;
  MP2Prop = false;
}
float GamessMP2Group::SetIntCutoff( float NewCutoff )
{
  if ( NewCutoff >= 0.0 ) CutOff = NewCutoff;
  return CutOff;
}
long GamessMP2Group::SetNumCoreElectrons( long NewNum )
{
  if ( NewNum>=-1 ) NumCoreElectrons = NewNum;
  return NumCoreElectrons;
}
long GamessMP2Group::SetMemory( long NewMem )
{
  if ( NewMem >= 0 ) Memory = NewMem;
  return Memory;
}
char GamessMP2Group::SetMethod( char NewMethod )
{
  if (( NewMethod==2 )||( NewMethod==3 ) ) Method = NewMethod;
  return Method;
}
const char * GamessMP2Group::GetAOIntMethodText( void ) const
{
  if ( AOInts == 0 ) return NULL;
  if ( AOInts == 1 ) return "DUP";
  return "DIST";
}
void GamessMP2Group::SetAOIntMethod( const char * t )
{
  if ( !t ) return;
  if ( !strcmp( t, "DUP" ) ) AOInts = 1;
  else if ( !strcmp( t, "DIST" ) ) AOInts = 2;
}
char GamessMP2Group::SetAOIntMethod( char NewMethod )
{
  if (( NewMethod == 1 )||( NewMethod == 2 ) ) AOInts = NewMethod;
  return AOInts;
}
bool GamessMP2Group::GetLMOMP2( void ) const
{
  if ( LMOMP2 ) return true;
  return false;
}
bool GamessMP2Group::SetLMOMP2( bool State )
{
  if ( State ) LMOMP2 = true;
  else LMOMP2 = false;
  return LMOMP2;
}
void GamessMP2Group::WriteToFile( ostream &File, GamessInputData *IData )
{
  long test=false;
  char Out[GAMESS_BUFF_LEN];

  //first determine wether or not the MP2 group needs to be punched
  if ( IData->Control->GetMPLevel() != 2 ) return; //Don't punch if MP2 isn't active
  if ( NumCoreElectrons>=0||Memory||Method>2||AOInts ) test = true;
  if ( GetLMOMP2() ) test = true;
  if ( CutOff > 0.0 ) test = true;

  if ( !test ) return;

  //Punch the group label
  File << " $MP2 ";
  //core electrons
  // was >= -dcurtis
  if ( NumCoreElectrons >= 0 ) {
    sprintf( Out,"NACORE=%ld ", NumCoreElectrons );
    File << Out;
    if ( IData->Control->GetSCFType() == GAMESS_UHF ) {
      sprintf( Out,"NBCORE=%ld ", NumCoreElectrons );
      File << Out;
    }
  }
  //MP2Prop
  if (( IData->Control->GetRunType() <= Energy ) && GetMP2Prop() ) {
    sprintf( Out, "MP2PRP=.TRUE. " );
    File << Out;
  }
  //LMOMP2
  if ( GetLMOMP2() ) {
    sprintf( Out, "LMOMP2=.TRUE. " );
    File << Out;
  } //Memory
  if ( Memory ) {
    sprintf( Out, "NWORD=%ld ",Memory );
    File << Out;
  } //CutOff
  if ( CutOff > 0.0 ) {
    sprintf( Out, "CUTOFF=%.2e ", CutOff );
    File << Out;
  } //Method
  if ( Method > 2 && !GetLMOMP2() ) {
    sprintf( Out, "METHOD=%d ", Method );
    File << Out;
  } //AO storage
  if ( AOInts ) {
    sprintf( Out, "AOINTS=%s ", GetAOIntMethodText() );
    File << Out;
  }

  File << "$END" << endl;
}
#pragma mark GamessHessianGroup
void GamessHessianGroup::InitData( void )
{
  DisplacementSize = 0.01;
  FrequencyScaleFactor = 1.0;
  BitOptions = 17; //bit 1 + bit 5
}
void GamessHessianGroup::WriteToFile( ostream &File, GamessInputData *IData )
{
  bool method=false;
  char Out[GAMESS_BUFF_LEN];

  //first determine wether or not the hessian group needs to be punched
  //punch for hessians and optimize/sadpoint runs using Hess=Calc
  if ( IData->Control->GetRunType() == 3 ) method = true;
  else if (( IData->Control->GetRunType() == 4 )||( IData->Control->GetRunType() == 6 ) ) {
    if ( IData->StatPt ) {
      if ( IData->StatPt->GetHessMethod() == 3 ) method = true;
    }
  }
  if ( !method ) return;

  bool AnalyticPoss = ((( IData->Control->GetSCFType() == 1 )||( IData->Control->GetSCFType() == 3 )||
                        ( IData->Control->GetSCFType() == 4 )||( IData->Control->GetSCFType() == 0 ) )&&
                       ( IData->Control->GetMPLevel() == 0 ) );
  method = GetAnalyticMethod() && AnalyticPoss;
  //Punch the group label
  File << " $FORCE ";
  //Method
  if ( method ) File << "METHOD=ANALYTIC ";
  else File << "METHOD=SEMINUM ";
  if ( !method ) {
    //NVIB
    if ( GetDoubleDiff() ) {
      File << "NVIB=2 ";
    } //Vib Size
    if ( DisplacementSize != 0.01 ) {
      sprintf( Out, "VIBSIZ=%f ", DisplacementSize );
      File << Out;
    }
  } //Purify
  if ( GetPurify() ) {
    File << "PURIFY=.TRUE. ";
  } //Print internal FC's
  if ( GetPrintFC() ) {
    File << "PRTIFC=.TRUE. ";
  } //vib analysis
  if ( GetVibAnalysis() ) {
    File << "VIBANL=.TRUE. ";
    if ( FrequencyScaleFactor != 1.0 ) {
      sprintf( Out, "SCLFAC=%f ", FrequencyScaleFactor );
      File << Out;
    }
  } else File << "VIBANL=.FALSE. ";

  File << "$END" << endl;
}
#pragma mark GamessDFTGroup
void GamessDFTGroup::InitData( void )
{
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
  SetAuxFunctions( true );
  SetMethodGrid( true );
}
void GamessDFTGroup::WriteToFile( ostream &File, GamessInputData *IData )
{
  char Out[GAMESS_BUFF_LEN];

  short SCFType = IData->Control->GetSCFType();
  //first determine wether or not the DFT group needs to be punched
  if (( SCFType > 3 )|| !IData->Control->UseDFT() ) return;//only punch for HF runtypes (RHF, ROHF, UHF)
  if ( MethodGrid() ) return; //Only need this group for gridfree method currently
  //Punch the group label
  File << " $DFT ";
  //Write out the functional, and any other optional parameters
  //Method
  if ( !MethodGrid() ) { //punch method if it needs to be grid-free
    sprintf( Out, "METHOD=GRIDFREE " );
    File << Out;
  }

  File << "$END" << endl;
}
const char * GamessDFTGroup::GetDFTGridFuncText( DFTFunctionalsGrid type )
{
  switch ( type ) {
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
const char * GamessDFTGroup::GetDFTGridFreeFuncText( DFTFunctionalsGridFree type )
{
  switch ( type ) {
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

const char * GamessDFTGroup::GetFunctionalText( void ) const
{
  short temp = Functional;
  if ( temp <= 0 ) temp = 1;
  if ( MethodGrid() ) {
    return GetDFTGridFuncText(( DFTFunctionalsGrid ) temp );
  } else { //Grid-free functional list is fairly different
    return GetDFTGridFreeFuncText(( DFTFunctionalsGridFree ) temp );
  }
  return NULL;
}
short GamessDFTGroup::SetFunctional( short newvalue )
{
  //Probably need some checks here??
  Functional = newvalue;
  return Functional;
}
#pragma mark GamessStatPtGroup
void GamessStatPtGroup::InitData( void )
{
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
  SetRadiusUpdate( true );
}
void GamessStatPtGroup::WriteToFile( ostream &File, GamessInputData *IData )
{
  char Out[GAMESS_BUFF_LEN];

  short runType = IData->Control->GetRunType();
  //first determine wether or not the statpt group needs to be punched
  if (( runType != 4 )&&( runType != 6 ) ) return; //only punch for optimize and sadpoint runs

  //Punch the group label
  File << " $STATPT ";
  //write out the convergance criteria and number of steps by default, just
  //to remind the user of their values. Everything else is optional.
  sprintf( Out, "OPTTOL=%g ", GetOptConvergance() );
  File << Out;
  sprintf( Out, "NSTEP=%d ", GetMaxSteps() );
  File << Out;
  //Method
  if ( GetMethod() != 3 ) {
    File << "Method=";
    switch ( GetMethod() ) {
      case 1:
        File << "NR ";
        break;
      case 2:
        File << "RFO ";
        break;
      case 3:
        File << "QA ";
        break;
      case 4:
        File << "SCHLEGEL ";
        break;
      case 5:
        File << "CONOPT ";
        break;
    }
  } //DXMAX if non-default and method is not NR
  if (( GetInitRadius() != 0.0 )&&( GetMethod() !=1 ) ) {
    sprintf( Out, "DXMAX=%g ", GetInitRadius() );
    File << Out;
  }
  if (( GetMethod()==2 )||( GetMethod()==3 ) ) {
    if ( !GetRadiusUpdate() ) File << "TRUPD=.FALSE. ";
    if ( GetMaxRadius() != 0.0 ) {
      sprintf( Out, "TRMAX=%g ", GetMaxRadius() );
      File << Out;
    }
    if ( fabs( GetMinRadius() - 0.05 )>1e-5 ) {
      sprintf( Out, "TRMIN=%g ", GetMinRadius() );
      File << Out;
    }
  }
  if (( runType == 6 )&&( GetModeFollow() != 1 ) ) {
    sprintf( Out, "IFOLOW=%d ", GetModeFollow() );
    File << Out;
  }
  if ( GetStatPoint() ) {
    File << "STPT=.TRUE. ";
    if ( fabs( GetStatJump() - 0.01 )>1e-5 ) {
      sprintf( Out, "STSTEP=%g ", GetStatJump() );
      File << Out;
    }
  }
  if ( GetHessMethod() ) {
    File << "HESS=";
    switch ( GetHessMethod() ) {
      case 1:
        File << "GUESS ";
        break;
      case 2:
        File << "READ ";
        break;
      case 3:
        File << "CALC ";
        break;
    }
  }
  if ( GetHessRecalcInterval() ) {
    sprintf( Out, "IHREP=%d ", GetHessRecalcInterval() );
    File << Out;
  }
  if ( AlwaysPrintOrbs() ) {
    File << "NPRT=1 ";
  }

  File << "$END" << endl;
}
// void MOPacInternals::WriteZMATToFile(BufferFile * File) {
//  char Out[GAMESS_BUFF_LEN];
//
//  File << " $ZMAT IZMAT(1)=" << endl;
//  for (long i=3; i<Count; i+=3) {
//   if (i>9) File << ", " << endl;
//   sprintf(Out, "1,%d,%d, ", (i+3)/3, ConnectionAtoms[i]+1);
//   File << Out << endl;
//   if (i>3) {
//    sprintf(Out, "2,%d,%d,%d, ", (i+3)/3, ConnectionAtoms[i]+1, ConnectionAtoms[i+1]+1);
//    File << Out << endl;
//    if (i>6) {
//     sprintf(Out, "3,%d,%d,%d,%d", (i+3)/3, ConnectionAtoms[i]+1,
//      ConnectionAtoms[i+1]+1, ConnectionAtoms[i+2]+1);
//     File << Out << endl;
//    }
//   }
//  }
//  File << " $END" << endl;
// }
// void MOPacInternals::WriteCoordinatesToFile(BufferFile * File, Molecule * MainData, WinPrefs * Prefs) {
//  UpdateAtoms(MainData); //First make sure the connectivity and values are up to date
//  CartesiansToInternals(MainData);
//   char Out[GAMESS_BUFF_LEN];
//   Str255 AtomLabel;
//   Frame * cFrame = MainData->GetCurrentFramePtr();
//
//  for (int iatom=0; iatom<cFrame->NumAtoms; iatom++) {
//   Prefs->GetAtomLabel(cFrame->Atoms[iatom].GetType()-1, AtomLabel);
//   AtomLabel[AtomLabel[0]+1] = 0;
//   if (iatom==0) sprintf(Out, "%s", (char *) &(AtomLabel[1]));
//   else if (iatom == 1)
//    sprintf(Out, "%s  %d %10.5f", (char *) &(AtomLabel[1]),
//     ConnectionAtoms[3*iatom]+1, Values[3*iatom]);
//   else if (iatom == 2)
//    sprintf(Out, "%s   %d %10.5f  %d %8.4f",
//     (char *) &(AtomLabel[1]), ConnectionAtoms[3*iatom]+1, Values[3*iatom],
//     ConnectionAtoms[3*iatom+1]+1, Values[3*iatom+1]);
//   else
//    sprintf(Out, "%s   %d %10.5f  %d %8.4f  %d %8.4f",
//     (char *) &(AtomLabel[1]), ConnectionAtoms[3*iatom]+1, Values[3*iatom],
//     ConnectionAtoms[3*iatom+1]+1, Values[3*iatom+1],
//     ConnectionAtoms[3*iatom+2]+1, Values[3*iatom+2]);
//   File << Out << endl;
//  }
// }
// //This if very similar to the prevous function, but the format is a little different
// void MOPacInternals::WriteMPCZMatCoordinatesToFile(BufferFile * File, Molecule * MainData, WinPrefs * Prefs) {
//  UpdateAtoms(MainData); //First make sure the connectivity and values are up to date
//  CartesiansToInternals(MainData);
//  char Out[GAMESS_BUFF_LEN];
//  Str255 AtomLabel;
//  Frame * cFrame = MainData->GetCurrentFramePtr();
//
//  for (int iatom=0; iatom<cFrame->NumAtoms; iatom++) {
//   Prefs->GetAtomLabel(cFrame->Atoms[iatom].GetType()-1, AtomLabel);
//   AtomLabel[AtomLabel[0]+1] = 0;
//   if (iatom==0) sprintf(Out, "%s", (char *) &(AtomLabel[1]));
//   else if (iatom == 1)
//    sprintf(Out, "%s   %10.5f", (char *) &(AtomLabel[1]),
//      Values[3*iatom]);
//   else if (iatom == 2)
//    sprintf(Out, "%s   %10.5f 0 %8.4f 0 %d %d",
//      (char *) &(AtomLabel[1]), Values[3*iatom],
//      Values[3*iatom+1], ConnectionAtoms[3*iatom]+1, ConnectionAtoms[3*iatom+1]+1);
//   else
//    sprintf(Out, "%s   %10.5f 0 %8.4f 0 %8.4f 0 %d %d %d",
//      (char *) &(AtomLabel[1]), Values[3*iatom], Values[3*iatom+1],
//      Values[3*iatom+2], ConnectionAtoms[3*iatom]+1,
//      ConnectionAtoms[3*iatom+1]+1, ConnectionAtoms[3*iatom+2]+1);
//   File << Out << endl;
//  }
// }
// void OrbitalRec::WriteVecGroup(BufferFile * File, const long & NumBasisFuncs, const long & OrbCount) const {
// //First check for and write out the vec label
//  if (Label) { //The label should include any necessary linefeeds
//   File << Label << endl;
//  }
// //Punch the Group title
//  File << " $VEC" << endl;
// //write out the vectors using the GAMESS format (I2,I3,5E15.8)
// #ifdef WIN32
//  //Visual studio defaults to using 3 digit exponent output which doesn't fit in the field
//  //This function changes that behavior to 2 digits.
//  _set_output_format(_TWO_DIGIT_EXPONENT);
// #endif
//   long iline, nVec, nn, nOrbs, pOrb;
//  if ((OrbCount > 0) && (OrbCount <= NumAlphaOrbs)) nOrbs = OrbCount;
//  else nOrbs = NumAlphaOrbs;
//   char Line[GAMESS_BUFF_LEN];
//   float * Vector=Vectors;
//  for (int ipass=0; ipass<2; ipass++) {
//   if (!Vector) {
//    File << "Error in Vectors request!" << endl;
//    return;
//   }
//   nn = 0; pOrb = 0;
//   for (long i=0; i<nOrbs; i++) {
//    iline = 1; nVec = 0;
//    pOrb++;
//    if (pOrb>=100) pOrb -= 100;
//    sprintf(Line, "%2d%3d", pOrb, iline);
//    File << Line << endl;
//    for (long ivec=0; ivec<NumBasisFuncs; ivec++) {
//     sprintf(Line, "%15.8E", Vector[nn]);
//     nn++;
//     File << Line << endl;
//     nVec++;
//     if ((nVec>=5)&&(ivec+1<NumBasisFuncs)) {//wrap line and start the next line
//      File << "" << endl;
//      iline ++;
//      sprintf(Line, "%2d%3d", pOrb, iline);
//      File << Line << endl;
//      nVec = 0;
//     }
//    }
//    File << "" << endl;
//   }
//   if (BaseWavefunction == UHF) { //Repeat for beta set of orbitals for UHF wavefunctions
//    Vector = VectorsB;
//    if ((OrbCount > 0) && (OrbCount <= NumBetaOrbs)) nOrbs = OrbCount;
//    else nOrbs = NumBetaOrbs;
//   } else ipass++;
//  }
// //finish off the group
//  File << " $END" << endl;
// }
