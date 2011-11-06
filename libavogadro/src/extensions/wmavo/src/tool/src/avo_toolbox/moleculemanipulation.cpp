
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

  This file is part of WmAvo (WiiChem project)
  WmAvo - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmAvo is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmAvo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmAvo. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#include "moleculemanipulation.h"


namespace Avogadro
{

  const Eigen::Vector3d MoleculeManipulation::m_vect3d0(Eigen::Vector3d(0., 0., 0.)) ;
  Eigen::Transform3d MoleculeManipulation::m_transf3d0 ;

  MoleculeManipulation::MoleculeManipulation( Molecule *molecule )
    : m_molecule(molecule), m_isMoveAtom(false), m_errorMsgGetFragment(),
      m_addHydrogens(WMEX_ADJUST_HYDROGEN), 
      m_atomicNumberCurrent(WMEX_CREATEDATOMDEFAULT), 
      m_barycenterRefMolecule(m_vect3d0), m_sumOfWeights(0), m_atomsBarycenter(m_vect3d0),

      m_rotationAxe(m_vect3d0), m_rotationAxeBond(NULL), m_rotationAxePoint(m_vect3d0),
      m_spanTreeNodeFirst(NULL)
  {
    m_transf3d0.matrix().setIdentity() ;
  }

  MoleculeManipulation::~MoleculeManipulation()
  {}


  /**
    * Rotate atoms. This is the 1st action to move atoms. Then, use moveAtomEnd(),
    * it is necesary to calculate the barycenter with less performance penality.
    * @param atomList The selected atoms which move
    * @param transfAtomRotate The transformation matrix to rotate atom(s)
    */
  void MoleculeManipulation::rotateAtomBegin( const QList<Atom*> &atomList, const Eigen::Transform3d& transfAtomRotate )
  {
    moveAtomBegin( atomList, false, m_vect3d0, true, transfAtomRotate ) ;
  }

  /**
    * Rotate atoms. This is the 1st action to move atoms. Then, use moveAtomEnd(),
    * it is necesary to calculate the barycenter with less performance penality.
    * @param primList The selected atoms which move
    * @param transfAtomRotate The transformation matrix to rotate atom(s)
    */
  void MoleculeManipulation::rotateAtomBegin( const QList<Primitive*> & primList, const Eigen::Transform3d& transfAtomRotate )
  {
    moveAtomBegin( primList, false, m_vect3d0, true, transfAtomRotate ) ;
  }

  /**
    * Translate atoms. This is the 1st action to move atoms. Then, use moveAtomEnd(),
    * it is necesary to calculate the barycenter with less performance penality.
    * @param atomList The selected atoms which move
    * @param vectAtomTranslate The transformation vector to translate atom(s)
    */
  void MoleculeManipulation::tranlateAtomBegin( const QList<Atom*> &atomList, const Eigen::Vector3d& vectAtomTranslate )
  {
    moveAtomBegin( atomList, true, vectAtomTranslate, false, m_transf3d0 ) ;
  }

  /**
    * Translate atoms. This is the 1st action to move atoms. Then, use moveAtomEnd(),
    * it is necesary to calculate the barycenter with less performance penality.
    * @param primList The selected atoms which move
    * @param vectAtomTranslate The transformation vector to translate atom(s)
    */
  void MoleculeManipulation::tranlateAtomBegin( const QList<Primitive*> &primList, const Eigen::Vector3d& vectAtomTranslate )
  {
    moveAtomBegin( primList, true, vectAtomTranslate, false, m_transf3d0 ) ;
  }


  /**
    * Move atoms. This is the 1st action to move atoms. Then, use moveAtomEnd(),
    * it is necesary to calculate the barycenter with less performance penality.
    * @param atomList The selected atoms which move
    * @param hasTranslation The user wants a translation for the selected atoms
    * @param vectAtomTranslate The transformation vector to translate atom(s)
    * @param hasRotation The user wants a rotation for the selected atoms
    * @param transfAtomRotate The transformation matrix to rotate atom(s)
    */
  void MoleculeManipulation::moveAtomBegin( const QList<Atom*> &atomList, bool hasTranslation, const Eigen::Vector3d& vectAtomTranslate, bool hasRotation, const Eigen::Transform3d& transfAtomRotate )
  {
    if( atomList.size()>0 )
    {
      foreach( Atom *atom, atomList )
      {
        if( atom!=NULL && atom->type()==Primitive::AtomType )
        {
          if( !m_isMoveAtom )
          { // Calcul the new barycenter foreach atom.
            // It must bo done once for all atoms.
            updateBarycenter( *(atom->pos()), false, false ) ;
          }

          if( hasTranslation )
            atom->setPos( *(atom->pos()) + vectAtomTranslate ) ;

          if( hasRotation )
            atom->setPos( transfAtomRotate * *(atom->pos()) ) ;
        }
      }

      if( !m_isMoveAtom )
      { // Calcul the new barycenter once.
        m_isMoveAtom = true ;
      }
    }
  }


