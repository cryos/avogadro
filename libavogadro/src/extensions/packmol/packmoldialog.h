/**********************************************************************
  PackmolDialog - Dialog for generating cubes and meshes

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

#ifndef PACKMOLDIALOG_H
#define PACKMOLDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QHash>
#include <QSettings>

#include "ui_packmoldialog.h"


namespace Avogadro
{

  class Molecule;
  class StructuresModel;

  class PackmolDialog : public QDialog
  {
    Q_OBJECT

  public:
    PackmolDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~PackmolDialog();

    void writeSettings(QSettings &settings) const;
    void readSettings(QSettings &settings);

  private:
    Ui::PackmolDialog ui;
    QHash<QString,QString> m_fileLookup; // translate short input filename to full path filenames
    QProcess *m_process;
    StructuresModel *m_model;

    double solvCalcVolume();
    void solvUpdateVolume();
    void solvUpdateSoluteNumber();
    QString solvContraintString();
    QString headerString();
    void createSodiumFile();
    void createChlorineFile();

    double bilayerCalculateL();

  public slots:
    void solvSoluteBrowseClicked();
    void solvSolventBrowseClicked();
    void solvGenerateClicked();
    void solvAdjustShapeClicked(int);
    void solvAddCounterIonsClicked(int);
    void solvGuessSolventNumberClicked(int);
    void solvVolumeChanged(double);

    void bilayerNewClicked();
    void bilayerRemoveClicked();
    void bilayerGenerateClicked();
    void bilayerUpdateNumber();

    void runButtonClicked();
    void abortButtonClicked();
    void visitWebsite();
    void updateStandardOutput();
    void processFinished(int,QProcess::ExitStatus);

  signals:
    void resultReady(Molecule*);
  };

} // End namespace Avogadro

#endif // PACKMOLDIALOG_H
