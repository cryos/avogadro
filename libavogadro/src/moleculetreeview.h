/**********************************************************************
  MoleculeTreeView - Tree View of a Molecule

  Copyright (C) 2007 Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation; either version 2 of the License, or 
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef __MOLECULETREEVIEW_H
#define __MOLECULETREEVIEW_H

#include <avogadro/global.h>
#include <avogadro/primitive.h>

#include <QTreeWidget>
#include <QItemDelegate>

namespace Avogadro {

  /**
   * @class MoleculeTreeView
   * @brief Tree based view of a Molecule.
   * @author Donald Ephraim Curtis
   *
   * This widget provides a list of the molecule's primitives (Atoms, Bonds, Residues)
   * grouped by their type.  In terms of a Model-View architecture we consider
   * the Molecule the model and MoleculeTreeView a view of this model.
   */
  class A_EXPORT MoleculeTreeView : public QTreeWidget
  {
    Q_OBJECT

    public:
      /**
       * Construct a new molecule treeview.
       * @param parent the widget parent
       */
      MoleculeTreeView(QWidget *parent=0);

      /** Construct a new molecule treeview. Don't stress yourself out.
       *
       * @param molecule the molecule to view
       * @param parent the widget parent
       */
      MoleculeTreeView(Molecule *molecule, QWidget *parent=0);

      /**
       * Set the molecule which is the model for the treeview.
       * @param molecule the molecule to view
       */
      void setMolecule(Molecule *molecule);

    private Q_SLOTS:
      /**
       * Add the primitive to the widget.  This slot is called whenever
       * a new primitive is added to our molecule model.  It adds the 
       * primitive to the list in the appropriate group.
       *
       * @param primitive pointer to a primitive to add to the view
       * @return the QTreeWidgetItem which was created as a result of this
       * primitive addition
       */
      QTreeWidgetItem* addPrimitive(Primitive *primitive);

      /**
       * Update a primitive.  This slot is called whenever a primitive of our 
       * molecule model has been changed and we need to check our view.
       *
       * @note In some cases we are passed the molecule itself meaning that more
       * than one thing has changed in the molecule.
       *
       * @param primitive primitive that was changed
       */
      void updatePrimitive(Primitive *primitive);

      /** Remove a primitive.  This slot is called whenever a primitive of our
       * molecule model has been removed and we need to take it off our list.  
       * Additionally we need to update other items in our view that are impacted
       * by this change.
       *
       * @param primitive primitive that was removed
       */
      void removePrimitive(Primitive *primitive);

      /** 
       * Update the entire model.  This goes though all items in the list and 
       * updates their view.
       */
      void updateModel();

      /**
       * Handles when the mouse is pressed on a certain item.
       *
       * @param item item which the mouse pressed
       */
      void handleMousePress(QTreeWidgetItem *item);

    protected:
      /**
       * Current molecule for this view.
       */
      Molecule *m_molecule;
      /**
       * Groups of item widgets for our various primitive types.  Each group
       * is essentially a single QTreeWidgetItem which has as it's children
       * all the members of the group.
       */
      QVector<QTreeWidgetItem *> m_groups;
      
      /**
       * A helper function to generate our item strings based off the 
       * primitive information.
       *
       * @param primitive primitive to generate the string for
       * @return item string (ie. "Atom X", "Bond (x, y)")
       */
      QString primitiveToItemText(Primitive *primitive);

      /**
       * Return the index in our list for the primitive.  Causes a search
       * on all our items O(n).
       *
       * @param primitive primitive to find
       * @return index of the specified primitive or -1 if not found
       */
      int primitiveToItemIndex(Primitive *primitive);

      /**
       * Update a group of items.  Given a parent item it updates
       * all the children items.
       *
       * @param group the parent item of the group to update
       */
      void updateGroup(QTreeWidgetItem *group);

      /**
       * Update a single item.
       * 
       * @param item item to update
       */
      void updatePrimitiveItem(QTreeWidgetItem *item);

      /**
       * Add a new group for a primitive type.
       *
       * @param type the type of the group to add
       * @return the parent item for the group
       */
      QTreeWidgetItem* addGroup(enum Primitive::Type type);

      /**
       * Add a new group.
       *
       * @param name name of the group
       * @param type group type
       * @return the item which is the parent item for the group
       */
      QTreeWidgetItem* addGroup(QString name, enum Primitive::Type type);
      
    private:
      /**
       * Common constructor function.
       */
      void constructor();
  };

  /**
   * @internal Used by the MoleculeTreeView to draw items in the list.
   * @class MoleculeItemDelegate
   * @author Donald Ephraim Curtis
   */
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
