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

#define THRESH 1.0e-1

namespace Avogadro
{
  ///@todo Needs to integrate with a Miller Plane engine to visualize the cut before building

  //Compute the greatest common divisor by subtraction
  //Fastest code on small integers (like Miller planes)
  // Based on code from Wikipedia (and elsewhere on the web)
  // (many implementations)
  int gcdSmall(const int aOriginal, const int bOriginal)
  {
    // Take an absolute value, since we may have negative Miller indices
    int a = abs(aOriginal);
    int b = abs(bOriginal);

    // Don't return 0, always keep 1 as the GCD of everything
    if (a == 0 || b == 0) return 1;

    while (a != b) {
      while (a < b)
        b -= a;
      while (b < a)
        a -= b;
    }
    return a;
  }

  bool vectorCompare(Eigen::Vector3d a, Eigen::Vector3d b) {
    return (a.squaredNorm() < b.squaredNorm());
  }

  CESlabBuilder::CESlabBuilder(CrystallographyExtension *ext, QMainWindow *w,
                               GLWidget *glwidget)
    : CEAbstractDockWidget(ext, w),
      m_glwidget(glwidget),
      m_beforeState(0),
      m_finished(false)
  {
    ui.setupUi(this);
    // Hide the "i" Miller index if not a hexagonal unit cell
    ui.spin_mi_i->hide();

    // Hide the Miller index warning label by default
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

    connect(ui.spin_mi_h, SIGNAL(valueChanged(int)),
            this, SLOT(updateSlabData()));
    connect(ui.spin_mi_k, SIGNAL(valueChanged(int)),
            this, SLOT(updateSlabData()));
    connect(ui.spin_mi_l, SIGNAL(valueChanged(int)),
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
      (static_cast<double>(ui.spin_mi_h->value()),
       static_cast<double>(ui.spin_mi_k->value()),
       static_cast<double>(ui.spin_mi_l->value()));

    // Check that the indices aren't 0,0,0
    if (millerIndices.norm() < 0.5) {
      ui.warningLabel->show();
      ui.buildButton->setEnabled(false);
      return;
    }

    // Check if hexagonal or rhombohedral and show the "i" index
    OpenBabel::OBUnitCell *cell = m_ext->currentCell();
    if (cell &&
        (cell->GetLatticeType() == OpenBabel::OBUnitCell::Rhombohedral
         || cell->GetLatticeType() == OpenBabel::OBUnitCell::Hexagonal)) {
      ui.spin_mi_i->setValue(-ui.spin_mi_h->value() - ui.spin_mi_k->value());
      ui.spin_mi_i->show();
    } else { // otherwise hide it
      ui.spin_mi_i->hide();
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
      (static_cast<double>(ui.spin_mi_h->value()),
       static_cast<double>(ui.spin_mi_k->value()),
       static_cast<double>(ui.spin_mi_l->value()));

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
      int numReplicas = (maxCutoffSize * 4.5) / minLatticeSize;
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

    // Determine third point in plane,
    // orthogonal to centerPoint - edgePoint
    const Eigen::Vector3d crossPoint
      (normalVec.cross(centerPoint - edgePoint));

    // Grab mol from current widget
    /// @todo cache this properly
    Molecule *mol = GLWidget::current()->molecule();

    // Pass corners to the Miller plane engine:
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
    //
    // First off, use the Miller plane normal (which will get rotated to the z-axis
    // Normalize it's length. This is one slab vector.
    const Eigen::Vector3d s1 (normalVec.normalized());

    // Set up the other 2 slab vectors
    /* Algorithm inspired by GDIS http://gdis.sf.net/
      Sean Fleming of GDIS said the code was based on MARVIN
      D.H. Gay and A.L. Rohl.
        Marvin: A new computer code for studying surfaces and interfaces and
        its application to calculating the crystal morphologies of corundum and
        zircon. J. Chem. Soc., Faraday Trans., 91:926-936, 1995.
    */
    std::vector<Eigen::Vector3d> baseVectors, surfaceVectors;
    int mi_h = ui.spin_mi_h->value();
    int mi_k = ui.spin_mi_k->value();
    int mi_l = ui.spin_mi_l->value();

    // Set up the surface lattice vectors
    Eigen::Vector3d s2, s3;
    // First, generate the basic Miller vectors -- linear combinations of v1,v2,v3
    Eigen ::Vector3d v;
    int common = gcdSmall(mi_h, mi_k);
    v = (mi_k/common) * v1 - (mi_h/common) * v2;
    if (v.squaredNorm() > THRESH) // i.e., if this is a non-zero vector
      baseVectors.push_back(v);

    common = gcdSmall(mi_h, mi_l);
    v = (mi_l/common) * v1 - (mi_h/common) * v3;
    if (v.squaredNorm() > THRESH)
      baseVectors.push_back(v);

    common = gcdSmall(mi_k, mi_l);
    v = (mi_l/common) * v2 - (mi_k/common) * v3;
    if (v.squaredNorm() > THRESH)
      baseVectors.push_back(v);

    // Now that we have the three basic Miller vectors, we iterate to find all linear combinations
    Eigen::Vector3d vA, vB;
    surfaceVectors = baseVectors; // copy the basic ones
    for (unsigned int i = 0; i < baseVectors.size() - 1; ++i) {
      vA = baseVectors[i];
      for (unsigned int j = i+1; j < baseVectors.size(); ++j) {
        vB = baseVectors[j];

        v = vA - vB;
        if (v.squaredNorm() > THRESH) // i.e., this is non-zero
          surfaceVectors.push_back(v);
        v = vA + vB;
        if (v.squaredNorm() > THRESH) // ditto
          surfaceVectors.push_back(v);
      }
    }
    // OK, now we sort all possible surfaceVectors by magnitude
    std::sort(surfaceVectors.begin(), surfaceVectors.end(), vectorCompare);

    // Set s2 to the shortest vector
    s2 = surfaceVectors[0].normalized();
    // Now loop through to find the next-shortest orthogonal to s1
    //  and mostly orthogonal to s2
    unsigned int i;
    for (i = 1; i < surfaceVectors.size(); ++i) {
      if (s1.cross(surfaceVectors[i]).squaredNorm() > 0.8
          && s2.cross(surfaceVectors[i]).squaredNorm() > THRESH)
          break;
    }
    s3 = surfaceVectors[i].normalized();

    // Now we set up the transformation matrix
    // We want s1 on the z-axis, and s2 on the x-axis
    // So we need to take the cross for the y-axis
    Eigen::Matrix3d rotation;
    rotation.row(0) = s2;
    rotation.row(1) = s2.cross(s1);
    rotation.row(2) = s1;

#ifdef DEBUG
    std::cout << std::endl << s1.transpose()
              << std::endl << s2.transpose()
              << std::endl << s3.transpose()
              << std::endl << (rotation * s2).transpose()
              << std::endl << (rotation * s3).transpose()
              << std::endl;
#endif DEBUG

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
        // The removeAtom calls can take a while, since it's renumbering the indices
        QCoreApplication::processEvents();
      }

      // Finally, after moving the atoms...
      // Update the unit cell matrix to allow building a supercell of the surface
      Eigen::Matrix3d mat;
      Eigen::Vector3d m1 = (rotation * s2); // Should be x-axis
      Eigen::Vector3d m2 = (rotation * s3); // should by y-axis
      double xScale = xCutoff * 2.0;
      double yScale = yCutoff * 2.0;
      mat << xScale * m1.x(), yScale * m1.y(), 0.0,
        xScale * m2.x(), yScale * m2.y(), 0.0,
        0.0, 0.0, zCutoff * 10.0; // the last makes a 3D slab effectively 2D;

      // We build everything in cartesian, so we want to preserve them when modifying the matrix
      CartFrac existingPreserveCartFrac = m_ext->coordsPreserveCartFrac();
      m_ext->setCoordsPreserveCartFrac(Cartesian);
      m_ext->setCurrentCellMatrix(mat);
      m_ext->setCoordsPreserveCartFrac(existingPreserveCartFrac);

      // Finish up
      m_ext->rebuildBonds();
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
    settings.beginGroup("crystallographyextension");
    settings.beginGroup(this->metaObject()->className());

    ui.spin_slab_x->setValue(settings.value("x", 10.0).toDouble());
    ui.spin_slab_y->setValue(settings.value("y", 10.0).toDouble());
    ui.spin_slab_z->setValue(settings.value("z", 5.0).toDouble());

    ui.spin_mi_h->setValue(settings.value("mi_h", 0).toInt());
    ui.spin_mi_k->setValue(settings.value("mi_k", 0).toInt());
    ui.spin_mi_l->setValue(settings.value("mi_l", 0).toInt());

    settings.endGroup(); // slabbuilder
    settings.endGroup(); // crystallography
  }

  void CESlabBuilder::writeSettings()
  {
    QSettings settings;
    settings.beginGroup("crystallographyextension");
    settings.beginGroup(this->metaObject()->className());

    settings.setValue("x", ui.spin_slab_x->value());
    settings.setValue("y", ui.spin_slab_y->value());
    settings.setValue("z", ui.spin_slab_z->value());

    settings.setValue("mi_h", ui.spin_mi_h->value());
    settings.setValue("mi_k", ui.spin_mi_k->value());
    settings.setValue("mi_l", ui.spin_mi_l->value());

    settings.endGroup(); // slabbuilder
    settings.endGroup(); // crystallography
  }

}
