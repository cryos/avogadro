
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  Some portions :
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2006,2008,2009 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch

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

#include "wmextension.h"

namespace Avogadro
{

  /**
    * Constructor.
    * @param parent Instanciate and initiate by Avogadro
    */
  WmExtension::WmExtension(QObject *parent) :
    Extension(parent),
    m_widget(NULL), m_wmTool(NULL)
  {
    #if defined WIN32 || defined _WIN32
    mytoolbox::InitializeConsoleStdIO() ;
    puts( "Your message2" ) ;
    #endif

    // Initiate the Wiimote pull-down menu before IHM starts.
    initPullDownMenu() ;
  }


  /**
    * Destructor.
    */
  WmExtension::~WmExtension()
  {
  }


  /**
    * Define the attribut which contains all actions in the Wiimote pull-down menu.
    * @return Something uses by Avogadro, and, maybe describes in Avogadro documentation.
    */
  QList<QAction*> WmExtension::actions() const
  {
    return m_pullDownMenuActions;
  }


  /**
    * Indicates where the Wiimote menu bar is located in the Avogadro menu bar.
    * @return Something uses by Avogadro, and, maybe describes in Avogadro documentation.
    * @param action Instanciate and initiate by Avogadro
    */
  QString WmExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions") + '>' + tr("&Wiimote") /*+ '>' + tr("4nd WiimoteAppz")*/ ;
  }


  /**
    * Really, I do not know when it is called. I never see the cout() displays it.
    * @param event A mouse event ...
    */
  void WmExtension::mouseMoveEvent( QMouseEvent * /*event*/ )
  {
    //cout << "WmExtension::mouseMoveEvent : [" << event->globalX() << ";" << event->globalY() << "]" << endl ;
    //m_widget->update() ;
  }


  /**
    * Perform the action realises by the user through the action of clicking on the menu bar (on the path defined in menuPath() method).
    * @return Something uses by Avogadro, and, maybe describes in Avogadro documentation.
    * @param action The id of the action define in m_wmMenuState structure.
    * @param widget The GLWidget object instanciate by Avogadro
    */
  QUndoCommand* WmExtension::performAction( QAction *action, GLWidget *widget )
  {
    //cout << "WmExtension::performAction" << endl ;

    QUndoCommand *undo=NULL ;
    int menuAction = action->data().toInt() ;

    // Initiate some data if it is not realize again.
    if( m_widget == NULL )
    {
      m_widget = widget ;

      if( searchToolPlugin() )
        connect( this, SIGNAL(startWmTool(GLWidget*)), m_wmTool, SLOT(initAndStart( GLWidget*)) ) ;

      // Let the user choose the global quality in the option.
      // So let it in commentary.
      //m_widget->setQuality(-1) ; // PAINTER_MAX_DETAIL_LEVEL

      // If quick render activates, it can produce a visual bug. It is du in openGL code
      // optimisation which can no update some figure and blabla and blabla ...
      //m_widget->setQuickRender(true) ;
    }

    // Realize other initialization according to the case.
    switch( menuAction )
    {
    case ConnectWm :
      m_pullDownMenuActions.at(ConnectWm)->setText( tr("Re-Connect Wiimote") ) ;

      if( m_wmTool != NULL )
        emit startWmTool( widget ) ;

      //m_pullDownMenuActions.at(ConnectWm)->setEnabled(false) ;
      //m_pullDownMenuActions.at(DisconnectWm)->setEnabled(true) ;
      // See initPullDownMenu
      break ;

    case DisconnectWm :

      /* Dead battery bug !

          [INFO] Wiimote disconnected [id 1].
          --- DISCONNECTED ---
           [wiimote id 1]
          QThread: Destroyed while thread is still running
          0xbfd34848
          The Wiimote is not connected, so it can not be disconnected ...
          Erreur de segmentation
        */

      /*
      It is in commentary because the Wiimote library has some difficulties to this job ...
      Be careful if you erase commentary, there is a part in commentary in beginWiimoteUse() method.

      if( m_wmavoThread != NULL )
      {
        cout << "wmex::PerformAction case Disconnect 1" << endl ;
        m_wmavoThread->wmDisconnect() ;
        cout << "wmex::PerformAction case Disconnect 2" << endl ;

        m_pullDownMenuActions.at(ConnectWm)->setEnabled(true) ;
        m_pullDownMenuActions.at(DisconnectWm)->setEnabled(false) ;
      }
      */
      break ;

    /*
    // See initPullDownMenu
    case OpMode1 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE1) ;
      break ;

    case OpMode2 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE2) ;
      break ;

    case OpMode3 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE3) ;
      break ;
    */
    default :
      break ;
    }

    return undo;
  }


  /**
    * Initiate the pull-down menu before use.
    */
  void WmExtension::initPullDownMenu()
  {
    QAction *action=NULL ;
    m_pullDownMenuActions.clear() ;

    /*
    action = new QAction( this ) ;
    action->setSeparator( true ) ;
    m_pullDownMenuActions.append( action ) ;
    */

    action =  new QAction(this) ;
    action->setText( tr("Connect Wiimote") ) ;
    action->setData( ConnectWm ) ;
    m_pullDownMenuActions.append( action ) ;
/*
    action =  new QAction(this) ;
    action->setText( tr("Disconnect Wiimote") ) ;
    action->setData( DisconnectWm ) ;
    m_pullDownMenuActions.append( action ) ;

    action = new QAction( this ) ;
    action->setSeparator( true ) ;
    m_pullDownMenuActions.append( action ) ;

    action =  new QAction(this) ;
    action->setText( tr("Wm1 + Nc1") ) ;
    action->setData( OpMode1 ) ;
    m_pullDownMenuActions.append( action ) ;

    action =  new QAction(this) ;
    action->setText( tr("Wm1 + Nc2") ) ;
    action->setData( OpMode2 ) ;
    m_pullDownMenuActions.append( action ) ;

    action =  new QAction(this) ;
    action->setText( tr("Wm2 + Nc1") ) ;
    action->setData( OpMode3 ) ;
    m_pullDownMenuActions.append( action ) ;
    */
    
  }


  /**
    * Search the Tool plugin associated with this Extension plugin.
    */
  bool WmExtension::searchToolPlugin()
  {
    bool ret=false ;

    if( m_wmTool == NULL )
    {
      QString wmPluginName=PLUGIN_WMTOOL_NAME ;

      ToolGroup *tg=m_widget->toolGroup() ;
      Tool *t=NULL ;
      int nbTools=tg->tools().size() ;

      // Search the WmTool.
      for( int i=0 ; i<nbTools ; i++ )
      {
        t = tg->tool(i) ;

        if( t->name() == wmPluginName )
        {
          m_wmTool = t ;
          ret = true ;
          break ;
        }
      }
    }

    return ret ;
  }


} // end namespace Avogadro

Q_EXPORT_PLUGIN2(WmExtension, Avogadro::WmExtensionFactory)
