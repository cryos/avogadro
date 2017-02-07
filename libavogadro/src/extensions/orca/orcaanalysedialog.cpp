
/**********************************************************************
  OrcaAnalyseDialog - reading orca output file (without openBabel)
                    - reading exyz files (without openBabel)
                    - rudimentary displaying vibrations/plot of spectra
                    - selection of fragments defined by orca (could not use openBabel because there are no relating structures for that up to now)

  Copyright (C) 2014 Dagmar Lenk

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#include "orcaanalysedialog.h"
#include "orcaspectra.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/fragment.h>
#include <avogadro/periodictableview.h>
#include <avogadro/glwidget.h>
#include <avogadro/toolgroup.h>
#include <avogadro/extension.h>
#include <avogadro/primitivelist.h>
#include <openbabel/mol.h>

#include <Eigen/Geometry>

#include <vector>

#include <QtGui>

#include <QString>
#include <QLabel>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

using namespace OpenBabel;
using namespace Eigen;
using namespace std;

namespace Avogadro {

OrcaAnalyseDialog::OrcaAnalyseDialog(QWidget *parent, Qt::WindowFlags f ) :
    QDialog( parent, f ), m_molecule(NULL),  m_widget(NULL), m_plotIR(NULL), m_animation(NULL), m_obmol(NULL),
    m_vibWidget(NULL), m_orbitalWidget (NULL)
{
    m_freqLabel = NULL;
    m_fileLabel = NULL;

    vibData = new OrcaVibrations;

    m_nFrames = 20;
    m_freqScale = 0.7;
    m_animationOptionChanged = false;
    m_animationStarted = false;
    m_fragment =false;
    m_coordsOK = false;
    m_framesOK = false;

    // This initializes the ui member function to contain pointers to
    // all GUI elements in the orcaanalysedialog.ui file
    ui.setupUi(this);

    connect(ui.loadButton, SIGNAL(clicked()), this, SLOT(loadFile()));
    connect (ui.vibrationTable, SIGNAL(cellClicked(int,int)), this, SLOT(selectVibration(int, int)));
    connect (this, SIGNAL(vibrationsChanged(OrcaVibrations *)), this, SLOT(updateVibrations()));

    connect (ui.plotButton, SIGNAL(clicked()), this, SLOT(plotSpectra()));

    connect (ui.fpsSpin, SIGNAL(valueChanged(int)), this, SLOT(setFps(int)));
    connect (ui.scaleDSpin, SIGNAL(valueChanged(double)),this, SLOT(setScale(double)));

    connect (ui.startButton, SIGNAL(clicked()), this, SLOT(startAnimation()));
    connect (ui.stopButton, SIGNAL(clicked()), this, SLOT(stopAnimation()));

    connect (ui.selectButton, SIGNAL(clicked()), this, SLOT(selectFragment()));

    connect (ui.closeButton, SIGNAL(clicked()), this, SLOT(close()));

    connect (this, SIGNAL(orcaWarning(QString)), this, SLOT(orcaWarningMessage(QString)));

    ui.animationGroup->setEnabled(false);
    ui.scaleDSpin->setValue(m_freqScale);
    ui.fpsSpin->setValue(m_nFrames);
    ui.loadButton->setDefault(true);

    ui.vibrationTable->setHorizontalHeaderLabels(QString("Frequencies ; Intensities;").split(";"));
    ui.vibrationTable->setColumnCount(2);
    ui.vibrationTable->setRowCount(0);
    ui.vibrationTable->resizeColumnsToContents();
    ui.vibrationTable->resizeRowsToContents();

    QSettings settings;
    readSettings(settings);
    ui.FilenameLine->setText(m_savePath);
}

OrcaAnalyseDialog::~OrcaAnalyseDialog()
{
    QSettings settings;
    writeSettings(settings);
    delete vibData;
    delete m_animation;

}
void OrcaAnalyseDialog::setMolecule(Molecule *molecule)
{
    m_molecule = molecule;

    connect(m_molecule, SIGNAL(atomRemoved(Atom *)), this, SLOT(forceFileReload(Atom*)));
    connect(m_molecule, SIGNAL(atomAdded(Atom *)), this, SLOT(forceFileReload(Atom*)));
    connect(m_molecule, SIGNAL(atomUpdated(Atom *)), this, SLOT(forceFileReload(Atom*)));

//    if (molecule->fileName() != "")  m_fileLabel->setText(molecule->fileName());
}

void OrcaAnalyseDialog::setWidget(GLWidget *widget)
{
    m_widget = widget;
    QString fileName = m_molecule->fileName();
    QList<QLabel*> list;
    if (!m_fileLabel) {
        m_fileLabel = new QLabel;
        list.append(m_fileLabel);
        m_fileLabel->setText(fileName);

        // Add to GLWidget
        m_widget->addTextOverlay(list);
    }
}
void OrcaAnalyseDialog::forceFileReload(Atom *atom)
{
    Q_UNUSED(atom);

    if (animationStarted()) {m_animation->stop();} //??? better OrcaAnalyseDialog::stopAnimation
    setAnimationStarted(false);
    setFramesChecked(false);
    setCoordsChecked(false);
    ui.vibrationTable->setEnabled(false);
    ui.vibrationTable->clearSelection();

    ui.animationGroup->setEnabled(false);
}
void OrcaAnalyseDialog::orcaWarningMessage(const QString &m)
{
    QMessageBox msgBox;

    msgBox.setWindowTitle(tr("OrcaExtension Warning"));
    msgBox.setText(m);
    msgBox.exec();
}

void OrcaAnalyseDialog::plotSpectra()
{

    if (vibData->checkOK()) {
        // Create the plot if needed
        if (!m_plotIR) {
            m_plotIR = new OrcaSpectra(qobject_cast<QWidget*>(parent()));
            connect (m_plotIR, SIGNAL(selectNewVibration(int)), this, SLOT(setVibration(int)));
            connect (this, SIGNAL(vibrationsChanged(OrcaVibrations*)), m_plotIR, SLOT(plotSpectra(OrcaVibrations*)));
        }
        m_plotIR->setWindowTitle("Orca IR Spectra");
        m_plotIR->plotSpectra(vibData);
        m_plotIR->show();
    } else {
        QMessageBox msgBox;

        msgBox.setWindowTitle(tr("OrcaExtension"));
        msgBox.setText(tr(" No vibration data found!"));
        msgBox.exec();

    }
}

void OrcaAnalyseDialog::setVibration(int n)
{
    ui.vibrationTable->clearSelection();
    ui.vibrationTable->selectRow(n);

    selectVibration(n,1);

}
void OrcaAnalyseDialog::selectVibration(int row, int column)
{
    Q_UNUSED(column);
    m_selectFreq = row;

    QTableWidgetItem *freqItem = new QTableWidgetItem;
    freqItem = ui.vibrationTable->item(m_selectFreq,0);

    QList<QLabel*> list;
    if (!m_freqLabel) {
        m_freqLabel = new QLabel;
        list.append(m_freqLabel);
        m_freqLabel->setText("Freq: " + freqItem->text());
        // Add to GLWidget
        m_widget->addTextOverlay(list);
    } else {
        m_freqLabel->setText("Freq: " + freqItem->text());
    }

    //    // Trigger render event
    if (m_molecule) {
      m_molecule->update();
    }

    if (m_animation && animationStarted()) {
        m_animation->stop();
    }
    if (createAnimation()) {
        if (animationStarted()){
            m_animation->start();
        }
        ui.animationGroup->setEnabled(true);
    }
}

void OrcaAnalyseDialog::setScale(double value)
{
    m_freqScale = value;
    m_animationOptionChanged = true;
}

void OrcaAnalyseDialog::setFps(int n)
{
    m_nFrames = n;
    m_animationOptionChanged = true;
}

bool OrcaAnalyseDialog::createAnimation()
{
    if (!m_animation) {
        m_animation = new Animation;
    } else {
        delete m_animation;
        m_animation = new Animation;
    }

    if (!checkCoords()) {
        QMessageBox msgBox;

        msgBox.setWindowTitle(tr("OrcaExtension"));
        msgBox.setText(tr(" No vibration data found or molecule changed outside Orca Plugin!"));
        msgBox.exec();
        return false;
    }
    if (m_curFrames.size()) {
        m_curFrames.clear();
        m_curFrames.resize(0);
    }

    uint nAtoms = m_molecule->numAtoms();
    m_molecule->setConformer(0);            // add always the "ground state" of the molecule to the displacements
    QList<Atom *> atomList;
    atomList = m_molecule->atoms();

    std::vector<std::vector<Eigen::Vector3d> *> curDisplacement;

    curDisplacement  = vibData->displacement();

    if (nAtoms != curDisplacement.at(0)->size()) {
        QMessageBox msgBox;

        msgBox.setWindowTitle(tr("OrcaExtension"));
        msgBox.setText(tr("Selected displacement data don't fit to the actual molecule!"));
        msgBox.exec();
        return false;
    }
    int selectedDisp = m_selectFreq + vibData->modes().at(0);

    for (int i=0;i<m_nFrames; i++) {
        m_curFrames.push_back(new std::vector<Eigen::Vector3d>(nAtoms));
        for (uint j=0; j<nAtoms; j++) {
            m_curFrames.at(i)->at(j).x() = atomList.at(j)->pos()->x() + (curDisplacement.at(selectedDisp)->at(j).x()*m_freqScale*sin(2*M_PI*i/(m_nFrames-1)));
            m_curFrames.at(i)->at(j).y() = atomList.at(j)->pos()->y() + (curDisplacement.at(selectedDisp)->at(j).y()*m_freqScale*sin(2*M_PI*i/(m_nFrames-1)));
            m_curFrames.at(i)->at(j).z() = atomList.at(j)->pos()->z() + (curDisplacement.at(selectedDisp)->at(j).z()*m_freqScale*sin(2*M_PI*i/(m_nFrames-1)));
        }
    }

    m_orcaFrames.resize(0);
    for (int i=0;i<m_nFrames; i++) {
        m_orcaFrames.push_back(std::vector<Eigen::Vector3d>(nAtoms));
        for (uint j=0; j<nAtoms;j++){
            m_orcaFrames.at(i).at(j) = m_curFrames.at(i)->at(j);
        }
    }

    m_animation->setFrame(1);
    m_animation->setFps(10);
    m_animation->setLoopCount(0);
    m_animation->setFrames(m_curFrames);
    m_animation->setMolecule(m_molecule);

    setFramesChecked(true);
    return true;

}
void OrcaAnalyseDialog::startAnimation()
{
    if (m_animationOptionChanged) {
        if (createAnimation()) {
            m_animation->setFrames(m_curFrames);
            m_animation->start();
            setAnimationStarted(true);
        }
        m_animationOptionChanged = false;
    } else if (m_animation && checkFrames() && !animationStarted()) {
        m_curFrames.clear();
        m_curFrames.resize(0);
        for (uint i=0;i<m_orcaFrames.size(); i++) {
            m_curFrames.push_back( new std::vector<Eigen::Vector3d>(m_orcaFrames.at(0).size()));
            for (uint j=0; j<m_orcaFrames.at(0).size();j++){
                m_curFrames.at(i)->at(j) = m_orcaFrames.at(i).at(j);
            }
        }
        m_animation->setFrames(m_curFrames);
        m_animation->start();
        setAnimationStarted(true);
    }


}

void OrcaAnalyseDialog::stopAnimation()
{

    if (m_animation && animationStarted()) {
        m_animation->stop();
        setAnimationStarted(false);
    }
}


void OrcaAnalyseDialog::updateVibrations()
{
    if (vibData->checkOK()) {
        ui.vibrationTable->setEnabled(true);
        ui.vibrationTable->clearSelection();
        ui.vibrationTable->clearContents();
        ui.animationGroup->setEnabled(false);

        QStringList freqLabel, intLabel, modeLabel;
        for (uint i=0; i<vibData->frequencies().size();i++) {
            QString tmpString;
            tmpString.setNum(vibData->frequencies().at(i));
            freqLabel += tmpString;
            tmpString.setNum(vibData->intensities().at(i));
            intLabel += tmpString;
            tmpString.setNum(vibData->modes().at(i));
            modeLabel += tmpString;
        }

        ui.vibrationTable->setRowCount(freqLabel.size());

        for (int i=0; i<freqLabel.size();i++){
            QTableWidgetItem *freqItem = new QTableWidgetItem;
            freqItem->setText(freqLabel.at(i));
            ui.vibrationTable->setItem(i,0,freqItem);
            QTableWidgetItem *intItem = new QTableWidgetItem;
            intItem->setText(intLabel.at(i));
            ui.vibrationTable->setItem(i,1,intItem);
        }
        ui.vibrationTable->setVerticalHeaderLabels(modeLabel);
        ui.vibrationTable->resizeColumnsToContents();
    } else {
        ui.vibrationTable->setEnabled(false);
        ui.vibrationTable->clearSelection();
        ui.vibrationTable->clearContents();
        ui.vibrationTable->setRowCount(0);
        ui.vibrationTable->resizeRowsToContents();

        ui.animationGroup->setEnabled(false);
    }
}

void OrcaAnalyseDialog::loadFile()
{
    if (m_animation && animationStarted()) {
        m_animation->stop();
        setAnimationStarted(false);
    }
    QString message;

    message = readOutputFile();

    if (message != "SUCCESS")
        emit orcaWarning (message);
    emit vibrationsChanged(vibData);
    m_widget->toolGroup()->setActiveTool("Navigate");

    selectFragment();
}

void OrcaAnalyseDialog::selectFragment()
{
    if (m_widget && m_fragment) {
        m_widget->clearSelected();
        m_widget->setSelected(m_orcaFragment, true);
        m_widget->update();
    }
}

QString OrcaAnalyseDialog::readOutputFile()
{

    // define variables for optimized geometry
    uint nAtoms = 0;
    std::vector<Eigen::Vector3d> geo;       // geometry of the molecule

    // remember widgets which also holds vibration or molecular orbital informations
    if (!m_orbitalWidget || !m_vibWidget) {
        foreach (QWidget *widget, QApplication::allWidgets())
            if (widget->objectName().contains("orbitalwidget",Qt::CaseInsensitive)) {
                m_orbitalWidget = widget;
            } else if (widget->objectName().contains("vibrationDock",Qt::CaseInsensitive)) {
                m_vibWidget = widget;
            }
    }


    // define variables for vibrational frequencies

    int nModeAll = 0;
    int nFreqAll = 0;
    std::vector<int> modeAll;
    std::vector<double> frequenciesAll;

    // define variables for IR spectra
    uint nMode = 0;
    uint nFrequencies = 0;
    std::vector<int> mode;
    std::vector<double> intensities;
    std::vector<double> frequencies;
    std::vector<double> ramanActivities;

    // define variables for normal modes
    std::vector<std::vector<Eigen::Vector3d> *> allConformers;
//    std::vector<std::vector<Eigen::Vector3d> *> m_curConformers;
    std::vector<std::vector<Eigen::Vector3d> *> displacements;

    // other stuff

    QStringList atomText;
    QStringList fragmentAtom;
    std::vector<int> fragmentIdx;
    std::vector<unsigned long int> fragmentAtomId;
    std::vector<bool> isFragment;

    bool unitCell = false;
    std::vector<vector3> unitCellVectors;
    bool virtualAtoms = false;
    //    std::vector<double> unitCellAngle;        // Orca always writes the unit cell information in tranlation vectors
    //    std::vector<double> unitCellOrigin;       // so these variables are not yet used


    // define variables for file parsing

    QStringList freqText, IRText, ramanText, infoText;
    QStringList energyText;
    QStringList coordText;


    // for Tests only
    QString retText;
    QTextStream ret(&retText);

    // reset old file informations

    setCoordsChecked(false);
    setFramesChecked(false);

    m_geoRead = false;
    m_orbitalsOK = false;
    m_orbRead = false;
    m_freqRead = false;
    m_modesRead = false;
    m_IRRead = false;
    m_ramanRead = false;
    m_unitCellRead = false;

    // Try to set default save path for dialog using the next sequence:
    //  1) directory of current file (if any);
    //  2) directory where previous deck was saved;
    //  3) $HOME

    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();

    QStringList filters;
    QString selectedFilter;

    if(m_savePath == "") {
        if (defaultPath.isEmpty())
            defaultPath = QDir::homePath();
    } else {
        defaultPath = m_savePath;
    }
    if (m_saveFilter != "") selectedFilter = m_saveFilter;
    filters
            << tr("Orca Output File") + " (*.out)"
            << tr("extended Geometry File") + " (*.exyz)"
            << tr("All Files") + " (* *.*)";

    QString defaultFileName = defaultPath + '/' + defaultFile.fileName();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Read Orca Outputfile"),
                                                    defaultFileName, filters.join(";;"), &selectedFilter);

    if (fileName == "") {
        return (tr("no file found"));
    }
    m_saveFilter = selectedFilter;
    QFile file(fileName);
    QTextStream in(&file);

    m_savePath = QFileInfo(file).absolutePath();

    if (QFileInfo(file).completeSuffix().contains("xyz")) {
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
        ui.FilenameLine->setText(fileName);

        m_geoRead = false;
        fragmentIdx.resize(0);
        fragmentAtom.clear();
        atomText.clear();
        while (!in.atEnd()) {

            QString outputText = in.readLine();
            //
            // look for the geometry of the molecule
            //
            infoText = outputText.split(" ", QString::SkipEmptyParts);
            nAtoms = 0;
            ret << "Orca Initial geometry \n";
            outputText = in.readLine(); // read comment
            if (outputText.contains("%PBC")) {
                unitCell = true;
            }
            if (outputText.contains("%VIRTUAL")) {
                virtualAtoms = true;
            }
            outputText = in.readLine();
            int i = 0;
            while (!outputText.isEmpty()) {
                outputText.replace("\t", " ");
                infoText = outputText.split(" ", QString::SkipEmptyParts);
                if (infoText.size()==0) break;
                if (infoText.size() >4) {
                    if (infoText.at(4) == "Fragment1") {
                        isFragment.push_back(true);
                        fragmentAtom += infoText.at(0);
                    } else {
                        isFragment.push_back(false);
                    }
                } else {
                    isFragment.push_back(false);
                }
                ret << infoText.at(1).toDouble() << " " << infoText.at(2).toDouble() << " " << infoText.at(3).toDouble() << "\n";
                atomText += infoText.at(0);
                Eigen::Vector3d tmpCoord;
                tmpCoord = Eigen::Vector3d(infoText.at(1).toDouble(), infoText.at(2).toDouble(),infoText.at(3).toDouble());

                geo.push_back(tmpCoord);
                ret << geo.at(i).x() << "\n";
                outputText = in.readLine();
                i++;
            }
            nAtoms = i;
            if (nAtoms != 0) {
                m_geoRead = true;
            } else {
                return (tr("Somethings wrong in the file structure"));
            }

            // read unitcell vectors if there are any

            if (unitCell) {
                unitCellVectors.resize(0);

                vector3 tmpVector;

                outputText = "";
                if (!in.atEnd()) outputText = in.readLine();

                while (!outputText.isEmpty()) {
                    outputText.replace("\t", " ");
                    infoText = outputText.split(" ", QString::SkipEmptyParts);
                    if (infoText.size()==4) {          // vectors are in angstroem
                        tmpVector = vector3(infoText.at(1).toDouble(), infoText.at(2).toDouble(),infoText.at(3).toDouble());
                        unitCellVectors.push_back(tmpVector);
                    }
                    if (!in.atEnd()) {
                        outputText = in.readLine();
                    } else {
                        break;
                    }
                }

                if (unitCellVectors.size()==4 ) {
                    m_unitCellRead = true;
                } else {
                    m_unitCellRead = false;
                }
            }
        }
    } else {

        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
        ui.FilenameLine->setText(fileName);

        m_geoRead = false;
        while (!in.atEnd()) {

            QString outputText = in.readLine();

            //
            // look for the optimized geometry of the molecule
            //
            if (outputText.trimmed() ==  "INPUT FILE") {
                atomText.clear();
                ret << "Orca Initial geometry \n";
                outputText = in.readLine();
                while (!outputText.contains("CARTESIAN COORDINATES (ANGSTROEM)") && !in.atEnd()) {
                    outputText = in.readLine();
                }
                QString skip = in.readLine(); // skip ---------------------
                outputText = in.readLine();
                int i = 0;
                while (!outputText.isEmpty()) {
                    infoText = outputText.split(" ", QString::SkipEmptyParts);
                    if (infoText.size() !=4) {
                        return (tr("Somethings wrong in the file structure"));
                    }
                    ret << infoText.at(1).toDouble() << " " << infoText.at(2).toDouble() << " " << infoText.at(3).toDouble() << "\n";
                    atomText += infoText.at(0);
                    Eigen::Vector3d tmpCoord;
                    tmpCoord = Eigen::Vector3d(infoText.at(1).toDouble(), infoText.at(2).toDouble(),infoText.at(3).toDouble());

                    geo.push_back(tmpCoord);
                    ret << geo.at(i).x() << "\n";
                    outputText = in.readLine();
                    i++;
                }
                nAtoms = i;
                if (nAtoms != 0) {
                    m_geoRead = true;
                } else {
                    return (tr("Somethings wrong in the file structure"));
                }

                //            ---------------------------------
                //            CARTESIAN COORDINATES (ANGSTROEM)
                //            ---------------------------------
                //              C     -0.000550    0.001980   -0.001700

            } else if (outputText.contains("HURRAY")) {
                outputText = in.readLine();
                while (!outputText.contains("CARTESIAN COORDINATES (ANGSTROEM)") && !in.atEnd()) {
                    outputText = in.readLine();
                }
                geo.resize(0);
                atomText.clear();
                ret << "orca optimized geometry.....";
                QString skip = in.readLine(); // skip ---------------------
                outputText = in.readLine();
                int i = 0;
                while (!outputText.isEmpty()) {
                    infoText = outputText.split(" ", QString::SkipEmptyParts);
                    if (infoText.size() !=4) {
                        return (tr("Somethings wrong in the file structure"));
                    }
                    ret << infoText.at(1).toDouble() << " " << infoText.at(2).toDouble() << " " << infoText.at(3).toDouble() << "\n";
                    atomText += infoText.at(0);
                    Eigen::Vector3d tmpCoord;
                    tmpCoord = Eigen::Vector3d(infoText.at(1).toDouble(), infoText.at(2).toDouble(),infoText.at(3).toDouble());

                    geo.push_back(tmpCoord);
                    ret << geo.at(i).x() << "\n";
                    outputText = in.readLine();
                    i++;
                }
                if (m_geoRead) {
                    if (nAtoms != geo.size()){
                        return (tr("Somethings wrong in the file structure"));
                    }
                }

            } else if (outputText.contains("UNIT CELL (ANGSTROEM)",Qt::CaseInsensitive)) {

                unitCellVectors.resize(0);
                vector3 tmpVector;

                outputText = "";
                if (!in.atEnd()) outputText = in.readLine();

                while (!outputText.isEmpty()) {
                    outputText.replace("\t", " ");
                    infoText = outputText.split(" ", QString::SkipEmptyParts);
                    if (infoText.size()==4) {          // vectors are in angstroem
                        tmpVector = vector3(infoText.at(1).toDouble(), infoText.at(2).toDouble(),infoText.at(3).toDouble());
                        unitCellVectors.push_back(tmpVector);
                    } else {
                        return (tr("Somethings wrong in the file structure - unitcells"));
                    }
                    if (!in.atEnd()) {
                        outputText = in.readLine();
                    } else {
                        break;
                    }
                }

                if (unitCellVectors.size()==4 ) {
                    m_unitCellRead = true;
                } else {
                    m_unitCellRead = false;
                }
                //
                // look for coordinate information relating to the unit cell calculations
                //

                ret << "Orca unit cell geometry \n";
                outputText = in.readLine();
                if (outputText.contains("CARTESIAN COORDINATES (ANGSTROEM)") && !in.atEnd()) {
                    atomText.clear();
                    geo.resize(0);
                    outputText = in.readLine();
                    int i = 0;
                    while (!outputText.isEmpty() && !outputText.contains("------------------")) {
                        infoText = outputText.split(" ", QString::SkipEmptyParts);
                        if (infoText.size() < 4) {
                            return (tr("Somethings wrong in the file structure"));
                        }
                        ret << infoText.at(1).toDouble() << " " << infoText.at(2).toDouble() << " " << infoText.at(3).toDouble() << "\n";
                        atomText += infoText.at(0);
                        Eigen::Vector3d tmpCoord;
                        tmpCoord = Eigen::Vector3d(infoText.at(1).toDouble(), infoText.at(2).toDouble(),infoText.at(3).toDouble());

                        geo.push_back(tmpCoord);
                        ret << geo.at(i).x() << "\n";
                        outputText = in.readLine();
                        i++;
                    }
                    nAtoms = i;
                    if (nAtoms != 0) {
                        m_geoRead = true;
                    } else {
                        return (tr("Somethings wrong in the file structure"));
                    }
                }
                //
                // read vibrational frequenies
                //
            } else if (outputText.trimmed() == "VIBRATIONAL FREQUENCIES"){
                QString skip = in.readLine();   // skip ---------------------
                skip = in.readLine();                 // skip empty line
                outputText = in.readLine();
                while (!outputText.isEmpty()) {
                    outputText.replace(":","");
                    freqText = outputText.split(" ", QString::SkipEmptyParts);
                    if (freqText.size() < 2) {
                        return (tr("Somethings wrong in the file structure"));
                    }
                    modeAll.push_back(freqText.at(0).toInt());
                    frequenciesAll.push_back(freqText.at(1).toDouble());
                    outputText = in.readLine();
                }
                nModeAll = modeAll.size();
                if (nModeAll != 0) {
                    m_freqRead = true;
                }
                //
                // look for the coordinates of the different modes
                //
            } else if (outputText.trimmed() == "NORMAL MODES"){
                QRegExp  rx("[0-9]-");
                ret << " orca normal modes \n";
                for (int j=0;j<6;j++) {
                    QString skip = in.readLine();         // skip ---------- comments and blank lines
                }
                int nBlock = nModeAll/6;
                if (nModeAll%6 != 0) nBlock++;
                //            qDebug ( ) << nBlock << nModeAll%6;

                for (int j=0;j<nBlock; j++) {
                    QStringList headText;
                    outputText = in.readLine();
                    //                qDebug() << outputText;
                    headText = outputText.split(" ", QString::SkipEmptyParts);
                    int nColumn = headText.size();

                    for (uint k=0; k<nAtoms; k++){
                        QStringList tmpText;

                        outputText = in.readLine();

                        while (rx.indexIn(outputText) != -1){          // avoid wrong splitting
                            outputText.insert(rx.indexIn(outputText)+1, " ");
                        }
                        tmpText = outputText.split(" ", QString::SkipEmptyParts);

                        for (int l=1;l<nColumn+1; l++) {
                            coordText += tmpText.at(l);
                        }
                        outputText = in.readLine();
                        while (rx.indexIn(outputText) != -1){          // avoid wrong splitting
                            outputText.insert(rx.indexIn(outputText)+1, " ");
                        }
                        tmpText = outputText.split(" ", QString::SkipEmptyParts);
                        for (int l=1;l<nColumn+1; l++) {
                            coordText += tmpText.at(l);
                        }
                        outputText = in.readLine();
                        while (rx.indexIn(outputText) != -1){          // avoid wrong splitting
                            outputText.insert(rx.indexIn(outputText)+1, " ");
                        }
                        tmpText = outputText.split(" ", QString::SkipEmptyParts);
                        for (int l=1;l<nColumn+1; l++) {
                            coordText += tmpText.at(l);
                        }
                    }
                }
                m_modesRead = true;
                //
                // look for the intensities for the IR spectra
                //
            } else if (outputText.trimmed() == "IR SPECTRUM"){
                ret << "orca IR spectrum \n";
                QString skip = in.readLine();         // skip ---------------------
                skip = in.readLine();                 // skip empty line
                skip = in.readLine();                 // skip headline line
                skip = in.readLine();                 // skip ---------------------
                outputText = in.readLine();
                while (!outputText.isEmpty() && !in.atEnd()) {
                    outputText.replace(":","");
                    outputText.replace ("(","");
                    IRText = outputText.split(" ", QString::SkipEmptyParts);
                    if (IRText.size() < 3) {
                        return (tr("Something is wrong in the IR output! "));
                    } else {
                        mode.push_back(IRText.at(0).toInt());
                        frequencies.push_back(IRText.at(1).toDouble());
                        intensities.push_back(IRText.at(2).toDouble());
                    }
                    outputText = in.readLine();
                }
                m_IRRead = true;
            } else if (outputText.trimmed() == "RAMAN SPECTRUM"){
                ret << "orca raman spectrum \n";
                QString skip = in.readLine();         // skip ---------------------
                skip = in.readLine();                 // skip empty line
                skip = in.readLine();                 // skip headline line
                skip = in.readLine();                 // skip ---------------------
                outputText = in.readLine();
                while (!outputText.isEmpty() && !in.atEnd()) {
                    outputText.replace(":","");
                    ramanText = outputText.split(" ", QString::SkipEmptyParts);
                    if (ramanText.size() < 3) {
                        return (tr("Something is wrong in the raman output! "));
                    } else {
                        ramanActivities.push_back(ramanText.at(2).toDouble());
                    }
                    outputText = in.readLine();
                }
                m_ramanRead = true;
            } else if (outputText.trimmed() == "ORBITAL ENERGIES") {
                m_orbRead = false;
                m_openShell = false;
                energyEh.resize(0);
                energyeV.resize(0);
                occ.resize(0);
                //            if (m_nBasisFunctions == 0) {
                //                return (tr("Something's wrong in the file structure"));
                //            }
                QString skip = in.readLine();         // skip ---------------------
                skip = in.readLine();                 // skip empty line
                if (skip.trimmed().contains("SPIN UP ORBITALS"))  m_openShell = true;  // we have open shells

                skip = in.readLine();                 // skip headline line

                //            int nOrbital = m_nBasisFunctions;

                outputText = in.readLine();
                while (!outputText.contains("---------") && !outputText.isEmpty() && !in.atEnd()) {

                    energyText = outputText.split(" ", QString::SkipEmptyParts);
                    if (energyText.size() !=  4) {
                        return (tr("Something is wrong in the orbital energy output! "));
                    } else {
                        occ.push_back(energyText.at(1).toDouble());
                        energyEh.push_back(energyText.at(2).toDouble());
                        energyeV.push_back(energyText.at(3).toDouble());
                    }
                    outputText = in.readLine();
                }
                //
                // if open shells read next block
                //
                if (m_openShell) {
                    energyBEh.resize(0);
                    energyBeV.resize(0);
                    occB.resize(0);
                    skip = in.readLine();                 // skip SPIN DOWN ...

                    skip = in.readLine();                 // skip headline line
                    outputText = in.readLine();
                    while (!outputText.contains("---------") && !outputText.isEmpty() && !in.atEnd()) {
                        energyText = outputText.split(" ", QString::SkipEmptyParts);

                        if (energyText.size() !=  4) {
                            return (tr("Something is wrong in the orbital energy output! "));
                        } else {
                            occB.push_back(energyText.at(1).toDouble());
                            energyBEh.push_back(energyText.at(2).toDouble());
                            energyBeV.push_back(energyText.at(3).toDouble());
                        }
                        outputText = in.readLine();
                    }
                }
                m_orbRead = true;
            } else if (outputText.trimmed() == "MOLECULAR ORBITALS") {
                m_orbitalsOK = true;        // just remember that molecular orbitals are written in the file
                // but they will be read in another extension
                outputText = in.readLine();
            }
        }
    }

    if (!m_geoRead) {
        return (tr("No geometry found in file!"));
    }


    nModeAll = modeAll.size();
    nFreqAll = frequenciesAll.size();
    nMode = mode.size();

    nFrequencies = frequencies.size();

    if (m_molecule->numAtoms() != nAtoms && m_molecule->numAtoms() != 0) {

            QMessageBox msgBox;

            msgBox.setWindowTitle(tr("OrcaExtension Warning"));
            msgBox.setText(tr("Molecule of the file does not fit the actual molecule structure! \n Would you like to override the actual molecule?"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

            switch (msgBox.exec()) {
            case QMessageBox::Yes:
                // YES was clicked - new Molecule needed
                // do something
                break;

            case QMessageBox::No:
                // NO was clicked - ignore loading file
                // do nothing
                return " ";
            default:
                // should never be reached
                break;
            }
    }

    if (m_modesRead) {
        for (int i=0;i<nModeAll; i++) {
            allConformers.push_back(new std::vector<Eigen::Vector3d>(nAtoms));
            displacements.push_back( new std::vector<Eigen::Vector3d>(nAtoms));
        }
        ret << coordText.size();
        int nColumn = 6;
        int nBlocks = nModeAll/6;

        for (int i=0;i<nBlocks;i++){
            for (uint j=0; j<nAtoms*3; j+=3) {
                for (int k=0; k<nColumn;k++) {
                    displacements.at(i*6+k)->at(j/3).x() = coordText.at(i*nFreqAll*nColumn + j*nColumn + k).toDouble();
                    allConformers.at(i*6+k)->at(j/3).x() = coordText.at(i*nFreqAll*nColumn + j*nColumn + k).toDouble() + geo.at(j/3).x();
                    //                  ret << i*nFreqAll*nRow + j*nRow + k << " " << coordText.at(i*nFreqAll*nRow + j*nRow + k).toDouble() << " ";
                }
                //              ret << "\n";
                for (int k=0; k<nColumn;k++) {
                    displacements.at(i*6+k)->at(j/3).y() = coordText.at(i*nFreqAll*nColumn + (j+1)*nColumn + k).toDouble();
                    allConformers.at(i*6+k)->at(j/3).y() = coordText.at(i*nFreqAll*nColumn + (j+1)*nColumn + k).toDouble() + geo.at(j/3).y();
                    //                  ret << coordText.at(i*nFreqAll*nRow + (j+1)*nRow + k).toDouble() << " ";
                }
                //              ret << "\n";
                for (int k=0; k<nColumn;k++) {
                    displacements.at(i*6+k)->at(j/3).z() = coordText.at(i*nFreqAll*nColumn + (j+2)*nColumn + k).toDouble();
                    allConformers.at(i*6+k)->at(j/3).z() = coordText.at(i*nFreqAll*nColumn + (j+2)*nColumn + k).toDouble() + geo.at(j/3).z();
                    //                  ret << coordText.at(i*nFreqAll*nRow + (j+2)*nRow + k).toDouble() << " ";
                }
                //              ret << "\n";
            }
        }
        if (nModeAll%6 != 0) {
            nColumn = nModeAll-nBlocks*6;

            int i=nBlocks;
            for (uint j=0; j<nAtoms*3; j+=3) {
                for (int k=0; k<nColumn;k++) {
                    displacements.at(i*6+k)->at(j/3).x() = coordText.at(i*nFreqAll*6 + j*nColumn + k).toDouble();
                    allConformers.at(i*6+k)->at(j/3).x() = coordText.at(i*nFreqAll*6 + j*nColumn + k).toDouble() + geo.at(j/3).x();
                    //                                  qDebug() << i*nFreqAll*6 + j*nColumn + k << " " << coordText.at(i*nFreqAll*6 + j*nColumn + k).toDouble() << " ";
                }
                //              ret << "\n";
                for (int k=0; k<nColumn;k++) {
                    displacements.at(i*6+k)->at(j/3).y() = coordText.at(i*nFreqAll*6 + (j+1)*nColumn + k).toDouble();
                    allConformers.at(i*6+k)->at(j/3).y() = coordText.at(i*nFreqAll*6 + (j+1)*nColumn + k).toDouble() + geo.at(j/3).y();
                    //                                  qDebug() << i*nFreqAll*6 + j*nColumn + k << " " << coordText.at(i*nFreqAll*6 + (j+1)*nColumn + k).toDouble() << " ";
                }
                //              ret << "\n";
                for (int k=0; k<nColumn;k++) {
                    displacements.at(i*6+k)->at(j/3).z() = coordText.at(i*nFreqAll*6 + (j+2)*nColumn + k).toDouble();
                    allConformers.at(i*6+k)->at(j/3).z() = coordText.at(i*nFreqAll*6 + (j+2)*nColumn + k).toDouble() + geo.at(j/3).z();
                    //                                  qDebug() << i*nFreqAll*6 + j*nColumn + k << " " << coordText.at(i*nFreqAll*6 + (j+2)*nColumn + k).toDouble() << " ";
                }
                //              ret << "\n";
            }
        }

    }
    // skip zero frequencies
    m_modeOffset = 0;
    for (int i=0;i<nModeAll; i++) {
        if (frequenciesAll.at(i) >= 0.000001 ||frequenciesAll.at(i) <=-0.000001 ) { // assume it's NOT zero
            m_modeOffset = i;
            break;
        }
    }

    // save informations in a new molecule structure

    m_obmol = new OBMol;
    m_obmol->BeginModify();

    int nfragment = 0;
    if (fragmentAtom.size() != 0) {
        m_fragment = true;
    } else {
        m_fragment = false;
    }

    // atomic labels or numbers used?

    bool atomicLabel = false;
    if (atomText.at(0).toInt() == 0) {
        atomicLabel = true;
    }
    for (uint i=0; i<nAtoms; i++) {
        OBAtom *atom  = m_obmol->NewAtom();
        int atomicNum;
        if (atomicLabel) {
            atomicNum = etab.GetAtomicNum(atomText.at(i).toAscii());
        } else {
            atomicNum = atomText.at(i).toInt();
        }

        ret <<  atomicNum << " " <<  atomText.at(i).toAscii() << "\n";
        atom->SetAtomicNum(atomicNum);
        atom->SetVector(geo.at(i).x(), geo.at(i).y(),geo.at(i).z()); //set coordinates

        ret << geo.at(i).x() << " "  << geo.at(i).y() << " " << geo.at(i).z() << " \n" ;
        if (m_fragment && isFragment.at(i)) {
            fragmentAtomId.push_back(atom->GetId());
            nfragment++;
        }
    }
    m_obmol->ConnectTheDots();
    m_obmol->PerceiveBondOrders();

    //
    // Store vibrations (frequencies and normal modes, IR intensities(if any), raman activities (if any)
    if (m_freqRead && m_modesRead && m_IRRead) {
        OpenBabel::OBVibrationData *vibrations;

        vibrations = new OpenBabel::OBVibrationData;
        std::vector<std::vector<vector3> > dispOpenBabel;
        for (uint i=0;i<nMode; i++) {
            dispOpenBabel.push_back(std::vector<vector3>(nAtoms));
        }

        for (uint i=0; i<nMode;i++) {
            for (uint j=0; j<nAtoms;j++){
                dispOpenBabel.at(i).at(j) = vector3(displacements.at(i+m_modeOffset)->at(j).x(), displacements.at(i+m_modeOffset)->at(j).y(),displacements.at(i+m_modeOffset)->at(j).z());
            }
        }
        if (m_ramanRead) {
            vibrations->SetData(dispOpenBabel,frequencies, intensities, ramanActivities);
        } else {
            vibrations->SetData(dispOpenBabel,frequencies, intensities);
        }
        m_obmol->SetData(vibrations);
    }
    //
    // store unit cell informations
    //
    if (m_unitCellRead) {
        OpenBabel::OBUnitCell *unitCell;
        unitCell = new OpenBabel::OBUnitCell;
        const OpenBabel::vector3 v1 (unitCellVectors.at(0));
        const OpenBabel::vector3 v2 (unitCellVectors.at(1));
        const OpenBabel::vector3 v3 (unitCellVectors.at(2));

        unitCell->SetData(v1, v2, v3);
        unitCell->SetOffset(unitCellVectors.at(3));
        m_obmol->SetData(unitCell);

    }
    m_obmol->EndModify();

    if (m_orbitalWidget) {
        if (m_orbitalsOK){
            if (!m_orbitalWidget->isVisible()) m_orbitalWidget->show();
        } else {
            m_orbitalWidget->hide();
        }
    }

    disconnect(m_molecule, 0, this,0);

    Molecule *newMol = new Molecule;

    newMol->setOBMol(m_obmol);
    newMol->setFileName(fileName);
    m_fileLabel->setText(fileName);
    if (m_fragment) {
        m_orcaFragment.clear();
        for (uint i=0;i<fragmentAtomId.size();i++) {
            m_orcaFragment.append(newMol->atom(fragmentAtomId.at(i)));
        }
    }

    if (m_orbRead) {
        QList<QVariant> alphaEnergies, alphaOcc;
        //            QStringList alphaSymmetries;

        for (uint i=0;i<energyeV.size();i++) {
            alphaEnergies.append(energyeV.at(i));
            alphaOcc.append(occ.at(i));
        }

        newMol->setProperty("alphaOrbitalEnergies", alphaEnergies);
        newMol->setProperty("alphaOrbitalOccupations", alphaOcc);

        if (m_openShell) {QList<QVariant> betaEnergies, betaOcc;
            //            QStringList betaSymmetries;

            for (uint i=0;i<energyBeV.size();i++) {
                betaEnergies.append(energyBeV.at(i));
                betaOcc.append(occB.at(i));
            }

            newMol->setProperty("betaOrbitalEnergies", betaEnergies);
            newMol->setProperty("betaOrbitalOccupations", betaOcc);
        }
    }
    emit moleculeChangedHere(newMol, Extension::DeleteOld);

    Molecule *oldMol = m_molecule;
    oldMol->deleteLater();
    m_molecule = newMol;
    connect(m_molecule, SIGNAL(atomRemoved(Atom *)), this, SLOT(forceFileReload(Atom*)));
    connect(m_molecule, SIGNAL(atomAdded(Atom *)), this, SLOT(forceFileReload(Atom*)));
    connect(m_molecule, SIGNAL(atomUpdated(Atom *)), this, SLOT(forceFileReload(Atom*)));
    m_molecule->update();

    // close widgets of vibrationExtension and use our own widget

    if (m_vibWidget) {
            if (m_vibWidget->isVisible()) m_vibWidget->hide();
    }

    if (m_modesRead) {
        // reserve one place more because conformer (0) is the unchanged structure of the molecule

        for (uint i=0;i<nMode+1; i++) {
            m_curConformers.push_back(new std::vector<Eigen::Vector3d>(nAtoms));
            m_curConformers.at(i) = allConformers.at(i+m_modeOffset-1);
        }

        ret << "Number of Conformers = " << m_molecule->numConformers();
        ret << " \n";

        // resize all class vectors

        m_orcaConformer.resize(0);

        for (uint i=0;i<nMode+1; i++) {
            m_orcaConformer.push_back(std::vector<Eigen::Vector3d>(nAtoms));
        }

        for (uint i=0; i<nMode+1;i++) {
            for (uint j=0; j<nAtoms;j++){
                m_orcaConformer.at(i).at(j) = m_curConformers.at(i)->at(j);
            }
        }
        m_molecule->clearConformers();
        m_molecule->setAllConformers(m_curConformers,true);

    }
    //
    // save vibration data in special structure
    //
    if (m_freqRead && m_modesRead && m_IRRead) {
        if (nFrequencies == nMode && nFrequencies != 0) {
            vibData->setModes(mode);
            vibData->setFrequencies(frequencies);
            vibData->setIntensities(intensities);
            vibData->setDisplacement(displacements);
            if (m_ramanRead && nFrequencies == ramanActivities.size())
                vibData->setRaman(ramanActivities);

            vibData->setOK(true);
        } else {
            vibData->setOK(false);
        }
    } else {
        vibData->setOK(false);
    }

    file.close(); // flush buffer!

    if (vibData->checkOK()) {
        emit vibrationsChanged(vibData);
    }
    setFramesChecked(false);

    if (m_geoRead) {
        setCoordsChecked(true);
        return "SUCCESS";
    }

//    return retText; // output debug messages if neccessary

    return (tr("Could not read file!"));
}

void OrcaAnalyseDialog::readSettings(QSettings& settings)
{
    m_savePath = settings.value("orca/analyse/savepath").toString();
    m_saveFilter = settings.value("orca/analyse/saveFilter").toString();
}

void OrcaAnalyseDialog::writeSettings(QSettings& settings) const
{
    settings.setValue("orca/analyse/savepath", m_savePath);
    settings.setValue("orca/analyse/saveFilter", m_saveFilter);
}

} // end namespace

// This includes the files generated by Qt's moc at compile time to
// ensure that signals/slots work. If you ever see errors about
// missing vtables with gcc, check that you haven't forgotten one of
// these:
//#include "orcaanalysedialog.moc"

