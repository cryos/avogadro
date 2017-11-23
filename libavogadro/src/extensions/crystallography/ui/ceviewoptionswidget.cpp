/**********************************************************************
  ceviewoptionswidget.cpp

  Copyright (C) 2011 by Geoffrey R. Hutchison
  Copyright (C) 2012 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#include "ceviewoptionswidget.h"

#include "../ceundo.h"

#include <avogadro/atom.h>
#include <avogadro/camera.h>
#include <avogadro/color.h>
#include <avogadro/engine.h>
#include <avogadro/glwidget.h>
#include <avogadro/obeigenconv.h>
#include <avogadro/primitivelist.h>

#include <openbabel/mol.h>

#include <QtGui/QColorDialog>

#include <QtCore/QDebug>

namespace Avogadro
{
  CEViewOptionsWidget::CEViewOptionsWidget(CrystallographyExtension *ext)
    : CEAbstractDockWidget(ext),
      m_glWidget(NULL),
      m_molecule(NULL),
      m_extraImagesMolecule(new Molecule()),
      m_currentArea(Qt::NoDockWidgetArea),
      m_ncc(NCC_Invalid),
      m_colorDialog(0),
      m_origColor(new QColor()),
      m_displayAllAtomImages(false)
  {
    this->setPreferredDockWidgetArea(Qt::BottomDockWidgetArea);

    ui.setupUi(this);

    // Initialize the view axis button group
    ui.rad_axis_default->setChecked(true);

    connect(ui.aCellSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateRepeatCells()));
    connect(ui.bCellSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateRepeatCells()));
    connect(ui.cCellSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateRepeatCells()));

    connect(ui.spin_mi_h, SIGNAL(valueChanged(int)),
            this, SLOT(millerIndexChanged()));
    connect(ui.spin_mi_k, SIGNAL(valueChanged(int)),
            this, SLOT(millerIndexChanged()));
    connect(ui.spin_mi_l, SIGNAL(valueChanged(int)),
            this, SLOT(millerIndexChanged()));
    connect(ui.buttonGroup_camera, SIGNAL(buttonClicked(int)),
            this, SLOT(updateCamera()));

    connect(ui.combo_numCells, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateCellRenderOptions()));

    connect(ui.push_changeColor, SIGNAL(clicked()),
            this, SLOT(selectCellColor()));

    /*
    connect(ui.aButton, SIGNAL(clicked()),
            this, SLOT(updateViewAxis()));
    connect(ui.aStarButton, SIGNAL(clicked()),
            this, SLOT(updateViewAxis()));
    connect(ui.bButton, SIGNAL(clicked()),
            this, SLOT(updateViewAxis()));
    connect(ui.aStarButton, SIGNAL(clicked()),
            this, SLOT(updateViewAxis()));
    connect(ui.cButton, SIGNAL(clicked()),
            this, SLOT(updateViewAxis()));
    connect(ui.cStarButton, SIGNAL(clicked()),
            this, SLOT(updateViewAxis()));
    */

    connect(ext, SIGNAL(cellChanged()),
            this, SLOT(cellChanged()));
    connect(ext, SIGNAL(cellChanged()),
            this, SLOT(resetExtraAtomImages()));

    // Rearrange the widgets when we change from left/right to top/bottom
    // position
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(updateLayout(Qt::DockWidgetArea)));
    // Also update when the floating state changes
    connect(this, SIGNAL(topLevelChanged(bool)),
            this, SLOT(updateLayout(bool)));

    // Display all atom images?
    connect(ui.cb_displayAllAtomImages, SIGNAL(toggled(bool)),
            this, SLOT(setDisplayAllAtomImages(bool)));

    this->updateLayout(this->isFloating());

    // Check if we have a hexagonal unit cell for mi_i box
    cellChanged();

    QSettings settings;
    int ncc = settings.value("crystallography/viewWidget/numCellChoice",
                             static_cast<int>(NCC_All)).toInt();
    ui.combo_numCells->setCurrentIndex(ncc);
  }

  CEViewOptionsWidget::~CEViewOptionsWidget()
  {
    delete m_extraImagesMolecule;
    if (m_colorDialog) {
      this->rejectColor(); // This will delete the dialog, too
    }

    delete m_origColor;
    m_origColor = NULL;
  }

  void CEViewOptionsWidget::makeMoleculeConnections()
  {
    if (m_molecule) {
      connect(m_molecule, SIGNAL(atomAdded(Atom*)),
              this, SLOT(resetExtraAtomImages()));
      connect(m_molecule, SIGNAL(atomUpdated(Atom*)),
              this, SLOT(resetExtraAtomImages()));
      connect(m_molecule, SIGNAL(atomRemoved(Atom*)),
              this, SLOT(resetExtraAtomImages()));
    }
  }

  void CEViewOptionsWidget::disconnectMoleculeConnections()
  {
    if (m_molecule)
      m_molecule->disconnect(this);
  }

  void CEViewOptionsWidget::setGLWidget(GLWidget *w)
  {
    m_glWidget = w;
    // Cache the pointer to the molecule...
    if (m_glWidget)
      setMolecule(m_glWidget->molecule());
  }

  void CEViewOptionsWidget::setMolecule(Molecule *m)
  {
    disconnectMoleculeConnections();
    m_molecule = m;
    makeMoleculeConnections();
  }

  void CEViewOptionsWidget::updateRepeatCells()
  {
    m_glWidget->setUnitCells( ui.aCellSpinBox->value(),
                              ui.bCellSpinBox->value(),
                              ui.cCellSpinBox->value() );
  }

  void CEViewOptionsWidget::updateCamera()
  {
    if (ui.rad_axis_miller->isChecked()) {
      this->updateMillerPlane();
    }
    else if (ui.rad_axis_default->isChecked()) {
      m_glWidget->updateGeometry();
      m_glWidget->camera()->initializeViewPoint();
      m_glWidget->update();
    }
  }

  void CEViewOptionsWidget::updateMillerPlane()
  {
    // View into a Miller plane
    Camera *camera = m_glWidget->camera();
    Eigen::Projective3d modelView;
    modelView.setIdentity();

    // OK, so we want to rotate to look along the normal at the plane
    // So we convert <h k l> into a Cartesian normal
    Eigen::Matrix3d cellMatrix = m_ext->unconvertLength(m_ext->currentCellMatrix()).transpose();
    // Get miller indices:
    const Eigen::Vector3d millerIndices
      (static_cast<double>(ui.spin_mi_h->value()),
       static_cast<double>(ui.spin_mi_k->value()),
       static_cast<double>(ui.spin_mi_l->value()));
    // Check to see if we have 0,0,0
    //  in which case, we do nothing
    if (millerIndices.squaredNorm() < 0.5)
      return;

    const Eigen::Vector3d normalVector ((cellMatrix * millerIndices).normalized());

    Eigen::Matrix3d rotation;
    rotation.row(2) = normalVector;
    rotation.row(0) = rotation.row(2).unitOrthogonal();
    rotation.row(1) = rotation.row(2).cross(rotation.row(0));

    // Translate camera to the center of the cell
    const Eigen::Vector3d cellDiagonal =
        cellMatrix.col(0) * m_glWidget->aCells() +
        cellMatrix.col(1) * m_glWidget->bCells() +
        cellMatrix.col(2) * m_glWidget->cCells();

    modelView.translate(-cellDiagonal*0.5);

    // Prerotate the camera to look down the specified normal
    modelView.prerotate(rotation);


    // Pretranslate in the negative Z direction
    modelView.pretranslate(Eigen::Vector3d(0.0, 0.0,
                                           -1.5 * cellDiagonal.norm()));

    camera->setModelview(modelView);

    // Call for a redraw
    m_glWidget->update();
  }

  void CEViewOptionsWidget::millerIndexChanged()
  {
    if (!ui.rad_axis_miller->isChecked())
      ui.rad_axis_miller->click();
    else
      this->updateCamera();
  }

  void CEViewOptionsWidget::updateCellRenderOptions()
  {
    if (m_glWidget == NULL) {
      return;
    }

    NumCellChoice ncc = static_cast<NumCellChoice>
        (ui.combo_numCells->currentIndex());

    if (m_ncc != ncc) {
      switch (ncc)
      {
      case Avogadro::CEViewOptionsWidget::NCC_Invalid:
        // Should happen, probably not initialized.
        qWarning() << "NumCellChoice is invalid.";
        break;
      case Avogadro::CEViewOptionsWidget::NCC_None:
        m_glWidget->setRenderUnitCellAxes(false);
        break;
      case Avogadro::CEViewOptionsWidget::NCC_One:
        m_glWidget->setRenderUnitCellAxes(true);
        m_glWidget->setOnlyRenderOriginalUnitCell(true);
        break;
      case Avogadro::CEViewOptionsWidget::NCC_All:
        m_glWidget->setRenderUnitCellAxes(true);
        m_glWidget->setOnlyRenderOriginalUnitCell(false);
        break;
      default:
        qWarning() << "Unknown numCellChoice:" << ncc;
        break;
      }
      m_ncc = ncc;

      m_glWidget->update();
    }
  }

  void CEViewOptionsWidget::setDisplayAllAtomImages(bool b)
  {
    m_displayAllAtomImages = b;

    if (!m_glWidget)
      return;

    if (m_displayAllAtomImages)
      generateExtraAtomImages();
    else
      removeExtraAtomImages();
  }

  // Whatever we pass to this needs to have values for [0], [1], and [2]
  template<typename T1, typename T2>
  inline double distance(const T1& a, const T2& b)
  {
    return sqrt(pow(a[0] - b[0], 2.0) +
                pow(a[1] - b[1], 2.0) +
                pow(a[2] - b[2], 2.0));
  }

  bool atomAlreadyPresent(Atom* atom, const QList<Atom*>& atoms, double tol)
  {
    for (int i = 0; i < atoms.size(); ++i) {
      if (distance(*atoms[i]->pos(), *atom->pos()) < tol)
        return true;
    }
    return false;
  }

  void CEViewOptionsWidget::generateExtraAtomImages()
  {
    if (!m_glWidget || !m_molecule || !m_molecule->OBUnitCell())
      return;

    OpenBabel::OBUnitCell* cell = m_molecule->OBUnitCell();

    QList<Atom*> atoms = m_molecule->atoms();

    // Set a tolerance. We will use 1e-5.
    double cartTol = 1e-5;
    double fracTol = 1e-5;

    // Loop through A, B, and C
    for (unsigned short vecInd = 0; vecInd < 3; ++vecInd) {
      for (int i = 0; i < atoms.size(); ++i) {
        OpenBabel::vector3 fracPos =
          cell->CartesianToFractional(Eigen2OB(*atoms[i]->pos()));

        // This will be true if the atom is close to the face of interest
        if (std::fabs(fracPos[vecInd] - 0.0) < fracTol ||
            std::fabs(fracPos[vecInd] - 1.0) < fracTol) {
          Atom* newAtom = m_extraImagesMolecule->addAtom();
          newAtom->setAtomicNumber(atoms[i]->atomicNumber());
          OpenBabel::vector3 newFracPos = fracPos;

          // Unfortunately, we can't do vector3[vecInd] = 1.0 ...
          OpenBabel::vector3 one(0.0, 0.0, 0.0);
          if (vecInd == 0)
            one.x() = 1.0;
          else if (vecInd == 1)
            one.y() = 1.0;
          else
            one.z() = 1.0;
          // Check if we are near (in frac coords) 0.0 or if we are near 1.0
          if (std::fabs(fracPos[vecInd] - 0.0) < fracTol)
            newFracPos += one;
          else
            newFracPos -= one;
          newAtom->setPos(OB2Eigen(cell->FractionalToCartesian(newFracPos)));
          if (!atomAlreadyPresent(newAtom, atoms, cartTol))
            atoms.append(newAtom);
          else
            // This should delete the atom
            m_extraImagesMolecule->removeAtom(newAtom);
        }
      }
    }

    QList<Atom*> extraImages = m_extraImagesMolecule->atoms();

    // Add the atoms to each of the engines that draw atoms
    QList<Engine*> engines = m_glWidget->engines();

    for (int i = 0; i < engines.size(); ++i) {
      // Does this engine draw atoms?
      if (engines[i]->primitiveTypes() & Engine::Atoms) {
        // If yes, add the extra atom images
        for (int j = 0; j < extraImages.size(); ++j)
          engines[i]->addAtomImage(extraImages[j]);
      }
    }
  }

  void CEViewOptionsWidget::removeExtraAtomImages()
  {
    QList<Atom*> extraAtomImages = m_extraImagesMolecule->atoms();
    if (extraAtomImages.empty())
      return;

    if (!m_glWidget) {
      m_extraImagesMolecule->clear();
      return;
    }

    // Remove the atoms from each of the engines that draw atoms
    QList<Engine*> engines = m_glWidget->engines();

    for (int i = 0; i < engines.size(); ++i) {
      // Does this engine draw atoms?
      if (engines[i]->primitiveTypes() & Engine::Atoms) {
        // If yes, remove all the extra atoms
        for (int j = 0; j < extraAtomImages.size(); ++j)
          engines[i]->removeAtomImage(extraAtomImages[j]);
      }
    }
    m_extraImagesMolecule->clear();
  }

  void CEViewOptionsWidget::resetExtraAtomImages()
  {
    removeExtraAtomImages();
    if (m_displayAllAtomImages)
      generateExtraAtomImages();
  }

  void CEViewOptionsWidget::selectCellColor()
  {
    if (m_colorDialog == NULL) {

      if (!m_glWidget)
        return;

      // Get current color from GLWidget
      *m_origColor = m_glWidget->unitCellColor().color();

      m_colorDialog = new QColorDialog (this);
      m_colorDialog->setWindowTitle(tr("Set Unit Cell Color"));
      m_colorDialog->setOption(QColorDialog::ShowAlphaChannel);
      m_colorDialog->setCurrentColor(*m_origColor);

      // Interactive preview
      connect(m_colorDialog, SIGNAL(currentColorChanged(QColor)),
              this, SLOT(previewColor(QColor)));

      // Keep or revert colors
      connect(m_colorDialog, SIGNAL(accepted()),
              this, SLOT(acceptColor()));
      connect(m_colorDialog, SIGNAL(rejected()),
              this, SLOT(rejectColor()));
    }

    m_colorDialog->show();
  }

  void CEViewOptionsWidget::previewColor(const QColor &color)
  {
    if (m_glWidget == NULL ||
        m_colorDialog == NULL)
      return;

    m_glWidget->setUnitCellColor(color);
    m_glWidget->update();
  }

  void CEViewOptionsWidget::acceptColor()
  {
    if (m_glWidget == NULL ||
        m_colorDialog == NULL)
      return;

    // Store color for later
    Color c = m_glWidget->unitCellColor();
    QSettings settings;
    settings.beginGroup("crystallographyextension/settings/cellColor");
    settings.setValue("r", c.red());
    settings.setValue("g", c.green());
    settings.setValue("b", c.blue());
    settings.setValue("a", c.alpha());
    settings.endGroup();

    m_glWidget->setUnitCellColor(m_colorDialog->currentColor());
    m_glWidget->update();
    this->cleanupColorDialog();
  }

  void CEViewOptionsWidget::rejectColor()
  {
    if (m_glWidget == NULL ||
        m_colorDialog == NULL)
      return;

    m_glWidget->setUnitCellColor(*m_origColor);
    m_glWidget->update();
    this->cleanupColorDialog();
  }

  void CEViewOptionsWidget::cleanupColorDialog()
  {
    if (m_colorDialog == NULL)
      return;

    m_colorDialog->hide();
    disconnect(m_colorDialog, NULL, this, NULL);
    m_colorDialog->deleteLater();
    m_colorDialog = NULL;
  }

  void CEViewOptionsWidget::cellChanged()
  {
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
  }

  void CEViewOptionsWidget::updateLayout(bool isFloating)
  {
    if (isFloating) {
      this->updateLayout(Qt::NoDockWidgetArea);
    }
  }

  void CEViewOptionsWidget::updateLayout(Qt::DockWidgetArea newArea)
  {
    if (this->isFloating()) {
      newArea = Qt::NoDockWidgetArea;
    }
    if (newArea == m_currentArea) {
      return;
    }

    switch (newArea) {
    case Qt::NoDockWidgetArea: // Treat floating as left/right configuration
    case Qt::LeftDockWidgetArea:
    case Qt::RightDockWidgetArea: {
      switch (m_currentArea) {
      // Did we just switch sides? Keep the current layout.
      case Qt::LeftDockWidgetArea:
      case Qt::RightDockWidgetArea:
      case Qt::NoDockWidgetArea: // Treat floating as left/right configuration
        break;
      // Otherwise, rearrange widgets
      default:
      case Qt::TopDockWidgetArea:
      case Qt::BottomDockWidgetArea:
      case Qt::AllDockWidgetAreas: // Invalid, always reset
        ui.grid->removeWidget(ui.gb_cell);
        ui.grid->removeWidget(ui.gb_camera);
        ui.grid->addWidget(ui.gb_cell, 0, 0);
        ui.grid->addWidget(ui.gb_camera, 1, 0);
        break;
      }
      break;
    }
    case Qt::TopDockWidgetArea:
    case Qt::BottomDockWidgetArea: {
      switch (m_currentArea) {
      // Did we just switch sides? Keep the current layout.
      case Qt::TopDockWidgetArea:
      case Qt::BottomDockWidgetArea:
        break;
      // Otherwise, rearrange widgets
      default:
      case Qt::LeftDockWidgetArea:
      case Qt::RightDockWidgetArea:
      case Qt::NoDockWidgetArea: // Treat floating as left/right configuration
      case Qt::AllDockWidgetAreas: // Invalid, always reset
        ui.grid->removeWidget(ui.gb_cell);
        ui.grid->removeWidget(ui.gb_camera);
        ui.grid->addWidget(ui.gb_cell, 0, 0);
        ui.grid->addWidget(ui.gb_camera, 0, 1);
        break;
      }
      break;
    }
      // Invalid case, just leave everything as is.
    default:
    case Qt::AllDockWidgetAreas:
      break;
    }

    // Update the current area
    m_currentArea = newArea;

    // Shrink down as small as we can
    /* This isn't working... removing for now.
    this->updateGeometry();
    QRect geo = this->geometry();
    this->setGeometry(geo.x(), geo.y(), 0, 0);// minSize.width(), minSize.height());
    */
  }

} // end namespace Avogadro
