/**********************************************************************
  POVRayDialog - Dialog to render images using POV-Ray

  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef POVRAYDIALOG_H
#define POVRAYDIALOG_H

#include <QDialog>
#include <QList>

#include <Eigen/Core>

#include "ui_povraydialog.h"

class QSettings;

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
     * @return The width of the POV-Ray scene in pixels.
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
     * @return The file name of the output.
     */
    QString fileName();

    /**
     * Set the path to the POV-Ray executable.
     */
    void setCommand(const QString& command);

    /**
     * @return The path to the POV-Ray executable.
     */
    QString command();

    /**
     * @return The command line to run POV-Ray and generate the image file.
     */
    QStringList commandLine();

    /**
     * @return True if the POV-Ray source should be kept after rendering.
     */
    bool keepSource();

    /**
     * @return True if POV-Ray should be called by Avogadro to render the image.
     */
    bool renderDirect();

     /**
     * Save the settings for this dialog.
     * @param settings Settings variable to write settings to.
     */
    virtual void writeSettings(QSettings &settings) const;

    /**
     * Read the settings for this dialog.
     * @param settings Settings variable to read settings from.
     */
    virtual void readSettings(QSettings &settings);

  private:
    Ui::POVRayDialog ui;

  Q_SIGNALS:
    void render();
    void preview(int, int);

  public Q_SLOTS:
    void selectFileName();
    void renderClicked();
    void resized();
    void updateCommandLine();

  };

} // End namespace Avogadro

#endif
