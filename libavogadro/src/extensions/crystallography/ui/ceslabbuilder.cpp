/**********************************************************************
  ceslabbuilder Widget for slab builder

  Copyright (C) 2011 by David C. Lonie
  Copyright (C) 2011,2012 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "config.h"

#include "ceslabbuilder.h"

#include "../ceundo.h"
#include "../crystallographyextension.h"

#include <avogadro/glwidget.h>
#include <avogadro/painter.h>
#include <avogadro/atom.h>

#include <Eigen/Geometry>

#include <QtGui/QCloseEvent>
#include <QtGui/QHideEvent>

#include <QDebug>

#include <algorithm>

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

  // Comparison for sorting possible surface lattice vectors
  bool vectorNormIsLessThan(Eigen::Vector3d a, Eigen::Vector3d b) {
    return (a.squaredNorm() < b.squaredNorm());
  }

  CESlabBuilder::CESlabBuilder(CrystallographyExtension *ext)
    : CEAbstractDockWidget(ext),
      m_glwidget(NULL),
      m_beforeState(0),
      m_finished(false)
  {
    ui.setupUi(this);
    // Hide the "i" Miller index if not a hexagonal unit cell
    ui.spin_mi_i->hide();

    // Hide the Miller index warning label by default
    ui.warningLabel->hide();
    readSettings(); // hides or shows the warning if needed

    // Add the suffix to the xyz distances
    updateLengthUnit();
    connect(m_ext, SIGNAL(lengthUnitChanged(LengthUnit)),
            this, SLOT(updateLengthUnit()));

    // Update the Miller indices
    connect(ui.spin_mi_h, SIGNAL(valueChanged(int)),
            this, SLOT(updateMillerIndices()));
    connect(ui.spin_mi_k, SIGNAL(valueChanged(int)),
            this, SLOT(updateMillerIndices()));
    connect(ui.spin_mi_l, SIGNAL(valueChanged(int)),
            this, SLOT(updateMillerIndices()));
    // No need to update "i" index -- it's locked and updated from h & k

    // If the x/y cutoffs change, we'll need to append a suffix for the unit
    connect(ui.xWidthUnits, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateLengthUnit()));
    connect(ui.yWidthUnits, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateLengthUnit()));

    connect(ui.buildButton, SIGNAL(clicked(bool)),
            this, SLOT(buildSlab()));
  }

  CESlabBuilder::~CESlabBuilder()
  {
    // @todo Revise to handle a general Miller Plane engine
    // Reset cube variants
    /*
      Molecule *mol = m_glwidget->molecule();
      mol->setProperty("Slab Cell offset", QVariant());
      mol->setProperty("Slab Cell v1", QVariant());
      mol->setProperty("Slab Cell v2", QVariant());
      mol->setProperty("Slab Cell v3", QVariant());
    */

    // Restore crystal to "before" state if the builder didn't finish
    if (!m_finished && m_beforeState != NULL) {
      m_beforeState->apply();
      delete m_beforeState;
      return;
    }
    delete m_beforeState;
  }

  void CESlabBuilder::closeEvent(QCloseEvent *event)
  {
    CEAbstractDockWidget::closeEvent(event);
  }

  void CESlabBuilder::hideEvent(QHideEvent *event)
  {
    CEAbstractDockWidget::hideEvent(event);
  }

  void CESlabBuilder::updateLengthUnit()
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

    ui.xWidthUnits->setItemText(0, lengthSuffix);
    ui.yWidthUnits->setItemText(0, lengthSuffix);

    if (ui.xWidthUnits->currentIndex() == 0)
      ui.spin_slab_x->setSuffix(lengthSuffix);
    else
      ui.spin_slab_x->setSuffix("");

    if (ui.yWidthUnits->currentIndex() == 0)
      ui.spin_slab_y->setSuffix(lengthSuffix);
    else
      ui.spin_slab_y->setSuffix("");

    ui.spin_slab_z->setSuffix(lengthSuffix);

  }

  void CESlabBuilder::updateMillerIndices()
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

    // Check if hexagonal or rhombohedral and show the redundant "i" index
    OpenBabel::OBUnitCell *cell = m_ext->currentCell();
    if (cell &&
        (cell->GetLatticeType() == OpenBabel::OBUnitCell::Rhombohedral
         || cell->GetLatticeType() == OpenBabel::OBUnitCell::Hexagonal)) {
      // i = -h - k
      // http://en.wikipedia.org/wiki/Miller_index#Case_of_hexagonal_and_rhombohedral_structures
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

    QApplication::setOverrideCursor( Qt::WaitCursor );

    updateSlabCell(true);

    QApplication::restoreOverrideCursor();
    ui.buildButton->setEnabled(true);
    ui.buildButton->setText(tr("Build"));

    // OK, get rid of ourselves to go back to normal crystal editing
    this->hide();
    emit finished();
  }

  void CESlabBuilder::updateSlabCell(bool build)
  {
    // Get miller indices:
    const Eigen::Vector3d millerIndices
      (static_cast<double>(ui.spin_mi_h->value()),
       static_cast<double>(ui.spin_mi_k->value()),
       static_cast<double>(ui.spin_mi_l->value()));

    // Get cell vectors
    Eigen::Matrix3d cellMatrix
      (m_ext->unconvertLength(m_ext->currentCellMatrix()).transpose());
    Eigen::Vector3d v1 (cellMatrix.col(0));
    Eigen::Vector3d v2 (cellMatrix.col(1));
    Eigen::Vector3d v3 (cellMatrix.col(2));

    if (build) {
      m_beforeState = new CEUndoState (m_ext);

      // First, wrap all the atoms into the cell
      // And if symmetry is defined, fill out
      // the symmetric atoms
      m_ext->fillUnitCell();
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
      //      qWarning() << "No non-zero miller index...";
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

    // Grab mol from our widget
    // We can't do this any earlier, since the molecule could change
    Molecule *mol = m_glwidget->molecule();

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

    // Generate new surface unit cell vectors
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
    Eigen::Vector3d s1, s2, s3;
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
    std::sort(surfaceVectors.begin(), surfaceVectors.end(), vectorNormIsLessThan);

    // Set s1 to the surface normal
    s1 = normalVec.normalized();
    // Set s2 to the shortest vector
    s2 = surfaceVectors[0].normalized();
    // Now loop through to find the next-shortest orthogonal to s1
    //  and mostly orthogonal to s2
    unsigned int nextDir;
    for (nextDir = 1; nextDir < surfaceVectors.size(); ++nextDir) {
      if (s1.cross(surfaceVectors[nextDir]).squaredNorm() > 0.8
          && s2.cross(surfaceVectors[nextDir]).squaredNorm() > THRESH)
          break;
    }
    s3 = surfaceVectors[nextDir];

    // Now we set up the normalized transformation matrix
    // We want s1 on the z-axis, and s2 on the x-axis
    // So we need to take the cross for the y-axis
    Eigen::Matrix3d rotation;
    rotation.row(0) = s2;
    rotation.row(1) = s2.cross(s1);
    rotation.row(2) = s1;

    // OK, now we un-normalize s1 and s2
    // The correct length for s1 should be the depth
    s1 *= d;
    // And we still have s2's un-normalized version
    s2 = surfaceVectors[0];
    // S3 is already un-normalized

#ifdef DEBUG
    std::cout << std::endl << s1.transpose()
              << std::endl << s2.transpose()
              << std::endl << s3.transpose()
              << std::endl << (rotation * s2).transpose()
              << std::endl << (rotation * s3).transpose()
              << std::endl;
#endif

    if (build) {
      // Estimate how many replicas we need to generate the unit cell
      // Get cutoff thresholds
      double zCutoff = ui.spin_slab_z->value();

      double maxUnitLength = std::max(v1.norm(), v2.norm());
      maxUnitLength = std::max(maxUnitLength, v3.norm());
      double maxSurfaceLength = std::max(s2.norm(), s3.norm());
      maxSurfaceLength = std::max(maxSurfaceLength, zCutoff);

      // Six times should be more than enough
      // We'll create the unit cell on the surface
      // And then replicate to fill out the user-requested dimensions
      const int replicas = static_cast<int>(6.0 * (maxSurfaceLength / maxUnitLength));
      m_ext->buildSuperCell(replicas, replicas, replicas);

      // Derive the unit cell matrix to allow building a supercell of the surface
      Eigen::Vector3d m1 = (rotation * (s2)); // Should be x-axis
      Eigen::Vector3d m2 = (rotation * (s3)); // should by y-axis

      // work out the number of repeat units
      double xCutoff = ui.spin_slab_x->value() / 2.0;
      double yCutoff = ui.spin_slab_y->value() / 2.0;
      double xSpacing = std::max(fabs(m1.x()), fabs(m2.x()));
      double ySpacing = std::max(fabs(m1.y()), fabs(m2.y()));

      int xRepeats, yRepeats;
      if (ui.xWidthUnits->currentIndex() == 1) {
        // spin box gave number of repeat units, not Cartesian cutoff
        xRepeats = ui.spin_slab_x->value();
        xCutoff = xRepeats * xSpacing / 2.0;
      }
      xCutoff += 1.0e-6; // add some slop for unit cell boundaries

      if (ui.yWidthUnits->currentIndex() == 1) {
        yRepeats = ui.spin_slab_y->value();
        yCutoff = yRepeats * ySpacing / 2.0;
      }
      yCutoff += 1.0e-6; // add some slop for unit cell boundaries

      // Here's the supercell matrix
      Eigen::Matrix3d surfaceMatrix;
      surfaceMatrix << m1.x(), m1.y(), 0.0,
        m2.x(), m2.y(), 0.0,
        0.0, 0.0, zCutoff*8;
      // The large z-spacing allows for surface/molecule calculations


      // Now rotate, translate, and trim the supercell
      Eigen::Vector3d translation(replicas*centerPoint);
      foreach(Atom *a, mol->atoms()) {
        if (a) {
          // Center the cube to the centerPoint of the Miller Plane
          Eigen::Vector3d translatedPos = (*a->pos() - translation);
          // Rotate to the new frame of reference
          Eigen::Vector3d newPos = rotation * (translatedPos);

          // OK, before we update the atom, see if we should trim it...
          if (newPos.z() > 0.01)
            // We use a slight slop factor, although in principle
            //   every atom should be in xy plane
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

      // We build everything in cartesian, so we want to preserve them when modifying the matrix
      CartFrac existingPreserveCartFrac = m_ext->coordsPreserveCartFrac();
      m_ext->setCoordsPreserveCartFrac(Cartesian);
      m_ext->setCurrentCellMatrix(m_ext->convertLength(surfaceMatrix));
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

    // show the label if needed
    if (ui.spin_mi_h->value() == 0
        && ui.spin_mi_k->value() == 0
        && ui.spin_mi_l->value() == 0)
      ui.warningLabel->show();
    else
      ui.warningLabel->hide();

    ui.xWidthUnits->setCurrentIndex(settings.value("x_units", 0).toInt());
    ui.yWidthUnits->setCurrentIndex(settings.value("y_units", 0).toInt());

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

    settings.setValue("x_units", ui.xWidthUnits->currentIndex());
    settings.setValue("y_units", ui.yWidthUnits->currentIndex());

    settings.endGroup(); // slabbuilder
    settings.endGroup(); // crystallography
  }

}
