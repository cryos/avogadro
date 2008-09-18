namespace Avogadro {

class Primitive : QObject
{
%TypeHeaderCode
#include <avogadro/primitive.h>
%End

    public:
      enum Type {
        OtherType=0,
        MoleculeType,
        AtomType,
        BondType,
        ResidueType,
        ChainType,
        SurfaceType,
        PlaneType,
        GridType,
        PointType,
        VectorType,
        NonbondedType,
        TextType,
        LastType,
        FirstType=OtherType
      };

      Primitive(QObject *parent = 0);
      explicit Primitive(Type type, QObject *parent=0);
      virtual ~Primitive();
      void update();
      Type type() const;
      QReadWriteLock *lock();
      void setId(unsigned long m_id);
      unsigned long id() const;

    signals:
      void updated();

};

class Atom : Avogadro::Primitive, OpenBabel::OBAtom
{
%TypeHeaderCode
#include <avogadro/primitive.h>
%End

    public:
      Atom(QObject *parent=0);
      const Eigen::Vector3d &pos () const;
      void setPos(const Eigen::Vector3d &vec);

};

class Bond : Avogadro::Primitive, OpenBabel::OBBond
{
%TypeHeaderCode
#include <avogadro/primitive.h>
%End

    public:
      Bond(QObject *parent=0);
  
};

class Residue : Avogadro::Primitive, OpenBabel::OBResidue
{
%TypeHeaderCode
#include <avogadro/primitive.h>
%End

    public:
      Residue(QObject *parent=0);

};



class Molecule : Avogadro::Primitive, OpenBabel::OBMol
{
%TypeHeaderCode
#include <avogadro/primitive.h>
%End

    public:
      Molecule(QObject *parent=0);
      Molecule(const Avogadro::Molecule &other);
      virtual ~Molecule();
      void update();
      Avogadro::Atom *CreateAtom();
      Avogadro::Bond * CreateBond();
      Avogadro::Residue * CreateResidue();
      void DestroyAtom(OpenBabel::OBAtom* atom);
      void DestroyBond(OpenBabel::OBBond* bond);
      void DestroyResidue(OpenBabel::OBResidue* residue);
      Avogadro::Atom *newAtom();
      Avogadro::Atom *newAtom(unsigned long id);
      Avogadro::Bond *newBond();
      Avogadro::Bond *newBond(unsigned long id);
      Avogadro::Atom *getAtomById(unsigned long id) const;
      Avogadro::Bond *getBondById(unsigned long id) const;
      const Eigen::Vector3d & center() const;
      const Eigen::Vector3d & normalVector() const;
      // const double & radius() const;
      const Avogadro::Atom *farthestAtom() const;
      // Molecule& operator=(const Molecule& other);
      Avogadro::Molecule& operator+=(const Avogadro::Molecule& other);

    signals:
      void primitiveAdded(Avogadro::Primitive *primitive);
      void primitiveUpdated(Avogadro::Primitive *primitive);
      void primitiveRemoved(Avogadro::Primitive *primitive);
  
};

};
