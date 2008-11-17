/**********************************************************************
  POVRayExtension - Extension for generating POV-Ray rendered images

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "povrayextension.h"

#include "povpainter.h"

#include <avogadro/toolgroup.h>

#include <QFileDialog>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QProcess>
#include <QDebug>

namespace Avogadro
{

  using Eigen::Vector3d;

  POVRayExtension::POVRayExtension(QObject* parent) : Extension(parent),
    m_glwidget(0), m_POVRayDialog(0), m_molecule(0), m_process(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("POV-Ray..."));
    m_actions.append(action);
  }

  POVRayExtension::~POVRayExtension()
  {
    if (m_POVRayDialog)
    {
      delete m_POVRayDialog;
      m_POVRayDialog = 0;
    }
  }

  QList<QAction *> POVRayExtension::actions() const
  {
    return m_actions;
  }

  QString POVRayExtension::menuPath(QAction*) const
  {
    return tr("&File") + '>' + tr("Export");
  }

  QUndoCommand* POVRayExtension::performAction(QAction *, GLWidget *widget)
  {
    m_glwidget = widget;
    if (!m_POVRayDialog)
    {
      m_POVRayDialog = new POVRayDialog();
      m_POVRayDialog->setImageSize(m_glwidget->width(), m_glwidget->height());
      connect(m_POVRayDialog, SIGNAL(render()),
              this, SLOT(render()));
      connect(m_glwidget, SIGNAL(resized()),
              m_POVRayDialog, SLOT(resized()));
      m_POVRayDialog->show();
    }
    else {
      m_POVRayDialog->setImageSize(m_glwidget->width(), m_glwidget->height());
      m_POVRayDialog->show();
    }
    return 0;
  }

  void POVRayExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void POVRayExtension::render()
  {
    // Render the scene using POV-Ray
    m_process = new QProcess(this);
    QString fileName = m_POVRayDialog->fileName().mid(0,
                                m_POVRayDialog->fileName().lastIndexOf("."));
    if(fileName.isEmpty()) {
      return;
    }
    double aspectRatio = m_POVRayDialog->imageWidth() / m_POVRayDialog->imageHeight();
    POVPainterDevice pd(fileName + ".pov", aspectRatio, m_glwidget);
    // Set up the environment for the process
    QStringList env = QProcess::systemEnvironment();
    env << "POVRAY_BETA=925866419";
    m_process->setEnvironment(env);
//    qDebug() << "Env:" << env.join("\n");
    //m_process->start("povray", m_POVRayDialog->commandLine());
    qDebug() << "Command:" << "povray " + m_POVRayDialog->commandLine().join(" ");
    m_process->start("povray " + m_POVRayDialog->commandLine().join(" "));
    qDebug() << "Rendering started...";
    if (!m_process->waitForStarted()) {
      qDebug() << "POV-Ray never started!";
    }
    if (!m_process->waitForFinished()) {
      qDebug() << "POV-Ray never finished!";
    }
    QByteArray result = m_process->readAll();
    qDebug() << "POV-Ray output:" << result << "Exit code:"
             << m_process->exitCode();
  }

} // End namespace Avogadro

#include "povrayextension.moc"

Q_EXPORT_PLUGIN2(povrayextension, Avogadro::POVRayExtensionFactory)
