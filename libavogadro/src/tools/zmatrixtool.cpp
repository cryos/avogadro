/**********************************************************************
  ZMatrixTool - Z Matrix Tool

  Copyright (C) 2009 Marcus D. Hanwell

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

#include "zmatrixtool.h"

#include "zmatrixdialog.h"

#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace Avogadro {

  ZMatrixTool::ZMatrixTool(QObject *parent) : Tool(parent), m_molecule(0),
      m_settingsWidget(0), m_zMatrixDialog(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QLatin1String(":/zmatrix/zmatrix.png")));
  }

  ZMatrixTool::~ZMatrixTool()
  {
  }

  QUndoCommand * ZMatrixTool::mousePressEvent(GLWidget *widget,
                                              QMouseEvent *event)
  {
    m_glwidget = widget;
    m_molecule = widget->molecule();
    return 0;
  }

  QUndoCommand * ZMatrixTool::mouseReleaseEvent(GLWidget *widget,
                                                QMouseEvent *event)
  {
    return 0;
  }

  QUndoCommand * ZMatrixTool::mouseMoveEvent(GLWidget *widget,
                                             QMouseEvent *event)
  {
    return 0;
  }

  QUndoCommand * ZMatrixTool::wheelEvent(GLWidget *widget,
                                         QWheelEvent *event)
  {
    return 0;
  }

  bool ZMatrixTool::paint(GLWidget *widget)
  {
    m_glwidget = widget;
    m_molecule = widget->molecule();
    return true;
  }

  QWidget * ZMatrixTool::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new QWidget;
      // Z matrix editor button
      QPushButton *zMatrixButton = new QPushButton(m_settingsWidget);
      zMatrixButton->setText(tr("Z Matrix Editor..."));
      QHBoxLayout* zMatrixLayout = new QHBoxLayout;
      zMatrixLayout->addStretch(1);
      zMatrixLayout->addWidget(zMatrixButton);
      zMatrixLayout->addStretch(1);
      connect(zMatrixButton, SIGNAL(clicked()), this, SLOT(showZMatrixDialog()));
      QVBoxLayout *layout = new QVBoxLayout();
      layout->addLayout(zMatrixLayout);
      m_settingsWidget->setLayout(layout);
      return m_settingsWidget;
    }
    return m_settingsWidget;
  }

  void ZMatrixTool::showZMatrixDialog()
  {
    if (!m_zMatrixDialog) {
      m_zMatrixDialog = new ZMatrixDialog(m_settingsWidget);
      m_zMatrixDialog->setMolecule(m_molecule);
    }
    if (m_zMatrixDialog->isVisible())
      m_zMatrixDialog->hide();
    else
      m_zMatrixDialog->show();
  }

}

Q_EXPORT_PLUGIN2(zmatrixtool, Avogadro::ZMatrixToolFactory)

