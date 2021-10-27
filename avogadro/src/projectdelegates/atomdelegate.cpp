/**********************************************************************
  AtomDelegate - Project Tree Item for atoms.

  Copyright (C) 2008 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#include "atomdelegate.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <openbabel/elements.h>
#include <openbabel/mol.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  AtomDelegate::AtomDelegate(ProjectTreeModel *model) : ProjectTreeModelDelegate(model), m_label(0), m_widget(0)
  {
  }

  AtomDelegate::~AtomDelegate()
  {
  }

  void AtomDelegate::initStructure(GLWidget *widget, ProjectTreeItem *parent)
  {
    // save the widget
    m_widget = widget;

    // add the labels
    m_label = insertExpandableItem(parent);
    m_label->setData(0, alias());
  }

  void AtomDelegate::fetchMore(ProjectTreeItem *)
  {
    // the user has expanded our label, we now initialize the bond items
    // and keep track of the using the signals...

    Molecule *molecule = m_widget->molecule();
    disconnect(molecule, 0, this, 0);
    // connect some signals to keep track of changes
    connect(molecule, SIGNAL(primitiveAdded(Primitive*)), this, SLOT(primitiveAdded(Primitive*)));
    connect(molecule, SIGNAL(primitiveUpdated(Primitive*)), this, SLOT(primitiveUpdated(Primitive*)));
    connect(molecule, SIGNAL(primitiveRemoved(Primitive*)), this, SLOT(primitiveRemoved(Primitive*)));

    initialize();
  }

  void AtomDelegate::initialize()
  {
    Molecule *molecule = m_widget->molecule();

    // remove any existing rows
    if (m_label->childCount())
      model()->removeRows(m_label, 0, m_label->childCount());

    // add the atoms...
    model()->insertRows(m_label, 0, molecule->numAtoms());
    for (int i = 0; i < m_label->childCount(); ++i) {
      ProjectTreeItem *item = m_label->child(i);
      item->setData(0, QString(OpenBabel::OBElements::GetSymbol(molecule->atom(i)->atomicNumber())));
      item->setData(1, QString("%1").arg(i));
      // set the primitive
      PrimitiveList primitives;
      primitives.append(molecule->atom(i));
      item->setPrimitives(primitives);
    }

  }

  void AtomDelegate::primitiveAdded(Primitive *primitive)
  {
    if (primitive->type() != Primitive::AtomType)
      return;

    Atom *atom = static_cast<Atom*>(primitive);

    // add the new primitive to the end
    model()->insertRows(m_label, m_label->childCount(), 1);
    ProjectTreeItem *item = m_label->child(m_label->childCount() - 1);
    item->setData(0, QString(OpenBabel::OBElements::GetSymbol(atom->atomicNumber())));
    item->setData(1, QString("%1").arg(primitive->index()));
    // set the primitive
    PrimitiveList primitives;
    primitives.append(primitive);
    item->setPrimitives(primitives);
  }

  void AtomDelegate::primitiveUpdated(Primitive *primitive)
  {
    if (primitive->type() == Primitive::MoleculeType) {
      initialize();
      return;
    }

    if (primitive->type() != Primitive::AtomType)
      return;

    Atom *atom = static_cast<Atom*>(primitive);

    ProjectTreeItem *item = m_label->child(primitive->index());
    item->setData(0, QString(OpenBabel::OBElements::GetSymbol(atom->atomicNumber())));
    item->setData(1, QString("%1").arg(primitive->index()));


    model()->emitDataChanged(m_label, primitive->index());
  }

  void AtomDelegate::primitiveRemoved(Primitive *primitive)
  {
    if (primitive->type() != Primitive::AtomType)
      return;

    // remove the row
    model()->removeRows(m_label, primitive->index(), 1);

    // loop over atoms below to fix their index if needed
    if ((primitive->index() + 1) < (unsigned long) m_label->childCount()) {
      for (int i = primitive->index(); i < m_label->childCount(); ++i) {
        ProjectTreeItem *item = m_label->child(i);
        item->setData(1, QString("%1").arg(i));
      }
    }
  }

  void AtomDelegate::writeSettings(QSettings &settings) const
  {
    ProjectTreeModelDelegate::writeSettings(settings);
  }

  void AtomDelegate::readSettings(QSettings &settings)
  {
    ProjectTreeModelDelegate::readSettings(settings);
  }

} // end namespace Avogadro

#include "atomdelegate.moc"
