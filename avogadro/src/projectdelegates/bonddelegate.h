/**********************************************************************
  BondDelegate - Project Tree Items for bonds.

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

#ifndef BONDDELEGATE_H
#define BONDDELEGATE_H

#include <avogadro/global.h>
#include "../projecttreemodeldelegate.h"
#include "../projecttreemodel.h"

#include <QString>

namespace Avogadro {

  class Primitive;

  class BondDelegate : public ProjectTreeModelDelegate
  {
    Q_OBJECT

    public:
      BondDelegate(ProjectTreeModel *model);
      ~BondDelegate();

      QString name() const { return QObject::tr("Bonds"); }
      
      void initStructure(GLWidget *, ProjectTreeItem *parent);
      
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);
      
      void fetchMore(ProjectTreeItem *parent);
    
    public slots:
      void primitiveAdded(Primitive*);
      void primitiveUpdated(Primitive*);
      void primitiveRemoved(Primitive*);

    private:
      void initialize();

      ProjectTreeItem  *m_label;
      GLWidget         *m_widget;
  };

} // end namespace Avogadro

#endif
