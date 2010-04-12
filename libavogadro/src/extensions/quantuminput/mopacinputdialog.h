/**********************************************************************
  MOPACInputDialog - Dialog for generating MOPAC input decks

  Copyright (C) 2009 Geoffrey Hutchison
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

#ifndef MOPACINPUTDIALOG_H
#define MOPACINPUTDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QProgressDialog>

#include <avogadro/glwidget.h>

#include "ui_mopacinputdialog.h"

namespace Avogadro
{
  class MOPACInputDialog : public QDialog
  {
  Q_OBJECT

  public:
    explicit MOPACInputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~MOPACInputDialog();

    void setMolecule(Molecule *molecule);
    void setWidget(GLWidget *widget);

    enum calculationType{SP, OPT, FREQ};
    enum theoryType{AM1, MNDO, MNDOD, PM3, PM6, RM1};
    enum coordType{CARTESIAN, ZMATRIX};

    /**
     * Save the settings for this extension.
     * @param settings Settings variable to write settings to.
     */
    virtual void writeSettings(QSettings &settings) const;

    /**
     * Read the settings for this extension.
     * @param settings Settings variable to read settings from.
     */
    virtual void readSettings(QSettings &settings);

  protected:
    /**
     * Reimplemented to update the dialog when it is shown
     */
    void showEvent(QShowEvent *event);

  private:
    Ui::MOPACInputDialog ui;
    Molecule* m_molecule;

    // Internal data structure for the calculation
    QString m_title;
    calculationType m_calculationType;
    theoryType m_theoryType;
    int m_multiplicity;
    int m_charge;
    coordType m_coordType;
    bool m_dirty;
    bool m_warned;
    bool m_previewVisible;
    QProcess *m_process;
    QProgressDialog *m_progress;
    QString m_inputFile;

    QString saveInputFile();

    // Generate an input deck as a string
    QString generateInputDeck();
    // Translate enums to strings
    QString getCalculationType(calculationType t);
    QString getTheoryType(theoryType t);

    static const QString mopacPath;

    // Enable/disable form elements
    void deckDirty(bool);

  Q_SIGNALS:
    void readOutput(const QString outputFileName);

  public Q_SLOTS:
    void updatePreviewText();

  private Q_SLOTS:
    //! Button Slots
    void resetClicked();
    void generateClicked();
    void computeClicked();
    void enableFormClicked();
    void moreClicked();
    void previewEdited();

    void finished(int);
    void stopProcess();

    void setTitle();
    void setCalculation(int);
    void setTheory(int);
    void setMultiplicity(int);
    void setCharge(int);
    void setCoords(int);
  };
}

#endif
