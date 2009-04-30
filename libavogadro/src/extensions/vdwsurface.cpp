/**********************************************************************
  VdWSurface - Class to calculate Van der Waals cubes

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "vdwsurface.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

#include <openbabel/mol.h>

#include <cmath>

#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>
#include <QReadWriteLock>
#include <QDebug>

using std::vector;
using Eigen::Vector3d;
using Eigen::Vector3i;

namespace Avogadro
{
  struct VdWStruct
  {
    vector<Vector3d> *atomPos;
    vector<double> *atomRadius;
    Cube *cube;       // The target cube, used to initialise temp cubes too
    unsigned int pos;  // The index ofposition of the point to calculate the MO for
  };

  VdWSurface::VdWSurface()
  {
  }

  VdWSurface::~VdWSurface()
  {
  }

  void VdWSurface::setAtoms(Molecule* mol)
  {
    // check if there is a selection in the current glwidget
    GLWidget *glwidget = GLWidget::current();
    if (glwidget) {
      QList<Primitive*> atoms = glwidget->selectedPrimitives().subList(Primitive::AtomType);
      if (!atoms.isEmpty()) {
        qDebug() << "VdWSurface: Number of atoms" << atoms.size();
        m_atomPos.resize(atoms.size());
        m_atomRadius.resize(atoms.size());

        for (unsigned int i = 0; i < m_atomPos.size(); ++i) {
          Atom *atom = static_cast<Atom*>(atoms.at(i));
          m_atomPos[i] = *atom->pos();
          m_atomRadius[i] = OpenBabel::etab.GetVdwRad(atom->atomicNumber());
        }

        return;
      }
    }

    qDebug() << "VdWSurface: Number of atoms" << mol->numAtoms();
    m_atomPos.resize(mol->numAtoms());
    m_atomRadius.resize(mol->numAtoms());

    for (unsigned int i = 0; i < m_atomPos.size(); ++i) {
      m_atomPos[i] = *mol->atom(i)->pos();
      m_atomRadius[i] = OpenBabel::etab.GetVdwRad(mol->atom(i)->atomicNumber());
    }
  }

  void VdWSurface::calculateCube(Cube *cube)
  {
    // Set up the calculation and ideally use the new QtConcurrent code to
    m_VdWvector.resize(cube->data()->size());
    m_cube = cube;

    for (int i = 0; i < m_VdWvector.size(); ++i) {
      m_VdWvector[i].atomPos = &m_atomPos;
      m_VdWvector[i].atomRadius = &m_atomRadius;
      m_VdWvector[i].cube = cube;
      m_VdWvector[i].pos = i;
    }

    // Lock the cube until we are done.
    cube->lock()->lockForWrite();

    // Watch for the future
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));

    // The main part of the mapped reduced function...
    m_future = QtConcurrent::map(m_VdWvector, VdWSurface::processPoint);
    // Connect our watcher to our future
    m_watcher.setFuture(m_future);
  }

  void VdWSurface::calculationComplete()
  {
    disconnect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));
    m_cube->lock()->unlock();
    m_cube->update();
  }

  void VdWSurface::processPoint(VdWStruct &vdw)
  {
    unsigned int size = vdw.atomPos->size();
    // Calculate our position
    Vector3d pos = vdw.cube->position(vdw.pos);

    // Now calculate the value at this point in space
    double tmp = -1.0E+10;
    for (unsigned int i = 0; i < size; ++i) {
      double distance = std::abs((pos - (*vdw.atomPos)[i]).norm()) - (*vdw.atomRadius)[i];
      if ((tmp < -1.0E+9) || (distance < tmp))
        tmp = distance;
    }

    vdw.cube->setValue(vdw.pos, tmp);
  }

}

#include "vdwsurface.moc"
