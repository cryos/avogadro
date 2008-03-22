/**********************************************************************
  WiiTrack - Wiimote head tracking extension

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

#ifndef __WIITRACKEXTENSION_H
#define __WIITRACKEXTENSION_H

#include <avogadro/extension.h>


#include <cwiid.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>

namespace Avogadro {

 class WiiTrackExtension : public Extension
  {
    Q_OBJECT
    
   public slots:
     void redraw(); 
   public:
      //! Constructor
      WiiTrackExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~WiiTrackExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("WiiTrack"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("WiiTrack Plugin"); };
      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, Molecule *molecule,
                                          GLWidget *widget, QTextEdit *messages=NULL);
      virtual QString menuPath(QAction *action) const;
      //@}
      
      
    private:
      QList<QAction *> m_actions;
      GLWidget *m_widget;
      QTimer *m_timer;
      
      // cwiid stuff
      cwiid_wiimote_t *m_wiimote;       /* wiimote handle */
      void cwiidConnect();
      void cwiidDisconnect();
      void cwiidSetReportMode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
      static cwiid_mesg_callback_t cwiid_callback; /* callback function */
       
      double m_lastDistance; 
      double m_lastDot1x; 
      double m_lastDot1y; 
      double m_lastDot2x; 
      double m_lastDot2y; 
  };

  class WiiTrackExtensionFactory : public QObject, public ExtensionFactory
  {
    Q_OBJECT;
    Q_INTERFACES(Avogadro::ExtensionFactory);

    public:
    Extension *createInstance(QObject *parent = 0) { return new WiiTrackExtension(parent); }
  };

} // end namespace Avogadro

#endif
