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

#include <QtCore/QAbstractTableModel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QSizePolicy>
#include <QtGui/QHeaderView>
#include <QtGui/QAction>
#include <QtGui/QDialog>
#include <QtGui/QScrollBar>
#include <QtGui/QVBoxLayout>

#include <QtCore/QDebug>

using namespace std;
using namespace OpenBabel;
using OpenBabel::OBGenericDataType::AngleData;
using OpenBabel::OBGenericDataType::TorsionData;

namespace Avogadro
{
  enum PropExtensionIndex
  {
    AtomPropIndex = 0,
    BondPropIndex,
    AnglePropIndex,
    TorsionPropIndex,
    //CartesianIndex,
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

    /*action = new QAction( this );
    action->setText( tr("Cartesian Editor..." ));
    action->setData(CartesianIndex);
    m_actions.append( action );*/
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
    dialog->setLayout(layout);
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
    /*case CartesianIndex: // cartesian editor
      // m_angleModel will be deleted in PropertiesView::hideEvent using deleteLater().
      model = new PropertiesModel(PropertiesModel::CartesianType);
      model->setMolecule( m_molecule );
      // m_view will delete itself in PropertiesView::hideEvent using deleteLater().
      view = new PropertiesView(PropertiesView::CartesianType, widget);
      connect(m_molecule, SIGNAL(atomAdded(Atom*)), model, SLOT( atomAdded(Atom*)));
      connect(m_molecule, SIGNAL(atomRemoved(Atom*)), model, SLOT(atomRemoved(Atom*)));
      break;*/
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
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->resizeColumnsToContents();
    layout->addWidget(view);
    dialog->setWindowTitle(view->windowTitle());
    QSize dialogSize = dialog->size();
    double width = view->horizontalHeader()->length()+view->verticalHeader()->width()+5;
	if (model->rowCount() < 13) { // no scrollbar
	  dialogSize.setHeight(view->horizontalHeader()->height()+model->rowCount()*30+5);
      dialogSize.setWidth(width);
    } else { // scrollbar is needed
      dialogSize.setHeight(width/1.618);
      dialogSize.setWidth(width+view->verticalScrollBar()->width());
    }
    dialog->resize(dialogSize);
	//dialog->setWindowFlags(Qt::Window);
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
    /*case CartesianType:
      title = tr("Cartesian Properties");
      break;*/
    case ConformerType:
      title = tr("Conformer Properties");
      break;
    default:
      break;
    }
    this->setWindowTitle(title);

    QHeaderView *horizontal = this->horizontalHeader();
    horizontal->setResizeMode(QHeaderView::Interactive);
    horizontal->setMinimumSectionSize(75);
    QHeaderView *vertical = this->verticalHeader();
    vertical->setResizeMode(QHeaderView::Interactive);
    vertical->setMinimumSectionSize(30);
    vertical->setDefaultAlignment(Qt::AlignCenter);

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
    bool ok = false;

    foreach (const QModelIndex &index, selected.indexes()) {
      if (!index.isValid())
        return;
      int rowNum = model()->headerData(index.row(), Qt::Vertical).toString().split(" ").last().toLong(&ok) - 1;
      if (!ok)
        return;
      
      if (m_type == AtomType /*|| m_type == CartesianType*/) {
        if ((unsigned int) index.row() >= m_molecule->numAtoms())
          return;

        matchedPrimitives.append( m_molecule->atom(rowNum) );
        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
      } else if (m_type == BondType) {
        if((unsigned int) index.row() >= m_molecule->numBonds())
          return;

        matchedPrimitives.append( m_molecule->bond(rowNum) );
        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
      } else if (m_type == AngleType && model() != 0) {
        OBMol *mol = new OBMol(m_molecule->OBMol());
        mol->FindAngles();
        OBAngleData *ad = static_cast<OBAngleData *>(mol->GetData(AngleData));
        if (!ad)
          return;
        vector<vector<unsigned int> > angles;
        ad->FillAngleArray(angles);
        delete mol;

        Atom *startAtom = m_molecule->atom((angles[rowNum][1]));
        Atom *vertex = m_molecule->atom((angles[rowNum][0]));
        Atom *endAtom = m_molecule->atom((angles[rowNum][2]));
        Bond *bond1 = startAtom->bond(vertex);
        Bond *bond2 = vertex->bond(endAtom);
        
        matchedPrimitives.append( startAtom );
        matchedPrimitives.append( vertex );
        matchedPrimitives.append( endAtom );
        matchedPrimitives.append( bond1 );
        matchedPrimitives.append( bond2 );

        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
      } else if (m_type == TorsionType && model() != 0) {
        OBMol *mol = new OBMol(m_molecule->OBMol());
        mol->FindTorsions();
        OBTorsionData *td = static_cast<OBTorsionData *>(mol->GetData(TorsionData));
        if (!td)
          return;
        vector<vector<unsigned int> > torsions;
        td->FillTorsionArray(torsions);
        delete mol;

        Atom *a = m_molecule->atom( torsions[rowNum][0] );
        Atom *b = m_molecule->atom( torsions[rowNum][1] );
        Atom *c = m_molecule->atom( torsions[rowNum][2] );
        Atom *d = m_molecule->atom( torsions[rowNum][3] );
        Bond *bond1 = a->bond(b);
        Bond *bond2 = b->bond(c);
        Bond *bond3 = c->bond(d);
        
        matchedPrimitives.append(a);
        matchedPrimitives.append(b);
        matchedPrimitives.append(c);
        matchedPrimitives.append(d);
        matchedPrimitives.append(bond1);
        matchedPrimitives.append(bond2);
        matchedPrimitives.append(bond3);

        m_widget->clearSelected();
        m_widget->setSelected(matchedPrimitives, true);
        m_widget->update();
      } else if (m_type == ConformerType) {
        if (index.row() >= static_cast<int>(m_molecule->numConformers()))
          return;

        m_molecule->setConformer(rowNum);
        m_molecule->update();
        return;
      }
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
    if ((m_widget) && model()) {
      m_widget->clearSelected();
      model()->deleteLater();
    }

    this->deleteLater();
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2( propextension, Avogadro::PropertiesExtensionFactory )

