/**********************************************************************
  ClickMeasureTool - ClickMeasureTool Tool for Avogadro

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "clickmeasuretool.h"

#include <avogadro/navigate.h>
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <openbabel/generic.h>

#include <math.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  ClickMeasureTool::ClickMeasureTool(QObject *parent) : Tool(parent),
    m_selectedAtoms(4), m_numSelectedAtoms(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/measure/measure.png")));
    action->setToolTip(tr("Click to Measure (F12)\n\n"
          "Left Mouse: \tSelect up to three Atoms.\n"
          "\tDistances are measured between 1-2 and 2-3\n"
          "\tAngle is measured between 1-3 using 2 as the common point\n"
          "Right Mouse: Reset the measurements."));
    action->setShortcut(Qt::Key_F12);
    m_lastMeasurement.resize(5);
    for (int i = 0; i < m_lastMeasurement.size(); ++i)
      m_lastMeasurement[i] = 0.0;
  }

  ClickMeasureTool::~ClickMeasureTool()
  {
  }

  QUndoCommand* ClickMeasureTool::mousePress(GLWidget *widget, const QMouseEvent *event)
  {
    Molecule *molecule = widget->molecule();
    if(!molecule) {
      return 0;
    }

    //! List of hits from initial click
    m_hits = widget->hits(event->pos().x()-2, event->pos().y()-2, 5, 5);

    // If there's a left button (and no modifier keys) continue adding to the list
    if(m_hits.size() && (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::NoModifier))
    {
      if(m_hits[0].type() != Primitive::AtomType)
        return 0;

      Atom *atom = (Atom *)molecule->GetAtom(m_hits[0].name());

      if(m_numSelectedAtoms < 4) {
        // Select another atom
        m_selectedAtoms[m_numSelectedAtoms++] = atom;
        widget->update();
      }
    }
    // Right button or Left Button + modifier (e.g., Mac)
    else
    {
      m_angle = 0;
      m_vector[0].loadZero();
      m_vector[1].loadZero();
      m_numSelectedAtoms = 0;
      widget->update();
    }
    return 0;
  }

  QUndoCommand* ClickMeasureTool::mouseMove(GLWidget*, const QMouseEvent *)
  {
    return 0;
  }

  QUndoCommand* ClickMeasureTool::mouseRelease(GLWidget*, const QMouseEvent*)
  {
    return 0;
  }

  QUndoCommand* ClickMeasureTool::wheel(GLWidget*widget, const QWheelEvent*event)
  {
    // let's set the reference to be the center of the visible
    // part of the molecule.
    Eigen::Vector3d atomsBarycenter(0., 0., 0.);
    double sumOfWeights = 0.;
    std::vector<OpenBabel::OBNodeBase*>::iterator i;
    for ( Atom *atom = static_cast<Atom*>(widget->molecule()->BeginAtom(i));
          atom; atom = static_cast<Atom*>(widget->molecule()->NextAtom(i))) {
      Eigen::Vector3d transformedAtomPos = widget->camera()->modelview() * atom->pos();
      double atomDistance = transformedAtomPos.norm();
      double dot = transformedAtomPos.z() / atomDistance;
      double weight = exp(-30. * (1. + dot));
      sumOfWeights += weight;
      atomsBarycenter += weight * atom->pos();
    }
    atomsBarycenter /= sumOfWeights;

    Navigate::zoom(widget, atomsBarycenter, - MOUSE_WHEEL_SPEED * event->delta());
    widget->update();

    return NULL;
  }

  void ClickMeasureTool::calculateParameters()
  {
    // Calculate all parameters and store them in member variables.
    if(m_numSelectedAtoms >= 2)
    {
      // Check the selected atoms still exist
      if (m_selectedAtoms[0].isNull() || m_selectedAtoms[1].isNull())
        return;

      // Two atoms selected - distance measurement only
      m_vector[0] = m_selectedAtoms[1]->pos() - m_selectedAtoms[0]->pos();
      QString distanceString = tr("Distance (1->2): %1 %2").arg(
                                QString::number(m_vector[0].norm()),
                                QString::fromUtf8("Å"));

      // Check whether we have already sent this out...
      if (m_lastMeasurement.at(0) != m_vector[0].norm()) {
        emit message(distanceString);
        m_lastMeasurement[0] = m_vector[0].norm();
      }
    }
    if(m_numSelectedAtoms >= 3)
    {
      // Check the selected atoms still exist
      if (m_selectedAtoms[2].isNull())
        return;

      // Two distances and the angle between the three selected atoms
      m_vector[1] = m_selectedAtoms[1]->pos() - m_selectedAtoms[2]->pos();
      QString distanceString = tr("Distance (2->3): %1 %2").arg(
                               QString::number(m_vector[1].norm()),
                               QString::fromUtf8("Å"));

      // Calculate the angle between the atoms
      m_angle = vectorAngle(vector3(m_vector[0].x(), m_vector[0].y(), m_vector[0].z()),
      		  				vector3(m_vector[1].x(), m_vector[1].y(), m_vector[1].z()));
      QString angleString = tr("Angle: %1 %2").arg(
                            QString::number(m_angle),
                            QString("°"));

      // Check whether we have already sent this out
      if (m_lastMeasurement.at(1) != m_vector[1].norm()) {
        emit message(distanceString);
        m_lastMeasurement[1] = m_vector[1].norm();
      }
      if (m_lastMeasurement.at(3) != m_angle) {
        emit message(angleString);
        m_lastMeasurement[3] = m_angle;
      }
    }
    if(m_numSelectedAtoms >= 4)
    {
      // Check the selected atoms still exist
      if (m_selectedAtoms[3].isNull())
        return;

      // Three distances, bond angle and dihedral angle
      m_vector[2] = m_selectedAtoms[2]->pos() - m_selectedAtoms[3]->pos();
      QString distanceString = tr("Distance (3->4): %1 %2").arg(
                                QString::number(m_vector[2].norm()),
                                QString::fromUtf8("Å"));
      m_dihedral = CalcTorsionAngle(vector3(m_selectedAtoms[0]->pos().x(),
                                m_selectedAtoms[0]->pos().y(),
                                m_selectedAtoms[0]->pos().z()),
                                vector3(m_selectedAtoms[1]->pos().x(),
                                m_selectedAtoms[1]->pos().y(),
                                m_selectedAtoms[1]->pos().z()),
                                vector3(m_selectedAtoms[2]->pos().x(),
                                m_selectedAtoms[2]->pos().y(),
                                m_selectedAtoms[2]->pos().z()),
                                vector3(m_selectedAtoms[3]->pos().x(),
                                m_selectedAtoms[3]->pos().y(),
                                m_selectedAtoms[3]->pos().z()));
      QString dihedralString = tr("Dihedral Angle: %1 %2").arg(
                                QString::number(m_dihedral),
                                QString("°"));

      // Check whether these measurements have been sent already
      if (m_lastMeasurement.at(2) != m_vector[2].norm()) {
        emit message(distanceString);
        m_lastMeasurement[2] = m_vector[2].norm();
      }
      if (m_lastMeasurement.at(4) != m_dihedral) {
        emit message(dihedralString);
        m_lastMeasurement[4] = m_angle;
      }
    }
  }

  bool ClickMeasureTool::paint(GLWidget *widget)
  {
    if(0 < m_numSelectedAtoms && m_selectedAtoms[0])
    {
      calculateParameters();

      // Try to put the labels in a reasonable place on the display
      QPoint labelPos(95, widget->height()-25);
      QPoint distancePos[3];
      distancePos[0] = QPoint(180, widget->height()-25);
      distancePos[1] = QPoint(240, widget->height()-25);
      distancePos[2] = QPoint(300, widget->height()-25);

      QPoint angleLabelPos(95, widget->height()-45);
      QPoint anglePos(180, widget->height()-45);

      QPoint dihedralLabelPos(95, widget->height()-65);
      QPoint dihedralPos(180, widget->height()-65);

      glColor3f(1.0,0.0,0.0);
      Vector3d pos = m_selectedAtoms[0]->pos();
      double radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[0]->GetAtomicNum()) * 0.3 ;

      Vector3d xAxis = widget->camera()->backTransformedXAxis();
      Vector3d zAxis = widget->camera()->backTransformedZAxis();

      // relative position of the text on the atom
      Vector3d textRelPos = radius * (zAxis + xAxis);

      Vector3d textPos = pos+textRelPos;
      widget->painter()->drawText(textPos, tr("*1", "*1 is a number. You most likely do not need to translate this" ));

      if(m_numSelectedAtoms >= 2 && m_selectedAtoms[1])
      {
        glColor3f(0.0,1.0,0.0);
        pos = m_selectedAtoms[1]->pos();
        Vector3d textPos = pos+textRelPos;
        radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[1]->GetAtomicNum()) * 0.3;
        widget->painter()->drawText(textPos, tr("*2", "*2 is a number. You most likely do not need to translate this"));

        if(m_numSelectedAtoms >= 3 && m_selectedAtoms[2])
        {
          // Display a label on the third atom
          pos = m_selectedAtoms[2]->pos();
          radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[2]->GetAtomicNum()) * 0.3;
          textPos = pos+textRelPos;
          glColor3f(0.0,0.0,1.0);
          widget->painter()->drawText(textPos, tr("*3", "*3 is a number. You most likely do not need to translate this"));
        }
        if(m_numSelectedAtoms >= 4 && m_selectedAtoms[3])
        {
          // Display a label on the fourth atom
          pos = m_selectedAtoms[3]->pos();
          radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[3]->GetAtomicNum()) * 0.3;
          textPos = pos + textRelPos;
          glColor3f(0.0,1.0,1.0);
          widget->painter()->drawText(textPos, tr("*4", "*4 is a number. You most likely do not need to translate this"));
        }
        //       glLoadIdentity();
        glColor3f(1.0,1.0,1.0);
        widget->painter()->drawText(labelPos, tr("Distance(s):"));

        glColor3f(1.0,1.0,0.0);
        widget->painter()->drawText(distancePos[0], QString::number(m_vector[0].norm(), 10, 2) + QString::fromUtf8(" Å"));

        if(m_numSelectedAtoms >= 3)
        {
          glColor3f(1.0,1.0,1.0);
          widget->painter()->drawText(angleLabelPos, QString("Angle:"));

          glColor3f(0.8, 0.8, 0.8);
          widget->painter()->drawText(anglePos, QString::number(m_angle, 10, 1) + QString::fromUtf8("°"));

          glColor3f(0.0,1.0,1.0);
          widget->painter()->drawText(distancePos[1], QString::number(m_vector[1].norm(), 10, 2) + QString::fromUtf8(" Å"));
        }

        if(m_numSelectedAtoms >= 4)
        {
          glColor3f(1.0, 1.0, 1.0);
          widget->painter()->drawText(dihedralLabelPos, QString("Dihedral:"));

          glColor3f(0.6, 0.6, 0.6);
          widget->painter()->drawText(dihedralPos, QString::number(m_dihedral, 10, 1) + QString::fromUtf8("°"));

          glColor3f(1.0, 1.0, 1.0);
          widget->painter()->drawText(distancePos[2], QString::number(m_vector[2].norm(), 10, 2) + QString::fromUtf8(" Å"));
        }


        // If there are three atoms selected, draw the angle in question
        if(m_numSelectedAtoms >= 3 && m_selectedAtoms[0] && m_selectedAtoms[1]
          && m_selectedAtoms[2])
        {
          Vector3d origin = m_selectedAtoms[1]->pos();
          Vector3d d1 = m_selectedAtoms[0]->pos() - origin;
          Vector3d d2 = m_selectedAtoms[2]->pos() - origin;
          // The vector length is half the average vector length
          double radius = (d1.norm()+d2.norm()) * 0.25;
          // Adjust the length of u and v to the length calculated above.
          d1 = (d1 / d1.norm()) * radius;
          d2 = (d2 / d2.norm()) * radius;
          if (m_angle < 1) return true;
          // Vector perpindicular to both d1 and d2
          Vector3d n = d1.cross(d2);

          Vector3d xAxis = Vector3d(1, 0, 0);
          Vector3d yAxis = Vector3d(0, 1, 0);

          if (n.norm() < 1e-16)
          {
            Eigen::Vector3d A = d1.cross(xAxis);
            Eigen::Vector3d B = d1.cross(yAxis);

            n = A.norm() >= B.norm() ? A : B;
          }

          n = n / n.norm();
          glEnable(GL_BLEND);
          glDepthMask(GL_FALSE);
          widget->painter()->setColor(0, 1.0, 0, 0.3);
          widget->painter()->drawShadedSector(origin, m_selectedAtoms[0]->pos(),
                                             m_selectedAtoms[2]->pos(), radius);
          glDepthMask(GL_TRUE);
          glDisable(GL_BLEND);
          widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
          widget->painter()->drawArc(origin, m_selectedAtoms[0]->pos(),
                                     m_selectedAtoms[2]->pos(), radius, 1.0);
        }
      }
    }

    return true;
  }
}

#include "clickmeasuretool.moc"

Q_EXPORT_PLUGIN2(clickmeasuretool, Avogadro::ClickMeasureToolFactory)
