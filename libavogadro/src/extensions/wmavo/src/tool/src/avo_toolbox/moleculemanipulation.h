
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

#pragma once
#ifndef __MOLECULEMANIPULATION_H__
#define __MOLECULEMANIPULATION_H__

#include "warning_disable_begin.h"
#include "variousfeatures.h"
#include "wmavo_const.h"

#define AVO_DEPRECATED_FCT 0
#if AVO_DEPRECATED_FCT
#include "undocommand.h"
#include "constraintsmodel.h"
#endif

#include <Eigen/Core>

#include <avogadro/extension.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <avogadro/navigate.h>
#include <avogadro/primitivelist.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/tool.h>
#include <avogadro/toolgroup.h>
#include <avogadro/periodictableview.h>
#include <avogadro/glhit.h>
#include <avogadro/undosequence.h>

#include <QUndoCommand>
#include <QMessageBox>
#include <QAction>
#include <QTimer>
#include <QToolButton>
#include <qapplication.h>
#include <qevent.h>
#include <qcursor.h>
#include <QString>
#include <QDebug>
#include <QDir>

#include <openbabel/mol.h>
#include <openbabel/builder.h>
#include <openbabel/obconversion.h>

#include "warning_disable_end.h"


namespace Avogadro
{
  class MoleculeManipulation : public QObject
  {
    Q_OBJECT // To manage signal


  public slots :
    void setAtomicNumberCurrent( int atomicNb ) ;

    /**
      * @name Tools to add/remove Hydrogen.
      * Generic code to help in the adjustment of Hydrogen atoms.
      * @{ */
    void addHydrogens() ;
    void removeHydrogens() ;
    void setHasAddedHydrogen( bool withH ) ;
    void invertHasAddHydrogen() ;
    // @}
    
  signals :
  void invertedHasAddHydrogen( bool enable ) ;
   

  public :
    MoleculeManipulation( Molecule *molecule ) ;
    ~MoleculeManipulation() ;

    /**
      * @name Tools for all basics manipulations
      * All basic methods rewrite/mainly encapsulate, and the Avogadro methods for the
      * same things (almost).
      * Avogadro methods are specify as deprecated (for some reason explained somewhere)
      * and they are the only to realize the undo/redo feature.
      * @{ */
    void rotateAtomBegin( const QList<Atom*> &atomList, const Eigen::Transform3d& transfAtomRotate ) ;
    void rotateAtomBegin( const QList<Primitive*> &primList, const Eigen::Transform3d& transfAtomRotate ) ;
    void tranlateAtomBegin( const QList<Atom*> &atomList, const Eigen::Vector3d& vectAtomTranslate ) ;
    void tranlateAtomBegin( const QList<Primitive*> &primList, const Eigen::Vector3d& vectAtomTranslate ) ;
    void moveAtomBegin( const QList<Atom*> &atomList, bool hasTranslation, const Eigen::Vector3d& vectAtomTranslate, bool hasRotation, const Eigen::Transform3d& transfAtomRotate ) ;
    void moveAtomBegin( const QList<Primitive*> &primList, bool hasTranslation, const Eigen::Vector3d& vectAtomTranslate, bool hasRotation, const Eigen::Transform3d& transfAtomRotate ) ;
    void moveAtomEnd( const QList<Atom*> &atomList ) ;
    void moveAtomEnd( const QList<Primitive*> &primList ) ;

    void changeOrderBondBy1( Bond *bond ) ;
    void changeAtomicNumber( Atom *atom, int atomicNumber ) ;

    Atom* addAtom( Eigen::Vector3d *pos, int atomicNumber ) ;
    Atom* addAtom( Eigen::Vector3d *pos, int atomicNumber, Atom* bondedAtom, int order ) ;
    PrimitiveList* addAtoms( Eigen::Vector3d *pos1, int atomicNumber1, Eigen::Vector3d *pos2, int atomicNumber2, int order) ;
    Bond* addBond( Atom *a1, Atom *a2, short order ) ;
    
    PrimitiveList* addFragment( Molecule *fragment ) ;
    PrimitiveList* addFragment( Molecule *fragment, Atom *bondedAtom ) ;
    PrimitiveList* addFragment( const QString &fragmentAbsPath, Atom* atomToSubstitute=NULL ) ;
    
