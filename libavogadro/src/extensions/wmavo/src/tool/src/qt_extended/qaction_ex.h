
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  This file is part of WmAvo (WiiChem project)
  WmAvo - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmAvo is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmAvo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmAvo. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#pragma once
#ifndef __QACTION_EX_H__
#define __QACTION_EX_H__

#include "warning_disable_begin.h"
#include <QObject>
#include <QWidget>
#include <QAction>
#include "warning_disable_end.h"

class QAction_ex : public QAction
{

  Q_OBJECT

signals :

  void triggeredInfo( QString info ) ;

private slots :

  void triggerInfo()
  {
    emit triggeredInfo( statusTip() ) ;
  } ;

public :

  QAction_ex( QObject * parent ) : QAction( parent )
  {
    connect( this, SIGNAL(triggered()), this, SLOT(triggerInfo()) ) ;
  };

  QAction_ex( const QString & text, QObject * parent ) : QAction( text, parent )
  {
    connect( this, SIGNAL(triggered()), this, SLOT(triggerInfo()) ) ;
  };

  QAction_ex( const QIcon & icon, const QString & text, QObject * parent )
    : QAction( icon, text, parent )
  {
    connect( this, SIGNAL(triggered()), this, SLOT(triggerInfo()) ) ;
  };

  QAction_ex( const QAction_ex& am ) : QAction(am.parent())
  {
    connect( this, SIGNAL(triggered()), this, SLOT(triggerInfo()) ) ;
  };

} ;

#endif