/**********************************************************************
  cesupercell Widget for supercell / nanoparticle builder

  Copyright (C) 2012 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "cesupercellbuilder.h"

#include "../ceundo.h"
#include "../crystallographyextension.h"

#include <avogadro/glwidget.h>
#include <avogadro/painter.h>
#include <avogadro/atom.h>

#include <Eigen/Geometry>

#include <QtGui/QCloseEvent>
#include <QtGui/QHideEvent>

Q_DECLARE_METATYPE(Eigen::Vector3d)

namespace Avogadro
{

  CESuperCellBuilder::CESuperCellBuilder(CrystallographyExtension *ext)
    : CEAbstractDockWidget(ext),
      m_glwidget(NULL),
      m_beforeState(0),
      m_finished(false)
  {
    ui.setupUi(this);

    readSettings(); // hides or shows the warning if needed

    // Add the suffix to the xyz distances
    updateLengthUnit();
    connect(m_ext, SIGNAL(lengthUnitChanged(LengthUnit)),
            this, SLOT(updateLengthUnit()));

    // If the x/y cutoffs change, we'll need to append a suffix for the unit
    connect(ui.aWidthUnits, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateLengthUnit()));
    connect(ui.bWidthUnits, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateLengthUnit()));
    connect(ui.cWidthUnits, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateLengthUnit()));

    connect(ui.buildButton, SIGNAL(clicked(bool)),
            this, SLOT(buildSuperCell()));

    updateGeometryType(ui.geometryComboBox->currentIndex());
    connect(ui.geometryComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateGeometryType(int)));
  }

  CESuperCellBuilder::~CESuperCellBuilder()
  {
    // Restore crystal to "before" state if the builder didn't finish
    if (!m_finished && m_beforeState != NULL) {
      m_beforeState->apply();
      delete m_beforeState;
      return;
    }
    delete m_beforeState;
  }

  void CESuperCellBuilder::closeEvent(QCloseEvent *event)
  {
    CEAbstractDockWidget::closeEvent(event);
  }

  void CESuperCellBuilder::hideEvent(QHideEvent *event)
  {
    CEAbstractDockWidget::hideEvent(event);
  }

  void CESuperCellBuilder::updateLengthUnit()
  {
    if (!m_ext)
      return;

    // Called when the user changes the length unit
    QString lengthSuffix;
    switch (m_ext->lengthUnit()) {
    case Angstrom:
      lengthSuffix = CE_ANGSTROM;
      break;
    case Bohr:
      lengthSuffix = " a" + CE_SUB_ZERO;
      break;
    case Nanometer:
      lengthSuffix = " nm";
      break;
    case Picometer:
      lengthSuffix = " pm";
      break;
    default:
      lengthSuffix = "";
      break;
    }

    ui.aWidthUnits->setItemText(0, lengthSuffix);
    ui.bWidthUnits->setItemText(0, lengthSuffix);
    ui.cWidthUnits->setItemText(0, lengthSuffix);

    if (ui.aWidthUnits->currentIndex() == 0)
      ui.aSpinBox->setSuffix(lengthSuffix);
    else
      ui.aSpinBox->setSuffix("");

    if (ui.bWidthUnits->currentIndex() == 0)
      ui.bSpinBox->setSuffix(lengthSuffix);
    else
      ui.bSpinBox->setSuffix("");

    if (ui.cWidthUnits->currentIndex() == 0)
      ui.cSpinBox->setSuffix(lengthSuffix);
    else
      ui.cSpinBox->setSuffix("");
  }

  void CESuperCellBuilder::updateGeometryType(int type)
  {
    switch (static_cast<GeometryType>(type)) {
    default:
    case GT_BOX:
      //    case GT_ELLIPSOID:
      ui.aLabel->setText(tr("A width"));
      ui.bLabel->setText(tr("B width"));
      ui.cLabel->setText(tr("C width"));
      break;
      //    case GT_CYLINDER:
      //      ui.bLabel->setText(tr("Axis Length"));
    case GT_SPHERE:
      ui.aLabel->setText(tr("Radius"));
      break;
    }

    // Now a switch for visibility
    switch (static_cast<GeometryType>(type)) {
    default:
    case GT_BOX:
      //    case GT_ELLIPSOID:
      // Show everything
      ui.bLabel->setVisible(true);
      ui.cLabel->setVisible(true);
      ui.bSpinBox->setVisible(true);
      ui.cSpinBox->setVisible(true);
      ui.bWidthUnits->setVisible(true);
      ui.cWidthUnits->setVisible(true);
      break;
      /*    case GT_CYLINDER: // show the axis, but not "C"
      ui.bLabel->setVisible(true);
      ui.bSpinBox->setVisible(true);
      ui.bWidthUnits->setVisible(true);

      ui.cLabel->setVisible(false);
      ui.cSpinBox->setVisible(false);
      ui.cWidthUnits->setVisible(false);
      break;
      */
    case GT_SPHERE:
      // Hide everything
      ui.bLabel->setVisible(false);
      ui.cLabel->setVisible(false);
      ui.bSpinBox->setVisible(false);
      ui.cSpinBox->setVisible(false);
      ui.bWidthUnits->setVisible(false);
      ui.cWidthUnits->setVisible(false);
      break;
    }
  }

  void CESuperCellBuilder::buildSuperCell()
  {
    writeSettings();
    ui.buildButton->setEnabled(false); // Indicate you can't do anything until we finish
    ui.buildButton->setText(tr("Working..."));

    // Save an undo state before we modify anything
    m_beforeState = new CEUndoState (m_ext);
    m_ext->fillUnitCell();

    // OK if we need distances, we must convert the cell matrix
    // Get cell vectors
    Eigen::Matrix3d cellMatrix
      (m_ext->unconvertLength(m_ext->currentCellMatrix()).transpose());
    Eigen::Vector3d v1 (cellMatrix.col(0));
    Eigen::Vector3d v2 (cellMatrix.col(1));
    Eigen::Vector3d v3 (cellMatrix.col(2));
    double distA = v1.norm();
    double distB = v2.norm();
    double distC = v3.norm(); // all in Angstroms

    int aRepeats, bRepeats, cRepeats;
    if (ui.aWidthUnits->currentIndex() == 0) { // distance
      double cutoffA = m_ext->unconvertLength(ui.aSpinBox->value());
      aRepeats = static_cast<int>(cutoffA / distA);
    } else {
      aRepeats = static_cast<int>(ui.aSpinBox->value());
    }
    if (ui.bWidthUnits->currentIndex() == 0) { // distance
      double cutoffB = m_ext->unconvertLength(ui.bSpinBox->value());
      bRepeats = static_cast<int>(cutoffB / distB);
    } else {
      bRepeats = static_cast<int>(ui.bSpinBox->value());
    }
    if (ui.cWidthUnits->currentIndex() == 0) { // distance
      double cutoffC = m_ext->unconvertLength(ui.cSpinBox->value());
      cRepeats = static_cast<int>(cutoffC / distC);
    } else {
      cRepeats = static_cast<int>(ui.cSpinBox->value());
    }

    // Grab mol from our widget
    // We can't do this any earlier, since the molecule could change
    Molecule *mol = m_glwidget->molecule();

    int type = ui.geometryComboBox->currentIndex();
    switch (static_cast<GeometryType>(type)) {
    default:
    case GT_BOX:
      m_ext->buildSuperCell(aRepeats, bRepeats, cRepeats);
      break;
      /*
    case GT_ELLIPSOID:
      break;
    case GT_CYLINDER:
      break;
      */
    case GT_SPHERE:
      int replicas = (aRepeats + 1) * 2;
      m_ext->buildSuperCell(replicas, replicas, replicas);
      double radius = distA * (aRepeats + 1);
      Eigen::Vector3d translation(radius, radius, radius);

      // OK, what's the radius cutoff
      double cutoff = aRepeats * distA; // in unit cells
      if (ui.aWidthUnits->currentIndex() == 0) // real distance
        cutoff = m_ext->unconvertLength(ui.aSpinBox->value());

      foreach(Atom *a, mol->atoms()) {
        if (a) {
          Eigen::Vector3d newPos = (*a->pos() - translation);
          if (newPos.norm() > aRepeats * distA)
            mol->removeAtom(a);
          else
            a->setPos(newPos);
        }
      }
      break;
    }

    // Finish up
    m_ext->rebuildBonds();
    // Push a new undo.
    m_finished = true;
    CEUndoState after (m_ext);
    m_ext->pushUndo(new CEUndoCommand(*m_beforeState, after,
                                      tr("Cut Slab From Crystal")));
    mol->update();

    ui.buildButton->setEnabled(true);
    ui.buildButton->setText(tr("Build"));

    // OK, get rid of ourselves to go back to normal crystal editing
    this->hide();
    emit finished();
  }

  void CESuperCellBuilder::readSettings()
  {
    QSettings settings;
    settings.beginGroup("crystallographyextension");
    settings.beginGroup(this->metaObject()->className());

    ui.geometryComboBox->setCurrentIndex(settings.value("geometryType", 0).toInt());

    ui.aWidthUnits->setCurrentIndex(settings.value("aWidthUnits", 0).toInt());
    ui.bWidthUnits->setCurrentIndex(settings.value("bWidthUnits", 0).toInt());
    ui.cWidthUnits->setCurrentIndex(settings.value("cWidthUnits", 0).toInt());

    ui.aSpinBox->setValue(settings.value("aCutOff", 1.0).toDouble());
    ui.bSpinBox->setValue(settings.value("bCutOff", 1.0).toDouble());
    ui.cSpinBox->setValue(settings.value("cCutOff", 1.0).toDouble());

    settings.endGroup(); // supercell
    settings.endGroup(); // crystallography
  }

  void CESuperCellBuilder::writeSettings()
  {
    QSettings settings;
    settings.beginGroup("crystallographyextension");
    settings.beginGroup(this->metaObject()->className());

    settings.setValue("geometryType", ui.geometryComboBox->currentIndex());

    settings.setValue("aWidthUnits", ui.aWidthUnits->currentIndex());
    settings.setValue("bWidthUnits", ui.bWidthUnits->currentIndex());
    settings.setValue("cWidthUnits", ui.cWidthUnits->currentIndex());

    settings.setValue("aCutOff", ui.aSpinBox->value());
    settings.setValue("bCutOff", ui.bSpinBox->value());
    settings.setValue("cCutOff", ui.cSpinBox->value());

    settings.endGroup(); // slabbuilder
    settings.endGroup(); // crystallography
  }

}
