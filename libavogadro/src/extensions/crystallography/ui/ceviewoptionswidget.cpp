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
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

#include <QtGui/QColorDialog>

#include <QtCore/QDebug>

namespace Avogadro
{
  CEViewOptionsWidget::CEViewOptionsWidget(CrystallographyExtension *ext)
    : CEAbstractDockWidget(ext),
      m_glWidget(NULL),
      m_currentArea(Qt::NoDockWidgetArea),
      m_ncc(NCC_Invalid),
      m_colorDialog(0),
      m_origColor(new QColor())
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

    QSettings settings;
    int ncc = settings.value("crystallography/viewWidget/numCellChoice",
                             static_cast<int>(NCC_All)).toInt();
    ui.combo_numCells->setCurrentIndex(ncc);
  }

  CEViewOptionsWidget::~CEViewOptionsWidget()
  {
    if (m_colorDialog) {
      this->rejectColor(); // This will delete the dialog, too
    }

    delete m_origColor;
    m_origColor = NULL;
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
