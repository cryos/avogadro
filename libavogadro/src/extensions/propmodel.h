/**********************************************************************
  propmodel.h - Models to hold properties

  Copyright (C) 2007 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef PROPMODEL_H
#define PROPMODEL_H


#include <openbabel/mol.h>
#include <openbabel/math/vector3.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QTableView>
#include <QItemSelectionModel>

#ifndef BUFF_SIZE
#define BUFF_SIZE 256
#endif

namespace Avogadro {

  using OpenBabel::OBTorsion;

 class PropertiesModel : public QAbstractTableModel
  {
    Q_OBJECT

     public slots:
       void updateTable();
       //       void primitiveAdded(Primitive *primitive);
       //       void primitiveRemoved(Primitive *primitive);
       void atomAdded(Atom *atom);
       void atomRemoved(Atom *atom);
       void bondAdded(Bond *bond);
       void bondRemoved(Bond *bond);
       void moleculeChanged();

     public:
       enum Type {
         OtherType=0,
         AtomType,
         BondType,
         AngleType,
         TorsionType,
         CartesianType,
         ConformerType,
         MoleculeType
       };

       explicit PropertiesModel(Type type, QObject *parent = 0);

       int rowCount(const QModelIndex &parent = QModelIndex()) const;
       int columnCount(const QModelIndex &parent = QModelIndex()) const;
       QVariant data(const QModelIndex &index, int role) const;
       Qt::ItemFlags flags(const QModelIndex &index) const;
       bool setData(const QModelIndex &index, const QVariant &value,
           int role = Qt::EditRole);
       QVariant headerData(int section, Qt::Orientation orientation,
           int role = Qt::DisplayRole) const;

       void setMolecule (Molecule *molecule);

       // determine whether we display multiple or just one conformer
       void setDisplayConformers(bool display);

       // Return what type of model this is
       int type() const { return m_type; };

       // Generate all data pertaining to atoms, bonds, angles etc
       void updateCache() const;

       // Empty all items in the cache
       void clearCache() const;

       // Return the number of conformers we are displaying
       unsigned int numConformers() const;

       // Given a model index, return the conformer it refers to
       unsigned int conformerFromIndex(const QModelIndex &index) const;

       // Returns the angle data for a given conformer
       std::vector< std::vector<unsigned int> > conformerAngles( unsigned int conformer );

       // Returns the torsion data for a given conformer
       std::vector< std::vector<unsigned int> > conformerTorsions( unsigned int conformer );


     private:
       int m_type;
       mutable int m_rowCount;
       Molecule *m_molecule;

       /*
	* For each category (atom, bond etc), an enum specifies which columns hold
	* which data.
	* There are then two data structures (e.g. m_bondData) which holds the data
	* that is shared by all conformers, and an additional structure (e.g. m_bondLengths)
	* that holds the data that differs for each conformer
	* For angles and torsions, there are additional data structures for the vectors defining
	* the angles as these are needed when editing the structures.
	*/

       // Controls whether we display the data for one or all conformers
       bool m_displayConformers;

       // Atom Data
       enum AtomColumn { AtomDataElement=0,
                         AtomDataType,
                         AtomDataValence,
                         AtomDataFormalCharge,
                         AtomDataPartialCharge };
       mutable std::vector< std::vector<QVariant> > m_atomData;
       mutable std::vector< std::vector<OpenBabel::vector3> > m_atomCoords;

       // Bond Data
       enum BondColumn { BondDataType=0,
                         BondDataAtom1,
                         BondDataAtom2,
                         BondDataOrder,
                         BondDataRotatable};
       mutable std::vector< std::vector<QVariant> > m_bondData;
       mutable std::vector< std::vector<double> > m_bondLengths;

       // Angle Data
       enum AngleColumn { AngleDataType=0,
			  AngleDataStartAtom,
			  AngleDataVertex,
			  AngleDataEndAtom};
       mutable std::vector< std::vector<QVariant> > m_angleData;
       mutable std::vector< std::vector<double> > m_angleValues;
       mutable std::vector< std::vector< std::vector<unsigned int> > > m_angles;

       // Torsion Data
       enum TorsionColumn { TorsionDataType=0,
			    TorsionDataAtom1,
			    TorsionDataAtom2,
			    TorsionDataAtom3,
			    TorsionDataAtom4};
       mutable std::vector< std::vector<QVariant> > m_torsionData;
       mutable std::vector< std::vector<double> > m_torsionValues;
       mutable std::vector< std::vector< std::vector<unsigned int> > > m_torsions;


       mutable bool m_validCache;
       mutable OpenBabel::OBMol *m_cachedOBMol;
 };

} // end namespace Avogadro

#endif
