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
    ui.dimX->setText(QString::number(0));
    ui.dimY->setText(QString::number(0));
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

  QStringList POVRayDialog::commandLine()
  {
    QString fileName = ui.fileName->text().mid(0,
                                    ui.fileName->text().lastIndexOf("."));
    QStringList tmp;
    tmp << "+I" + fileName + ".pov"
        << "+O" + fileName + ".png"
        << "+W" + ui.dimX->text()
        << "+H" + ui.dimY->text()
        << "+V" << "+D" << "+FN" << "+Q10";
//        << "+P";
    if (ui.antialias->isChecked())
      tmp << "+A";
    if (ui.transparency->isChecked())
      tmp << "+UA";
    return tmp;
  }

  void POVRayDialog::selectFileName()
  {
    // Load a file
    QString fileName = QFileDialog::getSaveFileName(this,
      tr("Save POV-Ray rendered image"), ui.fileName->text(),
      tr("Image files (*.png *.pbm)"));
    ui.fileName->setText(fileName);
//    emit fileName(file);
    ui.command->setText(commandLine().join(" "));
  }

  void POVRayDialog::renderClicked()
  {
    emit render();
  }

  void POVRayDialog::resized()
  {
    // Adjust the size here on the form
  }

} // End namespace Avogadro

#include "povraydialog.moc"