  /**
    * Move atoms. This is the 1st action to move atoms. Then, use moveAtomEnd(),
    * it is necesary to calculate the barycenter with less performance penality.
    * @param primList The selected atoms which move
    * @param hasTranslation The user wants a translation for the selected atoms
    * @param vectAtomTranslate The transformation vector to translate atom(s)
    * @param hasRotation The user wants a rotation for the selected atoms
    * @param transfAtomRotate The transformation matrix to rotate atom(s)
    */
  void MoleculeManipulation::moveAtomBegin( const QList<Primitive*> &primList, bool hasTranslation, const Eigen::Vector3d& vectAtomTranslate, bool hasRotation, const Eigen::Transform3d& transfAtomRotate )
  {
    if( primList.size()>0 )
    {
      Atom *a=NULL ;

      foreach( Primitive *p, primList )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
        {
          a = static_cast<Atom*>(p) ;

          if( !m_isMoveAtom )
          { // Calcul the new barycenter foreach atom.
            // It must bo done once for all atoms.
            updateBarycenter( *(a->pos()), false, false ) ;
          }

          if( hasTranslation )
            a->setPos( *(a->pos()) + vectAtomTranslate ) ;

          if( hasRotation )
            a->setPos( transfAtomRotate * *(a->pos()) ) ;
        }
      }

      if( !m_isMoveAtom )
      { // Calcul the new barycenter once.
        m_isMoveAtom = true ;
      }
    }
  }


  /**
    * Finish the action of "Move atoms". It let to calculate the barycenter
    * with less performance penality.
    * @param atomList The selected atoms which change the barycenter
    */
  void MoleculeManipulation::moveAtomEnd( const QList<Atom*> &atomList )
  {
    if( m_isMoveAtom )
    {
      m_isMoveAtom = false ;

      // Calculate the barycenter.
      foreach( Atom *a, atomList )
      {
        if( a!=NULL && a->type()==Primitive::AtomType )
          updateBarycenter( *(a->pos()), true, false ) ;
      }
    }
  }


  /**
    * Finish the action of "Move atoms". It let to calculate the barycenter
    * with less performance penality.
    * @param primList The selected atoms which change the barycenter
    */
  void MoleculeManipulation::moveAtomEnd( const QList<Primitive*> &primList )
  {
    if( m_isMoveAtom )
    {
      Atom *a=NULL ;
      m_isMoveAtom = false ;

      // Calculate the barycenter.
      foreach( Primitive *p, primList )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
        {
          a = static_cast<Atom*>(p) ;
          updateBarycenter( *(a->pos()), true, false ) ;
        }
      }
    }
  }


  /**
    * Change the order of a bond by +1 without adjustment of hydrogen.
    * There is a max of (WMEX_MAXBONDNUMBER_BETWEENATOM) authorized bond.
    * @param bond The bond where the change is realized
    */
  void MoleculeManipulation::changeOrderBondBy1( Bond *bond )
  {
    if( m_addHydrogens )
      changeOrderBondBy1WithHA( bond ) ;
    else
      changeOrderBondBy1WithoutHA( bond ) ;
  }


  /**
    * Change the order of a bond by +1 without adjustment of hydrogen.
    * There is a max of (WMEX_MAXBONDNUMBER_BETWEENATOM) authorized bond.
    * @param bond The bond where the change is realized
    */
  void MoleculeManipulation::changeOrderBondBy1WithoutHA( Bond *bond )
  {
    if( m_molecule!=NULL && bond != NULL )
    {
      int order=bond->order()+1 ;

      if( order > WMEX_MAXBONDNUMBER_BETWEENATOM )
        bond->setOrder( 1 ) ;
      else
        bond->setOrder( (short)order ) ;
    }
  }


  /**
    * Change the order of a bond by +1 with adjustment of hydrogen. 
    * There is a max of (WMEX_MAXBONDNUMBER_BETWEENATOM) authorized bond.
    * @param bond The bond where the change is realized
    */
  void MoleculeManipulation::changeOrderBondBy1WithHA( Bond *bond )
  {
    if( m_molecule!=NULL && bond != NULL )
    {
      Atom *a1=bond->beginAtom() ;
      Atom *a2=bond->endAtom() ;

      if( !(a1->isHydrogen() || a2->isHydrogen()) )
      {
        int order=bond->order()+1 ;

        removeHydrogen_p( a1, a2 ) ;
        removeHydrogen_p( a2, a1 ) ;

        if( order > WMEX_MAXBONDNUMBER_BETWEENATOM )
          bond->setOrder( 1 ) ;
        else
          bond->setOrder( (short)order ) ;

        OpenBabel::OBMol obmol=m_molecule->OBMol() ;

        addHydrogen_p( &obmol, a1 ) ;
        addHydrogen_p( &obmol, a2 ) ;

        adjustPartialCharge_p( &obmol ) ;
      }
    }
  }


  /**
    * Change the atomic number of an atom in a molecule without adjustment of hydrogen.
    * @param atom The atom to modify
    * @param atomicNumber The atomic number of the new atom
    */
  void MoleculeManipulation::changeAtomicNumber( Atom *atom, int atomicNumber )
  {
    if( m_addHydrogens )
      changeAtomicNumberWithHA( atom, atomicNumber ) ;
    else
      changeAtomicNumberWithoutHA( atom, atomicNumber ) ;
  }


  /**
    * Change the atomic number of an atom in a molecule without adjustment of hydrogen.
    * @param atom The atom to modify
    * @param atomicNumber The atomic number of the new atom
    */
  void MoleculeManipulation::changeAtomicNumberWithoutHA( Atom *atom, int atomicNumber )
  {
    if( m_molecule!=NULL && atom!=NULL )
      atom->setAtomicNumber( atomicNumber ) ;

    #if __WMDEBUG_MOLMANIP
    else
      mytoolbox::dbgMsg( "Bug in MoleculeManipulation::changeAtomicNumber() : NULL-object non expected." ) ;
    #endif
  }


  /**
    * Change the atomic number of an atom in a molecule with adjustment of hydrogen.
    * @param atom The atom to modify
    * @param atomicNumber The atomic number of the new atom
    */
  void MoleculeManipulation::changeAtomicNumberWithHA( Atom *atom, int atomicNumber )
  {
    if( m_molecule!=NULL && atom!=NULL )
    {
      // Remove current atom and its Hydrogens.
      //if( !atom->isHydrogen() )
        removeHydrogen_p( atom ) ;

      atom->setAtomicNumber( atomicNumber ) ;

      // Adjust the hydrogen.
      OpenBabel::OBMol obmol=m_molecule->OBMol() ;
      addHydrogen_p( &obmol, atom ) ;
      adjustPartialCharge_p( &obmol ) ;
    }
    
    #if __WMDEBUG_MOLMANIP
    else
      mytoolbox::dbgMsg( "Bug in MoleculeManipulation::changeAtomicNumberWithHA() : NULL-object non expected." ) ;
    #endif
  }


  /**
    * Add an atom in a molecule without adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    */
  Atom* MoleculeManipulation::addAtom( Eigen::Vector3d *pos, int atomicNumber )
  {
    if( m_addHydrogens )
      return addAtomWithHA( pos, atomicNumber ) ;
    else
      return addAtomWithoutHA( pos, atomicNumber ) ;
  }


  /**
    * Add an atom bonded with an other atom in a molecule.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    * @param bondedAtom The atom where the atom will be bonded
    * @param order Order of the new bond
    */
  Atom* MoleculeManipulation::addAtom( Eigen::Vector3d *pos, int atomicNumber, Atom *bondedAtom, int order )
  {
    if( m_addHydrogens )
      return addAtomWithHA( pos, atomicNumber, bondedAtom, order ) ;
    else
      return addAtomWithoutHA( pos, atomicNumber, bondedAtom, order ) ;
  }


  /**
    * Add an atom in a molecule without adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    */
  Atom* MoleculeManipulation::addAtomWithoutHA( Eigen::Vector3d *pos, int atomicNumber )
  {
    Atom *a=NULL ;

    if( m_molecule!=NULL && pos!=NULL )
    {
      a = m_molecule->addAtom() ;

      if( a != NULL )
      {
        a->setPos( pos ) ;
        a->setAtomicNumber( atomicNumber ) ;

        updateBarycenter( *(a->pos()), true ) ;
      }
      
      #if __WMDEBUG_MOLMANIP
      else
        mytoolbox::dbgMsg( "Bug in MoleculeManipulation::addAtom() : NULL-object non expected." ) ;
      #endif
    }

    return a ;
  }

  /**
    * Add an atom bonded with an other atom in a molecule without adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    * @param bondedAtom The atom where the atom will be bonded
    * @param order Order of the new bond
    */
  Atom* MoleculeManipulation::addAtomWithoutHA( Eigen::Vector3d *pos, int atomicNumber, Atom *bondedAtom, int order )
  {
    Atom *a=NULL ;

    if( m_molecule!=NULL && pos!=NULL )
    {
      a = addAtomWithoutHA( pos, atomicNumber ) ;

      if( a!=NULL && bondedAtom!=NULL )
      {
        if( !bondedAtom->isHydrogen() || (bondedAtom->isHydrogen() && bondedAtom->valence()<1) )
          addBondWithoutHA( a, bondedAtom, (short)order ) ;
      }
      
      #if __WMDEBUG_MOLMANIP
      else
        mytoolbox::dbgMsg( "Bug in MoleculeManipulation::addAtom() : NULL-object non expected." ) ;
      #endif
    }

    return a ;
  }


  /**
    * Add an atom in a molecule without adjustment of hydrogen.
    * See Atom::setOBAtom() method of Avogadro.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param obAtom Atom from open babel class to add in the molecule
    */
  Atom* MoleculeManipulation::addAtomWithoutHA( OpenBabel::OBAtom *obAtom )
  {
    Atom *a=NULL ;

    if( m_molecule!=NULL && obAtom!=NULL )
    {
      a = m_molecule->addAtom() ;

      if( a != NULL )
      {
        Eigen::Vector3d pos=Eigen::Vector3d(obAtom->x(), obAtom->y(), obAtom->z()) ;
        a->setPos( pos ) ;
        a->setAtomicNumber( obAtom->GetAtomicNum() ) ;
        a->setPartialCharge( obAtom->GetPartialCharge() ) ;

        if( obAtom->GetFormalCharge() != 0 )
          a->setFormalCharge( obAtom->GetFormalCharge() ) ;

        // And add any generic data as QObject properties
        std::vector<OpenBabel::OBGenericData*> data ;
        OpenBabel::OBDataIterator j ;
        OpenBabel::OBPairData *property ;
        data = obAtom->GetAllData( OpenBabel::OBGenericDataType::PairData ) ;

        j=data.begin() ;
        while( j != data.end() )
        {
          property = static_cast<OpenBabel::OBPairData *>(*j) ;

          #if AVO_VERSION_ABOVE_1_0_1
          if (property->GetAttribute() == "label")
          {
            a->setCustomLabel( property->GetValue().c_str() ) ;
            continue;
          }
          else if( property->GetAttribute() == "color" )
          {
            a->setCustomColorName( property->GetValue().c_str() ) ;
            continue;
          }
          else if( property->GetAttribute() == "radius" )
          {
            a->setCustomRadius( QString(property->GetValue().c_str()).toDouble() ) ;
            continue;
          }
          #endif

          a->setProperty( property->GetAttribute().c_str(), property->GetValue().c_str() ) ;

          j++ ;
        }

        updateBarycenter( *(a->pos()), true ) ;
      }
      
      #if __WMDEBUG_MOLMANIP
      else
        mytoolbox::dbgMsg( "Bug in MoleculeManipulation::addAtom() : There is a problem with the add of an atom." ) ;
      #endif
    }

    return a ;
  }


  /**
    * Add an atom in a molecule with adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    */
  Atom* MoleculeManipulation::addAtomWithHA( Eigen::Vector3d *pos, int atomicNumber )
  {
    Atom *a=addAtomWithoutHA( pos, atomicNumber ) ;

    if( a != NULL )
    { // Construct an OBMol, call AddHydrogens() and translate the changes

      OpenBabel::OBMol obmol=m_molecule->OBMol() ;

      addHydrogen_p( &obmol, a ) ;
      adjustPartialCharge_p( &obmol ) ;
    }

    return a ;
  }


  /**
    * Add an atom bonded with an other atom in a molecule with adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    * @param bondedAtom The atom where the atom will be bonded
    * @param order Order of the new bond
    */
  Atom* MoleculeManipulation::addAtomWithHA( Eigen::Vector3d *pos, int atomicNumber, Atom *bondedAtom, int order )
  {
    Atom *a=NULL ;

    if( bondedAtom==NULL || (bondedAtom->isHydrogen() && bondedAtom->valence()>0) )
      a = addAtomWithoutHA( pos, atomicNumber ) ;
    else
    {
      if( !bondedAtom->isHydrogen() )
        removeHydrogen_p( bondedAtom ) ;
      a = addAtomWithoutHA( pos, atomicNumber, bondedAtom, order ) ;
    }

    if( a != NULL )
    {
      OpenBabel::OBMol obmol=m_molecule->OBMol() ;

      if( !a->isHydrogen() )
        addHydrogen_p( &obmol, a ) ;

      if( !bondedAtom->isHydrogen() )
        addHydrogen_p( &obmol, bondedAtom ) ;

      adjustPartialCharge_p( &obmol ) ;
    }

    return a ;
  }


  
  /**
    * Add 2 bonded (or not) atoms without adjustment of hydrogen in a molecule.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return PrimitiveList* : Atoms newly created ; else NULL if nothing add.
    * @param pos1 Position of the 1st new atom
    * @param atomicNumber1 The atomic number of the 1st new atom
    * @param pos2 Position of the 2nd new atom
    * @param atomicNumber2 The atomic number of the 2nd new atom
    * @param order Order of the new bond
    */
  PrimitiveList* MoleculeManipulation::addAtoms( Eigen::Vector3d *pos1, int atomicNumber1, Eigen::Vector3d *pos2, int atomicNumber2, int order )
  {
    if( m_addHydrogens )
      return addAtomsWithHA( pos1, atomicNumber1, pos2, atomicNumber2, order ) ;
    else
      return addAtomsWithoutHA( pos1, atomicNumber1, pos2, atomicNumber2, order ) ;
  }


  /**
    * Add 2 bonded (or not) atoms without adjustment of hydrogen in a molecule.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return PrimitiveList* : Atoms newly created ; else NULL if nothing add.
    * @param pos1 Position of the 1st new atom
    * @param atomicNumber1 The atomic number of the 1st new atom
    * @param pos2 Position of the 2nd new atom
    * @param atomicNumber2 The atomic number of the 2nd new atom
    * @param order Order of the new bond
    */
  PrimitiveList* MoleculeManipulation::addAtomsWithoutHA( Eigen::Vector3d *pos1, int atomicNumber1, Eigen::Vector3d *pos2, int atomicNumber2, int order )
  {
    PrimitiveList *addedPrim=NULL ;

    if( m_molecule!=NULL && pos1!=NULL && pos2!=NULL )
    {
      Atom *a1=NULL, *a2=NULL ;
      Bond *b=NULL ;

      a1 = addAtomWithoutHA( pos1, atomicNumber1 ) ;
      a2 = addAtomWithoutHA( pos2, atomicNumber2 ) ;

      if( a1!=NULL || a2!=NULL )
        addedPrim = new PrimitiveList() ;

      if( a1 != NULL )
        addedPrim->append( static_cast<Primitive*>(a1) ) ;

      if( a2 != NULL )
        addedPrim->append( static_cast<Primitive*>(a2) ) ;

      if( a1!=NULL && a2!=NULL && order>0 )
      {
        b = addBondWithoutHA( a1, a2, (short)order ) ;

        if( b != NULL )
          addedPrim->append( static_cast<Primitive*>(b) ) ;
      }
    }

    return addedPrim ;
  }


  /**
    * Add 2 bonded (or not) atoms with adjustment of hydrogen in a molecule.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return PrimitiveList* : Atoms newly created ; else NULL if nothing add.
    * @param pos1 Position of the 1st new atom
    * @param atomicNumber1 The atomic number of the 1st new atom
    * @param pos2 Position of the 2nd new atom
    * @param atomicNumber2 The atomic number of the 2nd new atom
    * @param order Order of the new bond
    */
  PrimitiveList* MoleculeManipulation::addAtomsWithHA( Eigen::Vector3d *pos1, int atomicNumber1, Eigen::Vector3d *pos2, int atomicNumber2, int order )
  {
    PrimitiveList *addedPrim=addAtomsWithoutHA( pos1, atomicNumber1, pos2, atomicNumber2, order ) ;

    if( addedPrim != NULL )
    {
      OpenBabel::OBMol obmol=m_molecule->OBMol() ;
      bool addH=false ;
      Atom *a=NULL, *na=NULL ;

      foreach( Primitive *p, *addedPrim )
      {
        if( p->type() == Primitive::AtomType )
        {
          a = static_cast<Atom*>(p) ;
          addH = addHydrogen_p( &obmol, a ) ;

          // Add all new primitives.
          if( addH )
          {
            foreach( unsigned long i, a->neighbors() )
            {
              na = m_molecule->atomById(i) ;

              if( na != NULL )
              {
                addedPrim->append( na ) ;
                addedPrim->append( a->bond(na) ) ;
              }
            }
          }
        }
      }

      adjustPartialCharge_p( &obmol ) ;
    }

    return addedPrim ;
  }


  /**
    * Add a bond in a molecule.
    * @return Bond* : Bond newly created ; else NULL if nothing add.
    * @param a1 The 1st atom to connect
    * @param a2 The 2nd atom to connect
    * @param order Order of the new bond (less than WMEX_MAXBONDNUMBER_BETWEENATOM)
    */
  Bond* MoleculeManipulation::addBond( Atom *a1, Atom *a2, short order )
  {
    if( m_addHydrogens )
      return addBondWithHA( a1, a2, order ) ;
    else
      return addBondWithoutHA( a1, a2, order ) ;
  }


  /**
    * Add a bond in a molecule without adjusmtent of hydrogen.
    * @return Bond* : Bond newly created ; else NULL if nothing add.
    * @param a1 The 1st atom to connect
    * @param a2 The 2nd atom to connect
    * @param order Order of the new bond (less than WMEX_MAXBONDNUMBER_BETWEENATOM)
    */
  Bond* MoleculeManipulation::addBondWithoutHA( Atom *a1, Atom *a2, short order )
  {
    Bond *b=NULL ;

    if( m_molecule!=NULL && a1!=NULL && a2!=NULL && order>0 && order<WMEX_MAXBONDNUMBER_BETWEENATOM )
    {
      b = m_molecule->addBond() ;

      if( b != NULL )
      {
        b->setOrder( order ) ;
        b->setBegin( a1 ) ;
        b->setEnd( a2 ) ;
      }
      
      #if __WMDEBUG_MOLMANIP
      else
        mytoolbox::dbgMsg( "Bug in MoleculeManipulation::addBond() : a NULL-object not expected." ) ;
      #endif
    }

    return b ;
  }


  /**
    * Add a bond in a molecule with adjustment of hydrogen.
    * @return Bond* : Bond newly created ; else NULL if nothing add.
    * @param a1 The 1st atom to connect
    * @param a2 The 2nd atom to connect
    * @param order Order of the new bond
    */
  Bond* MoleculeManipulation::addBondWithHA( Atom *a1, Atom *a2, short order )
  {
    Bond *b=NULL ;

    if( a1!=NULL && a2!=NULL
        && (!a1->isHydrogen() || (a1->isHydrogen()&&a1->valence()==0))
        && (!a2->isHydrogen() || (a2->isHydrogen()&&a2->valence()==0))
      )
      b = addBondWithoutHA( a1, a2, order ) ;

    if( b != NULL )
    {
      // Remove Hydrogen of a1 and a2.

      if( !a1->isHydrogen() ) removeHydrogen_p( a1, a2 ) ;
      if( !a2->isHydrogen() ) removeHydrogen_p( a2, a1 ) ;

      // Then construct an OBMol, call AddHydrogens() and translate the changes.
      OpenBabel::OBMol obmol=m_molecule->OBMol() ;

      if( !a1->isHydrogen() ) addHydrogen_p( &obmol, a1 ) ;
      if( !a2->isHydrogen() ) addHydrogen_p( &obmol, a2 ) ;

      adjustPartialCharge_p( &obmol ) ;
    }

    return b ;
  }


  #if AVO_DEPRECATED_FCT
  /**
    * Deprecated : Avogadro style.
    * To add Hydrogen and Undo/Redo features after the creation.
    */
  void MoleculeManipulation::addAdjustHydrogenRedoUndo()
  {
    // Add Hydrogen ...
    //molecule->addHydrogens( atom ) ;
    // Add by :
    // AddAtomDrawCommand
    // => AddAtomDrawCommand::redo()
    //   => d->postCommand = new AdjustHydrogensPostCommand(d->molecule, d->id);
    //   => d->postCommand->redo();
    //     => AdjustHydrogensPostCommand::redo()
    //       => d->molecule->addHydrogens(atom);


    /// Add undo, redo & Hydrogen.

    // we added At least the beginAtom or we created a bond to
    // an existing atom or to endAtom that we also created


    AdjustHydrogens::Options atomAdjustHydrogens = AdjustHydrogens::Never;
    if( m_addHydrogens )
    {
      if( m_drawBond )
      // if bond then only remove on undo, rest is handled by bond
        atomAdjustHydrogens = AdjustHydrogens::OnUndo ;
      else
      // if no bond then add on undo and redo
        atomAdjustHydrogens = AdjustHydrogens::Always ;
    }


    //cout << "test5" << endl ;

    /*
    if( atomAdjustHydrogens == AdjustHydrogens::Always )
      cout << "always" << endl ;

    if( atomAdjustHydrogens == AdjustHydrogens::Never )
      cout << "never" << endl ;

    if( atomAdjustHydrogens == AdjustHydrogens::OnUndo )
      cout << "onundo" << endl ;
      */

    //cout << "test6" << endl ;
    AddAtomDrawCommand *beginAtomDrawCommand=NULL ;
    //if( m_hasAddedBeginAtom ) => Comment to adjust H
    {
      if( m_hasAddedBeginAtom
          /* // it is the bond command which adjust H.
          || (!m_hasAddedBeginAtom
              && m_beginAtomDraw!=NULL && !m_beginAtomDraw->isHydrogen())
          */
        )
      { // Either new atom => QUndo,
        // either "old" atom, verif if it is a H => No QUndo, else Adjust H => QUndo

        beginAtomDrawCommand = new AddAtomDrawCommand( m_molecule, m_beginAtomDraw, atomAdjustHydrogens ) ;
        beginAtomDrawCommand->setText( tr("Draw Atom") ) ;
      }
    }

    //cout << "test7" << endl ;

    AddAtomDrawCommand *currentAtomDrawCommand=NULL ;
    //if( m_hasAddedCurAtom )
    {
      if( m_hasAddedCurAtom
          /* // it is the bond command which adjust H.
          || (!m_hasAddedCurAtom && m_curAtomDraw!=NULL && !m_curAtomDraw->isHydrogen())
          */
        )
      { // Either new atom => QUndo,
        // either "old" atom, verif if it is a H => No QUndo, else Adjust H => QUndo

        currentAtomDrawCommand = new AddAtomDrawCommand( m_molecule, m_curAtomDraw, atomAdjustHydrogens ) ;
        currentAtomDrawCommand->setText( tr("Draw Atom") ) ;
      }

      /*
      // For a Magic update. The "click update" is specified on the last created atom.
      QPoint p=m_curAtomDraw->pos() ;
      m_pointForCreate1 = m_widget->mapToGlobal(p) ;
      */
    }

    //cout << "test8" << endl ;
    AddBondDrawCommand *bondCommand=NULL ;
    if( m_hasAddedBond )
    {
      //cout << "tes9.1" << endl ;
      AdjustHydrogens::Options adjBegin=AdjustHydrogens::Never ;
      AdjustHydrogens::Options adjEnd=AdjustHydrogens::Never ;

      if( m_addHydrogens )
      {
        //cout << "test9.2" << endl ;
        /*if( m_hydrogenCommand )
        {
          // don't try to remove/add hydrogens to the hydrogen which will be changed
          // by the ChangeElement command...
          adjBegin = adjEnd = AdjustHydrogens::AddOnRedo | AdjustHydrogens::RemoveOnUndo ;

          if( !m_endAtomAdded )
          {
            foreach( unsigned long id, m_bond->endAtom()->neighbors() )
            {
              Atom *nbr=widget->molecule()->atomById(id) ;
              if( nbr->isHydrogen() )
                adjEnd |= AdjustHydrogens::RemoveOnRedo | AdjustHydrogens::AddOnUndo ;
            }
          }
        }
        else
        */
        {
          Atom *nbr=NULL ;

          if( m_bondDraw->beginAtom()->isHydrogen() )
          {
            //cout << "1 m_bondDraw->endAtom()->isHydrogen()" << endl ;
            adjBegin=AdjustHydrogens::Never ;
          }
          else
          {
            adjBegin = AdjustHydrogens::AddOnRedo | AdjustHydrogens::RemoveOnUndo ;


            // pre-existing atoms might need extra work
            if( !m_hasAddedBeginAtom )
            {
              //cout << "test9.3" << endl ;
              foreach( unsigned long id, m_bondDraw->beginAtom()->neighbors() )
              {
                nbr = m_molecule->atomById(id) ;

                if( nbr->isHydrogen() )
                  adjBegin |= AdjustHydrogens::RemoveOnRedo | AdjustHydrogens::AddOnUndo ;
              }
            }
          }

          if( m_bondDraw->endAtom()->isHydrogen() )
          {
            //cout << "2 m_bondDraw->endAtom()->isHydrogen()" << endl ;
            adjEnd=AdjustHydrogens::Never ;
          }
          else
          {
            adjEnd = AdjustHydrogens::AddOnRedo | AdjustHydrogens::RemoveOnUndo ;

            if( !m_hasAddedCurAtom )
            {
              //cout << "test9.4" << endl ;
              foreach( unsigned long id, m_bondDraw->endAtom()->neighbors() )
              {
                nbr = m_molecule->atomById(id) ;

                if( nbr->isHydrogen() )
                  adjEnd |= AdjustHydrogens::RemoveOnRedo | AdjustHydrogens::AddOnUndo ;
              }
            }
          }
        }
      }

      bondCommand = new AddBondDrawCommand( m_molecule, m_bondDraw, adjBegin, adjEnd ) ;
      //cout << "  bondCommand:" << bondCommand << endl ;
      bondCommand->setText( tr("Draw Bond") ) ;
    }


    // Add undo command in undoStock of Avogadro.


    // Set the actual undo command -- combining sequence if possible
    // we can have a beginAtom w/out bond or endAtom
    // we can have bond w/out endAtom (i.e., to an existing atom)
    // we cannot have endAtom w/out bond
    if( currentAtomDrawCommand || (bondCommand && (beginAtomDrawCommand /*|| m_hydrogenCommand*/)) )
    {
      UndoSequence *seq=NULL ;

      //if( m_hydrogenCommand != NULL )
      //  seq->append( m_hydrogenCommand ) ;
      if( beginAtomDrawCommand != NULL )
      {
        if( seq == NULL )
        {
          seq = new UndoSequence() ;
          seq->setText(tr("Draw"));
        }

        //cout << "  seq->append( beginAtomDrawCommand" << beginAtomDrawCommand << endl ;
        seq->append( beginAtomDrawCommand ) ;
      }

      if( currentAtomDrawCommand != NULL )
      {
        if( seq == NULL )
        {
          seq = new UndoSequence() ;
          seq->setText(tr("Draw"));
        }

        //cout << "  seq->append( currentAtomDrawCommand" << currentAtomDrawCommand << endl ;
        seq->append( currentAtomDrawCommand ) ;
      }

      if( bondCommand != NULL )
      {
        if( seq == NULL )
        {
          seq = new UndoSequence() ;
          seq->setText(tr("Draw"));
        }

        //cout << "  seq->append( bondCommand:" << bondCommand << endl ;
        seq->append( bondCommand ) ;
      }

      if( beginAtomDrawCommand!=NULL || currentAtomDrawCommand!=NULL || bondCommand!=NULL )
      {
        //cout << "  undoStack->push( seq ), sequence undo:" << seq << endl ;
        m_widget->undoStack()->push( seq ) ;
      }
    }
    else if( bondCommand != NULL )
    {
      //cout << "  undoStack->push( bondCommand )" << endl ;
      m_widget->undoStack()->push( bondCommand ) ;
    }
    else if( beginAtomDrawCommand != NULL )
    {
      //cout << "  undoStack->push( beginAtomDrawCommand )" << endl ;
      m_widget->undoStack()->push( beginAtomDrawCommand ) ;
    }
  }
  #endif


  #if AVO_DEPRECATED_FCT
  /**
    * Copy the fragment in the molecule with adjustment of hydrogen.
    * Deprecated : Avogadro code style : no new elements returned.
    * @param fragment The molecule which is included in (molecule)
    */
  void MoleculeManipulation::addFragment1( Molecule *fragment )
  {
    *molecule += fragment ;
  }
  #endif


  /**
    * Copy the fragment in the molecule.
    * It copies just all atoms and bonds without Hydrogen atom.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param fragment The molecule which is included in (molecule)
    */
  PrimitiveList* MoleculeManipulation::addFragment( Molecule *fragment )
  {
    if( m_addHydrogens )
      return addFragmentWithHA( fragment ) ;
    else
      return addFragmentWithoutHA( fragment ) ;
  }

  /**
    * Copy the fragment in the molecule without adjutment of hydrogen.
    * It copies just all atoms and bonds without Hydrogen atom.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param fragment The molecule which is included in (molecule)
    */
  PrimitiveList* MoleculeManipulation::addFragmentWithoutHA( Molecule *fragment )
  {
    PrimitiveList *addedPrim=NULL ;

    if( m_molecule!=NULL && fragment!=NULL )
    {
      #if 1 // AVO_THING
      // Original code of Avogadro
      // (except for updateBarycenter, and the returned value).

      unsigned int initialAtoms=m_molecule->numAtoms()-1 ;
      bool emptyMol=(m_molecule->numAtoms()==0) ;
      addedPrim = new PrimitiveList() ;

      if( emptyMol )
        initialAtoms = 0 ;

      *(m_molecule) += (*fragment) ;
      m_molecule->update() ;

      if( emptyMol )  // we'll miss atom 0, so add it now
        addedPrim->append( m_molecule->atom(0) ) ;

      foreach( Atom *atom, m_molecule->atoms() )
      {
        if( atom->index() > initialAtoms )
        {
          addedPrim->append( const_cast<Atom*>(atom) ) ;

          // Update barycenter.
          updateBarycenter( *(atom->pos()), true ) ;
        }
      }

      // Remove H-Atoms of the fragment.
      foreach( Primitive* p, addedPrim->subList(Primitive::AtomType) )
      {
        Atom *a=static_cast<Atom*>( p ) ;
        if( a!=NULL && a->type()==Primitive::AtomType && a->isHydrogen() )
          removeAtomWithoutHA( a ) ;

        #if __WMDEBUG_MOLMANIP
        else
        {
          if( a->type() != Primitive::AtomType ) 
            mytoolbox::dbgMsg( "Bug : MoleculeManipulation::addFragmentWithoutHA : Atom type expected ..." ) ;
        }
        #endif
      }

      
      #else // MY_THING
      // This code causes problem with acetylene only ...
      // But at the end, it doesn't check every atom of the molecule to
      // update some information (new atoms and to update the barycenter).

      QList<Atom*> newAtomList, oldAtomList ; // To retreive the atoms for the bond.
      Atom *newAtom=NULL, *a1=NULL, *a2=NULL, *a3=NULL, *a4=NULL ;
      Bond *newBond=NULL ;
      addedPrim = new PrimitiveList() ;
      int i=0, nbElt=0 ;
      bool find=false ;

      // "Copy" all atoms.
      foreach( Atom *oldAtom, fragment->atoms() )
      {
        if( !oldAtom->isHydrogen() )
        {
          // "Copy" atom of the fragment.
          newAtom = addAtomWithoutHA( const_cast<Eigen::Vector3d*>(oldAtom->pos()), oldAtom->atomicNumber() ) ;

          // Store for the bonds.
          newAtomList.append( newAtom ) ;
          oldAtomList.append( oldAtom ) ;

          // Store for the return of the function.
          addedPrim->append( newAtom ) ;
        }
      }

      // "Copy" all bonds.
      foreach (Bond *oldBond, fragment->bonds() )
      {
        a1 = oldBond->beginAtom() ;
        a2 = oldBond->endAtom() ;

        if( !(a1->isHydrogen() || a2->isHydrogen()) )
        {
          a3 = NULL ;
          a4 = NULL ;
          i = 0 ;
          nbElt = oldAtomList.size() ;
          find = false ;

          // Search correspondence between fragment atoms and molecule atoms.
          while( i<nbElt && !find )
          {
            if( oldAtomList.at(i) == a1 )
              a3 = newAtomList.at(i) ;
            else if( oldAtomList.at(i) == a2 )
              a4 = newAtomList.at(i) ;

            if( a3!=NULL && a4!=NULL )
              find = true ;
            i++ ;
          }

          if( a3!=NULL && a4!=NULL )
          {
            // "Copy" bond of the fragment.
            newBond = addBondWithoutHA( a3, a4, oldBond->order() ) ;

            // Store for the return of the function.
            addedPrim->append( newBond ) ;
          }
        }
      }
      #endif
    }

    if( addedPrim==NULL || addedPrim->size()<=0 )
    {
      if( addedPrim != NULL )
        delete addedPrim ;
      return NULL ;
    }
    else
      return addedPrim ;
  }



  /**
    * Copy the fragment in the molecule with adjustment of hydrogen.
    * It copies just all atoms and bonds.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param fragment The molecule which is included in (molecule)
    */
  PrimitiveList* MoleculeManipulation::addFragmentWithHA( Molecule *fragment )
  {
    PrimitiveList *addedPrim=NULL ;

    if( m_molecule!=NULL && fragment!=NULL )
    {
      #if 1 // AVO_THING
      // Original code of Avogadro
      // (except for updateBarycenter, and the returned value).

      unsigned int initialAtoms=m_molecule->numAtoms()-1 ;
      bool emptyMol=(m_molecule->numAtoms()==0) ;
      addedPrim = new PrimitiveList() ;

      if( emptyMol )
        initialAtoms = 0 ;

      *(m_molecule) += (*fragment) ;
      m_molecule->update() ;

      if( emptyMol )  // we'll miss atom 0, so add it now
        addedPrim->append( m_molecule->atom(0) ) ;

      foreach( Atom *atom, m_molecule->atoms() )
      {
        if( atom->index() > initialAtoms )
        {
          addedPrim->append( const_cast<Atom*>(atom) ) ;

          // Update barycenter.
          updateBarycenter( *(atom->pos()), true ) ;
        }
      }

      #else // MY_THING
      // This code causes problem with acetylene only ...
      // But at the end, it doesn't check every atom of the molecule to
      // update some information (new atoms and to update the barycenter).

      QList<Atom*> newAtomList, oldAtomList ; // To retreive the atoms for the bond.
      Atom *newAtom=NULL, *a1=NULL, *a2=NULL, *a3=NULL, *a4=NULL ;
      Bond *newBond=NULL ;
      addedPrim = new PrimitiveList() ;
      int i=0, nbElt=0 ;
      bool find=false ;

      // "Copy" all atoms.
      foreach( Atom *oldAtom, fragment->atoms() )
      {
        // "Copy" atom of the fragment.
        newAtom = addAtomWithoutHA( const_cast<Eigen::Vector3d*>(oldAtom->pos()), oldAtom->atomicNumber() ) ;

        // Store for the bonds.
        newAtomList.append( newAtom ) ;
        oldAtomList.append( oldAtom ) ;

        // Store for the return of the function.
        addedPrim->append( newAtom ) ;
      }

      // "Copy" all bonds.
      foreach (Bond *oldBond, fragment->bonds() )
      {
        a1 = oldBond->beginAtom() ;
        a2 = oldBond->endAtom() ;
        a3 = NULL ;
        a4 = NULL ;
        i = 0 ;
        nbElt = oldAtomList.size() ;
        find = false ;

        // Search correspondence between fragment atoms and molecule atoms.
        while( i<nbElt && !find )
        {
          if( oldAtomList.at(i) == a1 )
            a3 = newAtomList.at(i) ;
          else if( oldAtomList.at(i) == a2 )
            a4 = newAtomList.at(i) ;

          if( a3!=NULL && a4!=NULL )
            find = true ;
          i++ ;
        }

        if( a3!=NULL && a4!=NULL )
        {
          // "Copy" bond of the fragment.
          newBond = addBondWithoutHA( a3, a4, oldBond->order() ) ;

          // Store for the return of the function.
          addedPrim->append( newBond ) ;
        }
      }
      #endif
    }    

    if( addedPrim==NULL || addedPrim->size()<=0 )
    {
      if( addedPrim != NULL )
        delete addedPrim ;
      return NULL ;
    }
    else
      return addedPrim ;
    
  }


  /**
    * Copy the fragment in the molecule.
    * It copies just all atoms and bonds without Hydrogen atom.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param fragment The molecule which is included in (molecule)
    * @param bondedAtom The atom where the fragment will be bonded
    */
  PrimitiveList* MoleculeManipulation::addFragment( Molecule *fragment, Atom *bondedAtom )
  {
    if( m_addHydrogens )
      return addFragmentWithHA( fragment, bondedAtom ) ;
    else
      return addFragmentWithoutHA( fragment, bondedAtom ) ;
  }


  /**
    * Copy a fragment in the molecule without hydrogen and without adjustment of hydrogen.
    * And it bonds it on an other atom.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param fragment The molecule which is included in (molecule)
    * @param bondedAtom The atom where the fragment will be bonded
    */
  PrimitiveList* MoleculeManipulation::addFragmentWithoutHA( Molecule *fragment, Atom *bondedAtom )
  {
    PrimitiveList* addedPrim=NULL ;

    if( m_molecule!=NULL && fragment!=NULL && bondedAtom!=NULL )
    {
      bool emptyMol=(m_molecule->numAtoms() == 0) ;
      Atom *startAtom=NULL, *endAtom=NULL ;
      unsigned long startAtomIndex=0, endAtomIndex=0 ;
      int nbAtomInFrag=fragment->numAtoms() ;

      // Add fragment in the molecule.
      addedPrim = addFragmentWithoutHA( fragment ) ;

      // Verify if the (bondedAtom) belongs to the molecule. (fr:appartient)
      if( !emptyMol )
      {
        int tmp=bondedAtom->index() ;
        Atom *a=m_molecule->atom(tmp) ;
        if( a != bondedAtom )
          emptyMol = true ;
      }

      if( !emptyMol && addedPrim!=NULL && addedPrim->size()>0 )
      {
        startAtom = bondedAtom ;

        // Search the bonded non-hydrogen of the molecule, if it exists.
        // First, if (startAtom) is a Hydrogen, change the (startAtom).
        if( startAtom->isHydrogen() )
        {
          Atom *hydrogen=startAtom ;

          // Get the bonded non-hydrogen and remove this atom.
          if( hydrogen->neighbors().size() > 0 )
          {
            startAtom = m_molecule->atomById( hydrogen->neighbors()[0] ) ;
            removeAtomWithoutHA( hydrogen ) ;
          }
        }

        startAtomIndex = startAtom->index() ;
        removeHydrogen_p( startAtom ) ;


        // Connect to the first atom of the fragment.
        // The 1st elements are atoms.
        Primitive *p=addedPrim->list().at(0) ;

        if( p!=NULL && p->type()==Primitive::AtomType )
          endAtom = static_cast<Atom*>(p) ;

        if( endAtom == NULL )
        {
          mytoolbox::dbgMsg( " BUG in MoleculeManipulation::addFragment3() : A NULL-object not expected." ) ;
          mytoolbox::dbgMsg( " Result : No connection realized between the molecule and the fragment." ) ;
        }
        else
        {
          // Search the bonded non-hydrogen of the fragment, if it exists.
          if( endAtom->isHydrogen() )
          {
            // Get the bonded non-hydrogen and remove this atom.
            Atom *hydrogen=endAtom ;

            if( hydrogen->neighbors().size() > 0 )
            {
              // the first bonded atom to this "H".
              endAtom = m_molecule->atomById( hydrogen->neighbors()[0] ) ;
              removeAtomWithoutHA( hydrogen ) ;
              nbAtomInFrag-- ;
            }
          }
          // else
            // Stay like that. Else, active a message error where there is not.
        }
        
        endAtomIndex = endAtom->index() ;


        if( (startAtom==NULL || endAtom==NULL) //  If something to do
            || (startAtomIndex == endAtomIndex ) // If bug ...
          )
        {
          mytoolbox::dbgMsg( " BUG : The connection between the fragment and the molecule are eguals !!" ) ;
          mytoolbox::dbgMsg( " Result : No connection realized between the molecule and the fragment." ) ;
        }
        else
        {
          // Transform Avogadro molecule to Openbabel molecule.
          OpenBabel::OBMol obmol=m_molecule->OBMol() ;

          // Arrange the added fragment with OpenBabel method.
          // Open Babel indexes atoms from 1, not 0
          OpenBabel::OBBuilder::Connect( obmol, startAtom->index()+1, endAtom->index()+1 ) ;

          // Clear and initiate the Avogadro molecule by the OpenBabel molecule.
          m_molecule->setOBMol( &obmol ) ;
          m_molecule->updateMolecule() ;
        }
      }

      // Get the added atoms. 
      // In fact, the molecule has been cleared  (by the m_molecule->setOBMol()).
      // So we must find new adress, and adjust the barycenter.
      if( endAtom != NULL )
      {
        int i=0 ;

        // Initiate at zero the primitive list of selected atoms.
        if( addedPrim == NULL )
          addedPrim = new PrimitiveList() ;
        else
          addedPrim->clear() ;

        resetBarycenter_p() ;
        foreach( Atom *atom, m_molecule->atoms() )
        {
          updateBarycenter( *(atom->pos()), true ) ;

          if( atom->index()>=endAtom->index() && i++<nbAtomInFrag )
            addedPrim->append( atom ) ;
        }
        
        // Init the rotation axe before turn around.
        Bond *b=m_molecule->bond(startAtomIndex, endAtomIndex) ;
        Atom *a1=m_molecule->atom( startAtomIndex ) ;
        Atom *a2=m_molecule->atom( endAtomIndex ) ;
        if( a1!=NULL || a2!=NULL || b!=NULL )
          setRotationAxe( const_cast<Eigen::Vector3d*>(a1->pos()), 
                          const_cast<Eigen::Vector3d*>(a2->pos()), 
                          b ) ;
      }
    }

    return addedPrim ;
  }


  /**
    * Copy a fragment in the molecule. And it attaches it on an other atom and adjust
    * its hydrogens.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param fragment The molecule which is included in (molecule)
    * @param bondedAtom The atom where the fragment will be bonded
    */
  PrimitiveList* MoleculeManipulation::addFragmentWithHA( Molecule *fragment, Atom *bondedAtom )
  {
    PrimitiveList* addedPrim=NULL ;

    if( m_molecule!=NULL && fragment!=NULL && bondedAtom!=NULL )
    {
      bool emptyMol=(m_molecule->numAtoms() == 0) ;
      Atom *startAtom=NULL, *endAtom=NULL ;
      unsigned long startAtomIndex=0, endAtomIndex=0 ;
      int nbAtomInFrag=fragment->numAtoms() ;

      // Add fragment in the molecule.
      addedPrim = addFragmentWithHA( fragment ) ;

      // Verify if the (bondedAtom) belongs to the molecule. (fr:appartient)
      if( !emptyMol )
      {
        Atom *a=m_molecule->atom( bondedAtom->index() ) ;
        if( a != bondedAtom )
          emptyMol = true ;
      }

      if( !emptyMol && addedPrim!=NULL && addedPrim->size()>0 )
      {
        startAtom = bondedAtom ;

        // Search the bonded non-hydrogen of the molecule, if it exists.
        // First, if (startAtom) is a Hydrogen, change the (startAtom).
        if( startAtom->isHydrogen() )
        {
          Atom *hydrogen=startAtom ;

          // Get the bonded non-hydrogen and remove this atom.
          if( hydrogen->neighbors().size() > 0 )
          {
            startAtom = m_molecule->atomById( hydrogen->neighbors()[0] ) ;
            removeAtomWithoutHA( hydrogen ) ;
          }
        }

        startAtomIndex = startAtom->index() ;
        //removeHydrogen_p( m_molecule, startAtom ) ;


        // Connect to the first atom of the fragment.
        // The 1st elements are atoms.
        Primitive *p=addedPrim->list().at(0) ;

        if( p!=NULL && p->type()==Primitive::AtomType )
          endAtom = static_cast<Atom*>(p) ;

        if( endAtom == NULL )
        {
          mytoolbox::dbgMsg( " BUG in MoleculeManipulation::addFragment3() : A NULL-object not expected." ) ;
          mytoolbox::dbgMsg( " Result : No connection realized between the molecule and the fragment." ) ;
        }
        else
        {
          // Search the bonded non-hydrogen of the fragment, if it exists.
          if( endAtom->isHydrogen() )
          {
            // Get the bonded non-hydrogen and remove this atom.
            Atom *hydrogen=endAtom ;

            if( hydrogen->neighbors().size() > 0 )
            {
              // the first bonded atom to this "H".
              endAtom = m_molecule->atomById( hydrogen->neighbors()[0] ) ;
              removeAtomWithoutHA( hydrogen ) ;
              nbAtomInFrag-- ;
            }
          }

          endAtomIndex = endAtom->index() ;
        }


        if( (startAtom==NULL || endAtom==NULL) //  If something to do
            || (startAtomIndex == endAtomIndex) // If bug ...
          )
        {
          mytoolbox::dbgMsg( " BUG : The connection between the fragment and the molecule are eguals !!" ) ;
          mytoolbox::dbgMsg( " Result : No connection realized between the molecule and the fragment." ) ;
        }
        else
        {
          removeHydrogen_p( endAtom ) ; //

          // Transform Avogadro molecule to Openbabel molecule.
          OpenBabel::OBMol obmol=m_molecule->OBMol() ;

          // Arrange the added fragment with OpenBabel method.
          // Open Babel indexes atoms from 1, not 0
          OpenBabel::OBBuilder::Connect( obmol, startAtomIndex+1, endAtomIndex+1 ) ;

         
          // No Adjust the hydrogen of the startAtom.
          // Because No hydrogen atoms removed on startAtom.
          //addHydrogen_p( &obmol, startAtom ) ; *
 
          // Adjust the hydrogen of the endAtom.
          addHydrogen_p( &obmol, endAtom ) ; //

          // Clear and initiate the Avogadro molecule by the OpenBabel molecule.
          m_molecule->setOBMol( &obmol ) ;
          m_molecule->updateMolecule() ;
        }
      }

      // Get the added atoms. In fact, the molecule has been cleared.
      // So we must find its, and adjust the barycenter.
      if( endAtom != NULL )
      {
        // --> If there was remove hydrogen atoms on startAtom.
        //bool find=false ;
        //QList<Atom*> neighborsStartAtom ;
        //Atom *startA=m_molecule->atomById( startAtomIndex ) ;
        //Atom *n=NULL ;

        // Get the Hydrogen neighbors of the start atom.
        //foreach( unsigned long ai, startA->neighbors() )
        //{
        //  n = m_molecule->atomById( ai ) ;
        //  if( n->isHydrogen() )
        //    neighborsStartAtom.append( n ) ;
        //}
        // <--

        int i=0 ;

        // Initiate the primitive list of selected atoms ...
        if( addedPrim == NULL )
          addedPrim = new PrimitiveList() ;
        else
          addedPrim->clear() ;

        // Calculate the barycenter and construct the selected atom list.
        resetBarycenter_p() ;
        foreach( Atom *atom, m_molecule->atoms() )
        {
          updateBarycenter( *(atom->pos()), true ) ;

          if( atom->index() >= endAtomIndex )
          {
            i ++ ;
            addedPrim->append( atom ) ;
          }
        }
      }
      /*
      if( endAtom != NULL )
      {
        QList<Primitive *> matchedAtoms ;
        int i=0 ;
        resetBarycenter_p() ;

        foreach( Atom *atom, m_molecule->atoms() )
        {
          //m_wmavoThread->setWmAtomPos( *(atom->pos()), true, false ) ;
          updateBarycenter( *(atom->pos()), true ) ;

          if( atom->index()>=endAtom->index() && i++<nbAtomInFrag)
            matchedAtoms.append( atom ) ;
        }

        (*addedPrim) = matchedAtoms ;
      }
      */
      
      // Init the rotation axe before turn around.
      Bond *b=m_molecule->bond( startAtomIndex, endAtomIndex ) ;
      Atom *a1=m_molecule->atom( endAtomIndex ) ;
      Atom *a2=m_molecule->atom( startAtomIndex ) ;
      if( a1!=NULL || a2!=NULL || b!=NULL )
          setRotationAxe( const_cast<Eigen::Vector3d*>(a1->pos()), 
                          const_cast<Eigen::Vector3d*>(a2->pos()), 
                          b ) ;
    }

    return addedPrim ;
  }


  /**
    * Copy a fragment in the molecule. And it attaches it on an other atom and adjust
    * its hydrogens.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of new atoms. CAUTION !!! Do not forget to delete this objet after use it !!!
    * @param fragmentAbsPath Absolute path where the fragment can be read.
    * @param bondedAtom The atom where the fragment will be bonded
    */
  PrimitiveList* MoleculeManipulation::addFragment( const QString &fragmentAbsPath, Atom* bondedAtom )
  {
    if( m_addHydrogens )
      return addFragmentWithHA( fragmentAbsPath, bondedAtom ) ;
    else
      return addFragmentWithoutHA( fragmentAbsPath, bondedAtom ) ;
  }


  /**
    * Copy a fragment in the molecule. And it attaches it on an other atom and adjust
    * its hydrogens.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of new atoms. CAUTION !!! Do not forget to delete this objet after use it !!!
    * @param fragmentAbsPath Absolute path where the fragment can be read.
    * @param bondedAtom The atom where the fragment will be bonded
    */
  PrimitiveList* MoleculeManipulation::addFragmentWithoutHA( const QString &fragmentAbsPath, Atom* bondedAtom )
  {
    PrimitiveList* newElement=NULL ;

    if( m_molecule != NULL )
    {
      Molecule *frag=getFragment(fragmentAbsPath) ;

      if( frag != NULL )
      {
        if( bondedAtom == NULL )
        {
          #if !AVO_DEPRECATED_FCT
          // 2.
          newElement = addFragmentWithoutHA( frag ) ;
          
          #else
          // 1. Deprecated : Avogadro style, impossibility to know the new atoms.
          addFragment1( m_widget->molecule(), frag ) ;
          #endif
        }
        else
        {
          #if !AVO_DEPRECATED_FCT
          // 2.
          newElement = addFragmentWithoutHA( frag, bondedAtom ) ;

          #else
          // 1. For SubstituteAtomByFragment() Avogadro method.
          int selectedAtom = selectedAtoms[0]->index() ; //or id() if there are bugs.        
          // Add fragment & co, and realize selection after action.
          addFragmentWithUndoRedoHydrogen( m_widget->molecule(), &frag, selectedAtom ) ;
          #endif
        }

        delete frag ;
      }
    }

    return newElement ;
  }


  /**
    * Copy a fragment in the molecule. And it attaches it on an other atom and adjust
    * its hydrogens.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of new atoms. CAUTION !!! Do not forget to delete this objet after use it !!!
    * @param fragmentAbsPath Absolute path where the fragment can be read.
    * @param bondedAtom The atom where the fragment will be bonded
    */
  PrimitiveList* MoleculeManipulation::addFragmentWithHA( const QString &fragmentAbsPath, Atom* bondedAtom )
  {
    PrimitiveList* newElement=NULL ;

    if( m_molecule != NULL )
    {
      Molecule *frag=getFragment(fragmentAbsPath) ;

      if( frag != NULL )
      {
        if( bondedAtom == NULL )
        {
          #if !AVO_DEPRECATED_FCT
          // 2.
          newElement = addFragmentWithHA( frag ) ;
          
          #else
          // 1. Deprecated : Avogadro style, impossibility to know the new atoms.
          addFragment1( m_widget->molecule(), frag ) ;
          #endif
        }
        else
        {
          #if !AVO_DEPRECATED_FCT
          // 2.
          newElement = addFragmentWithHA( frag, bondedAtom ) ;

          #else
          // 1. For SubstituteAtomByFragment() Avogadro method.
          int selectedAtom = selectedAtoms[0]->index() ; //or id() if there are bugs.        
          // Add fragment & co, and realize selection after action.
          addFragmentWithUndoRedoHydrogen( m_widget->molecule(), &frag, selectedAtom ) ;
          #endif
        }

        delete frag ;
      }
    }

    return newElement ;
  }

  
  #if AVO_DEPRECATED_FCT
  /**
    * Deprecated : Avogadro style.
    * Add a fragment in a molecule with "undo/redo" and "adjustment of hydrogen" features.
    * Bug in the bond with the selected atom.
    * @param molecule The molecule where the change is realized
    * @param fragment The molecule which is included in (molecule)
    * @param selectedAtom The atom where the atom will be bonded
    */
  void MoleculeManipulation::addFragmentWithUndoRedoHydrogen( Molecule *molecule, Molecule *fragment, int selectedAtom )
  {
    m_widget->undoStack()->push(
          new InsertAtomByFragmentCommand( molecule, fragment,
                                           m_widget, tr("Insert Fragment"),
                                           selectedAtom) ) ;
  }
  #endif


  /**
    * Remove an atom in the molecule without adjustment of hydrogen.
    * @param atom Atom to remove
    */
  void MoleculeManipulation::removeAtom( Atom *atom )
  {
    if( m_addHydrogens ) 
      removeAtomWithHA( atom ) ;
    else
      removeAtomWithoutHA( atom ) ;
  }


  /**
    * Remove an atom in the molecule without adjustment of hydrogen.
    * @param atom Atom to remove
    */
  void MoleculeManipulation::removeAtomWithoutHA( Atom *atom )
  {
    if( m_molecule!=NULL && atom!=NULL )
    {
      updateBarycenter( *(atom->pos()), false ) ;
      m_molecule->removeAtom(atom) ;
    }
  }


  /**
    * Remove an atom in the molecule with its hydrogen and ajust the hydrogens of neighbors.
    * @param atom Atom to remove
    */
  void MoleculeManipulation::removeAtomWithHA( Atom *atom )
  {
    if( m_molecule!=NULL && atom!=NULL )
    {
      if( !atom->isHydrogen()
          || (atom->isHydrogen() && atom->valence()==0.0) )
      {
        QList<Atom*> neighborNoH ;

        // 1st, recup all objects ... Else, after remove and add the id (or index?)
        // are unused.
        Atom *n=NULL ;
        foreach( unsigned int ai, atom->neighbors() )
        {
          n = m_molecule->atomById( ai ) ;
          if( !n->isHydrogen() )
            neighborNoH.append( n ) ;
        }

        // Remove current atom and its Hydrogens.
        removeHydrogen_p( atom ) ;
        removeAtomWithoutHA( atom ) ;

        // Adjust the others atoms.
        OpenBabel::OBMol obmol=m_molecule->OBMol() ;
        foreach( Atom* a, neighborNoH )
          addHydrogen_p( &obmol, a ) ;
        adjustPartialCharge_p( &obmol ) ;
      }
      else if( atom->isHydrogen() && atom->valence()==1.0 )
      { // Maybe an hydrogen atom bonded with an other hydrogen atom.

        Atom *n=m_molecule->atomById( atom->neighbors()[0] ) ;

        if( n!=NULL && n->isHydrogen() )
          removeAtomWithoutHA( atom ) ;
      }
    }
  }


  #if AVO_DEPRECATED_FCT
  /**
    * Deprecated : Avogadro style.
    * Remove an atom in the molecule with undo/redo and adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atom Atom to remove
    */
  void MoleculeManipulation::removeAtomWithUndoRedoHydrogen( Molecule *molecule, Atom *atom )
  {
    if( molecule!=NULL && atom!=NULL )
    {
      //cout << m_widget->molecule() << " " << atom->index() << " " << m_addHydrogens << endl ;

      // don't delete H-? atom when adjust hydrogens is on
      if( !(m_addHydrogens && atom->isHydrogen() && int(atom->valence())) )
      {
        //cout << "  Create DeleteAtomDrawCommand" << endl ;
        DeleteAtomDrawCommand *undo=new DeleteAtomDrawCommand( m_widget->molecule(),
                                                               atom->index(),
                                                               m_addHydrogens);
        //cout << " undoStack()->push( DeleteAtomDrawCommand )" << endl ;
        m_widget->undoStack()->push( undo ) ;
        //cout << " fin undoStack()->push( DeleteAtomDrawCommand )" << endl ;
      }
    }
  }
  #endif

  /**
    * Delete atoms in the molecule without adjustment of hydrogen.
    * @param atoms Atoms to remove
    */
  void MoleculeManipulation::removeAtoms( QList<Atom*> *atoms )
  {
    if( m_addHydrogens )
      removeAtomsWithHA( atoms ) ;
    else
      removeAtomsWithoutHA( atoms ) ;
  }


  /**
  * Delete atoms in the molecule without adjustment of hydrogen.
  * @param atoms Atoms to remove
  */
  void MoleculeManipulation::removeAtoms( QList<Primitive*> *atoms )
  {
    if( m_addHydrogens )
      removeAtomsWithHA( atoms ) ;
    else
      removeAtomsWithoutHA( atoms ) ;
  }


  /**
    * Delete atoms in the molecule without adjustment of hydrogen.
    * @param atoms Atoms to remove
    */
  void MoleculeManipulation::removeAtoms( PrimitiveList *atoms )
  {
    if( m_addHydrogens )
      removeAtomsWithHA( atoms ) ;
    else
      removeAtomsWithoutHA( atoms ) ;
  }


  /**
    * Delete atoms in the molecule without adjustment of hydrogen.
    * @param atoms Atoms to remove
    */
  void MoleculeManipulation::removeAtomsWithoutHA( QList<Atom*> *atoms )
  {
    if( m_molecule!=NULL && atoms!=NULL && atoms->size()>0 )
    {
      foreach( Atom* a, *atoms )
        removeAtomWithoutHA( a ) ;
    }
  }


  /**
    * Delete atoms in the molecule without adjustment of hydrogen.
    * @param atoms Atoms to remove
    */
  void MoleculeManipulation::removeAtomsWithoutHA( QList<Primitive*> *atoms )
  {
    if( m_molecule!=NULL && atoms!=NULL && atoms->size()>0 )
    {
      foreach( Primitive* p, *atoms )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
          removeAtomWithoutHA( static_cast<Atom*>(p) ) ;
      }
    }
  }


  /**
    * Delete atoms in the molecule without adjustment of hydrogen.
    * @param atoms Atoms to remove
    */
  void MoleculeManipulation::removeAtomsWithoutHA( PrimitiveList *atoms )
  {
    if( m_molecule!=NULL && atoms!=NULL )
    {
      QList<Primitive*> pl=atoms->subList(Primitive::AtomType) ;
      if( atoms->size() > 0 )
        removeAtomsWithoutHA( &pl ) ;
    }
  }


  /**
    * Delete atoms in the molecule with adjustment of hydrogen.
    * @param atoms Atoms to remove
    */
  void MoleculeManipulation::removeAtomsWithHA( QList<Atom*> *atoms )
  {
    if( m_molecule!=NULL && atoms!=NULL && atoms->size()>0 )
    {
      int i=0, j=atoms->size() ;
      QList<Atom*> neighborsToAdjust ;
      Atom *n=NULL ;

      // Firstly, remove hydrogen which can be remove by removeHydrogen_p() in the 2nd part.
      // If a selected H-atom is not erased here, the removeH_p() method can erase it after.
      // So when the foreach reaches this selected atom, there is a pointer to nothing (to avoid crash)
      while( i < j )
      {
        if( atoms->at(i)->isHydrogen() && int(atoms->at(i)->valence())>0 )
        {
          // Adjust atoms to remove.
          atoms->removeAt(i) ;
          j-- ;
        }
        else
        {
          // Adjust atoms to remove
          i++ ;

          // Recup the neighbors to adjust.
          foreach( unsigned long ai, atoms->at(i)->neighbors() )
          {
            n = m_molecule->atomById( ai ) ;
            if( !n->isHydrogen() )
              neighborsToAdjust.append( n ) ;
          }
        }
      }

      // Secondly, remove the neighbors already in the atoms to remove.
      i = 0 ;
      j = neighborsToAdjust.size() ;
      if( j > 0 )
      {
        foreach( Atom *a, *atoms )
        {
          while( i < j )
          {
            if( neighborsToAdjust.at(i) == a )
            {
              neighborsToAdjust.removeAt(i) ;
              j-- ;
              // Do not stop the traitment here, because it can have an other same atom.
              // Neighbors of a neighbor can be a neighbor of a neighbor.
              // What else !?
            }
            else
              i++ ;
          }
        }
      }

      // Remove all atoms.
      foreach( Atom* a, *atoms )
      {
        removeHydrogen_p( a ) ;
        removeAtomWithoutHA( a ) ;
      }

      // Adjust Hydrogen for the old neighbors.
      OpenBabel::OBMol obmol=m_molecule->OBMol() ;
      foreach( Atom *a, neighborsToAdjust )
        addHydrogen_p( &obmol, a ) ;
      adjustPartialCharge_p( &obmol ) ;
    }
  }


  /**
    * Delete atoms in the molecule with adjustment of hydrogen.
    * @param atoms Atoms to remove
    */
  void MoleculeManipulation::removeAtomsWithHA( QList<Primitive*> *atoms )
  {
    if( m_molecule!=NULL && atoms!=NULL && atoms->size()>0 )
    {
      int i=0, j=atoms->size() ;
      Atom *a=NULL ;
      QList<Atom*> neighborsToAdjust ;
      Atom *n=NULL ;

      // Firstly, remove hydrogen which can be remove by removeHydrogen_p() in the 2nd part.
      while( i < j )
      {
        if( atoms->at(i)->type() == Primitive::AtomType )
        {
          a = static_cast<Atom*>(atoms->at(i)) ;

          if( a->isHydrogen() && int(a->valence())>0 )
          {
            atoms->removeAt(i) ;
            j-- ;
          }
          else
          {
            // Adjust atoms to remove
            i++ ;

            // Recup the neighbors to adjust.
            foreach( unsigned long ai, a->neighbors() )
            {
              n = m_molecule->atomById( ai ) ;
              if( !n->isHydrogen() )
                neighborsToAdjust.append( n ) ;
            }
          }
        }
      }

      // Secondly, remove the neighbors already in the atoms to remove.
      i = 0 ;
      j = neighborsToAdjust.size() ;
      if( j > 0 )
      {
        foreach( Primitive *p, *atoms )
        {
          if( p->type() == Primitive::AtomType )
          {
            a = static_cast<Atom*>(p) ;

            while( i < j )
            {
              if( neighborsToAdjust.at(i) == a )
              {
                neighborsToAdjust.removeAt(i) ;
                j-- ;
                // Do not stop the traitment here, because it can have an other same atom.
                // Neighbors of a neighbor can be a neighbor of a neighbor.
                // What else !?
              }
              else
                i++ ;
            }
          }
        }
      }

      // Remove all atoms.
      foreach( Primitive* p, *atoms )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
        {
          a = static_cast<Atom*>(p) ;
          removeHydrogen_p( a ) ;
          removeAtomWithoutHA( a ) ;
        }
      }

      // Adjust Hydrogen for the old neighbors.
      OpenBabel::OBMol obmol=m_molecule->OBMol() ;
      foreach( Atom *a, neighborsToAdjust )
        addHydrogen_p( &obmol, a ) ;
      adjustPartialCharge_p( &obmol ) ;
    }
  }



  /**
    * Delete atoms in the molecule with adjustment of hydrogen.
    * @param atoms Atoms to remove
    */
  void MoleculeManipulation::removeAtomsWithHA( PrimitiveList *atoms )
  {
    if( m_molecule!=NULL && atoms!=NULL )
    {
      QList<Primitive*> pl=atoms->subList(Primitive::AtomType) ;

      if( atoms->size()>0 )
        removeAtomsWithHA( &pl ) ;
    }
  }


  /**
    * Delete a bond in the molecule without adjustment of hydrogen.
    * @param bond Bond to remove
    */
  void MoleculeManipulation::removeBond( Bond *bond )
  {
    if( m_addHydrogens )
      removeBondWithHA( bond ) ;
    else
      removeBondWithoutHA( bond ) ;
  }


  /**
    * Delete a bond in the molecule without adjustment of hydrogen.
    * @param bond Bond to remove
    */
  void MoleculeManipulation::removeBondWithoutHA( Bond *bond )
  {
    if( m_molecule!=NULL && bond!=NULL )
    {
      if( bond->order() > 1 )
      {
        int tmp=bond->order() ;
        bond->setOrder( (short)(tmp-1) ) ;
      }
      else
        m_molecule->removeBond( bond ) ;
    }
  }



  /**
    * Delete a bond in the molecule with adjustment of hydrogen.
    * @param bond Bond to remove
    */
  void MoleculeManipulation::removeBondWithHA( Bond *bond )
  {
    if( m_molecule!=NULL && bond!=NULL )
    {
      Atom *a1=bond->beginAtom() ;
      Atom *a2=bond->endAtom() ;

      if( a1!=NULL && a2!=NULL )
      {
        if( !(a1->isHydrogen() || a2->isHydrogen()) )
        {
          removeBondWithoutHA( bond ) ;

          removeHydrogen_p( a1 ) ;
          removeHydrogen_p( a2 ) ;
          OpenBabel::OBMol obmol=m_molecule->OBMol() ;
          addHydrogen_p( &obmol, a1 ) ;
          addHydrogen_p( &obmol, a2 ) ;

          adjustPartialCharge_p( &obmol ) ;
        }
      }
      else
      {
        const QString title=tr("Error ... No fatal.") ;
        const QString msg=tr("A bug appeared. The current action must be canceled (remove a bond with hydrogen adjustment).") ;
        QMessageBox( QMessageBox::Critical, title, msg ) ;
      }
    }
  }

  #if AVO_DEPRECATED_FCT
  /**
    * Deprecated : Avogadro style.
    * Delete a bond in the molecule with undo/redo feature. There is
    * an adjustment of Hydrogen.
    * @param molecule The molecule where the change is realized
    * @param bond Bond to remove
    */
  void MoleculeManipulation::deleteBondWithUndoRedo( Molecule *molecule, Bond *bond )
  {
    if( bond != NULL )
    {
      if( molecule->lock()->tryLockForWrite() )
      {
        if( bond->order() > 1 )
        {
          int tmp=bond->order() ;
          bond->setOrder( (short)(tmp-1) ) ;

         ChangeBondOrderDrawCommand *undo = new ChangeBondOrderDrawCommand( m_widget->molecule(), bond, tmp, m_addHydrogens ) ;
          m_widget->undoStack()->push( undo ) ;
        }
        else
        {
          if( !(
                m_addHydrogens
                && ( molecule->atomById(bond->beginAtomId())->isHydrogen()
                     || molecule->atomById(bond->endAtomId())->isHydrogen()
                   )
                ))
          {
            DeleteBondDrawCommand *undo=new DeleteBondDrawCommand( m_widget->molecule(),
                                                                   bond->index(),
                                                                   m_addHydrogens) ;
            m_widget->undoStack()->push( undo ) ;
          }
        }
      }

      molecule->lock()->unlock() ;
    }
  }
  #endif


  #if AVO_DEPRECATED_FCT
  /**
    * Deprecated : Avogadro style.
    * Delete elements in the molecule with undo/redo features. There is
    * no adjustment of hydrogen !
    * @param molecule The molecule where the change is realized
    */
  void MoleculeManipulation::deleteSelectedElementUndoRedo( Molecule *molecule )
  {

    // Clear the molecule or a set of atoms
    // has the inteligence to figure out based on the number of selected items
    ClearCommand *command=new ClearCommand( molecule,
                                            m_widget->selectedPrimitives() ) ;
    m_widget->undoStack()->push( command ) ;
  }
  #endif


  /**
    * Delete all elements in the molecule.
    */
  void MoleculeManipulation::deleteAllElement()
  {
    if( m_molecule != NULL )
    {
      if( m_molecule->lock()->tryLockForWrite() )
      {
        m_molecule->clear() ;
        m_molecule->lock()->unlock() ;

        resetBarycenter_p() ;
      }
    }
  }


  /**
    * Fill out the molecule with Hydrogens.
    */
  void MoleculeManipulation::addHydrogens()
  {
    if( m_molecule != NULL )
    {
      OpenBabel::OBMol obmol=m_molecule->OBMol() ;
      int nba=m_molecule->numAtoms() ;
      int nb=nba ;
      
      foreach( Atom *atom, m_molecule->atoms() )
      {
        if( --nb < 0 )
          break;
        else
          addHydrogen_p( &obmol, atom ) ;
      }

      adjustPartialCharge_p( &obmol ) ;

      /* Just longer to execut ...
      OpenBabel::OBMol obmol=m_molecule->OBMol() ;
      int nba=m_molecule->numAtoms() ;
      int nboba=0 ;
      
      OpenBabel::OBAtom *obh=NULL, *oba=NULL, *oba2=NULL ;
      OpenBabel::OBBond *obb=NULL ;
      OpenBabel::OBBondIterator obbi ;
      Atom *h=NULL, *a=NULL, *a2=NULL ;

      obmol.AddHydrogens() ; // Take many time to execut !
      nboba = obmol.NumAtoms() ;

      // All new atoms in the OBMol must be the additional hydrogens.
      for( int i=nba+1 ; i<=nboba ; ++i )
      {
        obh = obmol.GetAtom(i) ;

        if( obh!=NULL && obh->IsHydrogen() )
        {
          h = addAtom( obh ) ;
          
          OpenBabel::OBBondIterator iter ;
          oba = obh->BeginNbrAtom(iter) ;
          a = m_molecule->atom(oba->GetIdx()-1) ;

          if( h!=NULL && oba!=NULL && a!=NULL )
            addBond( a, h, 1 ) ;
          else
            puts( "Bug in MoleculeManipulation::addHydrogens() : a NULL-object not expected" ) ;
        }
      }
      */
     }
  }

  /**
    * Remove all Hydrogen of the molecule.
    */
  void MoleculeManipulation::removeHydrogens()
  {
    if( m_molecule != NULL )
    {
      foreach( Atom *atom, m_molecule->atoms() )
      {
        if( atom->isHydrogen() )
          removeAtomWithoutHA( atom ) ;
      }
    }
  }

  /**
    * Avogadro code :
    * In theorical, add feature in the "add Hydrogen" feature to correct OpenBabel.
    * But, OpenBabel has maybe corrected this thing already ...
    * @param molecule The OpenBabel molecule where the change is realized
    * @param atom The OpenBabel atom to modify
    */
  OpenBabel::OBAtom* MoleculeManipulation::setImplicitValence_p( OpenBabel::OBMol *obMol, OpenBabel::OBAtom* obAtom )
  {
    // Set implicit valence for unusual elements not managed by OpenBabel
    // PR#2803076
    switch( obAtom->GetAtomicNum() )
    {
    case 3:
    case 11:
    case 19:
    case 37:
    case 55:
    case 85:
    case 87:
      obAtom->SetImplicitValence(1);
      obAtom->SetHyb(1);
      obMol->SetImplicitValencePerceived();
      break;
    case 4:
    case 12:
    case 20:
    case 38:
    case 56:
    case 88:
      obAtom->SetImplicitValence(2);
      obAtom->SetHyb(2);
      obMol->SetImplicitValencePerceived();
      break;
    case 84: // Po
      obAtom->SetImplicitValence(2);
      obAtom->SetHyb(3);
      obMol->SetImplicitValencePerceived();
      break;
    default: // do nothing
      break;
    }

    return obAtom ;
  }


  /**
    * Adjust partial charge between an OpenBabel molecule and an Avogadro molecule.
    * @param obmol The OpenBabel molecule which contains the new values
    */
  void MoleculeManipulation::adjustPartialCharge_p( OpenBabel::OBMol *obmol )
  {
    if( m_molecule!=NULL && obmol!=NULL )
    {
      unsigned int i=0, nba=m_molecule->numAtoms() ;

      for( i=1 ; i<=nba ; ++i )
        m_molecule->atom(i-1)->setPartialCharge( obmol->GetAtom(i)->GetPartialCharge() ) ;
    }
  }


  /**
    * Fill out (atom) of hydrogen atoms.
    * (m_molecule) and (obmol) must be identical (same atoms, same number ...).
    * Do not use it before know exactly how it works.
    * @return TRUE if hydrogen atom are added ; else FALSE.
    * @param obmol The OpenBabel molecule identical at (m_molecule)
    * @param atom Atom where the hydrogen must be adjust
    */
  bool MoleculeManipulation::addHydrogen_p( OpenBabel::OBMol *obmol, Atom* atom )
  {
    bool hasAddedH=false ;

    if( m_molecule!=NULL && obmol!=NULL && atom!=NULL )
    {
      unsigned int nba=0, i=0 ;
      Atom *h=NULL ;
      OpenBabel::OBAtom *oba=NULL ;

      if( !atom->isHydrogen() )
      {
        oba = obmol->GetAtom( atom->index()+1 ) ;

        if( oba != NULL )
        {
          setImplicitValence_p( obmol, oba ) ;
          hasAddedH = obmol->AddHydrogens( oba ) ;

          // All new atoms in the OBMol must be the additional hydrogens.
          if( hasAddedH )
          {
            nba = m_molecule->numAtoms() ;
            for( i=nba+1 ; i<=obmol->NumAtoms() ; ++i )
            {
              if( obmol->GetAtom(i)->IsHydrogen() )
              {
                oba = obmol->GetAtom(i) ;
                h = addAtomWithoutHA( oba ) ;

                if( h != NULL )
                  addBondWithoutHA( atom, h, 1 ) ;
              }
            }
          }
        }
        #if __WMDEBUG_MOLMANIP
        else
          mytoolbox::dbgMsg( "Bug in MoleculeManipulation::addHydrogen_p() : a NULL-object not expected" ) ;
        #endif
      }
      #if __WMDEBUG_MOLMANIP
      else
        mytoolbox::dbgMsg( "Rq in MoleculeManipulation::addHydrogen_p() : atom is a Hydrogen, do not add Hydrogen" ) ;
      #endif
    }

    return hasAddedH ;
  }


  /**
    * Remove hydrogen(s) of an atom.
    * Do not use it before know exactly how it works.
    * @return TRUE if hydrogen atom are removed ; else FALSE.
    * @param atom Atom where the hydrogen must be removed
    * @param atomNoRemove Atom which does not remove
    */
  bool MoleculeManipulation::removeHydrogen_p( Atom *atom, Atom *atomNoRemove )
  {
    bool hasRemoveH=false ;

    if( m_molecule!=NULL && atom!=NULL )
    {
      if( !atom->isHydrogen() )
      {
        Atom *nbrAtom=NULL ;

        foreach( unsigned long i, atom->neighbors() )
        {
          nbrAtom = m_molecule->atomById(i) ;

          if( nbrAtom!=NULL )
          {
            if( nbrAtom != atomNoRemove )
            {
              if( nbrAtom->isHydrogen() )
              {
                removeAtomWithoutHA( nbrAtom ) ;
                if( !hasRemoveH ) hasRemoveH=true ;
              }
            }
            //else
            //  mytoolbox::dbgMsg( "Rq in MoleculeManipulation::removeHydrogen_p() : do not erase the (atomNoRemove)." ;
          }
          #if __WMDEBUG_MOLMANIP
          else
            mytoolbox::dbgMsg( "Bug in MoleculeManipulation::removeHydrogen_p() : a NULL-object not expected." ) ;
          #endif
        }
      }
      #if __WMDEBUG_MOLMANIP
      else
        mytoolbox::dbgMsg( "Rq in MoleculeManipulation::removeHydrogen_p() : atom is a Hydrogen, do not remove its neighbor." ) ;
      #endif
    }

    return hasRemoveH ;
  }


  /**
    * Get the barycenter of the molecule.
    * @return A Eigen::Vector3d type for the barycenter of the molecule.
    */
  const Eigen::Vector3d& MoleculeManipulation::getBarycenterMolecule()
  {
    return m_barycenterRefMolecule ;
  }


  /**
    * Reset the current barycenter value.
    * Do not use it before know exactly how it works.
    */
  void MoleculeManipulation::resetBarycenter_p()
  {
    m_barycenterRefMolecule = m_vect3d0 ;
    m_sumOfWeights = 0 ;
    m_atomsBarycenter = m_vect3d0 ;
  }


  /**
    * Update the current barycenter value.
    * @param atomPos The position to add/del in the barycenter
    * @param addOrDel Add or del the position according the need
    * @param testToRecalculateBarycenter Force not to test if a recalculation must be realized.
    */
  void MoleculeManipulation::updateBarycenter( const Eigen::Vector3d& atomPos, bool addOrDel, bool testIfNeedToRecalculateBarycenter )
  {
    bool recalculateB=false ;
    unsigned int numAtoms=m_molecule->numAtoms() ;

    if( testIfNeedToRecalculateBarycenter
        && ( (addOrDel &&  numAtoms!=(unsigned int)(m_sumOfWeights+1)) // UdateBarycenter after the adding.
             || (!addOrDel && numAtoms!=(unsigned int)(m_sumOfWeights)) // UdateBarycenter before the removing.
            ))
    {
      #if __WMDEBUG_MOLMANIP
      mytoolbox::dbgMsg( "recalculate barycenter" ) ;
      #endif
      recalculateBarycenter() ;
      recalculateB = true ;
    }

    if( (addOrDel && !recalculateB) || !addOrDel )
    { // The new atom has been added just before if a calculation has been realized.

      if( addOrDel )
      {
        m_sumOfWeights ++ ;
        m_atomsBarycenter += atomPos ;
      }
      else
      {
        if( m_sumOfWeights > 0 )
        {
          m_sumOfWeights -- ;
          m_atomsBarycenter -= atomPos ;
        }
      }

      //cout << "3 m_sumOfWeights:" << m_sumOfWeights << endl ;
      //cout << "3 m_atomsBarycenter:" << m_atomsBarycenter[0] << "," << m_atomsBarycenter[1] << "," << m_atomsBarycenter[2] << endl ;
      //cout << "3 m_barycenterRefMolecule:" << m_barycenterRefMolecule[0] << "," << m_barycenterRefMolecule[1] << "," << m_barycenterRefMolecule[2] << "," << endl ;
      if( m_sumOfWeights > 0 )
        m_barycenterRefMolecule = m_atomsBarycenter / m_sumOfWeights ;
      else
      {
        m_sumOfWeights = 0 ;
        m_atomsBarycenter = m_vect3d0 ;
        m_barycenterRefMolecule = m_vect3d0 ;
      }
    }
    //cout << "4 m_barycenterRefMolecule:" << m_barycenterRefMolecule[0] << "," << m_barycenterRefMolecule[1] << "," << m_barycenterRefMolecule[2] << "," << endl ;
  }

  /**
    * Reset and calculate the barycenter.
    * molecule The molucule where the barycenter must be calculate
    */
  void MoleculeManipulation::recalculateBarycenter()
  {
    resetBarycenter_p() ;

    foreach( Atom* a, m_molecule->atoms() )
    {
      m_sumOfWeights ++ ;
      m_atomsBarycenter += *(a->pos()) ;
    }

    if( m_sumOfWeights > 0 )
      m_barycenterRefMolecule = m_atomsBarycenter / m_sumOfWeights ;
  }


  /**
    * Calculate the nearest atom and return it.
    * @param posAtom The position of the reference atom for the distance with others atoms
    * @param atomNotUse The atom exception which does not take in the calcul.
    */
  Atom* MoleculeManipulation::calculateNearestAtom( const Eigen::Vector3d *posAtom, const Atom *atomNotUse )
  {
    QList<Atom*> atomList=m_molecule->atoms() ;
    Atom *a=NULL ;
    double act=std::numeric_limits<double>::max() ; // Angstrom distance.
    Eigen::Vector3d dist ;
    const Eigen::Vector3d *tmp ;

    // Search the nearest distance between current atom and the others.
    for( int i=0 ; i<atomList.size() ; i++ )
    {
      if( atomList.at(i) != atomNotUse )
      {
        tmp = atomList.at(i)->pos() ;

        dist = (*posAtom) - (*tmp) ;
        if( dist.norm() < act  )
        {
          act = dist.norm() ;
          a = atomList.at(i) ;
        }

        // To limit other calculation.
        if( act < 0.1 )
          break ;
      }
      //else
        //mytoolbox::dbgMsg( " BOUM : identical atom" ) ;
    }

    return a ;
  }


  /**
    * Realize an optimization of the geometry with default parameters.
    * 0, // force field id
    * 500, // nSteps, a reasonable starting point (not too much time)
    * 0, // algorithm, steepest descent
    * 7, // convergence
    * 0 // task
    */
  void MoleculeManipulation::optimizeGeometry()
  {

    // Voir e-mail ...

    /*
    ConstraintsModel constraints ;
    OpenBabel::OBForceField *forceField=NULL ;
    OpenBabel::OBMol obmol=molecule->OBMol() ;
    ostringstream buff;
    
    forceField = OBForceField::FindForceField( "MMFF94" );

    if( !forceField->Setup( obmol, constraints.constraints() ) )
    {
      forceField = OBForceField::FindForceField("UFF") ;
      forceField->SetLogFile( &buff ) ;
      forceField->SetLogLevel( OBFF_LOGLVL_LOW ) ;
    }

    QUndoCommand undo = new ForceFieldCommand( molecule, forceField, constraints,
                                  0, // force field id
                                  500, // nSteps, a reasonable starting point (not too much time)
                                  0, // algorithm, steepest descent
                                  7, // convergence
                                  0 // task
                                  );
    undo->setText( QObject::tr( "Geometric Optimization" ) );
    */
  }


  /**
    * Get the fragment store in a files (see OpenBabel::OBFormat)
    * @param fragmentAbsPath Absolute path where the fragment can be read.
    * @return The Avogadro::Molecule* of the fragment ; else NULL if there is a problem (see getMsgErrorGetFragment()).
    */
  Molecule* MoleculeManipulation::getFragment( const QString &fragmentAbsPath )
  {
    Molecule *frag=NULL ;
    OpenBabel::OBConversion conv ;
    OpenBabel::OBFormat *inFormat=conv.FormatFromExt( fragmentAbsPath.toAscii() ) ;
    std::ostringstream oss ;

    m_errorMsgGetFragment.clear() ;

    if( inFormat==NULL || !conv.SetInFormat(inFormat) )
    {
      oss << "Error during getFragment() method:" 
          << "Cannot read file format of file" << fragmentAbsPath.toStdString() ;
      m_errorMsgGetFragment.append( tr(oss.str().c_str()) ) ;
      return NULL ;
    }

    std::ifstream ifs ;
    ifs.open( QFile::encodeName(fragmentAbsPath) ) ;

    if( !ifs )
    {
      oss << "Error during getFragment() method:" 
          << "Cannot read file" << fragmentAbsPath.toStdString() ;
      m_errorMsgGetFragment.append( tr(oss.str().c_str()) ) ;
      return NULL ;
    }

    OpenBabel::OBMol obfragment ;
    conv.Read( &obfragment, &ifs ) ;

    frag = new Molecule() ;
    frag->setOBMol( &obfragment ) ;
    frag->center() ;
    ifs.close() ;

    #if __WMDEBUG_MOLMANIP
    QString str=tr("nbAtom:") + QString::number(frag->numAtoms()) + tr(", nbBond:") + QString::number(frag->numBonds()) ;
    mytoolbox::dbgMsg(str) ;

    foreach( Atom* a, frag->atoms() )
    {
      str = QString::number(a->atomicNumber()) + tr("(valence:") + QString::number(a->valence()) + tr(")")
              + tr(": nbBond:") + QString::number(a->bonds().size()) 
              + tr(", nbNeighbors:") + QString::number(a->neighbors().size()) ;
      mytoolbox::dbgMsg(str) ;

      foreach( unsigned int bId, a->bonds() )
      {
        Bond *b=m_molecule->bondById(bId) ;

        if( b == NULL )
          str = QString::number(bId) + tr(":order:00 : Bug ...") ;
        else
          str = QString::number(bId) + tr(":order:") + QString::number(b->order()) ;
        mytoolbox::dbgMsg(str) ;
      }
    }
    #endif

    return frag ;
  }


  /** 
    * Get the stored current atomic number.
    * @return The current atomic number.
    */
  int MoleculeManipulation::getAtomicNumberCurrent()
  {
    return m_atomicNumberCurrent ;
  }


  /** 
    * Set the current atomic number.
    * @param atomicNb The future current atomic number.
    */
  void MoleculeManipulation::setAtomicNumberCurrent( int atomicNb )
  {
    if( atomicNb > 0 )
      m_atomicNumberCurrent = atomicNb ;
  }

  /** 
    * Get if the hydrogen atoms are adjusted during an added actions.
    * @param TRUE if H-atoms are adjusted ; FALSE else.
    */
  bool MoleculeManipulation::hasAddedHydrogen()
  {
    return m_addHydrogens ;
  }

  /** 
    * Set if H-atoms are adjusted during an added actions.
    * @param withH The state of the H-atom adjustment.
    */
  void MoleculeManipulation::setHasAddedHydrogen( bool withAdjustment )
  {
   if( m_addHydrogens != withAdjustment )
     m_addHydrogens = withAdjustment ;
  }

  /** 
    * Reverse if H-atoms are adjusted during an added actions.
    */
  void MoleculeManipulation::invertHasAddHydrogen()
  {
    m_addHydrogens = !m_addHydrogens ;
    emit invertedHasAddHydrogen( m_addHydrogens ) ;
  }
 
  
  /**
   * Set m_rotationAxe.
   * @param p1Ref 1st point to calcul a vector AND the "centre of the rotation".
   * @param p2 2nd point to calcul a vector.
   * @param axeRot If !NULL, the bond will be used to have a visual information.
   */
  void MoleculeManipulation::setRotationAxe( Eigen::Vector3d *p1Ref, Eigen::Vector3d *p2, Bond* axeRot )
  {
      m_rotationAxeBond = axeRot ;
      
      if( p1Ref==NULL || p2==NULL )
      {
          m_rotationAxe = m_vect3d0 ;
          m_rotationAxePoint = m_vect3d0 ;
      }
      else
      {
          Eigen::Vector3d axeRot=(*p1Ref)-(*p2) ;
          double sum=axeRot[0]*axeRot[0] + axeRot[1]*axeRot[1] + axeRot[2]*axeRot[2] ;
          axeRot /= sqrt( sum ) ;
          m_rotationAxe = axeRot ;
          m_rotationAxePoint = (*p1Ref) ;
      }
  }
  
  /**
   * Set m_rotationAxe. But not defined what is the atom used to define the referentiel point.
   * @param axeRot Bond used to define a rotation axe.
   */
  void MoleculeManipulation::setRotationAxe( Bond* axeRot )
  {
      if( axeRot == NULL )
      {
          resetRotationAxe() ;
          return ;
      }

      Atom *a1=axeRot->beginAtom() ;
      Atom *a2=axeRot->endAtom() ;

      if( a1==NULL || a2==NULL )
      {
          resetRotationAxe() ;
          return ;
      }

      Eigen::Vector3d *p1=const_cast<Eigen::Vector3d*>(a1->pos()) ;
      Eigen::Vector3d *p2=const_cast<Eigen::Vector3d*>(a2->pos()) ;

      if( p1==NULL || p2==NULL )   
      {
          resetRotationAxe() ;
          return ;
      }
      
      setRotationAxe( p1, p2, axeRot ) ;
  }
  
  
  /**
   * Set m_rotationAxe to zero.
   */
  void MoleculeManipulation::resetRotationAxe()
  {
      m_rotationAxe = m_vect3d0 ;
      m_rotationAxeBond = NULL ;
      m_rotationAxePoint = m_vect3d0 ;
  }
  
  /**
   * Get the rotation axe vector.
   * @return Eigen::Vector object not nul if rotation axe exists.
   */
  Eigen::Vector3d MoleculeManipulation::getRotationAxe()
  {
      return m_rotationAxe ;
  }


  /**
   * Get the bond used for the rotation axe.
   * @return Bond* if existing bond ; else NULL.
   */
  Bond* MoleculeManipulation::getRotationAxeBond()
  {
      return m_rotationAxeBond ;
  }
  
  /**
   * Get one point on the rotation axe.
   * @return Eigen::Vector object not nul if rotation axe exists.
   */
  Eigen::Vector3d MoleculeManipulation::getRotationAxePoint()
  {
      return m_rotationAxePoint ;
  }
  
      
  /**
   * Select all atoms which bond with the atom.
   * CAUTION: Do not forget to delete after use!!!
   * 
   * @param primList The selected atoms used to get all bonded atom.
   * @return A list of bonded atoms.
   */
  PrimitiveList* MoleculeManipulation::getAllBondedAtom( const PrimitiveList &primList )
  {
    PrimitiveList *primListOut=NULL ;
    
    // Is it already visited ?
    if( m_molecule!=NULL && primList.size()>0 ) 
    { // No visited.
      
      Atom *a=NULL ;
      QList<Atom*> atomList ;      
      primListOut = new PrimitiveList() ;
      int sizeFinal=0 ;

      foreach( Primitive *p, primList )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
        {
          a = static_cast<Atom*>(p) ;
          
          if( !(atomList.contains(a)) )
          {
            m_spanTreeNodeFirst = new SpanningTreeNode( NULL, a ) ;
            getAllBondedAtom_p( m_spanTreeNodeFirst, atomList ) ;
          }
        }
      }
      
      sizeFinal = atomList.size() ;
      if( sizeFinal > 0 )
      {
        for( int i=0 ; i<sizeFinal ; i++ )
          (*primListOut).append(atomList.at(i)) ;
      }
    }
    // else if visited
      // Nothing to do.
    
    
    return primListOut ;
  }
  
  /**
   * Select all atoms which bond with the atom (recurive method).
   * @param atomFirst The atom used to get all bonded atom.
   */
  void MoleculeManipulation::getAllBondedAtom_p( SpanningTreeNode *aNode, QList<Atom*> &atomList )
  {
    if( aNode!=NULL && aNode->atom!=NULL )
    {
      // Get the list of the visited atom.
      Atom *atom=aNode->atom ; // Get the current atom.
      
      if( !atomList.contains(atom) )
      { // Not visited.
                
        // The current atom is visited.
        atomList.append( atom ) ;
        //aNode->isVisited = true ;

        // Check the sons.
        Atom *atomTmp=NULL ;
        const QList<unsigned long> &neighbors=atom->neighbors() ;

        foreach( unsigned long nai, neighbors )
        {
          atomTmp = m_molecule->atomById( nai ) ;

          if( atomTmp == NULL )
          {
            #if __WMDEBUG_MOLMANIP
            mytoolbox::dbgMsg( "Bug in MoleculeManipulation::selectAllBondedAtom(Atom*) : NULL-object non expected." ) ;
            #endif
            continue ;
          }

          if( !atomList.contains(atomTmp) )
          { // Not visited.

            SpanningTreeNode *sonNode=new SpanningTreeNode(aNode, atomTmp) ;          
            getAllBondedAtom_p( sonNode, atomList ) ;
          }
        } 
      }
    }
  }

}
