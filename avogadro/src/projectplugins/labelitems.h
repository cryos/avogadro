/**********************************************************************
  LabelItems - Project Tree Items for labels.

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

#ifndef LABELITEMS_H
#define LABELITEMS_H

#include <avogadro/global.h>
#include "../projectplugin.h"
#include "../projecttreemodel.h"

#include <QString>

namespace Avogadro {

  class Primitive;

  class A_EXPORT LabelItems : public ProjectPlugin
  {
    Q_OBJECT

    public:
      LabelItems();
      ~LabelItems();

      QString name() const { return QObject::tr("Label"); }
      
      void setupModelData(ProjectTreeModel *, GLWidget *, ProjectTreeItem *parent);
      
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);

    private:
      ProjectTreeItem *m_label;
  };

} // end namespace Avogadro

#endif
