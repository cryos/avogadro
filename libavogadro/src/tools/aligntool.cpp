/**********************************************************************
  AlignTool - AlignTool Tool for Avogadro

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "aligntool.h"

#include <avogadro/navigate.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/painter.h>

#include <openbabel/mol.h>
#include <openbabel/obiter.h>
#include <openbabel/generic.h>

#include <cmath>

#include <QDebug>
#include <QAction>
#include <QtPlugin>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  AlignTool::AlignTool(QObject *parent) : Tool(parent),  m_molecule(0),
    m_selectedAtoms(2), m_numSelectedAtoms(0), m_axis(2), m_alignType(0),
    m_settingsWidget(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/align/align.png")));
    action->setToolTip(tr("Align Molecules\n\n"
          "Left Mouse: \tSelect up to two atoms.\n"
          "\tThe first atom is centered at the origin.\n"
          "\tThe second atom is aligned to the selected axis.\n"
          "Right Mouse: \tReset alignment."));
    action->setShortcut(Qt::Key_F12);

    // clear the selected atoms
    int size = m_selectedAtoms.size();
    for(int i=0; i<size; i++)
      m_selectedAtoms[i] = NULL;
  }

  AlignTool::~AlignTool()
  {
    if (m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  QUndoCommand* AlignTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_molecule = widget->molecule();
    if(!m_molecule)
      return 0;

    //! List of hits from initial click
    QList<GLHit> m_hits = widget->hits(event->pos().x()-2, event->pos().y()-2, 5, 5);

    // If there's a left button (and no modifier keys) continue adding to the list
    if(m_hits.size() && (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::NoModifier))
    {
      if(m_hits[0].type() != Primitive::AtomType)
        return 0;

      Atom *atom = m_molecule->atom(m_hits[0].name());
      event->accept();

      if(m_numSelectedAtoms < 2)
      {
        // Select another atom
        m_selectedAtoms[m_numSelectedAtoms++] = atom;
        widget->update();
      }
    }
    // Right button or Left Button + modifier (e.g., Mac)
    else
    {
      event->accept();
      m_numSelectedAtoms = 0;
      widget->update();
    }
    return 0;
  }

  QUndoCommand* AlignTool::mouseMoveEvent(GLWidget*, QMouseEvent *)
  {
    return 0;
  }

  QUndoCommand* AlignTool::mouseReleaseEvent(GLWidget*, QMouseEvent*)
  {
    return 0;
  }

  QUndoCommand* AlignTool::wheelEvent(GLWidget *widget, QWheelEvent *event)
  {
    Q_UNUSED(widget);
    Q_UNUSED(event);
    return 0;
  }

  bool AlignTool::paint(GLWidget *widget)
  {
    if(m_numSelectedAtoms > 0)
    {
      Vector3d xAxis = widget->camera()->backTransformedXAxis();
      Vector3d zAxis = widget->camera()->backTransformedZAxis();
      // Check the atom is still around...
      if (m_selectedAtoms[0])
      {
        glColor3f(1.0,0.0,0.0);
        widget->painter()->setColor(1.0, 0.0, 0.0);
        const Vector3d *pos = m_selectedAtoms[0]->pos();

        // relative position of the text on the atom
        double radius = widget->radius(m_selectedAtoms[0]) + 0.05;
        Vector3d textRelPos = radius * (zAxis + xAxis);

        Vector3d textPos = *pos + textRelPos;
        widget->painter()->drawText(textPos, "*1");
        widget->painter()->drawSphere(pos, radius);
      }

      if(m_numSelectedAtoms >= 2)
      {
        // Check the atom is still around...
        if (m_selectedAtoms[1])
        {
          glColor3f(0.0,1.0,0.0);
          widget->painter()->setColor(0.0, 1.0, 0.0);
          const Vector3d *pos = m_selectedAtoms[1]->pos();
          double radius = widget->radius(m_selectedAtoms[1]) + 0.05;
          widget->painter()->drawSphere(pos, radius);
          Vector3d textRelPos = radius * (zAxis + xAxis);
          Vector3d textPos = *pos + textRelPos;
          widget->painter()->drawText(textPos, "*2");
        }
      }
    }

    return true;
  }

  void AlignTool::align()
  {
    // Check we have a molecule, otherwise we can't do anything
    if (m_molecule.isNull())
      return;

    QList<Atom*> neighborList;
    if (m_numSelectedAtoms) {
      // Check the first atom still exists, return if not
      if (m_selectedAtoms[0].isNull())
        return;

      // If m_alignType is 0 we want everything, otherwise just the fragment
      /// FIXME Add back fragment alignment too!
/*      if (m_alignType) {
        OBMolAtomDFSIter iter(m_molecule, m_selectedAtoms[0]->index());
        Atom *tmpNeighbor;
        do {
          tmpNeighbor = static_cast<Atom*>(&*iter);
          neighborList.append(tmpNeighbor);
        } while ((iter++).next()); // this returns false when we are done
      }
      else { */
      neighborList = m_molecule->atoms();
    }
    // Align the molecule along the selected axis
    if (m_numSelectedAtoms >= 1) {
      // Translate the first selected atom to the origin
      Vector3d pos = *m_selectedAtoms[0]->pos();
      foreach(Atom *a, neighborList) {
        if (a) {
          a->setPos(*a->pos() - pos);
        }
      }
      m_molecule->update();
    }
    if (m_numSelectedAtoms >= 2)
    {
      // Check the second atom still exists, return if not
      if (m_selectedAtoms[1].isNull())
        return;
      // Now line up the line from atom[0] to atom[1] with the axis selected
      double alpha, beta, gamma;
      alpha = beta = gamma = 0.0;

      Vector3d pos = *m_selectedAtoms[1]->pos();
      pos.normalize();
      Vector3d axis;

      if (m_axis == 0) // x-axis
        axis = Vector3d(1., 0., 0.);
      else if (m_axis == 1) // y-axis
        axis = Vector3d(0., 1., 0.);
      else if (m_axis == 2) // z-axis
        axis = Vector3d(0., 0., 1.);

      // Calculate the angle of the atom from the axis
      double angle = acos(axis.dot(pos));

      // If the angle is zero then we don't need to do anything here
      if (angle > 0)
      {
        // Get the axis for the rotation
        axis = axis.cross(pos);
        axis.normalize();

        // Now to rotate the fragment
        foreach(Atom *a, neighborList) {
          a->setPos(Eigen::AngleAxisd(-angle,axis) * *a->pos());
        }
        m_molecule->update();
      }
    }
    m_numSelectedAtoms = 0;
  }

  QWidget* AlignTool::settingsWidget()
  {
    if(!m_settingsWidget) {
      m_settingsWidget = new QWidget;

      QLabel *labelAxis = new QLabel(tr("Axis:"), m_settingsWidget);
      labelAxis->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelAxis->setMaximumHeight(15);

      // Combo box to select desired aixs to align to
      QComboBox *comboAxis = new QComboBox(m_settingsWidget);
      comboAxis->addItem("x");
      comboAxis->addItem("y");
      comboAxis->addItem("z");
      comboAxis->setCurrentIndex(2);

      QLabel *labelAlign = new QLabel(tr("Align:"));
      labelAlign->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelAlign->setMaximumHeight(15);
      // Combo to choose what should be aligned
      QComboBox *comboAlign = new QComboBox(m_settingsWidget);
      comboAlign->addItem(tr("Everything"));
      comboAlign->addItem(tr("Molecule"));

      // Button to actually perform actions
      QPushButton *buttonAlign = new QPushButton(m_settingsWidget);
      buttonAlign->setText(tr("Align"));
      connect(buttonAlign, SIGNAL(clicked()), this, SLOT(align()));

      QGridLayout *gridLayout = new QGridLayout();
      gridLayout->addWidget(labelAxis,0, 0, 1, 1, Qt::AlignRight);
      QHBoxLayout *hLayout = new QHBoxLayout;
      hLayout->addWidget(comboAxis);
      hLayout->addStretch(1);
      gridLayout->addLayout(hLayout, 0, 1);
      gridLayout->addWidget(labelAlign, 1, 0, 1, 1, Qt::AlignRight);
      QHBoxLayout *hLayout2 = new QHBoxLayout;
      hLayout2->addWidget(comboAlign);
      hLayout2->addStretch(1);
      gridLayout->addLayout(hLayout2, 1, 1);
      QHBoxLayout *hLayout3 = new QHBoxLayout();
      hLayout3->addStretch(1);
      hLayout3->addWidget(buttonAlign);
      hLayout3->addStretch(1);
      QVBoxLayout *layout = new QVBoxLayout();
      layout->addLayout(gridLayout);
      layout->addLayout(hLayout3);
      layout->addStretch(1);
      m_settingsWidget->setLayout(layout);

      connect(comboAxis, SIGNAL(currentIndexChanged(int)),
              this, SLOT(axisChanged(int)));
      connect(comboAlign, SIGNAL(currentIndexChanged(int)),
              this, SLOT(alignChanged(int)));

      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
    }

    return m_settingsWidget;
  }

  void AlignTool::axisChanged(int axis)
  {
    // Axis to use - x=0, y=1, z=2
    m_axis = axis;
  }

  void AlignTool::alignChanged(int align)
  {
    // Type of alignment - 0=everything, 1=molecule
    m_alignType = align;
  }

  void AlignTool::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

}

Q_EXPORT_PLUGIN2(aligntool, Avogadro::AlignToolFactory)
