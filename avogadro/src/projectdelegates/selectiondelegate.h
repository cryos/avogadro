/**********************************************************************
  SelectionDelegate - Project Tree Item for user selections.

  Copyright (C) 2009 by Tim Vandermeersch

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

#ifndef SELECTIONDELEGATE_H
#define SELECTIONDELEGATE_H

#include <avogadro/global.h>
#include "../projecttreemodeldelegate.h"
#include "../projecttreemodel.h"

#include <QString>

namespace Avogadro {

  class Primitive;

  class SelectionDelegate : public ProjectTreeModelDelegate
  {
    Q_OBJECT

    public:
      SelectionDelegate(ProjectTreeModel *model);
      ~SelectionDelegate();

      QString name() const { return QObject::tr("User Selections"); }
      
      void initStructure(GLWidget *widget, ProjectTreeItem *parent);
      
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);
      
      void fetchMore(ProjectTreeItem *parent);

    public slots:
      void initialize();

    private:
      ProjectTreeItem  *m_label;
      ProjectTreeModel *m_model;
      GLWidget         *m_widget;
  };

} // end namespace Avogadro

#endif
