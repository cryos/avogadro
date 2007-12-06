/**********************************************************************
  propextension.h - Properties Plugin for Avogadro

  Copyright (C) 2007 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "propextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <QtGui>
#include <QWriteLocker>
#include <QMutex>
#include <QMutexLocker>
#include <QAbstractTableModel>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  enum PropExtensionIndex
  {
    AtomPropIndex = 0,
    BondPropIndex,
    CartesianIndex,
    ConformerIndex
  };

  PropertiesExtension::PropertiesExtension( QObject *parent ) : QObject( parent )
  {
    QAction *action;
    m_atomModel = new AtomPropModel;
    m_bondModel = new BondPropModel;
    m_cartesianModel = new CartesianModel;
    m_conformerModel = new ConformerModel;
    
    action = new QAction( this );
    action->setText( tr("Atom properties" ));
    action->setData(AtomPropIndex);
    m_actions.append( action );

    action = new QAction( this );
    action->setText( tr("Bond properties" ));
    action->setData(BondPropIndex);
    m_actions.append( action );

    action = new QAction( this );
    action->setText( tr("Cartesian editor" ));
    action->setData(CartesianIndex);
    m_actions.append( action );
    
    action = new QAction( this );
    action->setText( tr("Conformers" ));
    action->setData(ConformerIndex);
    m_actions.append( action );
  }

  PropertiesExtension::~PropertiesExtension()
  {}

  QList<QAction *> PropertiesExtension::actions() const
  {
    return m_actions;
  }

  QString PropertiesExtension::menuPath(QAction *action) const
  {
    int i = action->data().toInt();
    switch(i) {
      case AtomPropIndex:
      case BondPropIndex:
      case ConformerIndex:
        return tr("&Extensions") + ">" + tr("&Properties");
      case CartesianIndex:
        return tr("&Build");
        break;
      default:
        break;
    };
    return QString();
  }

  QUndoCommand* PropertiesExtension::performAction( QAction *action, Molecule *molecule,
      GLWidget *widget, QTextEdit *textEdit )
  {
    QUndoCommand *undo = NULL;
    ConformerView *cview = NULL;
    AtomTableView *aview = NULL;
    BondTableView *bview = NULL;
    ostringstream buff;

    int i = action->data().toInt();
    switch ( i ) {
      case AtomPropIndex: // atom properties
        aview = new AtomTableView;
	m_atomModel->setMolecule( molecule );
	aview->setMolecule( molecule );
	aview->setWidget( widget );
	aview->setModel( m_atomModel );
	aview->resize(860, 400);
	aview->show();
        break;
      case BondPropIndex: // bond properties
        bview = new BondTableView;
	m_bondModel->setMolecule( molecule );
	bview->setMolecule( molecule );
	bview->setWidget( widget );
	bview->setModel( m_bondModel );
	bview->resize(550, 400);
	bview->show();
        break;
      case CartesianIndex: // cartesian editor
        aview = new AtomTableView;
	m_cartesianModel->setMolecule( molecule );
	aview->setMolecule( molecule );
	aview->setWidget( widget );
	aview->setModel( m_cartesianModel );
	aview->resize(360, 400);
	aview->show();
        break;
      case ConformerIndex: // conformers
        cview = new ConformerView;
	m_conformerModel->setMolecule( molecule );
	cview->setMolecule( molecule );
	cview->setModel( m_conformerModel );
	cview->resize(180, 500);
	cview->show();
        break;
    }

    return undo;
  }
  
  void ConformerView::selectionChanged(const QItemSelection &selected, const QItemSelection &previous)
  {
    QModelIndex index;
    QModelIndexList items = selected.indexes();
   
    foreach (index, items) {
      if (!index.isValid())
        return;
    
      if (index.row() >= m_molecule->NumConformers())
        return;
    
      m_molecule->SetConformer(index.row());
      m_molecule->update();
    }
  }
  
  void ConformerView::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }
 
  void AtomTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &previous)
  {
    QModelIndex index;
    QList<Primitive *> matchedAtoms;
    QModelIndexList items = selected.indexes();
    
    foreach (index, items) {
      if (!index.isValid())
        return;
    
      if (index.row() >= m_molecule->NumAtoms())
        return;
    
      matchedAtoms.append(static_cast<Atom*>(m_molecule->GetAtom(index.row()+1)));
    }
    
    m_widget->clearSelected();
    m_widget->setSelected(matchedAtoms, true);
    m_widget->update();
  }
  
  void AtomTableView::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }
  
  void AtomTableView::setWidget(GLWidget *widget)
  {
    m_widget = widget;
  }
  
  void AtomTableView::hideEvent(QHideEvent *event)
  {
    m_widget->clearSelected();
  }
  
  void BondTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &previous)
  {
    QModelIndex index;
    QList<Primitive *> matchedBonds;
    QModelIndexList items = selected.indexes();
    
    foreach (index, items) {
      if (!index.isValid())
        return;
    
      if (index.row() >= m_molecule->NumAtoms())
        return;
    
      matchedBonds.append(static_cast<Bond*>(m_molecule->GetBond(index.row())));
    }
    
    m_widget->clearSelected();
    m_widget->setSelected(matchedBonds, true);
    m_widget->update();
  }
  
  void BondTableView::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }
  
  void BondTableView::setWidget(GLWidget *widget)
  {
    m_widget = widget;
  }
  
  void BondTableView::hideEvent(QHideEvent *event)
  {
    m_widget->clearSelected();
  }
 
} // end namespace Avogadro

#include "propextension.moc"
Q_EXPORT_PLUGIN2( propextension, Avogadro::PropertiesExtensionFactory )
