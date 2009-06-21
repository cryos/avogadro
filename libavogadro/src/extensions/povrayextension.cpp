/**********************************************************************
  POVRayExtension - Extension for generating POV-Ray rendered images

  Copyright (C) 2008-2009 Marcus D. Hanwell

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

#include "povrayextension.h"

#include "povpainter.h"

#include <avogadro/molecule.h>
#include <avogadro/toolgroup.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
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
      m_POVRayDialog = new POVRayDialog(static_cast<QWidget*>(parent()));
//      m_POVRayDialog->setImageSize(m_glwidget->width(), m_glwidget->height());
      connect(m_POVRayDialog, SIGNAL(render()),
              this, SLOT(render()));
      connect(m_glwidget, SIGNAL(resized()),
              m_POVRayDialog, SLOT(resized()));
      m_POVRayDialog->show();
    }
    else {
//      m_POVRayDialog->setImageSize(m_glwidget->width(), m_glwidget->height());
      QFileInfo info(m_molecule->fileName());
      m_POVRayDialog->setFileName(info.absolutePath() + '/' + info.baseName()
                                  + ".png");
      m_POVRayDialog->show();
    }
    return 0;
  }

  void POVRayExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
    if (m_POVRayDialog) {
      m_POVRayDialog->writeSettings(settings);
    }
  }

  void POVRayExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
    if (m_POVRayDialog) {
      m_POVRayDialog->readSettings(settings);
    }
    else {
      m_POVRayDialog = new POVRayDialog(static_cast<QWidget*>(parent()));
      m_POVRayDialog->readSettings(settings);
      connect(m_POVRayDialog, SIGNAL(render()),
              this, SLOT(render()));
      // This is connecting a NULL on my Mac - Geoff
      // QObject::connect: Cannot connect (null)::resized() to Avogadro::POVRayDialog::resized()
      if (m_glwidget)
        connect(m_glwidget, SIGNAL(resized()),
                m_POVRayDialog, SLOT(resized()));
    }
  }

  void POVRayExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void POVRayExtension::render()
  {
    // Render the scene using POV-Ray
    QString fileName = m_POVRayDialog->fileName().mid(0,
                                m_POVRayDialog->fileName().lastIndexOf("."));

    // Check a filename was supplied
    if (fileName.isEmpty()) {
      QMessageBox::warning(m_POVRayDialog, tr("No filename supplied."),
                           tr("No valid filename was supplied."));
      return;
    }

    if (!m_POVRayDialog->renderDirect() && !m_POVRayDialog->keepSource()) {
      QMessageBox::warning(m_POVRayDialog, tr("Does not compute."),
                           tr("You requested no direct rendering using POV-Ray and not to keep the POV-Ray file. This will result in no output being saved. Are you sure that is what you want?"));
      return;
    }

    // Check that the povray executable exists - FIXME implement path search...
/*    QFileInfo info(m_POVRayDialog->command());
    if (!info.exists()) {
      QMessageBox::warning(m_POVRayDialog, "POV-Ray executable not found.",
                           "The POV-Ray executable, normally named 'povray', cannot be found.");
      return;
    } */

    // Check whether the .pov file can be written
    QFile povFile(fileName + ".pov");
    if (povFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      double aspectRatio = static_cast<double>(m_POVRayDialog->imageWidth())
                           / m_POVRayDialog->imageHeight();
      qDebug() << "Aspect ratio:" << aspectRatio;
      povFile.close();
      POVPainterDevice pd(fileName + ".pov", aspectRatio, m_glwidget);
    }
    else {
      QMessageBox::warning(m_POVRayDialog, tr("Cannot Write to File."),
                           tr("Cannot write to file %1. Do you have permissions to write to that location?").arg(fileName+".pov"));
      return;
    }

    if (m_POVRayDialog->renderDirect()) {
      m_process = new QProcess(this);
      QFile pngFile(fileName + ".png");
      if (pngFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        pngFile.close();
        QFileInfo info(fileName + ".png");
        m_process->setWorkingDirectory(info.absolutePath());
        m_process->start(m_POVRayDialog->command(), m_POVRayDialog->commandLine());
        qDebug() << "Command:" << m_POVRayDialog->command() + ' ' +
                 m_POVRayDialog->commandLine().join(" ");
        qDebug() << "Rendering started...";
        if (!m_process->waitForStarted()) {
          QMessageBox::warning(m_POVRayDialog, tr("POV-Ray failed to start."),
                               tr("POV-Ray failed to start. May be the path to the executable is not set correctly."));
        }
        connect(m_process, SIGNAL(finished(int)), this, SLOT(finished(int)));
      }
      else { // Should not really get here in general, but best to check
        QMessageBox::warning(m_POVRayDialog, tr("Cannot Write to File."),
                           tr("Cannot write to file %1. Do you have permissions to write to that location?").arg(fileName+".png"));
        return;
      }
    }
  }

  void POVRayExtension::finished(int exitCode)
  {
    if (!m_POVRayDialog->keepSource()) {
      QString fileName = m_POVRayDialog->fileName().mid(0,
                                   m_POVRayDialog->fileName().lastIndexOf("."));
      QFile povSource(fileName + ".pov");
      povSource.remove();
    }
    qDebug() << "Rendering complete.";
    QByteArray result = m_process->readAllStandardError();//m_process->readAll();
    qDebug() << "POV-Ray output:" << result << "Exit code:"
             << exitCode;
    disconnect(m_process, 0, this, 0);
    m_process->deleteLater();
    m_process = 0;
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(povrayextension, Avogadro::POVRayExtensionFactory)

