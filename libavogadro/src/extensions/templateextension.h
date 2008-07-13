/**********************************************************************
  Template - Extension Template

  Copyright (C) 2008 by Author

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

#ifndef TEMPLATEEXTENSION_H
#define TEMPLATEEXTENSION_H

#include <avogadro/extension.h>
#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

namespace Avogadro {

  class TemplateExtension : public Extension
  {
    Q_OBJECT

    public:
      //! Constructor
      TemplateExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~TemplateExtension();

      virtual QString name() const { return QObject::tr("Template"); }
      virtual QString description() const { return QObject::tr("Template for developing extensions"); };

      virtual QList<QAction *> actions() const;
      virtual QString menuPath(QAction *action) const;

      virtual QDockWidget * dockWidget();
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

      virtual void setMolecule(Molecule *molecule);

    private:
      QList<QAction *> m_actions;
      Molecule *m_molecule;

    private Q_SLOTS:

  };

  class TemplateExtensionFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::ExtensionFactory)

    AVOGADRO_EXTENSION_FACTORY(TemplateExtension, 
        tr("Template Extension"),
        tr("Template Extension Example"));
  };

} // end namespace Avogadro

#endif
