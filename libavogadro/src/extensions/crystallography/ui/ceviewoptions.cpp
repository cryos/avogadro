/**********************************************************************
  ceviewoptions.cpp

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

#include "ceviewoptions.h"

#include "../ceundo.h"

#include <avogadro/atom.h>
#include <avogadro/camera.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

namespace Avogadro
{
  CEViewOptions::CEViewOptions(CrystallographyExtension *ext,
                               QMainWindow *w)
    : CEAbstractEditor(ext, w)
  {
    ui.setupUi(this);

    connect(ui.aCellSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateRepeatCells()));
    connect(ui.bCellSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateRepeatCells()));
    connect(ui.cCellSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateRepeatCells()));

    connect(ui.spin_mi_h, SIGNAL(valueChanged(int)),
            this, SLOT(updateMillerPlane()));
    connect(ui.spin_mi_k, SIGNAL(valueChanged(int)),
            this, SLOT(updateMillerPlane()));
    connect(ui.spin_mi_l, SIGNAL(valueChanged(int)),
            this, SLOT(updateMillerPlane()));

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

    // Check if we have a hexagonal unit cell for mi_i box
    cellChanged();
  }

  void CEViewOptions::updateRepeatCells()
  {
    //@todo this isn't right with multiple windows
    GLWidget::current()->setUnitCells( ui.aCellSpinBox->value(),
                                       ui.bCellSpinBox->value(),
                                       ui.cCellSpinBox->value() );
  }

  void CEViewOptions::updateMillerPlane()
  {
    // View into a Miller plane
    Camera *currentCamera = GLWidget::current()->camera();
    Eigen::Transform3d modelView = currentCamera->modelview();

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

    // set the camera's matrix to be (the 4x4 version of) this rotation.
    modelView.linear() = rotation;
    currentCamera->setModelview(modelView);

    // Call for a redraw
    GLWidget::current()->update();
  }

  void CEViewOptions::cellChanged()
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


}
