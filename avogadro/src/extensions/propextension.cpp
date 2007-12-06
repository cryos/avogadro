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
      GLWidget *, QTextEdit *textEdit )
  {
    QUndoCommand *undo = NULL;
    QTableView *view = NULL;
    ostringstream buff;

    int i = action->data().toInt();
    switch ( i ) {
      case AtomPropIndex: // atom properties
        view = new QTableView;
	m_atomModel->setMolecule( molecule );
	view->setModel( m_atomModel );
	view->resize(600, 400);
	view->show();
        break;
      case BondPropIndex: // bond properties
        view = new QTableView;
	m_bondModel->setMolecule( molecule );
	view->setModel( m_bondModel );
	view->resize(600, 400);
	view->show();
        break;
      case CartesianIndex: // cartesian editor
        view = new QTableView;
	m_cartesianModel->setMolecule( molecule );
	view->setModel( m_cartesianModel );
	view->resize(400, 600);
	view->show();
        break;
      case ConformerIndex: // conformers
        view = new QTableView;
	m_conformerModel->setMolecule( molecule );
	view->setModel( m_conformerModel );
	view->resize(400, 600);
	view->show();
        break;
    }

    return undo;
  }

} // end namespace Avogadro

#include "propextension.moc"
Q_EXPORT_PLUGIN2( propextension, Avogadro::PropertiesExtensionFactory )
