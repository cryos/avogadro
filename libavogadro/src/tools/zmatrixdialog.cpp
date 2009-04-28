/**********************************************************************
  ZMatrixTool - ZMatrix Manipulation Tool for Avogadro

  Copyright (C) 2009 by Marcus D. Hanwell

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

#include "zmatrixdialog.h"
#include "zmatrixmodel.h"

#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>

namespace Avogadro {

  ZMatrixDialog::ZMatrixDialog(QWidget *parent, Qt::WindowFlags) :
      QDialog(parent), m_zMatrixModel(new ZMatrixModel), m_molecule(0)
  {
    setWindowFlags(Qt::Dialog | Qt::Tool);
    ui.setupUi(this);
    ui.tableView->setModel(m_zMatrixModel);

    // Connect our signals and slots...
    connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addAtom()));
    connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removeAtom()));
  }

  ZMatrixDialog::~ZMatrixDialog()
  {
    delete m_zMatrixModel;
  }

  void ZMatrixDialog::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    if (!m_molecule)
      return;
    if (m_molecule->zMatrix(0))
      m_zMatrixModel->setZMatrix(m_molecule->zMatrix(0));
    else {
      m_zMatrixModel->setZMatrix(m_molecule->addZMatrix());
    }
  }

  void ZMatrixDialog::setGLWidget(GLWidget *widget)
  {
    m_glwidget = widget;
  }

  void ZMatrixDialog::addAtom()
  {
    m_zMatrixModel->insertRows(ui.tableView->currentIndex().row()+1, 1);

  }

  void ZMatrixDialog::removeAtom()
  {

  }

  void ZMatrixDialog::importSelectedAtoms()
  {
    // Only possible if we have a valid GLWidget to retrieve selected atoms from
    if (!m_glwidget)
      return;


  }

} // End namespace Avogadro

#include "zmatrixdialog.moc"
