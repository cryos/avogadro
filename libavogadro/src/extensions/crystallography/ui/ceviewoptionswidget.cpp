/**********************************************************************
  ceviewoptionswidget.cpp

  Copyright (C) 2011 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

#include <QtCore/QDebug>

namespace Avogadro
{
  CEViewOptionsWidget::CEViewOptionsWidget(CrystallographyExtension *ext)
    : CEAbstractDockWidget(ext),
      m_currentArea(Qt::NoDockWidgetArea)
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

    connect(ui.push_axis_update, SIGNAL(clicked()),
            this, SLOT(updateCamera()));
    connect(ui.spin_mi_h, SIGNAL(valueChanged(int)),
            this, SLOT(selectMillerRadio()));
    connect(ui.spin_mi_k, SIGNAL(valueChanged(int)),
            this, SLOT(selectMillerRadio()));
    connect(ui.spin_mi_l, SIGNAL(valueChanged(int)),
            this, SLOT(selectMillerRadio()));

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

    // Rearrange the widgets when we change from left/right to top/bottom
    // position
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(updateLayout(Qt::DockWidgetArea)));
    // Also update when the floating state changes
    connect(this, SIGNAL(topLevelChanged(bool)),
            this, SLOT(updateLayout(bool)));

    this->updateLayout(this->isFloating());

    // Check if we have a hexagonal unit cell for mi_i box
    cellChanged();
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
    Eigen::Transform3d modelView;
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

  void CEViewOptionsWidget::selectMillerRadio()
  {
    if (!ui.rad_axis_miller->isChecked())
      ui.rad_axis_miller->setChecked(true);
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
