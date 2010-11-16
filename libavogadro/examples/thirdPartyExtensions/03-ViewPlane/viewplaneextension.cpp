/**********************************************************************
  ViewPlaneExtension

  Copyright (C) 2010 David C. Lonie

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

#include "viewplaneextension.h"

#include <avogadro/atom.h>
#include <avogadro/camera.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

#include <Eigen/Geometry>

#include <QtGui/QAction>
#include <QtGui/QMessageBox>

using namespace Avogadro;

namespace ViewPlane {

  ViewPlaneExtension::ViewPlaneExtension(QObject *parent)
    : Extension(parent)
  {
    QAction *action = new QAction(this);
    action->setText(tr("0&3: View Plane..."));
    m_actions.append(action);
  }

  QList<QAction *> ViewPlaneExtension::actions() const
  {
    return m_actions;
  }

  QString ViewPlaneExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions") + '>' + tr("&Tutorial");
  }

  QUndoCommand* ViewPlaneExtension::performAction( QAction *, GLWidget *gl)
  {
    Camera *camera = gl->camera();
    if (!camera) {
      return 0;
    }

    QList<Primitive*> selected =
      gl->selectedPrimitives().subList(Primitive::AtomType);

    if (selected.size() != 3) {
      QMessageBox::warning(0, tr("View Plane"),
                           tr("Please select exactly three atoms."));
      return 0;
    }

    // Determine the vector normal to the plane
    Eigen::Vector3d v1, v2, normal;
    v1 = (*qobject_cast<Atom*>(selected.at(0))->pos())
      - (*qobject_cast<Atom*>(selected.at(1))->pos());
    v2 = (*qobject_cast<Atom*>(selected.at(0))->pos())
      - (*qobject_cast<Atom*>(selected.at(2))->pos());
    normal = v1.cross(v2);

    // determine our goal matrix
    Eigen::Matrix3d linearGoal;
    linearGoal.row(2) = normal.normalized();
    linearGoal.row(0) = linearGoal.row(2).unitOrthogonal();
    linearGoal.row(1) = linearGoal.row(2).cross(linearGoal.row(0));

    // calculate the translation matrix
    Eigen::Transform3d goal (linearGoal);

    goal.pretranslate(- 3.0 * (gl->radius() + CAMERA_NEAR_DISTANCE)
                      * Eigen::Vector3d::UnitZ());

    // Calculate the centroid of the selection
    Eigen::Vector3d selectedCenter(0.0, 0.0, 0.0);
    for (int i = 0; i < selected.size(); i++) {
      selectedCenter += *(qobject_cast<Atom*>(selected.at(i))->pos());
    }
    selectedCenter /= static_cast<double>(selected.size());
    goal.translate(-selectedCenter);

    camera->setModelview(goal);
    gl->update();
    return 0;
  }
}

// Include Qt moc'd headers
#include "viewplaneextension.moc"

// Set up for the plugin to work correctly
Q_EXPORT_PLUGIN2(viewplaneextension, ViewPlane::ViewPlaneExtensionFactory)
