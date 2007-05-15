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
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <math.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

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

  if(m_hits.size() && event->buttons() & Qt::LeftButton)
  {
    Atom *atom = (Atom *)molecule->GetAtom(m_hits[0].name());
    if(m_hits[0].type() != Primitive::AtomType) {
      return 0;
    }

    if(m_numSelectedAtoms < 3) {
      // select the third one
      m_selectedAtoms[m_numSelectedAtoms++] = atom;
      widget->update();
    }
  }
  else
  {
    int size = m_selectedAtoms.size();
    for(int i=0; i<size; i++)
    {
      m_selectedAtoms[i] = NULL;
    }
    m_numSelectedAtoms = 0;
    widget->update();
  }
  return 0;
}

QUndoCommand* ClickMeasureTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  return 0;
}

QUndoCommand* ClickMeasureTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  return 0;
}

QUndoCommand* ClickMeasureTool::wheel(GLWidget *widget, const QWheelEvent *event)
{
  return 0;
}

bool ClickMeasureTool::paint(GLWidget *widget)
{
  if(0 < m_numSelectedAtoms)
  {
    // get GL Coordinates for text
    glPushMatrix();

    glPushMatrix();
    glLoadIdentity();
    QPoint labelPos(5, widget->height()-20);
    QPoint distancePos[2];
    distancePos[0] = QPoint(90, widget->height()-20 );
    distancePos[1] = QPoint(150, widget->height()-20);
    QPoint anglePos(50, widget->height()-40);
    QPoint angleLabelPos( 5, widget->height()-40);
    glPopMatrix();

    widget->painter()->beginText();

    glColor3f(1.0,0.0,0.0);
    Vector3d pos = m_selectedAtoms[0]->pos();
    double radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[0]->GetAtomicNum()) * 0.3;
    const MatrixP3d & m = widget->camera()->modelview();

    Vector3d xAxis = widget->camera()->backtransformedXAxis();
    Vector3d zAxis = widget->camera()->backtransformedZAxis();

    // relative position of the text on the atom
    Vector3d textRelPos = radius * (zAxis + xAxis);

    Vector3d textPos = pos+textRelPos;
    widget->painter()->drawText(textPos, tr("*1"));

    if(m_numSelectedAtoms >= 2)
    {
      Vector3d vector[2];
      vector[0] = m_selectedAtoms[0]->pos() - m_selectedAtoms[1]->pos();
      double distance = vector[0].norm();
      double angle;

      glColor3f(0.0,1.0,0.0);
      pos = m_selectedAtoms[1]->pos();
      Vector3d textPos = pos+textRelPos;
      radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[1]->GetAtomicNum()) * 0.3;
      widget->painter()->drawText(textPos, tr("*2"));

      if(m_numSelectedAtoms == 3)
      {
        vector[1] = m_selectedAtoms[2]->pos() - m_selectedAtoms[1]->pos();

        Vector3d normalizedVectors[2];
        normalizedVectors[0] = vector[0].normalized();
        normalizedVectors[1] = vector[1].normalized();

        angle = acos(normalizedVectors[0].dot(normalizedVectors[1])) * 180/M_PI;
        pos = m_selectedAtoms[2]->pos();
        radius = 0.18 + etab.GetVdwRad(m_selectedAtoms[2]->GetAtomicNum()) * 0.3;
        textPos = pos+textRelPos;
        glColor3f(0.0,0.0,1.0);
        widget->painter()->drawText(textPos, tr("*3"));
      }
      glLoadIdentity();
      glColor3f(1.0,1.0,1.0);
      widget->painter()->drawText(labelPos, tr("Distance(s):"));
      if(m_numSelectedAtoms == 3) {
        glColor3f(1.0,1.0,1.0);
        widget->painter()->drawText(angleLabelPos, QString("Angle:"));
        glColor3f(0.8, 0.8, 0.8);
        widget->painter()->drawText(anglePos, QString::number(angle));
        glColor3f(0.0,1.0,1.0);
        widget->painter()->drawText(distancePos[1], QString::number(vector[1].norm()));
      }
      glColor3f(1.0,1.0,0.0);
      widget->painter()->drawText(distancePos[0], QString::number(vector[0].norm()));
    }

    widget->painter()->endText();
    glPopMatrix();
  }

  return true;
}

#include "clickmeasuretool.moc"

Q_EXPORT_PLUGIN2(clickmeasuretool, ClickMeasureToolFactory)
