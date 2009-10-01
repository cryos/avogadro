/**********************************************************************
  DaltonInputDialog - Dialog for generating Dalton input files

  Source code shamelessly copied from gaussianinputdialog.h

 **********************************************************************/

#ifndef DALTONINPUTDIALOG_H
#define DALTONINPUTDIALOG_H

#include <QDialog>

#include <avogadro/glwidget.h>

#include "ui_daltoninputdialog.h"

namespace Avogadro
{
  class DaltonInputDialog : public QDialog
  {
    Q_OBJECT

  public:
    explicit DaltonInputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~DaltonInputDialog();

    void setMolecule(Molecule *molecule);

    enum calculationType{SP, PROP};
    enum theoryType{B3LYP, PBE0};
    enum basisType{ccpVDZ, ccpVTZ};

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

//    QString pathToDalton() const;

  protected:
    /**
     * Reimplemented to update the dialog when it is shown
     */
    void showEvent(QShowEvent *event);

  private:
    Ui::DaltonInputDialog ui;
    Molecule* m_molecule;

    // Internal data structure for the calculation
    QString m_title;
    calculationType m_calculationType;
    theoryType m_theoryType;
    basisType m_basisType;
    bool m_dirty;
    bool m_warned;
    QString m_inputFile;

    // Generate an input deck as a string
    QString generateInputDeck();
    // Translate enums to strings
    QString getCalculationType(calculationType t);
    QString getTheoryType(theoryType t);
    QString getBasisType(basisType t);

    // Enable/disable form elements
    void deckDirty(bool);
    
    QString saveInputFile();

  public Q_SLOTS:
    void updatePreviewText();

  private Q_SLOTS:
    //! Button Slots
    void resetClicked();
    void generateClicked();
    void moreClicked();
    void previewEdited();
    void setTitle();
    void setCalculation(int);
    void setTheory(int);
    void setBasis(int);
  };
}

#endif
