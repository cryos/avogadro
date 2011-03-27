/**********************************************************************
  ceslabbuilder Widget for slab builder

  Copyright (C) 2011 by David C. Lonie
  Copyright (C) 2011 by Geoffrey R. Hutchison

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

#include "ceslabbuilder.h"

#include "../ceundo.h"
#include "../crystallographyextension.h"

#include <avogadro/glwidget.h>
#include <avogadro/painter.h>
#include <avogadro/atom.h>

#include <Eigen/Geometry>

#include <QtGui/QCloseEvent>
#include <QtGui/QHideEvent>

#include <QtCore/QDebug>

Q_DECLARE_METATYPE(Eigen::Vector3d)

namespace Avogadro
{

  ///@todo Needs to remove unit cell information (until lattice vectors are finished)
  ///@todo Needs to update the unit cell for the new frame of reference
  ///@todo Needs to integrate with a Miller Plane engine to visualize the cut before building

  CESlabBuilder::CESlabBuilder(CrystallographyExtension *ext, QMainWindow *w,
                               GLWidget *glwidget)
    : CEAbstractDockWidget(ext, w),
      m_glwidget(glwidget),
      m_beforeState(0),
      m_finished(false)
  {
    ui.setupUi(this);

    // Hide the miller index warning label by default
    ui.warningLabel->hide();
    readSettings();
    updateSlabData(); // we may need to show the warning label, depending on settings

    // Grab snapshot of system for undo
    m_beforeState = new CEUndoState (m_ext);

    // Add the suffix to the xyz distances
    updateLengthUnit();
    connect(m_ext, SIGNAL(lengthUnitChanged(LengthUnit)),
	    this, SLOT(updateLengthUnit()));

    connect(ui.spin_slab_x, SIGNAL(valueChanged(double)),
            this, SLOT(updateSlabData()));
    connect(ui.spin_slab_y, SIGNAL(valueChanged(double)),
            this, SLOT(updateSlabData()));
    connect(ui.spin_slab_z, SIGNAL(valueChanged(double)),
            this, SLOT(updateSlabData()));

    connect(ui.spin_mi_l, SIGNAL(valueChanged(int)),
            this, SLOT(updateSlabData()));
    connect(ui.spin_mi_m, SIGNAL(valueChanged(int)),
            this, SLOT(updateSlabData()));
    connect(ui.spin_mi_n, SIGNAL(valueChanged(int)),
            this, SLOT(updateSlabData()));

    connect(ui.buildButton, SIGNAL(clicked(bool)),
	    this, SLOT(buildSlab()));
  }

  CESlabBuilder::~CESlabBuilder()
  {
    // Use for "slab engine"
    // @todo Revise to handle a general Miller Plane engine
    // Reset cube variants
    /*
      Molecule *mol = GLWidget::current()->molecule();
      mol->setProperty("Slab Cell offset", QVariant());
      mol->setProperty("Slab Cell v1", QVariant());
      mol->setProperty("Slab Cell v2", QVariant());
      mol->setProperty("Slab Cell v3", QVariant());
    */

    // Restore crystal to "before" state if the builder didn't finish
    if (!m_finished) {
      qDebug() << "Restoring state...";
      m_beforeState->apply();
      delete m_beforeState;
      return;
    }
    delete m_beforeState;
  }

  void CESlabBuilder::closeEvent(QCloseEvent *event)
  {
    CEAbstractDockWidget::closeEvent(event);
    this->deleteLater();
  }

  void CESlabBuilder::hideEvent(QHideEvent *event)
  {
    CEAbstractDockWidget::hideEvent(event);
    this->deleteLater();
  }

  void CESlabBuilder::updateLengthUnit()
  {
    if (!m_ext)
      return;

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

    ui.spin_slab_x->setSuffix(lengthSuffix);
    ui.spin_slab_y->setSuffix(lengthSuffix);
    ui.spin_slab_z->setSuffix(lengthSuffix);
  }

  void CESlabBuilder::updateSlabData()
  {
    // Get miller indices:
    const Eigen::Vector3d millerIndices
      (static_cast<double>(ui.spin_mi_l->value()),
       static_cast<double>(ui.spin_mi_m->value()),
       static_cast<double>(ui.spin_mi_n->value()));

    // Check that the indices aren't 0,0,0
    if (millerIndices.norm() < 0.5) {
      /*
	mol->setProperty("Slab Cell offset", QVariant());
	mol->setProperty("Slab Cell v1", QVariant());
	mol->setProperty("Slab Cell v2", QVariant());
	mol->setProperty("Slab Cell v3", QVariant());
      */
      ui.warningLabel->show();
      ui.buildButton->setEnabled(false);
      return;
    }

    // In case warning was visible:
    ui.warningLabel->hide();
    ui.buildButton->setEnabled(true);
    writeSettings();
  }

  void CESlabBuilder::buildSlab()
  {
    writeSettings();
    ui.buildButton->setEnabled(false); // Indicate you can't do anything until we finish
    ui.buildButton->setText(tr("Working..."));
    updateSlabCell(true);
    ui.buildButton->setEnabled(true);
    ui.buildButton->setText(tr("Build"));
  }

  void CESlabBuilder::updateSlabCell(bool build)
  {
    // Get miller indices:
    const Eigen::Vector3d millerIndices
      (static_cast<double>(ui.spin_mi_l->value()),
       static_cast<double>(ui.spin_mi_m->value()),
       static_cast<double>(ui.spin_mi_n->value()));

    // Get cutoff thresholds
    const double xCutoff = ui.spin_slab_x->value() / 2.0;
    const double yCutoff = ui.spin_slab_y->value() / 2.0;
    const double zCutoff = ui.spin_slab_z->value();

    // Get cell vectors
    Eigen::Matrix3d cellMatrix
      (m_ext->unconvertLength(m_ext->currentCellMatrix()).transpose());
    Eigen::Vector3d v1 (cellMatrix.col(0));
    Eigen::Vector3d v2 (cellMatrix.col(1));
    Eigen::Vector3d v3 (cellMatrix.col(2));

    if (build) {
      // Before we do anything, generate the supercell
      // First, wrap all the atoms into the cell
      m_ext->wrapAtomsToCell();
      m_ext->fillUnitCell();

      // Then generate a supercell
      // We intentionally create something much larger than we need
      //  So, let's work out how many replicas we need
      //  What's the smallest lattice distance?
      double minLatticeSize = v1.norm();
      if (v2.norm() < minLatticeSize)
	minLatticeSize = v2.norm();
      if (v3.norm() < minLatticeSize)
	minLatticeSize = v3.norm();

      // What's the largest cutoff?
      double maxCutoffSize = zCutoff;
      if (xCutoff * 2 > maxCutoffSize)
	maxCutoffSize = xCutoff * 2;
      if (yCutoff * 2 > maxCutoffSize)
	maxCutoffSize = yCutoff * 2;

      // OK, we'll ensure there are at least 3 times as many replicas to be safe
      int numReplicas = (maxCutoffSize * 3.5) / minLatticeSize;
#ifdef DEBUG
      std::cout << std::endl << " Replicas: " << numReplicas << std::endl;
#endif
      m_ext->buildSuperCell(numReplicas, numReplicas, numReplicas);

      // update the cell vectors for the supercell
      cellMatrix = m_ext->unconvertLength(m_ext->currentCellMatrix()).transpose();
      v1 = cellMatrix.col(0);
      v2 = cellMatrix.col(1);
      v3 = cellMatrix.col(2);
    }


#ifdef DEBUG
    std::cout << "Cell Matrix:"
	      << std::endl << v1.transpose()
              << std::endl << v2.transpose()
              << std::endl << v3.transpose()
	      << std::endl;
#endif

    // Calculate vectors of the slab cell
    //
    // Define a normal vector to the plane
    // (i.e., if Miller plane is <2 1 1> then normal in realspace will be cellMatrix*<2 1 1>)
    const Eigen::Vector3d normalVec
      ((cellMatrix * millerIndices).normalized());

    // And the cell body diagonal <1 1 1>
    const Eigen::Vector3d bodyDiagonal (v1 + v2 + v3);

    // Find a point in the plane along a cell edge other than (0,0,0)
    // or v1+v2+v3:
    Eigen::Vector3d edgePoint;
    if ((fabs(millerIndices(0)) > 1e-8))
      edgePoint = v1 / millerIndices(0);
    else if ((fabs(millerIndices(1)) > 1e-8))
      edgePoint = v2 / millerIndices(1);
    else if ((fabs(millerIndices(2)) > 1e-8))
      edgePoint = v3 / millerIndices(2);
    else {
      qWarning() << "No non-zero miller index...";
      return;
    }

    ////////////////////////////////////////////////////////////////////
    // Find the point in the Miller Plane that intersects the diagonal
    //  between (0,0,0) and v1+v2+v3
    //
    // Equation of the plane w/ point and normal:
    //  (p - p0).dot(n) = 0
    //
    // p0: point on plane
    const Eigen::Vector3d &p0 (edgePoint);
    // n : vector normal to plane
    const Eigen::Vector3d &n (normalVec.normalized());
    //
    // Define p as some point on the unit cell body diagonal (origin
    // -> origin + v1 + v2 + v3), described here by the line:
    //  p = d * l + l0
    //
    // Where
    //  l : translation vector
    const Eigen::Vector3d l (bodyDiagonal.normalized());
    //  l0: point on line
    const Eigen::Vector3d l0 (bodyDiagonal * 0.5); // center of unit cell
    //  d : translation factor to be found
    //
    // Plug our line into the our plane equation:
    //  ( (d * l + l0) - p0).dot(n) = 0
    //
    // Solve for d:
    const double d = (p0 - l0).dot(n) / l.dot(n);
    //
    // Now find our centerPoint by evaluating the line equation:
    const Eigen::Vector3d centerPoint (d * l + l0);

#ifdef DEBUG
    std::cout << std::endl << centerPoint.transpose()
              << std::endl << d
              << std::endl << l.transpose()
              << std::endl << l0.transpose()
              << std::endl << n.transpose()
              << std::endl << p0.transpose()
              << std::endl;
#endif

    // Determine third point in plane, orthogonal to centerPoint -
    // edgePoint
    const Eigen::Vector3d crossPoint
      (normalVec.cross(centerPoint - edgePoint));

    // Grab mol from current widget
    /// @todo cache this properly
    Molecule *mol = GLWidget::current()->molecule();

    // Pass four corners to the slabbuilder engine:
    mol->setProperty("Slab Plane center", QVariant::fromValue(centerPoint));
    mol->setProperty("Slab Plane p1", QVariant::fromValue(edgePoint));
    mol->setProperty("Slab Plane p2", QVariant::fromValue(crossPoint));

#ifdef DEBUG
    std::cout << std::endl << "Extension (center, p1, p2)"
              << std::endl << centerPoint.transpose()
              << std::endl << edgePoint.transpose()
              << std::endl << crossPoint.transpose() << std::endl;
#endif

    // Generate periodic unit cell
    // Now our new sizes are dictated by the user:
    //
    // First off, use the Miller plane normal (which will get rotated to the z-axis)
    // Normalize it's length. This is the first slab vector.
    const Eigen::Vector3d s1 (normalVec.normalized());
    const Eigen::Vector3d z(0.0, 0.0, 1.0);

    // Construct transformation to rotate s1 onto z axis
    double angle = acos(z.dot(s1) / (z.norm() * s1.norm()));
    Eigen::Vector3d axis (z.cross(s1).normalized());
    if (angle < 1e-6) {
      axis << 1,0,0;
      angle = 0.0;
    }
    const Eigen::AngleAxis<double> rotation (angle, axis);
#ifdef DEBUG
    std::cout << "Axis: " << axis.transpose() << " angle: "
              << angle << std::endl;
#endif

    // Set up the two new slab vectors
    ///@todo Fix this -- it doesn't work
    /*
    const Eigen::Vector3d v1n (v1.normalized());
    Eigen::Vector3d s2 ((v1n - v1n.dot(s1)*s1).normalized());
    s2 = rotation * s2;
    const Eigen::Vector3d v3n (v3.normalized());
    Eigen::Vector3d s3 ((v3n - v3n.dot(s1)*s1).normalized());
    s3 = rotation * s3;

    // These should still be scaled by the cutoffs
    std::cout << std::endl << s1.transpose()
              << std::endl << s2.transpose()
              << std::endl << s3.transpose()
	      << std::endl << (rotation * s2).transpose()
	      << std::endl << (rotation * s3).transpose()
	      << std::endl;
    */
    if (build) {
      // Now rotate, translate, and trim the supercell
      foreach(Atom *a, mol->atoms()) {
	if (a) {
	  // Translate to new origin and rotate
	  Eigen::Vector3d translatedPos = (*a->pos() - centerPoint);
	  Eigen::Vector3d newPos = rotation * translatedPos;
	  // OK, before we update the atom, see if we should trim it...

	  if (newPos.z() > 0.1) // We use a slight slop factor, although in principle every atom should be in xy plane
	    mol->removeAtom(a);
	  else if (newPos.z() < -zCutoff) // the z-thickness should all be negative
	    mol->removeAtom(a);
	  else if (newPos.x() < -xCutoff || newPos.x() > xCutoff)
	    mol->removeAtom(a);
	  else if (newPos.y() < -yCutoff || newPos.y() > yCutoff)
	    mol->removeAtom(a);
	  else // Fits within the criteria
	    a->setPos(newPos);
	}
      }
      // Finally, after moving the atoms...
      // Update the unit cell matrix to allow building a supercell of the surface
      ///@todo Fix this -- once the s2/s3 bits above work
      /*
      Eigen::Matrix3d mat;
      mat.row(0) = s2;
      mat.row(1) = s3;
      mat.row(2) << 0.0, 0.0, zCutoff * 10.0; // the last makes a 3D slab effectively 2D;
      m_ext->setCurrentCellMatrix(mat);
      */

      // Finish up
      m_ext->rebuildBonds();
      // Delete the unit cell (until the above stuff works)
      /*
      mol->setOBUnitCell(0);
      m_ext->setMolecule(mol);
      m_ext->hideEditors();
      m_ext->refreshActions();
      */
      // Push a new undo.
      m_finished = true;
      CEUndoState after (m_ext);
      m_ext->pushUndo(new CEUndoCommand(*m_beforeState, after,
					tr("Cut Slab From Crystal")));
    } // end build
    mol->update();
  }

  void CESlabBuilder::readSettings()
  {
    QSettings settings;
    QString basename = "crystallographyextension/dockwidgets/"
      + QString(this->metaObject()->className());

    ui.spin_slab_x->setValue(settings.value(basename + "/x", 10.0).toDouble());
    ui.spin_slab_y->setValue(settings.value(basename + "/y", 10.0).toDouble());
    ui.spin_slab_z->setValue(settings.value(basename + "/z", 5.0).toDouble());

    ui.spin_mi_l->setValue(settings.value(basename + "/mi_l", 0).toInt());
    ui.spin_mi_m->setValue(settings.value(basename + "/mi_m", 0).toInt());
    ui.spin_mi_n->setValue(settings.value(basename + "/mi_n", 0).toInt());
  }

  void CESlabBuilder::writeSettings()
  {
    QSettings settings;
    QString basename = "crystallographyextension/dockwidgets/"
      + QString(this->metaObject()->className());

    settings.setValue(basename + "/x", ui.spin_slab_x->value());
    settings.setValue(basename + "/y", ui.spin_slab_y->value());
    settings.setValue(basename + "/z", ui.spin_slab_z->value());

    settings.setValue(basename + "/mi_l", ui.spin_mi_l->value());
    settings.setValue(basename + "/mi_m", ui.spin_mi_m->value());
    settings.setValue(basename + "/mi_n", ui.spin_mi_n->value());
  }

}
