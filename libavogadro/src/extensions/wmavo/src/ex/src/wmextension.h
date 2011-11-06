
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

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

#pragma once
#ifndef __WMEXTENSION_H__
#define __WMEXTENSION_H__

#include "warning_disable_begin.h"
#include "variousfeatures.h"
#include "wmavo_const.h"
#include <avogadro/glwidget.h>
#include <avogadro/extension.h>
#include <avogadro/tool.h>
#include <avogadro/toolgroup.h>
#include <QAction>
#include "warning_disable_end.h"


namespace Avogadro
{
  /**
    * @class WmExtension
    * @brief
    */
  class WmExtension : public Extension
  {
    /**
      * @name Object Qt initialization.
      * @{ */
    Q_OBJECT
      AVOGADRO_EXTENSION("WmExtension", tr("WmExtension"),
                         tr("Launch the WmTool plugin to start the Wiimote."))
      // @}


      // Signal.
    signals :
      void startWmTool( GLWidget *widget ) ;
     
    // Public methods.
    public:
      /**
        * @name Avogadro default methods
        * @{ */
      WmExtension(QObject *parent=0) ; ///< Constructor
      ~WmExtension() ; ///< Destructor

      virtual QList<QAction*> actions() const ;
      virtual QString menuPath(QAction *action) const;
      virtual void mouseMoveEvent( QMouseEvent *event ) ;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget) ;
      //@}

    // Private methods.
    private :
      void initPullDownMenu() ; ///< To manipulate the pull-down menu in the Avogadro menu bar.
      bool searchToolPlugin() ;     

    // Private attributs.
    private:
      /** Use in the pull-down menu to represent the actions 
        * of each button of the Wiimote menu. */
      enum m_wmMenuState
      {
          ConnectWm = 0, DisconnectWm,
          OpMode1, OpMode2, OpMode3
      } ;

      GLWidget *m_widget ; ///< (shortcut)
      Tool *m_wmTool ; ///< (shortcut)
      QList<QAction*> m_pullDownMenuActions ;
  };


  /**
   * @class WmExtensionFactory wmextension.h
   * @brief Factory class to create instances of the WmExtension class.
   */
  class WmExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(WmExtension)
  };

} // end namespace Avogadro

#endif

