/**********************************************************************
  POVRayDialog - Dialog to render images using POV-Ray

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

#include "povraydialog.h"

#include <QFileDialog>
#include <QString>
#include <QSettings>
#include <QDebug>

namespace Avogadro
{
  using Eigen::Vector3d;

  POVRayDialog::POVRayDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
  {
    ui.setupUi(this);
    connect(ui.selectFileName, SIGNAL(clicked()), this, SLOT(selectFileName()));
    connect(ui.render, SIGNAL(clicked()), this, SLOT(renderClicked()));
    connect(ui.dimX, SIGNAL(editingFinished()), this, SLOT(updateCommandLine()));
    connect(ui.dimY, SIGNAL(editingFinished()), this, SLOT(updateCommandLine()));
    connect(ui.antialias, SIGNAL(clicked()), this, SLOT(updateCommandLine()));
    connect(ui.transparency, SIGNAL(clicked()), this, SLOT(updateCommandLine()));
    connect(ui.povrayPath, SIGNAL(editingFinished()),
            this, SLOT(updateCommandLine()));
    connect(ui.fileName, SIGNAL(editingFinished()),
            this, SLOT(updateCommandLine()));
    ui.dimX->setText(QString::number(0));
    ui.dimY->setText(QString::number(0));
    updateCommandLine();
  }

  POVRayDialog::~POVRayDialog()
  {
  }

  void POVRayDialog::setImageSize(int width, int height)
  {
    ui.dimX->setText(QString::number(width));
    ui.dimY->setText(QString::number(height));
  }

  int POVRayDialog::imageWidth()
  {
    return ui.dimX->text().toInt();
  }

  int POVRayDialog::imageHeight()
  {
    return ui.dimY->text().toInt();
  }

  void POVRayDialog::setFileName(const QString& fileName)
  {
    ui.fileName->setText(fileName);
  }

  QString POVRayDialog::fileName()
  {
    return ui.fileName->text();
  }

  void POVRayDialog::setCommand(const QString& command)
  {
    ui.povrayPath->setText(command);
  }

  QString POVRayDialog::command()
  {
    return ui.povrayPath->text();
  }

  QStringList POVRayDialog::commandLine()
  {
    QString fileName = ui.fileName->text();
    QFileInfo info(fileName);
    QStringList tmp;
    tmp << "+I" + info.baseName() + ".pov"
        << "+O" + info.baseName() + ".png"
        << "+W" + ui.dimX->text()
        << "+H" + ui.dimY->text()
        << "+V" << "+D" << "+FN" << "+Q9"
        << "+P" << "+UD" << "+UL" << "+UV";
    if (ui.antialias->isChecked())
      tmp << "+A" << "+AM2";
    if (ui.transparency->isChecked())
      tmp << "+UA";
    return tmp;
  }

  bool POVRayDialog::keepSource()
  {
    return ui.keepSource->isChecked();
  }

  bool POVRayDialog::renderDirect()
  {
    return ui.renderDirect->isChecked();
  }

  void POVRayDialog::writeSettings(QSettings &settings) const
  {
    settings.setValue("imageWidth", ui.dimX->text().toInt());
    settings.setValue("imageHeight", ui.dimY->text().toInt());
    settings.setValue("antialias", ui.antialias->isChecked());
    settings.setValue("transparency", ui.transparency->isChecked());
    settings.setValue("keepSource", ui.keepSource->isChecked());
    settings.setValue("renderDirect", ui.renderDirect->isChecked());
    settings.setValue("povrayPath", ui.povrayPath->text());
  }

  void POVRayDialog::readSettings(QSettings &settings)
  {
    ui.dimX->setText(settings.value("imageWidth", 1024).toString());
    ui.dimY->setText(settings.value("imageHeight", 768).toString());
    ui.antialias->setChecked(settings.value("antialias", true).toBool());
    ui.transparency->setChecked(settings.value("transparency", true).toBool());
    ui.keepSource->setChecked(settings.value("keepSource", true).toBool());
    ui.renderDirect->setChecked(settings.value("renderDirect", true).toBool());
    ui.povrayPath->setText(settings.value("povrayPath", "povray").toString());
    updateCommandLine();
  }


  void POVRayDialog::selectFileName()
  {
    // Load a file
    QString fileName = QFileDialog::getSaveFileName(this,
      tr("Save POV-Ray rendered image"), ui.fileName->text(),
      tr("Image files (*.png *.pbm)"));
    ui.fileName->setText(fileName);
  }

  void POVRayDialog::renderClicked()
  {
    emit render();
  }

  void POVRayDialog::resized()
  {
    // Adjust the size here on the form
  }

  void POVRayDialog::updateCommandLine()
  {
    // Should update the command line when anything is changed
    ui.command->setText(ui.povrayPath->text() + " " + commandLine().join(" "));
  }

} // End namespace Avogadro

#include "povraydialog.moc"
