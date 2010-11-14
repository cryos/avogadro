/**********************************************************************
  RotateSelectionExtension

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

#include "rotateselectionextension.h"
#include "rotateselectiondialog.h"

#include <avogadro/bond.h>
#include <avogadro/atom.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

#include <Eigen/Geometry>

#include <QtCore/QDebug>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>

#include "rotateselectiondialog.h"

#define DEG_TO_RAD 0.0174532925

using namespace Avogadro;

namespace RotateSelection {

  RotateSelectionExtension::RotateSelectionExtension(QObject *parent)
    : Extension(parent),
      m_dialog(0),
      m_gl(0)
  {
    // This block sets the text for menu entry
    QAction *action = new QAction(this);
    // Wrap all user visible strings in tr() so they can be translated
    action->setText(tr("0&4: Rotate Selection..."));
    m_actions.append(action);
  }

  QList<QAction *> RotateSelectionExtension::actions() const
  {
    return m_actions;
  }

  QString RotateSelectionExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions") + '>' + tr("&Tutorial");
  }

  QUndoCommand* RotateSelectionExtension::performAction( QAction *, GLWidget *gl )
  {
    m_gl = gl;
    // Create the dialog if needed
    if (!m_dialog) {
      m_dialog = new RotateSelectionDialog(qobject_cast<QWidget*>(parent()));
      connect(m_dialog, SIGNAL(requestUpdateAxisFromBond()),
              this, SLOT(updateAxisFromBond()));
      connect(m_dialog, SIGNAL(requestApply()),
              this, SLOT(apply()));
    }
    m_dialog->show();
    return NULL;
  }

  void RotateSelectionExtension::updateAxisFromBond()
  {
    // No widget?
    if (!m_gl) return;

    QList<Primitive*> selected =
        m_gl->selectedPrimitives().subList(Primitive::BondType);

    if (selected.size() != 1) {
      QMessageBox::warning(m_dialog,
                           tr("Rotate Selection"),
                           tr("Please select exactly one bond."));
      return;
    }

    Bond *bond = qobject_cast<Bond*>(selected.first());

    Eigen::Vector3d p1 = (*bond->beginPos());
    Eigen::Vector3d p2 = (*bond->endPos());

    m_dialog->setAxis(p2-p1, p1);
  }

  void RotateSelectionExtension::apply()
  {
    // No widget?
    if (!m_gl) return;

    QList<Primitive*> selected =
        m_gl->selectedPrimitives().subList(Primitive::AtomType);

    // Nothing to do:
    if (selected.size() == 0) {
      return;
    }

    Eigen::Vector3d axis, offset;
    double angle;
    m_dialog->getTransform(&axis,
                           &offset,
                           &angle);
    Eigen::AngleAxis<double> rot (angle * DEG_TO_RAD,
                                  axis.normalized());

    Atom *atom;
    Eigen::Vector3d coord;
    for (int i = 0; i < selected.size(); i++) {
      atom = qobject_cast<Atom*>(selected.at(i));
      coord = *atom->pos();
      coord -= offset;
      coord = rot * coord;
      coord += offset;
      atom->setPos(coord);
    }
    m_gl->update();
  }
}

// Include Qt moc'd headers
#include "rotateselectionextension.moc"

// Set up for the plugin to work correctly
Q_EXPORT_PLUGIN2(rotateselectionextension, RotateSelection::RotateSelectionExtensionFactory)