    void removeAtom( Atom *atom ) ;
    void removeAtoms( QList<Atom*> *atoms ) ;
    void removeAtoms( QList<Primitive*> *atoms ) ;
    void removeAtoms( PrimitiveList *atoms ) ;
    void removeBond( Bond *bond ) ;
    
    void deleteAllElement() ;
    // @}

    /**
      * @name For the barycenter
      * @{ */
    const Eigen::Vector3d& getBarycenterMolecule() ;
    void recalculateBarycenter() ;
    // @}
    
    /** 
     * @name For the other referentiel
     * @{ */
    void setRotationAxe( Bond* axeRot ) ; //< Set m_rotationAxe.
    void setRotationAxe( Eigen::Vector3d *p1Ref, Eigen::Vector3d *p2, Bond* axeRot ) ; 
        //< Set m_rotationAxe.
    void resetRotationAxe() ; //< Put m_rotationAxe to zero.
    Eigen::Vector3d getRotationAxe() ;
    Bond* getRotationAxeBond() ;
    Eigen::Vector3d getRotationAxePoint() ; //< A point on the rotation axe.
    // @}

    /** 
      * @name Getter/Setter/Tester.
      * @{ */
    int getAtomicNumberCurrent() ;
    Molecule* getFragment( const QString &fragmentAbsPath ) ;
    PrimitiveList* getAllBondedAtom( const PrimitiveList &primList ) ;
    bool hasAddedHydrogen() ;
    // @}

    /** 
      * @name Miscellanious.
      * @{ */
    Atom* calculateNearestAtom( const Eigen::Vector3d *posAtom, const Atom *atomNotUse=NULL ) ;
    void optimizeGeometry() ;
    // @}


    #if AVO_DEPRECATED_FCT
    void addAdjustHydrogenRedoUndo( Molecule *molecule ) ;
        ///< Ajust hydrogen and add undo/redo once addAtom() or addBond() are called. Deprecated : Avogadro style.
    void addFragment1( Molecule *fragment ) ;
        ///< Deprecated : Avogadro style.
    void addFragmentWithUndoRedoHydrogen( Molecule *fragment, int selectedAtom ) ;
        ///< Deprecated : Avogadro style.
    void removeAtomWithUndoRedoHydrogen( Atom *atom ) ;  
        ///< Deprecated : Avogadro style.
    void deleteBondWithUndoRedo( Bond *bond ) ; 
        ///< Deprecated : Avogadro style.
    void deleteSelectedElementUndoRedo( Molecule *molecule ) ; 
        ///< Deprecated : Avogadro style.
    #endif


    // Private methods and structure.
    private :
      
      struct SpanningTreeNode
      {
        Atom *atom ;
        bool isVisited ;
        SpanningTreeNode *father ;
        QList<SpanningTreeNode*> sons ;
        
        SpanningTreeNode()
        {
          atom = NULL ;
          father = NULL ;
          isVisited = false ;
          
        };
        
        SpanningTreeNode( SpanningTreeNode* aFather, Atom* anAtom )
        {
          atom = anAtom ;
          father = aFather ;
          isVisited = false ;
        }; 
      };

       /**
        * @name Tools for all "basics" manipulations.
        * @{ */
      void changeOrderBondBy1WithoutHA( Bond *bond ) ;
      void changeOrderBondBy1WithHA( Bond *bond ) ;

      void changeAtomicNumberWithoutHA( Atom *atom, int atomicNumber ) ;
      void changeAtomicNumberWithHA( Atom *atom, int atomicNumber ) ;

      Atom* addAtomWithoutHA( Eigen::Vector3d *pos, int atomicNumber ) ;
      Atom* addAtomWithoutHA( Eigen::Vector3d *pos, int atomicNumber, Atom* bondedAtom, int order ) ;
      Atom* addAtomWithoutHA( OpenBabel::OBAtom *atom ) ;
      Atom* addAtomWithHA( Eigen::Vector3d *pos, int atomicNumber ) ;
      Atom* addAtomWithHA( Eigen::Vector3d *pos, int atomicNumber, Atom* bondedAtom, int order ) ;

      PrimitiveList* addAtomsWithoutHA( Eigen::Vector3d *pos1, int atomicNumber1, Eigen::Vector3d *pos2, int atomicNumber2, int order ) ;
      PrimitiveList* addAtomsWithHA( Eigen::Vector3d *pos1, int atomicNumber1, Eigen::Vector3d *pos2, int atomicNumber2, int order) ;

