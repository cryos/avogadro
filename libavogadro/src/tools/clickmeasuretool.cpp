/**********************************************************************
  ClickMeasureTool - ClickMeasureTool Tool for Avogadro

  Copyright (C) 2007 Donald Ephraim Curtis

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
#include "navigate.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>

#include <math.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  ClickMeasureTool::ClickMeasureTool(QObject *parent) : Tool(parent),  m_numSelectedAtoms(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/measure/measure.png")));
    action->setToolTip(tr("Click to Measure (F12)\n\n"
          "Left Mouse: \tSelect up to three Atoms.\n"
          "\tDistances are measured between 1-2 and 2-3\n"
          "\tAngle is measured between 1-3 using 2 as the common point\n"
          "Right Mouse: Reset the measurements."));
    action->setShortcut(Qt::Key_F12);

    // clear the selected atoms
    int size = m_selectedAtoms.size();
    for(int i=0; i<size; i++)
    {
      m_selectedAtoms[i] = NULL;
    }
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
      Atom *atom = (Atom *)molecule->GetAtom(m_hits[0].name());
      if(m_hits[0].type() != Primitive::AtomType) {
        return 0;
      }

      if(m_numSelectedAtoms < 3) {
        // select the third one
        m_selectedAtoms[m_numSelectedAtoms++] = atom;

        if(m_numSelectedAtoms == 2)
        {
          m_vector[0] = m_selectedAtoms[0]->pos() - m_selectedAtoms[1]->pos();
          QString distanceString = tr("Distance: %1 %3").arg(
              QString::number(m_vector[0].norm()),
              QString::fromUtf8("Å"));
          emit message(distanceString);
        }
        else if(m_numSelectedAtoms == 3)
        {
          m_vector[1] = m_selectedAtoms[1]->pos() - m_selectedAtoms[2]->pos();
          QString distanceString = tr("Distance: %1 %3  %2 %3").arg(
              QString::number(m_vector[0].norm()),
              QString::number(m_vector[1].norm()),
              QString::fromUtf8("Å"));


          Vector3d normalizedVectors[2];
          normalizedVectors[0] = m_vector[0].normalized();
          normalizedVectors[1] = m_vector[1].normalized();

          m_angle = acos(normalizedVectors[0].dot(normalizedVectors[1])) * 180/M_PI;
          QString angleString = trUtf8("Angle: %1 %2").arg(
              QString::number(m_angle),
              QString::fromUtf8("°"));

          emit message(angleString);
          emit message(distanceString);
        }

        widget->update();
      }
    }
    // Right button or Left Button + modifier (e.g., Mac)
    else
    {
      int size = m_selectedAtoms.size();
      for(int i=0; i<size; i++)
      {
        m_selectedAtoms[i] = NULL;
      }
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

  bool ClickMeasureTool::paint(GLWidget *widget)
  {
    if(0 < m_numSelectedAtoms)
    {
      // get GL Coordinates for text
      //     glPushMatrix();

      QPoint labelPos(5, widget->height()-20);
      QPoint distancePos[2];
      distancePos[0] = QPoint(90, widget->height()-20 );
      distancePos[1] = QPoint(150, widget->height()-20);
      QPoint anglePos(50, widget->height()-40);
      QPoint angleLabelPos( 5, widget->height()-40);

      glColor3f(1.0,0.0,0.0);
      Vector3d pos = m_selectedAtoms[0]->pos();
      double radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[0]->GetAtomicNum()) * 0.3;

      Vector3d xAxis = widget->camera()->backTransformedXAxis();
      Vector3d zAxis = widget->camera()->backTransformedZAxis();

      // relative position of the text on the atom
      Vector3d textRelPos = radius * (zAxis + xAxis);

      Vector3d textPos = pos+textRelPos;
      widget->painter()->drawText(textPos, tr("*1", "*1 is a number. You most likely don't need to translate this" ));

      if(m_numSelectedAtoms >= 2)
      {
        glColor3f(0.0,1.0,0.0);
        pos = m_selectedAtoms[1]->pos();
        Vector3d textPos = pos+textRelPos;
        radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[1]->GetAtomicNum()) * 0.3;
        widget->painter()->drawText(textPos, tr("*2", "*2 is a number. You most likely don't need to translate this"));

        if(m_numSelectedAtoms == 3)
        {
          // Then calculate the angle between the three selected atoms and display it
          pos = m_selectedAtoms[2]->pos();
          radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[2]->GetAtomicNum()) * 0.3;
          textPos = pos+textRelPos;
          glColor3f(0.0,0.0,1.0);
          widget->painter()->drawText(textPos, tr("*3", "*3 is a number. You most likely don't need to translate this"));
        }
        //       glLoadIdentity();
        glColor3f(1.0,1.0,1.0);
        widget->painter()->drawText(labelPos, tr("Distance(s):"));

        glColor3f(1.0,1.0,0.0);
        widget->painter()->drawText(distancePos[0], QString::number(m_vector[0].norm(), 10, 2) + QString::fromUtf8(" Å"));

        if(m_numSelectedAtoms == 3) {
          glColor3f(1.0,1.0,1.0);
          widget->painter()->drawText(angleLabelPos, QString("Angle:"));

          glColor3f(0.8, 0.8, 0.8);
          widget->painter()->drawText(anglePos, QString::number(m_angle, 10, 1) + QString::fromUtf8("°"));

          glColor3f(0.0,1.0,1.0);
          widget->painter()->drawText(distancePos[1], QString::number(m_vector[1].norm(), 10, 2) + QString::fromUtf8(" Å"));
        }


        // If there are three atoms selected, draw the angle in question
        if(m_numSelectedAtoms == 3)
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
          widget->painter()->drawShadedSector(origin, m_selectedAtoms[0]->pos(), m_selectedAtoms[2]->pos(), radius);
          glDepthMask(GL_TRUE);
          glDisable(GL_BLEND);
          widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
          widget->painter()->drawArc(origin, m_selectedAtoms[0]->pos(), m_selectedAtoms[2]->pos(), radius, 1.0);
        }
      }

      //     glPopMatrix();
    }

    return true;
  }
}

#include "clickmeasuretool.moc"

Q_EXPORT_PLUGIN2(clickmeasuretool, Avogadro::ClickMeasureToolFactory)
