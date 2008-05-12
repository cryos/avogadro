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

#include "wiitrackextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <QMessageBox>


using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  #define toggle_bit(bf,b)        \
          (bf) = ((bf) & b)       \
                 ? ((bf) & ~(b))  \
                 : ((bf) | (b))
  
  //cwiid_mesg_callback_t cwiid_callback; /* callback function */
  static double WiimoteDot1x; 
  static double WiimoteDot1y; 
  static double WiimoteDot2x; 
  static double WiimoteDot2y; 
  
  enum WiiTrackExtensionIndex
  {
      StartIndex = 0,
      StopIndex
  };


  WiiTrackExtension::WiiTrackExtension(QObject *parent) : Extension(parent)
  {
    QAction *action;

    action = new QAction(this);
    action->setText(tr("Connect"));
    action->setData(StartIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Disconnect"));
    action->setData(StopIndex);
    action->setEnabled(false);
    m_actions.append(action);
    
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(redraw()));
  }

  WiiTrackExtension::~WiiTrackExtension()
  {
  }

  QList<QAction *> WiiTrackExtension::actions() const
  {
    return m_actions;
  }

  QString WiiTrackExtension::menuPath(QAction *) const
  {
    return tr("&Extensions") + '>' + tr("&WiiTrack");
  }

  QUndoCommand* WiiTrackExtension::performAction(QAction *action,
      Molecule *molecule, GLWidget *widget, QTextEdit *)
  {
    int i = action->data().toInt();
    
    m_widget = widget;    
 
    // dispatch to the appropriate method for that wiitrackion command
    switch (i) {
    case StartIndex:
      cwiidConnect();
      m_timer->start(50); 
      break;
    case StopIndex:
      cwiidDisconnect();
      m_timer->stop(); 
      break;
    default:
      break;
    }

    // WiiTracks are per-view and as such are not saved or undo-able
    return NULL;
  }

  void WiiTrackExtension::cwiidConnect()
  {
    bdaddr_t bdaddr;        /* bluetooth device address */
    bdaddr = *BDADDR_ANY;
    
    // connect
    if (!(m_wiimote = cwiid_connect(&bdaddr, 0))) {
      qDebug() << "Unable to connect to wiimote";
      return;
    } else {
      qDebug() << "Connected to wiimote";
      m_actions.at(StartIndex)->setEnabled(false);
      m_actions.at(StopIndex)->setEnabled(true);
    }
    
    // Set the callback function
    if (cwiid_set_mesg_callback(m_wiimote, cwiid_callback)) {
      qDebug() << "Unable to set message callback";
      cwiidDisconnect(); 
    }
    
    // set the report mode
    unsigned char rpt_mode = 0;
    toggle_bit(rpt_mode, CWIID_RPT_IR);
    cwiidSetReportMode(m_wiimote, rpt_mode);
    
    // enable messaging
    if (cwiid_enable(m_wiimote, CWIID_FLAG_MESG_IFC)) {
      qDebug() << "Error enabling messages";
    }
      
    m_lastDistance = 0.0;
    m_lastDot1x = 0.0;
    m_lastDot1y = 0.0;
    m_lastDot2x = 0.0;
    m_lastDot2y = 0.0;
  }
  
  void WiiTrackExtension::cwiidDisconnect()
  {
    if (cwiid_disconnect(m_wiimote)) {
      qDebug << "Error on wiimote disconnect";
      return;
    } else {
      qDebug() << "Disconnected from wiimote";
      m_actions.at(StartIndex)->setEnabled(true);
      m_actions.at(StopIndex)->setEnabled(false);
    }
  }
  
  void WiiTrackExtension::cwiidSetReportMode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
  {
    if (cwiid_command(wiimote, CWIID_CMD_RPT_MODE, rpt_mode)) {
      qDebug() << "Error setting report mode";
    }
  }
  
  void WiiTrackExtension::redraw()
  {
    if ((WiimoteDot1x == 0) || (WiimoteDot1y == 0) || (WiimoteDot2x == 0) || (WiimoteDot2y == 0))
      return;

    if (m_lastDot1x == 0.0) {
      m_lastDot1x = WiimoteDot1x;
      return;
    }
    double angle = 0.003 * (m_lastDot1x - WiimoteDot1x);
    m_lastDot1x = WiimoteDot1x;
    
    if (m_lastDot1y == 0.0) {
      m_lastDot1y = WiimoteDot1y;
      return;
    }
    double angle2 = 0.003 * (m_lastDot1y - WiimoteDot1y);
    m_lastDot1y = WiimoteDot1y;
   
    Eigen::Vector3d center(0., 0., 0.);
    double sumOfWeights = 0.;
    std::vector<OpenBabel::OBNodeBase*>::iterator i;
    for ( Atom *atom = static_cast<Atom*>(m_widget->molecule()->BeginAtom(i));
          atom; atom = static_cast<Atom*>(m_widget->molecule()->NextAtom(i))) {
      Eigen::Vector3d transformedAtomPos = m_widget->camera()->modelview() * atom->pos();
      double atomDistance = transformedAtomPos.norm();
      double dot = transformedAtomPos.z() / atomDistance;
      double weight = exp(-30. * (1. + dot));
      sumOfWeights += weight;
      center += weight * atom->pos();
    }
    center /= sumOfWeights;
 
    Eigen::Vector3d xAxis = m_widget->camera()->backTransformedXAxis();
    Eigen::Vector3d yAxis = m_widget->camera()->backTransformedYAxis();
    m_widget->camera()->translate(center);
    m_widget->camera()->rotate(-angle, yAxis);
    m_widget->camera()->rotate(-angle2, xAxis);
    m_widget->camera()->translate(-center);
    
    // zoom based on distance between dots
    int deltaX = WiimoteDot2x - WiimoteDot1x;
    int deltaY = WiimoteDot2y - WiimoteDot1y;
    Eigen::Vector3d zAxis(0,0,1);
    double distance = sqrt(deltaX*deltaX + deltaY*deltaY);

    if (m_lastDistance == 0.0) {
      m_lastDistance = distance;
      return;
    }
    double delta = 0.1 * (m_lastDistance - distance);
    m_lastDistance = distance;

    m_widget->camera()->modelview().pretranslate( - delta * ( CAMERA_NEAR_DISTANCE ) * zAxis );
    m_widget->update();  
  }

  void WiiTrackExtension::cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count, union cwiid_mesg mesg[])
  { 
    int valid_source = 0;
    WiimoteDot1x = 0;
    WiimoteDot1y = 0;
    WiimoteDot2x = 0;
    WiimoteDot2y = 0;
    for (int i = 0; i < mesg_count; i++) {
      switch (mesg[i].type) {
        case CWIID_MESG_IR:
          printf("IR Report: ");
          for (int j = 0; j < CWIID_IR_SRC_COUNT; j++) {
            if (mesg[i].ir_mesg.src[j].valid) {
              valid_source = 1;
              printf("(%d,%d) ", mesg[i].ir_mesg.src[j].pos[CWIID_X],
                                 mesg[i].ir_mesg.src[j].pos[CWIID_Y]);
              if (j == 0) {
                WiimoteDot1x = mesg[i].ir_mesg.src[j].pos[CWIID_X];
                WiimoteDot1y = mesg[i].ir_mesg.src[j].pos[CWIID_Y];
              } else if (j == 1) {
                WiimoteDot2x = mesg[i].ir_mesg.src[j].pos[CWIID_X];
                WiimoteDot2y = mesg[i].ir_mesg.src[j].pos[CWIID_Y];
              }
            }
          }
          if (!valid_source) {
            printf("no sources detected");
          }
          printf("\n");
          break;
        case CWIID_MESG_ERROR:
          if (cwiid_disconnect(wiimote)) {
            printf("Error on wiimote disconnect\n");
          }
          break;
        default:
          printf("Unknown Report");
          break;
      }
    }
  }






















} // end namespace Avogadro

#include "wiitrackextension.moc"
Q_EXPORT_PLUGIN2(wiitrackextension, Avogadro::WiiTrackExtensionFactory)