      Bond* addBondWithoutHA( Atom *a1, Atom *a2, short order ) ;
      Bond* addBondWithHA( Atom *a1, Atom *a2, short order ) ;
      
      PrimitiveList* addFragmentWithoutHA( Molecule *fragment ) ;
      PrimitiveList* addFragmentWithoutHA( Molecule *fragment, Atom *bondedAtom ) ;
      PrimitiveList* addFragmentWithoutHA( const QString &fragmentAbsPath, Atom* atomToSubstitute=NULL ) ;
      PrimitiveList* addFragmentWithHA( Molecule *fragment ) ;
      PrimitiveList* addFragmentWithHA( Molecule *fragment, Atom *bondedAtom ) ;
      PrimitiveList* addFragmentWithHA( const QString &fragmentAbsPath, Atom* atomToSubstitute=NULL ) ;
      
      void removeAtomWithoutHA( Atom *atom ) ;
      void removeAtomWithHA( Atom *atom ) ;

      void removeAtomsWithoutHA( QList<Atom*> *atoms ) ;
      void removeAtomsWithoutHA( QList<Primitive*> *atoms ) ;
      void removeAtomsWithoutHA( PrimitiveList *atoms ) ;
      void removeAtomsWithHA( QList<Atom*> *atoms ) ;
      void removeAtomsWithHA( QList<Primitive*> *atoms ) ;
      void removeAtomsWithHA( PrimitiveList *atoms ) ;

      void removeBondWithoutHA( Bond *bond ) ;
      void removeBondWithHA( Bond *bond ) ;
      // @}


      /**
      * @name Tools to add/remove Hydrogen.
      * Generic code to help in the adjustment of Hydrogen atoms.
      * @{ */
      OpenBabel::OBAtom* setImplicitValence_p( OpenBabel::OBMol *molecule, OpenBabel::OBAtom* atom ) ;
      void adjustPartialCharge_p( OpenBabel::OBMol *obmol ) ;
      bool addHydrogen_p( OpenBabel::OBMol *obmol, Atom* atom ) ;
      bool removeHydrogen_p( Atom* atom, Atom *atomNoRemove=NULL ) ;
      // @}

      /**
        * @name For the barycenter
        * @{ */
      void updateBarycenter( const Eigen::Vector3d& atomPos, bool addOrDel, bool testIfNeedToRecalculateBarycenter=true ) ;
      void resetBarycenter_p() ;
       // @}
      
      /** 
      * @name Miscellanious.
      * @{ */
      void getAllBondedAtom_p( SpanningTreeNode *spanTreeNode, QList<Atom*> &atomList ) ;
      // @}


    // Private attributs.
    private :
      Molecule *m_molecule ; //< (shortcut)
      bool m_isMoveAtom ; //< For the movement of an atom.
      QString m_errorMsgGetFragment ;
      bool m_addHydrogens ;
      int m_atomicNumberCurrent ;

      /**
        * @name Barycenters
        * For the barycenter of the molecule.
        * @{ */
      Eigen::Vector3d m_barycenterRefMolecule ;
      int m_sumOfWeights ;
      Eigen::Vector3d m_atomsBarycenter ;
      // @}
      
      /**
       * @name Some other referentiel
       * Stock some referentiel like axe of rotation (according to a bond) ...
       * @{ */
      Eigen::Vector3d m_rotationAxe ; //< (0;0;0) = no defined axe.
      Bond *m_rotationAxeBond ; //< Just for information (and manage selection)
      Eigen::Vector3d m_rotationAxePoint ; //< A point on the rotation axe.
      // @}
      
      /**
       * @name Select all bonded atoms.
       * @{ */
      SpanningTreeNode *m_spanTreeNodeFirst ;
      // @}

      /**
        * @name Static definition
        * @{ */
      static const Eigen::Vector3d m_vect3d0 ; ///< A null vector3d.
      static Eigen::Transform3d m_transf3d0 ; ///< A null transform3d.
      // @}


  // Eigen need.
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      // http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html
      // And NOT EIGEN_DONT_ALIGN in the begin of the file (see below).

  };
}

#endif
