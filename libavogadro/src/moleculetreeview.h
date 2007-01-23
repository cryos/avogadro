/**********************************************************************
  MoleculeWidget - Molecule Widget

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.  For more
  information, see <http://avogadro.sourceforge.net/>

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

#ifndef __MOLECULETREEVIEW_H
#define __MOLECULETREEVIEW_H


#include "primitives.h"
#include <QTreeWidget>
#include <QItemDelegate>

namespace Avogadro {

  class MoleculeTreeView : public QTreeWidget
  {
    Q_OBJECT

    public:
      MoleculeTreeView(QWidget *parent=0);
      MoleculeTreeView(Molecule *molecule=0, QWidget *parent=0);

      void setMolecule(Molecule *molecule);
      QTreeWidgetItem* addGroup(enum Primitive::Type type);
      QTreeWidgetItem* addGroup(QString name, enum Primitive::Type type);

    public slots:
      QTreeWidgetItem* addPrimitive(Primitive *primitive);
      void updatePrimitive(Primitive *primitive);
      void removePrimitive(Primitive *primitive);

      void updateModel();
//dc:       void update();

    private slots:
      void handleMousePress(QTreeWidgetItem *item);

    protected:
      Molecule *m_molecule;
      QVector<QTreeWidgetItem *> m_groups;
      
      QString primitiveToItemText(Primitive *primitive);
      int primitiveToItemIndex(Primitive *primitive);
      void updateGroup(QTreeWidgetItem *item);
      void updatePrimitiveItem(QTreeWidgetItem *item);
      
    private:
      void constructor();
  };

  class MoleculeItemDelegate : public QItemDelegate
  {
    Q_OBJECT
    public:
      MoleculeItemDelegate(QTreeView *view, QWidget *parent);

      virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
      virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;

    private:
      QTreeView *m_view;

  };

}

#endif
