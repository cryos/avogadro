/**********************************************************************
  GAMESS - GAMESS Input Deck Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

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

#include "gamessextension.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>

#include <QDockWidget>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <QtGui>
#include <QFrame>
#include <QSpacerItem>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  enum GamessExtensionIndex {
    InputDeckAction = 0,
    EFPAction,
    QMAction
  };

  GamessExtension::GamessExtension( QObject *parent ) : QObject( parent ), m_inputDialog( NULL ), m_inputData( NULL ), m_dockWidget( 0 )
  {
    Action *action = new Action( this );
    action->setText( "Input Deck Generator" );
    action->setMenuPath( "&Tools>GAMESS" );
    m_actions.append( action );
    action->setData( InputDeckAction );

    action = new Action( this );
    action->setText( "EFP Selection" );
    action->setMenuPath( "&Tools>GAMESS" );
    m_actions.append( action );
    action->setData( EFPAction );

    action = new Action( this );
    action->setText( "QM Selection" );
    action->setMenuPath( "&Tools>GAMESS" );
    m_actions.append( action );
    action->setData( QMAction );
  }

  GamessExtension::~GamessExtension()
  {}

  QList<Action *> GamessExtension::actions() const
  {
    return m_actions;
  }

  QDockWidget * GamessExtension::dockWidget()
  {
    if ( !m_dockWidget ) {
      m_dockWidget = new QDockWidget( tr( "GAMESS EFP Information" ) );

      QWidget *widget = new QWidget( m_dockWidget );
      QVBoxLayout *layout = new QVBoxLayout();

      QTreeView *view= new QTreeView();
      layout->addWidget( view );

      widget->setLayout( layout );
      m_dockWidget->setWidget( widget );

      connect( m_dockWidget, SIGNAL( destroyed() ), this, SLOT( dockWidgetDestroyed ) );
    }

    return m_dockWidget;
  }

  void GamessExtension::dockWidgetDestroyed()
  {
    m_dockWidget = 0;
  }

  QUndoCommand* GamessExtension::performAction( Action *action, Molecule *molecule, GLWidget *widget, QTextEdit * )
  {

    qDebug() << "Perform Action";
    int i = action->data().toInt();
    switch ( i ) {
      case InputDeckAction:
        if ( !m_inputData ) {
          m_inputData = new GamessInputData( molecule );
        } else {
          m_inputData->SetMolecule( molecule );
        }
        if ( !m_inputDialog ) {
          m_inputDialog = new GamessInputDialog( m_inputData );
          m_inputDialog->show();
        } else {
          m_inputDialog->setInputData( m_inputData );
          m_inputDialog->show();
        }
        break;
      case EFPAction:
      case QMAction:
        findMatches(molecule, widget);
        break;
    }

    return 0;
  }

  void GamessExtension::findMatches(Molecule *molecule, GLWidget *widget)
  {
    qDebug() << "Find Matches";
  }

}

#include "gamessextension.moc"

Q_EXPORT_PLUGIN2( gamessextension, Avogadro::GamessExtensionFactory )
