/**********************************************************************
  propextension.h - Properties Plugin for Avogadro

  Copyright (C) 2007-2008 by Tim Vandermeersch
  Some portions Copyright (C) 2009 by Konstantin Tokarev

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

#include "propextension.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/primitivelist.h>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>

#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  enum PropExtensionIndex
  {
    AtomPropIndex = 0,
    BondPropIndex,
    AnglePropIndex,
    TorsionPropIndex,
    CartesianIndex,
    ConformerIndex
  };

  PropertiesExtension::PropertiesExtension( QObject *parent ) : Extension( parent )
  {
    QAction *action;

    action = new QAction( this );
    action->setSeparator(true);
    action->setData(-1);
    m_actions.append(action);

    action = new QAction( this );
    action->setText( tr("Atom Properties..." ));
    action->setData(AtomPropIndex);
    m_actions.append( action );

    action = new QAction( this );
    action->setText( tr("Bond Properties..." ));
    action->setData(BondPropIndex);
    m_actions.append( action );

    action = new QAction( this );
    action->setText( tr("Angle Properties..." ));
    action->setData(AnglePropIndex);
    m_actions.append( action );

    action = new QAction( this );
    action->setText( tr("Torsion Properties..." ));
    action->setData(TorsionPropIndex);
    m_actions.append( action );

    action = new QAction( this );
    action->setText( tr("Conformer Properties..." ));
    action->setData(ConformerIndex);
    m_actions.append( action );

    action = new QAction( this );
    action->setText( tr("Cartesian Editor..." ));
    action->setData(CartesianIndex);
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
    case -1: // separator
      return tr("&View");
    case AtomPropIndex:
    case BondPropIndex:
    case AnglePropIndex:
    case TorsionPropIndex:
    case ConformerIndex:
      return tr("&View") + '>' + tr("&Properties");
    case CartesianIndex:
    default:
      return tr("&Build");
      break;
    };
    return QString();
  }

  void PropertiesExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* PropertiesExtension::performAction(QAction *action,
                                                   GLWidget *widget)
  {
    QUndoCommand *undo = 0;
    PropertiesModel *model;
    PropertiesView  *view;
    QDialog *dialog = new QDialog(qobject_cast<QWidget *>(parent()));
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    // Don't show whitespace around the PropertiesView
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    int i = action->data().toInt();
    switch (i) {
    case AtomPropIndex: // atom properties
      // model will be deleted in PropertiesView::hideEvent using deleteLater().
      model = new PropertiesModel(PropertiesModel::AtomType);
      model->setMolecule(m_molecule);
      // view will delete itself in PropertiesView::hideEvent using deleteLater().
      view = new PropertiesView(PropertiesView::AtomType, dialog);
      connect(m_molecule, SIGNAL( atomAdded(Atom*) ),
              model, SLOT( atomAdded(Atom*) ));
      connect(m_molecule, SIGNAL( atomRemoved(Atom*) ),
              model, SLOT( atomRemoved(Atom*) ));
      break;
    case BondPropIndex: // bond properties
      // model will be deleted in PropertiesView::hideEvent using deleteLater().
      model = new PropertiesModel(PropertiesModel::BondType);
      model->setMolecule( m_molecule );
      // view will delete itself in PropertiesView::hideEvent using deleteLater().
      view = new PropertiesView(PropertiesView::BondType, widget);
      connect(m_molecule, SIGNAL( bondAdded(Bond*) ),
              model, SLOT( bondAdded(Bond*) ));
      connect(m_molecule, SIGNAL( bondRemoved(Bond*) ),
              model, SLOT( bondRemoved(Bond*) ));
      break;
    case AnglePropIndex: // angle properties
      // model will be deleted in PropertiesView::hideEvent using deleteLater().
      model = new PropertiesModel(PropertiesModel::AngleType);
      model->setMolecule( m_molecule );
      // view will delete itself in PropertiesView::hideEvent using deleteLater().
      view = new PropertiesView(PropertiesView::AngleType, widget);
      break;
    case TorsionPropIndex: // torsion properties
      // model will be deleted in PropertiesView::hideEvent using deleteLater().
      model = new PropertiesModel(PropertiesModel::TorsionType);
      model->setMolecule( m_molecule );
      // view will delete itself in PropertiesView::hideEvent using deleteLater().
      view = new PropertiesView(PropertiesView::TorsionType, widget);
      break;
    case CartesianIndex: // cartesian editor
      // m_angleModel will be deleted in PropertiesView::hideEvent using deleteLater().
      model = new PropertiesModel(PropertiesModel::CartesianType);
      model->setMolecule( m_molecule );
      // m_view will delete itself in PropertiesView::hideEvent using deleteLater().
      view = new PropertiesView(PropertiesView::CartesianType, widget);
      connect(m_molecule, SIGNAL(atomAdded(Atom*)), model, SLOT( atomAdded(Atom*)));
      connect(m_molecule, SIGNAL(atomRemoved(Atom*)), model, SLOT(atomRemoved(Atom*)));
      break;
    case ConformerIndex: // conformers
      // model will be deleted in PropertiesView::hideEvent using deleteLater().
      model = new PropertiesModel(PropertiesModel::ConformerType, dialog);
      model->setMolecule( m_molecule );
      // view will delete itself in PropertiesView::hideEvent using deleteLater().
      view = new PropertiesView(PropertiesView::ConformerType, dialog);
      break;
    default:
      delete dialog;
      layout = 0; // deleted as a child of the dialog
      return 0;
    }

    connect(m_molecule, SIGNAL(moleculeChanged()), model, SLOT(moleculeChanged()));
    connect(m_molecule, SIGNAL( updated() ), model, SLOT( updateTable() ));

    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortLocaleAware(true);
    // this role will received direct floating-point numbers from the model
    proxyModel->setSortRole(Qt::UserRole);

    view->setMolecule( m_molecule );
    view->setWidget( widget );
    view->setModel( proxyModel );

    layout->addWidget(view);
    dialog->setWindowTitle(view->windowTitle());
    QSize dialogSize = dialog->size();
    dialogSize.setWidth(model->columnCount()*120);
	if (model->rowCount() < 10)
	  dialogSize.setHeight(model->rowCount()*40);
    dialog->resize(dialogSize);
	dialog->setWindowFlags(Qt::Window);
    dialog->show();

    return undo;
  }

  PropertiesView::PropertiesView(Type type, QWidget *parent) : QTableView(parent),
      m_molecule(NULL), m_widget(NULL)
  {
    m_type = type;

    QString title;
    switch(type){
    case AtomType:
      title = tr("Atom Properties");
      break;
    case BondType:
      title = tr("Bond Properties");
      break;
    case AngleType:
      title = tr("Angle Properties");
      break;
    case TorsionType:
      title = tr("Torsion Properties");
      break;
    case CartesianType:
      title = tr("Cartesian Properties");
      break;
    case ConformerType:
      title = tr("Conformer Properties");
      break;
    default:
      break;
    }
    this->setWindowTitle(title);

    QHeaderView *horizontal = this->horizontalHeader();
    horizontal->setResizeMode(QHeaderView::Stretch);
    QHeaderView *vertical = this->verticalHeader();
    vertical->setResizeMode(QHeaderView::Stretch);

    // Don't allow selecting everything
    setCornerButtonEnabled(false);
    // Alternating row colors
    setAlternatingRowColors(true);
    // Allow sorting the table
    setSortingEnabled(true);
  }


  void PropertiesView::selectionChanged(const QItemSelection &selected, const QItemSelection &)
  {
    QList<Primitive *> matchedPrimitives;

    foreach (const QModelIndex &index, selected.indexes()) {
      if (!index.isValid())
        return;

      if (m_type == AtomType || m_type == CartesianType) {
        if ((unsigned int) index.row() >= m_molecule->numAtoms())
          return;

        matchedPrimitives.append( m_molecule->atom(index.row()) );
        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
      } else if (m_type == BondType) {
        if((unsigned int) index.row() >= m_molecule->numBonds())
          return;

        matchedPrimitives.append( m_molecule->bond(index.row()) );
        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
      } else if (m_type == AngleType && model() != 0) {
        int aIndex = model()->data(index.sibling(index.row(), 0)).toInt();
        int bIndex = model()->data(index.sibling(index.row(), 1)).toInt();
        int cIndex = model()->data(index.sibling(index.row(), 2)).toInt();
        
        matchedPrimitives.append( m_molecule->atom( aIndex - 1) );
        matchedPrimitives.append( m_molecule->atom( bIndex - 1) );
        matchedPrimitives.append( m_molecule->atom( cIndex - 1) );

        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
      } else if (m_type == TorsionType && model() != 0) {
        int aIndex = model()->data(index.sibling(index.row(), 0)).toInt();
        int bIndex = model()->data(index.sibling(index.row(), 1)).toInt();
        int cIndex = model()->data(index.sibling(index.row(), 2)).toInt();
        int dIndex = model()->data(index.sibling(index.row(), 3)).toInt();
        
        matchedPrimitives.append( m_molecule->atom( aIndex - 1) );
        matchedPrimitives.append( m_molecule->atom( bIndex - 1) );
        matchedPrimitives.append( m_molecule->atom( cIndex - 1) );
        matchedPrimitives.append( m_molecule->atom( dIndex - 1) );

        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
      } else if (m_type == ConformerType) {
        if (index.row() >= static_cast<int>(m_molecule->numConformers()))
          return;

        m_molecule->setConformer(index.row());
        m_molecule->update();
        return;
      }
      // TODO: Highlight angles and torsions
    }
  }

  void PropertiesView::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void PropertiesView::setWidget(GLWidget *widget)
  {
    m_widget = widget;
  }

  void PropertiesView::hideEvent(QHideEvent *)
  {
    if (m_widget)
      m_widget->clearSelected();

    QAbstractItemModel *m_model = model();
    if (m_model)
      m_model->deleteLater();

    this->deleteLater();
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2( propextension, Avogadro::PropertiesExtensionFactory )

