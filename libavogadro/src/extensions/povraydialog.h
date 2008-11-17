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

#ifndef POVRAYDIALOG_H
#define POVRAYDIALOG_H

#include <QDialog>
#include <QList>

#include <Eigen/Core>

#include "ui_povraydialog.h"

namespace Avogadro
{
  class POVRayDialog : public QDialog
  {
  Q_OBJECT

  public:
    explicit POVRayDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~POVRayDialog();

    /**
     * Set the width and height of the POV-Ray scene
     */
    void setImageSize(int width, int height);

    /**
     * @return the width of the POV-Ray scene in pixels.
     */
    int imageWidth();

    /**
     * @return the height of the POV-Ray scene in pixels.
     */
    int imageHeight();

    /**
     * Set the file name of the POV-Ray file
     */
    void setFileName(const QString& fileName);

    /**
     * @return the file name of the output.
     */
    QString fileName();

    /**
     * @return the command line to run POV-Ray and generate the image file.
     */
     QStringList commandLine();

  private:
    Ui::POVRayDialog ui;

  Q_SIGNALS:
    void render();

  public Q_SLOTS:
    void selectFileName();
    void renderClicked();
    void resized();

  };

} // End namespace Avogadro

#endif
