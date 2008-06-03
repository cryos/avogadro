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

#include "templateextension.h"

#include <QAction>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  // this is a trick to identify what action we are taking
  enum TemplateExtensionIndex {
    FirstAction = 0,
    SecondAction
  };

  TemplateExtension::TemplateExtension( QObject *parent ) : Extension( parent )  
  {
    // create an action for our second action
    QAction *action = new QAction( this );
    action->setText( tr("First Action" ));
    m_actions.append( action );
    action->setData( FirstAction );

    // create an action for our second action
    action = new QAction( this );
    action->setText( tr("Second Action" ));
    m_actions.append( action );
    action->setData( SecondAction );

  }

  TemplateExtension::~TemplateExtension()
  {
  }

  QList<QAction *> TemplateExtension::actions() const
  {
    return m_actions;
  }

  // allows us to set the intended menu path for each action
  QString TemplateExtension::menuPath(QAction *action) const
  {
    int i = action->data().toInt();

    switch ( i ) {
      case FirstAction:
        return tr("&Extensions") + '>' + tr("&Template");
        break;
      case SecondAction:
        return tr("&Edit") + '>' + tr("&Template");
        break;
    }
    return "";
  }

  QDockWidget * TemplateExtension::dockWidget()
  {
    // if we need a dock widget we can set one here
    return 0;
  }

  void TemplateExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* TemplateExtension::performAction(QAction *action, GLWidget *)
  {

    int i = action->data().toInt();

    switch ( i ) {
      case FirstAction:
        // perform first action
        break;
      case SecondAction:
        // perform second action
        break;
    }

    return 0;
  }

}

#include "templateextension.moc"

Q_EXPORT_PLUGIN2(templateextension, Avogadro::TemplateExtensionFactory)
