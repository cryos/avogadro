//%Module openbabel 0

namespace OpenBabel {

/*
//////////////////////////////////////////////////
//  OBConversion
//////////////////////////////////////////////////
class OBConversion
{
%TypeHeaderCode
#include <openbabel/obconversion.h>
%End
    
  public:
    OBConversion(std::istream* is=NULL, std::ostream* os=NULL);
    OBConversion(const OBConversion& o);
    virtual ~OBConversion(); 
    bool SetInAndOutFormats(const char* inID, const char* outID);
    std::string WriteString(OBBase* pOb, bool trimWhitespace = false);
    bool WriteFile(OBBase* pOb, std::string filePath);
    bool ReadString(OBBase* pOb, std::string input);
    bool ReadFile(OBBase* pOb, std::string filePath);
};
*/



//////////////////////////////////////////////////
//  OBAtom
//////////////////////////////////////////////////
class OBAtom
{
%TypeHeaderCode
#include <openbabel/atom.h>
%End

  public:
    void SetAtomicNum(int atomicnum);

};

//////////////////////////////////////////////////
//  OBBond
//////////////////////////////////////////////////
class OBBond
{
%TypeHeaderCode
#include <openbabel/bond.h>
%End

  public:

};

//////////////////////////////////////////////////
//  OBResidue
//////////////////////////////////////////////////
class OBResidue
{
%TypeHeaderCode
#include <openbabel/residue.h>
%End

  public:

};

//////////////////////////////////////////////////
//  OBMol
//////////////////////////////////////////////////
class OBMol
{
%TypeHeaderCode
#include <openbabel/mol.h>
%End

};



};

