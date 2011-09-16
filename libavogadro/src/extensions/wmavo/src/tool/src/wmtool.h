
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

  Some portions :
  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Benoit Jacob

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
#ifndef __WMTOOL_H__
#define __WMTOOL_H__


#include "warning_disable_begin.h"
#include "wmavo_const.h"
#include "wiwo.h"

#include "settingswidget.h"
#include "drawobject.h"
#include "rendertext.h"
#include "distanceanglediedre.h"
#include "wmdevice.h"
#include "chemicalwrapper.h"
#include "wrapper_chemicalcmd_to_avoaction.h"
#include "contextmenu_to_avoaction.h"

#include <avogadro/tool.h>
#include <QAction>
#include <QMutex>
#include <QAtomicInt>
#include <QTime>

/*
QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE
//! [0]
*/

// For connect().
//qRegisterMetaType<...>("...") ; // before the connect method()
Q_DECLARE_METATYPE( Eigen::Vector3d )

#include "warning_disable_end.h"

namespace Avogadro
{
  /**
    * @class WmTool
    * @brief It serves many things : realize quick render, calculate for some feature ...
    *
    * But it is not here to manipulate the Wiimote. It is just to complet some lack in
    * the extension class (the paint() method, desactivate all mouse use ...).
    */
  class WmTool : public Tool
  {
    /**
      * @name Object Qt initialization.
      * @{ */
    Q_OBJECT
      AVOGADRO_TOOL( PLUGIN_WMTOOL_IDENTIFIER,
                     tr( PLUGIN_WMTOOL_NAME ),
                     tr( PLUGIN_WMTOOL_DESCR ),
                     tr( PLUGIN_WMTOOL_WIDGET_TITLE )
                     )
      // @}

  private :
    /**
      * @name Try to solve doxygen bug with the previous macro
      * Try to solve by add a no used method. This message does not appear in the doc.
      * @{ */
    void solveDoxygenBug(){} ;
    //@}


  //
  // Public methods.
  public:

    /** @name [Des|Cons]tructor methods
     * @{ */
    WmTool(QObject *parent = 0); //< Constructor.
    ~WmTool(); //< Destructor.
    // @}

    /** @name Tool Methods
     * @{
     * @brief Callback methods for ui.actions on the canvas.
     */
    virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);
    virtual QUndoCommand* keyPressEvent(GLWidget *widget, QKeyEvent *event);
    virtual QUndoCommand* keyReleaseEvent(GLWidget *widget, QKeyEvent *event);
    QWidget* settingsWidget() ;
    void settingsWidgetDestroyed() ;
    virtual int usefulness() const ;
    virtual bool paint(GLWidget *widget) ;
        ////< The paint() method of a Tool class to render in the render zone.
    //@}


  // Private methods.
  private :
    void initAllMainObject() ;
    void waitNewAction() ;
    void switchToThisTool( int state ) ;
    void connectSignals() ;

  // Public slots.
  public slots :
    void initAndStart( GLWidget *widget ) ;
    void applyActions() ;

    inline void setIRSensitiveNow( int sensitive )
    {
      if( m_chemWrap != NULL )
      {
        WITD::ChemicalWrapData_to chemDataTo ;
        chemDataTo.setIRSensitive(sensitive) ;
        m_chemWrap->setWrapperDataTo( chemDataTo ) ;
      }
    } ;

    inline void setWmVibrationNow( int enable )
    {
      if( m_wm != NULL )
      {
        InputDevice::WmDeviceData_to wmDataTo ;
        InputDevice::RumbleSettings rumble ;

        if( enable > 0 )
          rumble.setEnable( true ) ;
        else
          rumble.setEnable( false ) ;

        wmDataTo.setRumble( rumble ) ;
        m_wm->setDeviceDataTo( wmDataTo ) ;
      }
    } ;

    inline void setSleepThreadNow( int enable )
    {
      if( m_wm != NULL )
      {
        bool hasSleepThread=(enable>0?true:false) ;
        InputDevice::WmDeviceData_to wmDataTo ;
        WrapperInputToDomain::ChemicalWrapData_to chemWrapTo ;

        wmDataTo.setHasSleepThread(hasSleepThread) ;
        chemWrapTo.setHasSleepThread(hasSleepThread) ;

        m_wm->setDeviceDataTo( wmDataTo ) ;
        m_chemWrap->setWrapperDataTo( chemWrapTo ) ;
      }
    } ;

  // Signals.
  signals :
    void actionsApplied() ;

  // Private attributs.
  private :

    /**
      * Miscellaneous 
      * @{ */
    GLWidget *m_widget ; //< (shortcut)
    SettingsWidget *m_settingsWidget ; //< (object)
    // @}

    /**
      * Input device data.
      * @{ */
    InputDevice::WmDevice *m_wm ; //< (object)
    // @}

    /**
      * Wrapper device do domain.
      * @{ */
    WITD::ChemicalWrap *m_chemWrap ; //< (object)
    //@}

    /**
      * Avogadro command.
      * @{ */
    WrapperChemicalCmdToAvoAction *m_wrapperChemToAvo ; // (object)
    // @}

    /**
      * Apply Actions.
      * @{ */
    QAtomicInt m_updateActionDisplay, m_updateInfoDeviceDisplay ;
    //@}

    /**
      * Paint/Draw objects.
      * @{ */
    DrawObject *m_drawObject ; //< (object)
    RenderText *m_renderText ; //< (object)
    DistanceAngleDiedre *m_distAngleDiedre ; //< (object)
    // @}

    /**
      * Count nb actions by seconds (used with breakpoint).
      * @{ */
    WIWO<unsigned int> *m_nbUpdateTool, *m_nbUpdatePaint ;
    QTime m_time ;
    unsigned int m_t1Tool, m_t2Tool, m_t1Paint, m_t2Paint ;
    // @}

  };

  /**
   * @class WmToolFactory wmtool.h
   * @brief Factory class to create instances of the WmTool class.
   */
  class WmToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(WmTool)
  };

} // end namespace Avogadro

#endif
