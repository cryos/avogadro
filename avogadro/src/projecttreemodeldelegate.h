/**********************************************************************
  selectionitem.h - ProjectItem for named selections.

  Copyright (C) 2008 by Tim Vandermeersch

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

#ifndef PROJECTTREEMODELDELEGATE_H
#define PROJECTTREEMODELDELEGATE_H

#include <avogadro/global.h>

#include "projecttreemodel.h"

#include <QSettings>

namespace Avogadro {

  class GLWidget;

  class ProjectTreeModelDelegatePrivate;   
  class ProjectTreeModelDelegate : public QObject
  {
    Q_OBJECT

    public:
      ProjectTreeModelDelegate(ProjectTreeModel *model);
      /**
       * The name for this project tree model delegate
       */
      virtual QString name() const = 0;
      /**
       * Add the tree items for this project tree model delegate to parent
       */
//      virtual void setupModelData(ProjectTreeModel *model, GLWidget *widget, ProjectTreeItem *parent) = 0;
      virtual void initStructure(GLWidget *widget, ProjectTreeItem *parent) = 0;
      /**
       * @return a QWidget containing the settings or 0
       * if no settings widget is available.
       */
      virtual QWidget *settingsWidget();
      /**
       * Set the alias for this project plugin
       */
      void setAlias(const QString &alias);
      /**
       * @return The alias for this project plugin.
       */
      QString alias() const;
      /**
       * Write the engine settings so that they can be saved between sessions.
       */
      virtual void writeSettings(QSettings &settings) const;
      /**
       * Read in the settings that have been saved for the engine instance.
       */
      virtual void readSettings(QSettings &settings);
      /**
       * @return The ProjectTreeModel for this delegate.
       */
      ProjectTreeModel* model() const;

      /**
       * Insert a new expandable item at the end of parent. This item is also stored
       * so that it can be retrieved by the ProjectTreeModel to if determine the user
       * just expanded this item. If so, the model will call 
       * ProjectTreeModelDelegate::fetchMore(ProjectTreeItem*) and we can further 
       * initialize the data and connect signals if needed.
       *
       * Note: these are never removed, if a new tree structure is set in the editor,
       * the current model is deleted (along with all its delegates and expandable items)
       * and a new model is created.
       */
      ProjectTreeItem* insertExpandableItem(ProjectTreeItem *parent);
      /**
       * @return true if this delegate has inserted expandable item @p parent.
       */
      bool hasExpandableItem(ProjectTreeItem *parent) const;
      /**
       * Do the actual model initialization for parent (create the children)
       */
      virtual void fetchMore(ProjectTreeItem *) {}

      /**
       * Some delegates may delegate their work to other delegates. However, to keep the
       * model informed about all the delegates, you need to call exportDelegate once you 
       * created them. This will also steal the pointer, you don't need to delete them.
       *
       * See MoleculeDelegate for example.
       */
      void exportDelegate(ProjectTreeModelDelegate *delegate);

    private:
      ProjectTreeModelDelegatePrivate * const d; 
  };
 
} // end namespace Avogadro

#endif
