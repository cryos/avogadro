/**********************************************************************
  CrystalBuilderDialog - Dialog for Crystal Builder

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

/**
 * @todo Action: Advanced copy to clipboard config dialog
 * @todo Action: Perceive spacegroup (Need spglib)
 * @todo Cache list of spacegroups
 * @todo Select spacegroup name style (HM or Hall)
 * @todo UnitCellParameter::isValid()
 * @todo Read locks
 * @todo Undo support
 */

#include "crystalbuilderdialog.h"

#include "stablecomparison.h"

#include <avogadro/atom.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <QtGui/QClipboard>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include <QtGlobal>

using namespace OpenBabel;

namespace Avogadro {

  const int CB_FONTSIZE = 10;
  const char *CB_DIALOG_TITLE =
    QT_TRANSLATE_NOOP("Crystallographic Toolkit Extension",
                      "Crystallographic Toolkit");

  const unsigned short CB_ANGSTROM_UTF16 = 0x212B;
  const QString CB_ANGSTROM =
    QString::fromUtf16(&CB_ANGSTROM_UTF16, 1);

  const unsigned short CB_SUB_ZERO_UTF16 = 0x2080;
  const QString CB_SUB_ZERO =
    QString::fromUtf16(&CB_SUB_ZERO_UTF16, 1);

  const unsigned short CB_DEGREE_UTF16 = 0x00B0;
  const QString CB_DEGREE =
    QString::fromUtf16(&CB_DEGREE_UTF16, 1);

  const unsigned short CB_SUPER_THREE_UTF16 = 0x00B3;
  const QString CB_SUPER_THREE =
    QString::fromUtf16(&CB_SUPER_THREE_UTF16, 1);

  CrystalBuilderDialog::CrystalBuilderDialog(QWidget *parent,
                                             Qt::WindowFlags f )
    : QMainWindow(parent, f),
      m_molecule(0),
      m_lengthUnit(Angstrom),
      m_angleUnit(Degree),
      m_coordsCartFrac(Cartesian),
      m_coordsPreserveCartFrac(Cartesian),
      m_matrixCartFrac(Cartesian),
      m_matrixVectorStyle(RowVectors)
  {
    ui.setupUi(this);

    createActions();
    createMenuBar();
    initializeStatusBar();

    // Internal connections
    connect(this, SIGNAL(cellChanged()),
            this, SLOT(refreshGui()));

    // GUI Connections
    connect(ui.push_close, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(this, SIGNAL(lengthUnitChanged(LengthUnit)),
            this, SLOT(refreshGui()));
    connect(this, SIGNAL(angleUnitChanged(AngleUnit)),
            this, SLOT(refreshParameters()));
    connect(this, SIGNAL(angleUnitChanged(AngleUnit)),
            this, SLOT(refreshUnitIndicators()));
    connect(this, SIGNAL(coordsCartFracChanged(CartFrac)),
            this, SLOT(refreshCoords()));
    connect(this, SIGNAL(matrixCartFracChanged(CartFrac)),
            this, SLOT(refreshMatrix()));
    connect(this, SIGNAL(matrixVectorStyleChanged(VectorStyle)),
            this, SLOT(refreshMatrix()));

    // GUI edit updates
    connect(ui.edit_matrix, SIGNAL(textChanged()),
            this, SLOT(updateGuiForMatrixEdit()));
    connect(ui.spin_a, SIGNAL(valueChanged(double)),
            this, SLOT(updateGuiForParametersEdit()));
    connect(ui.spin_b, SIGNAL(valueChanged(double)),
            this, SLOT(updateGuiForParametersEdit()));
    connect(ui.spin_c, SIGNAL(valueChanged(double)),
            this, SLOT(updateGuiForParametersEdit()));
    connect(ui.spin_alpha, SIGNAL(valueChanged(double)),
            this, SLOT(updateGuiForParametersEdit()));
    connect(ui.spin_beta, SIGNAL(valueChanged(double)),
            this, SLOT(updateGuiForParametersEdit()));
    connect(ui.spin_gamma, SIGNAL(valueChanged(double)),
            this, SLOT(updateGuiForParametersEdit()));
    connect(ui.edit_coords, SIGNAL(textChanged()),
            this, SLOT(updateGuiForCoordsEdit()));

    // Apply / reset connections
    connect(ui.push_matrix_apply, SIGNAL(clicked()),
            this, SLOT(updateMatrix()));
    connect(ui.push_params_apply, SIGNAL(clicked()),
            this, SLOT(updateParameters()));
    connect(ui.push_coords_apply, SIGNAL(clicked()),
            this, SLOT(updateCoords()));
    connect(ui.push_matrix_reset, SIGNAL(clicked()),
            this, SLOT(refreshGui()));
    connect(ui.push_params_reset, SIGNAL(clicked()),
            this, SLOT(refreshGui()));
    connect(ui.push_coords_reset, SIGNAL(clicked()),
            this, SLOT(refreshGui()));

    // Validation
    connect(ui.edit_matrix, SIGNAL(textChanged()),
            this, SLOT(checkMatrixWidget()));
    connect(ui.spin_a, SIGNAL(valueChanged(double)),
            this, SLOT(checkParametersWidget()));
    connect(ui.spin_b, SIGNAL(valueChanged(double)),
            this, SLOT(checkParametersWidget()));
    connect(ui.spin_c, SIGNAL(valueChanged(double)),
            this, SLOT(checkParametersWidget()));
    connect(ui.spin_alpha, SIGNAL(valueChanged(double)),
            this, SLOT(checkParametersWidget()));
    connect(ui.spin_beta, SIGNAL(valueChanged(double)),
            this, SLOT(checkParametersWidget()));
    connect(ui.spin_gamma, SIGNAL(valueChanged(double)),
            this, SLOT(checkParametersWidget()));
    connect(ui.edit_coords, SIGNAL(textChanged()),
            this, SLOT(checkCoordsWidget()));

    // Validation indications
    connect(this, SIGNAL(matrixInvalid()),
            this, SLOT(markMatrixAsInvalid()));
    connect(this, SIGNAL(matrixValid()),
            this, SLOT(markMatrixAsValid()));
    connect(this, SIGNAL(parametersInvalid()),
            this, SLOT(markParametersAsInvalid()));
    connect(this, SIGNAL(parametersValid()),
            this, SLOT(markParametersAsValid()));
    connect(this, SIGNAL(coordsInvalid()),
            this, SLOT(markCoordsAsInvalid()));
    connect(this, SIGNAL(coordsValid()),
            this, SLOT(markCoordsAsValid()));

    ui.edit_coords->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));
    ui.edit_matrix->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));

    m_matrixCharFormat =
      ui.edit_matrix->textCursor().charFormat();
    m_coordsCharFormat =
      ui.edit_coords->textCursor().charFormat();
  }

  CrystalBuilderDialog::~CrystalBuilderDialog()
  {
  }

  void CrystalBuilderDialog::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;

    if (m_molecule == 0) {
      return;
    }

    connect(this, SIGNAL(cellChanged()),
            m_molecule, SLOT(updateMolecule()));

    // Prompt user to create unit cell if none exists
    if (!currentCell()) {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::information
        (this, CB_DIALOG_TITLE,
         tr("The current document is an isolated molecule "
            "and does not have a unit cell associated with "
            "it.\n\nWould you like to add a unit cell to "
            "the current document?"),
         QMessageBox::Yes | QMessageBox::No,
         QMessageBox::Yes);
      if (reply != QMessageBox::Yes) {
        this->close();
        return;
      }

      OpenBabel::OBUnitCell *cell
        = new OpenBabel::OBUnitCell;
      cell->SetData(3.0, 3.0, 3.0,
                    90.0, 90.0, 90.0);
      m_molecule->setOBUnitCell(cell);
      emit cellChanged();
    }
  }

  void CrystalBuilderDialog::createActions()
  {
    m_actionSetSpacegroup =
      new QAction(tr("Set &Spacegroup..."), this);
    m_actionFillCell =
      new QAction(tr("&Fill Cell"), this);
    m_actionFillCell->setStatusTip(tr("Fill the unit cell using"
                                      " current spacegroup."));
    m_actionWrapAtoms =
      new QAction(tr("&Wrap Atoms to Cell"), this);
    m_actionWrapAtoms->
      setShortcut(tr("Ctrl+W",
                     "Shortcut key for wrap atoms"
                     " to unit cell"));

    m_actionCopyToClipboard =
      new QAction(tr("&Copy to Clipboard"), this);
    m_actionCopyToClipboard->
      setShortcut(tr("Ctrl+C",
                     "Shortcut key for copy"));

    m_actionNiggliReduce =
      new QAction(tr("Reduce Cell (&Niggli)"), this);

    m_actionScaleToVolume =
      new QAction(tr("Scale Cell To &Volume..."), this);


    // Length units
    m_actionUnitsLengthGroup = new QActionGroup(this);

    m_actionUnitsLengthAngstrom =
      new QAction(tr("&Angstrom"), m_actionUnitsLengthGroup);
    m_actionUnitsLengthBohr =
      new QAction(tr("&Bohr"), m_actionUnitsLengthGroup);

    // Angle units
    m_actionUnitsAngleGroup = new QActionGroup(this);

    m_actionUnitsAngleDegree =
      new QAction(tr("&Degree"), m_actionUnitsAngleGroup);
    m_actionUnitsAngleRadian =
      new QAction(tr("&Radian"), m_actionUnitsAngleGroup);

    // Coordinate cart/frac
    m_actionCoordsCartFracGroup = new QActionGroup(this);

    m_actionCoordsCart =
      new QAction(tr("Display &cartesian coordinates"),
                  m_actionCoordsCartFracGroup);
    m_actionCoordsFrac =
      new QAction(tr("Display &fractional coordinates"),
                  m_actionCoordsCartFracGroup);

    // Coordinate preservation
    m_actionCoordsPreserveGroup = new QActionGroup(this);

    m_actionCoordsPreserveCart =
      new QAction(tr("Preserve &Cartesian Coordinates "
                     "During Cell Modification"),
                  m_actionCoordsPreserveGroup);
    m_actionCoordsPreserveFrac =
      new QAction(tr("Preserve &Fractional Coordinates "
                     "During Cell Modification"),
                  m_actionCoordsPreserveGroup);

    // Matrix cart/frac
    m_actionMatrixCartFracGroup = new QActionGroup(this);

    m_actionMatrixCart =
      new QAction(tr("Display &cartesian matrix"),
                  m_actionMatrixCartFracGroup);
    m_actionMatrixFrac =
      new QAction(tr("Display &fractional matrix"),
                  m_actionMatrixCartFracGroup);

    // Matrix vector representation
    m_actionMatrixVectorGroup = new QActionGroup(this);

    m_actionMatrixRowVectors =
      new QAction(tr("Display as &row vectors"),
                  m_actionMatrixVectorGroup);
    m_actionMatrixColumnVectors =
      new QAction(tr("Display as &column vectors"),
                  m_actionMatrixVectorGroup);

    // Connect
    // Actions
    connect(m_actionSetSpacegroup, SIGNAL(triggered()),
            this, SLOT(actionSetSpacegroup()));
    connect(m_actionFillCell, SIGNAL(triggered()),
            this, SLOT(actionFillCell()));
    connect(m_actionWrapAtoms, SIGNAL(triggered()),
            this, SLOT(actionWrapAtoms()));
    connect(m_actionCopyToClipboard, SIGNAL(triggered()),
            this, SLOT(actionCopyToClipboard()));
    connect(m_actionNiggliReduce, SIGNAL(triggered()),
            this, SLOT(actionNiggliReduce()));
    connect(m_actionScaleToVolume, SIGNAL(triggered()),
            this, SLOT(actionScaleToVolume()));

    // Settings
    connect(m_actionUnitsLengthAngstrom, SIGNAL(triggered()),
            this, SLOT(actionUnitsLengthAngstrom()));
    connect(m_actionUnitsLengthBohr, SIGNAL(triggered()),
            this, SLOT(actionUnitsLengthBohr()));

    connect(m_actionUnitsAngleDegree, SIGNAL(triggered()),
            this, SLOT(actionUnitsAngleDegree()));
    connect(m_actionUnitsAngleRadian, SIGNAL(triggered()),
            this, SLOT(actionUnitsAngleRadian()));

    connect(m_actionCoordsCart, SIGNAL(triggered()),
            this, SLOT(actionCoordsCart()));
    connect(m_actionCoordsFrac, SIGNAL(triggered()),
            this, SLOT(actionCoordsFrac()));

    connect(m_actionCoordsPreserveCart, SIGNAL(triggered()),
            this, SLOT(actionCoordsPreserveCart()));
    connect(m_actionCoordsPreserveFrac, SIGNAL(triggered()),
            this, SLOT(actionCoordsPreserveFrac()));

    connect(m_actionMatrixCart, SIGNAL(triggered()),
            this, SLOT(actionMatrixCart()));
    connect(m_actionMatrixFrac, SIGNAL(triggered()),
            this, SLOT(actionMatrixFrac()));

    connect(m_actionMatrixRowVectors, SIGNAL(triggered()),
            this, SLOT(actionMatrixRowVectors()));
    connect(m_actionMatrixColumnVectors, SIGNAL(triggered()),
            this, SLOT(actionMatrixColumnVectors()));

    // Initialize action groups
    //  m_actionUnitsLengthGroup
    m_actionUnitsLengthAngstrom->setCheckable(true);
    m_actionUnitsLengthBohr->setCheckable(true);
    m_actionUnitsLengthAngstrom->setChecked(true);
    //  m_actionUnitsAngleGroup
    m_actionUnitsAngleDegree->setCheckable(true);
    m_actionUnitsAngleRadian->setCheckable(true);
    m_actionUnitsAngleDegree->setChecked(true);
    //  m_actionCoordsCartFracGroup
    m_actionCoordsCart->setCheckable(true);
    m_actionCoordsFrac->setCheckable(true);
    m_actionCoordsCart->setChecked(true);
    //  m_actionCoordsPreserveGroup
    m_actionCoordsPreserveCart->setCheckable(true);
    m_actionCoordsPreserveFrac->setCheckable(true);
    m_actionCoordsPreserveCart->setChecked(true);
    //  m_actionMatrixCartFracGroup
    m_actionMatrixCart->setCheckable(true);
    m_actionMatrixFrac->setCheckable(true);
    m_actionMatrixCart->setChecked(true);
    //  m_actionMatrixVectorGroup
    m_actionMatrixRowVectors->setCheckable(true);
    m_actionMatrixColumnVectors->setCheckable(true);
    m_actionMatrixRowVectors->setChecked(true);
  }

  void CrystalBuilderDialog::createMenuBar()
  {
    m_menuTools = menuBar()->addMenu(tr("&Tools"));
    m_menuTools->addAction(m_actionSetSpacegroup);
    m_menuTools->addAction(m_actionFillCell);
    m_menuTools->addSeparator();
    m_menuTools->addAction(m_actionWrapAtoms);
    m_menuTools->addSeparator();
    m_menuTools->addAction(m_actionCopyToClipboard);
    m_menuTools->addSeparator();
    m_menuTools->addAction(m_actionNiggliReduce);
    m_menuTools->addSeparator();
    m_menuTools->addAction(m_actionScaleToVolume);

    m_menuSettings = menuBar()->addMenu(tr("&Settings"));

    m_menuSettingsUnits
      = m_menuSettings->addMenu(tr("&Units"));

    m_menuSettingsUnitsLength
      = m_menuSettingsUnits->addMenu(tr("&Length"));
    m_menuSettingsUnitsLength->
      addAction(m_actionUnitsLengthAngstrom);
    m_menuSettingsUnitsLength->
      addAction(m_actionUnitsLengthBohr);

    m_menuSettingsUnitsAngle
      = m_menuSettingsUnits->addMenu(tr("&Angle"));
    m_menuSettingsUnitsAngle->
      addAction(m_actionUnitsAngleDegree);
    m_menuSettingsUnitsAngle->
      addAction(m_actionUnitsAngleRadian);

    m_menuSettingsCoords
      = m_menuSettings->addMenu(tr("&Coordinates"));
    m_menuSettingsCoords->addAction(m_actionCoordsCart);
    m_menuSettingsCoords->addAction(m_actionCoordsFrac);
    m_menuSettingsCoords->addSeparator();
    m_menuSettingsCoords->
      addAction(m_actionCoordsPreserveCart);
    m_menuSettingsCoords->
      addAction(m_actionCoordsPreserveFrac);

    m_menuSettingsMatrix
      = m_menuSettings->addMenu(tr("&Matrix"));
    m_menuSettingsMatrix->addAction(m_actionMatrixCart);
    m_menuSettingsMatrix->addAction(m_actionMatrixFrac);
    m_menuSettingsMatrix->addSeparator();
    m_menuSettingsMatrix->
      addAction(m_actionMatrixRowVectors);
    m_menuSettingsMatrix->
      addAction(m_actionMatrixColumnVectors);
  }

  void CrystalBuilderDialog::initializeStatusBar()
  {
    statusBar()->addPermanentWidget(m_statusVolumeLabel
                                  = new QLabel(statusBar()));
    statusBar()->addPermanentWidget(m_statusLatticeLabel
                                  = new QLabel(statusBar()));
    statusBar()->addPermanentWidget(m_statusSpaceGroupLabel
                                  = new QLabel(statusBar()));
    statusBar()->addPermanentWidget(m_statusLengthUnitLabel
                                  = new QLabel(statusBar()));
    statusBar()->addPermanentWidget(m_statusAngleUnitLabel
                                  = new QLabel(statusBar()));

    // Apply style
    QObjectList objs (statusBar()->children());
    for (QObjectList::iterator
           it = objs.begin(),
           it_end = objs.end();
         it != it_end;
         ++it ) {
      QFrame *f = qobject_cast<QFrame*>(*it);
      if (!f) {
        continue;
      }
      f->setFrameStyle(QFrame::Panel | QFrame::Sunken);
      f->setLineWidth(1);
    }
  }

  void CrystalBuilderDialog::writeSettings()
  {
    QSettings settings;

    settings.beginGroup("crystalbuilderextension");

    settings.beginGroup("settings");

    settings.beginGroup("units");
    settings.setValue("length", lengthUnit());
    settings.setValue("angle",  angleUnit());
    settings.endGroup();

    settings.beginGroup("coords");
    settings.setValue("cartFrac", coordsCartFrac());
    settings.setValue("preserveCartFrac",
                      coordsPreserveCartFrac());
    settings.endGroup();

    settings.beginGroup("matrix");
    settings.setValue("cartFrac", matrixCartFrac());
    settings.setValue("vectorStyle", matrixVectorStyle());
    settings.endGroup();

    settings.endGroup(); // "settings"

    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    settings.endGroup(); // "crystalbuilderextension"
  }

  void CrystalBuilderDialog::readSettings()
  {
    QSettings settings;

    settings.beginGroup("crystalbuilderextension");

    settings.beginGroup("settings");

    settings.beginGroup("units");
    setLengthUnit(static_cast<LengthUnit>
                  (settings.value("length", Angstrom)
                   .toInt()));
    setAngleUnit(static_cast<AngleUnit>
                 (settings.value("angle", Degree).toInt()));
    settings.endGroup();

    settings.beginGroup("coords");
    setCoordsCartFrac(static_cast<CartFrac>
                      (settings.value("cartFrac", Cartesian)
                       .toInt()));
    setCoordsPreserveCartFrac(static_cast<CartFrac>
                              (settings.
                               value("preserveCartFrac",
                                     Cartesian).toInt()));
    settings.endGroup();

    settings.beginGroup("matrix");
    setMatrixCartFrac(static_cast<CartFrac>
                      (settings.value("cartFrac",
                                      Cartesian).toInt()));
    setMatrixVectorStyle(static_cast<VectorStyle>
                         (settings.value("vectorStyle",
                                         RowVectors)
                          .toInt()));
    settings.endGroup();

    settings.endGroup(); // "settings"

    restoreGeometry(settings.value
                    ("geometry").toByteArray());
    restoreState(settings.value
                 ("windowState").toByteArray());

    settings.endGroup(); // "crystalbuilderextension"

    refreshGui();
  }

  void CrystalBuilderDialog::refreshGui()
  {
    refreshSettings();
    refreshParameters();
    refreshMatrix();
    refreshCoords();
    refreshProperties();
    refreshUnitIndicators();
  }

  void CrystalBuilderDialog::refreshSettings()
  {
    switch (lengthUnit()) {
    case Angstrom:
      m_actionUnitsLengthAngstrom->setChecked(true);
      m_actionUnitsLengthBohr->setChecked(false);
      break;
    case Bohr:
      m_actionUnitsLengthAngstrom->setChecked(false);
      m_actionUnitsLengthBohr->setChecked(true);
      break;
    default:
      break;
    }

    switch (angleUnit()) {
    case Degree:
      m_actionUnitsAngleDegree->setChecked(true);
      m_actionUnitsAngleRadian->setChecked(false);
      break;
    case Radian:
      m_actionUnitsAngleDegree->setChecked(false);
      m_actionUnitsAngleRadian->setChecked(true);
      break;
    default:
      break;
    }

    switch (coordsCartFrac()) {
    case Cartesian:
      m_actionCoordsCart->setChecked(true);
      m_actionCoordsFrac->setChecked(false);
      break;
    case Fractional:
      m_actionCoordsCart->setChecked(false);
      m_actionCoordsFrac->setChecked(true);
      break;
    default:
      break;
    }

    switch (coordsPreserveCartFrac()) {
    case Cartesian:
      m_actionCoordsPreserveCart->setChecked(true);
      m_actionCoordsPreserveFrac->setChecked(false);
      break;
    case Fractional:
      m_actionCoordsPreserveCart->setChecked(false);
      m_actionCoordsPreserveFrac->setChecked(true);
      break;
    default:
      break;
    }

    switch (matrixCartFrac()) {
    case Cartesian:
      m_actionMatrixCart->setChecked(true);
      m_actionMatrixFrac->setChecked(false);
      break;
    case Fractional:
      m_actionMatrixCart->setChecked(false);
      m_actionMatrixFrac->setChecked(true);
      break;
    default:
      break;
    }

    switch (matrixVectorStyle()) {
    case RowVectors:
      m_actionMatrixRowVectors->setChecked(true);
      m_actionMatrixColumnVectors->setChecked(false);
      break;
    case ColumnVectors:
      m_actionMatrixRowVectors->setChecked(false);
      m_actionMatrixColumnVectors->setChecked(true);
      break;
    default:
      break;
    }
  }

  void CrystalBuilderDialog::refreshParameters()
  {
    UnitCellParameters params = currentCellParameters();

    ui.spin_a->blockSignals(true);
    ui.spin_b->blockSignals(true);
    ui.spin_c->blockSignals(true);
    ui.spin_alpha->blockSignals(true);
    ui.spin_beta->blockSignals(true);
    ui.spin_gamma->blockSignals(true);

    ui.spin_a->setValue(params.a);
    ui.spin_b->setValue(params.b);
    ui.spin_c->setValue(params.c);
    ui.spin_alpha->setValue(params.alpha);
    ui.spin_beta->setValue(params.beta);
    ui.spin_gamma->setValue(params.gamma);

    QString lengthSuffix;
    QString angleSuffix;
    switch (lengthUnit()) {
    case Angstrom:
      lengthSuffix = " " + CB_ANGSTROM;
      break;
    case Bohr:
      lengthSuffix = " a" + CB_SUB_ZERO;
      break;
    default:
      lengthSuffix = "";
      break;
    }

    switch(angleUnit()) {
    case Degree:
      angleSuffix = CB_DEGREE;
      break;
    case Radian:
      angleSuffix = " rad";
      break;
    default:
      angleSuffix = "";
      break;
    }

    ui.spin_a->setSuffix(lengthSuffix);
    ui.spin_b->setSuffix(lengthSuffix);
    ui.spin_c->setSuffix(QString(lengthSuffix));
    ui.spin_alpha->setSuffix(QString(angleSuffix));
    ui.spin_beta->setSuffix(QString(angleSuffix));
    ui.spin_gamma->setSuffix(QString(angleSuffix));

    ui.spin_a->blockSignals(false);
    ui.spin_b->blockSignals(false);
    ui.spin_c->blockSignals(false);
    ui.spin_alpha->blockSignals(false);
    ui.spin_beta->blockSignals(false);
    ui.spin_gamma->blockSignals(false);

    ui.gb_params->setEnabled(true);
    ui.spin_a->setEnabled(true);
    ui.spin_b->setEnabled(true);
    ui.spin_c->setEnabled(true);
    ui.spin_alpha->setEnabled(true);
    ui.spin_beta->setEnabled(true);
    ui.spin_gamma->setEnabled(true);
    ui.push_params_apply->setEnabled(false);
    ui.push_params_reset->setEnabled(false);
    emit parametersValid();
  }

  void CrystalBuilderDialog::refreshMatrix()
  {
    Eigen::Matrix3d mat;

    switch (matrixCartFrac()) {
    case Cartesian:
      switch (matrixVectorStyle()) {
      case RowVectors:
        mat = currentCellMatrix();
        break;
      case ColumnVectors:
        mat = currentCellMatrix().transpose();
        break;
      }
      ui.gb_matrix->setTitle(tr("Cell Matrix",
                                "Unit cell matrix"));
      break;
    case Fractional:
      switch (matrixVectorStyle()) {
      case RowVectors:
        mat = currentFractionalMatrix();
        break;
      case ColumnVectors:
        // TODO check math here:
        mat = currentFractionalMatrix().transpose();
        break;
      }
      ui.gb_matrix->setTitle(tr("Fractional Cell Matrix",
                                "Fractional unit cell matrix"));
      break;
    }

    // Clean up matrix
    for (unsigned short row = 0; row < 3; ++row) {
      for (unsigned short col = 0; col < 3; ++col) {
        double &current = mat(row,col);
        // Remove negative zeros:
        if (fabs(current) < 1e-10) {
          current = 0.0;
        }
      }
    }

    QString text =
      QString("%1 %2 %3\n"
              "%4 %5 %6\n"
              "%7 %8 %9")
      .arg(mat(0, 0), -9, 'f', 5, '0')
      .arg(mat(0, 1), -9, 'f', 5, '0')
      .arg(mat(0, 2), -9, 'f', 5, '0')
      .arg(mat(1, 0), -9, 'f', 5, '0')
      .arg(mat(1, 1), -9, 'f', 5, '0')
      .arg(mat(1, 2), -9, 'f', 5, '0')
      .arg(mat(2, 0), -9, 'f', 5, '0')
      .arg(mat(2, 1), -9, 'f', 5, '0')
      .arg(mat(2, 2), -9, 'f', 5, '0');

    ui.edit_matrix->blockSignals(true);
    ui.edit_matrix->setText(text);
    ui.edit_matrix->blockSignals(false);

    ui.edit_matrix->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));

    ui.gb_matrix->setEnabled(true);
    ui.edit_matrix->setEnabled(true);
    if (matrixCartFrac() == Fractional) {
      ui.edit_matrix->setReadOnly(true);
    }
    else {
      ui.edit_matrix->setReadOnly(false);
    }
    ui.push_matrix_apply->setEnabled(false);
    ui.push_matrix_reset->setEnabled(false);
    emit matrixValid();
  }

  void CrystalBuilderDialog::refreshCoords()
  {
    QList<Eigen::Vector3d> pos;

    switch (coordsCartFrac()) {
    case Cartesian:
      pos = currentCartesianCoords();
      ui.gb_coords->setTitle(tr("Cartesian Coordinates"));
      break;
    case Fractional:
      pos = currentFractionalCoords();
      ui.gb_coords->setTitle(tr("Fractional Coordinates"));
      break;
    }

    QList<QString> symbols = currentAtomicSymbols();

    // Non-fatal assert. If the number of atoms changed
    // in between the two calls, tail-recurse this function.
    if (pos.size() != symbols.size()) {
      return refreshCoords();
    }

    // Clean up number display
    for (QList<Eigen::Vector3d>::iterator
           it = pos.begin(),
           it_end = pos.end();
         it != it_end; ++it) {
      // Remove negative zeros
      if (fabs(it->x()) < 1e-10) {
        it->x() = 0.0;
      }
      if (fabs(it->y()) < 1e-10) {
        it->y() = 0.0;
      }
      if (fabs(it->z()) < 1e-10) {
        it->z() = 0.0;
      }
    }
    QString text;

    for (int i = 0; i < pos.size(); ++i) {
      text += QString("%1 %2 %3 %4\n")
        .arg(symbols[i], -2)
        .arg(pos[i][0], -9, 'f', 5, '0')
        .arg(pos[i][1], -9, 'f', 5, '0')
        .arg(pos[i][2], -9, 'f', 5, '0');
    }

    ui.edit_coords->blockSignals(true);
    ui.edit_coords->setText(text);
    ui.edit_coords->blockSignals(false);

    ui.edit_coords->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));

    ui.gb_coords->setEnabled(true);
    ui.edit_coords->setEnabled(true);
    ui.push_coords_apply->setEnabled(false);
    ui.push_coords_reset->setEnabled(false);
    emit coordsValid();
  }

  void CrystalBuilderDialog::refreshProperties()
  {
    refreshVolume();
    refreshLattice();
    refreshSpacegroup();
  }

  void CrystalBuilderDialog::refreshVolume()
  {
    QString lengthSuffix;
    switch (lengthUnit()) {
    case Angstrom:
      lengthSuffix = " " + CB_ANGSTROM + CB_SUPER_THREE;
      break;
    case Bohr:
      lengthSuffix = " a" + CB_SUB_ZERO + CB_SUPER_THREE;
      break;
    default:
      lengthSuffix = "";
      break;
    }

    m_statusVolumeLabel->setText(tr("Volume: %1%2",
                                    "Unit cell volume")
                                 .arg(currentVolume(), 0,
                                      'f', 2)
                                 .arg(lengthSuffix));
  }

  void CrystalBuilderDialog::refreshLattice()
  {
    OpenBabel::OBUnitCell *cell = currentCell();

    if (!cell) {
      m_statusLatticeLabel->setVisible(false);
      return;
    }

    QString lattice;
    switch (cell->GetLatticeType()) {
    case OBUnitCell::Undefined:
      m_statusLatticeLabel->setVisible(false);
      return;
    case OBUnitCell::Triclinic:
      lattice = tr("Triclinic", "Lattice type");
      break;
    case OBUnitCell::Monoclinic:
      lattice = tr("Monoclinic", "Lattice type");
      break;
    case OBUnitCell::Orthorhombic:
      lattice = tr("Orthoorhombic", "Lattice type");
      break;
    case OBUnitCell::Tetragonal:
      lattice = tr("Tetragonal", "Lattice type");
      break;
    case OBUnitCell::Rhombohedral:
      lattice = tr("Rhombohedral", "Lattice type");
      break;
    case OBUnitCell::Hexagonal:
      lattice = tr("Hexagonal", "Lattice type");
      break;
    case OBUnitCell::Cubic:
      lattice = tr("Cubic", "Lattice type");
      break;
    }

    m_statusLatticeLabel->setText(lattice);
    m_statusLatticeLabel->setVisible(true);
  }

  void CrystalBuilderDialog::refreshSpacegroup()
  {
    OpenBabel::OBUnitCell *cell = currentCell();

    if (cell) {
      const OpenBabel::SpaceGroup *sg;
      sg = cell->GetSpaceGroup();
      if (sg) {
        m_statusSpaceGroupLabel->
          setText(tr("%1 (%2)")
                  .arg(sg->GetHMName().c_str())
                  .arg(sg->GetId()));
        m_statusSpaceGroupLabel->setVisible(true);
        return;
      }
    }
    m_statusSpaceGroupLabel->setVisible(false);
    return;
  }

  void CrystalBuilderDialog::refreshUnitIndicators()
  {
    switch (lengthUnit()) {
    case Angstrom:
      m_statusLengthUnitLabel->setText(CB_ANGSTROM);
      m_statusLengthUnitLabel->setVisible(true);
      break;
    case Bohr:
      m_statusLengthUnitLabel->setText("a" + CB_SUB_ZERO);
      m_statusLengthUnitLabel->setVisible(true);
      break;
    default:
      m_statusLengthUnitLabel->setVisible(false);
      break;
    }

    switch (angleUnit()) {
    case Degree:
      m_statusAngleUnitLabel->setText(CB_DEGREE);
      m_statusAngleUnitLabel->setVisible(true);
      break;
    case Radian:
      m_statusAngleUnitLabel->setText("rad");
      m_statusAngleUnitLabel->setVisible(true);
      break;
    default:
      m_statusAngleUnitLabel->setVisible(false);
      break;
    }
  }

  void CrystalBuilderDialog::updateGuiForMatrixEdit()
  {
    ui.gb_params->setEnabled(false);
    ui.gb_coords->setEnabled(false);
    ui.push_matrix_apply->setEnabled(true);
    ui.push_matrix_reset->setEnabled(true);
  }

  void CrystalBuilderDialog::updateGuiForParametersEdit()
  {
    ui.gb_matrix->setEnabled(false);
    ui.gb_coords->setEnabled(false);
    ui.push_params_apply->setEnabled(true);
    ui.push_params_reset->setEnabled(true);
  }

  void CrystalBuilderDialog::updateGuiForCoordsEdit()
  {
    ui.gb_params->setEnabled(false);
    ui.gb_matrix->setEnabled(false);
    ui.push_coords_apply->setEnabled(true);
    ui.push_coords_reset->setEnabled(true);
  }

  Eigen::Matrix3d CrystalBuilderDialog::checkMatrixWidget()
  {
    // Editing fractional matrix
    Q_ASSERT(matrixCartFrac() != Fractional);

    QString text = ui.edit_matrix->document()->toPlainText();
    QStringList lines = text.split("\n",
                                   QString::SkipEmptyParts);
    if (lines.size() != 3) {
      emit matrixInvalid();
      return Eigen::Matrix3d::Zero();
    }

    QList<QStringList> stringVecs;
    Eigen::Matrix3d mat;
    for (int row = 0; row < 3; ++row) {
      stringVecs.append(lines.at(row).simplified()
                        .split(QRegExp("\\s+|,|;")));
      QStringList &stringVec = stringVecs[row];
      if (stringVec.size() != 3) {
        emit matrixInvalid();
        return Eigen::Matrix3d::Zero();
      }
      for (int col = 0; col < 3; ++col) {
        bool ok;
        double val = stringVec[col].toDouble(&ok);
        if (!ok) {
          emit matrixInvalid();
          return Eigen::Matrix3d::Zero();
        }
        mat(row,col) = val;
      }
    }

    if (matrixVectorStyle() == ColumnVectors) {
      mat = mat.transpose();
    }

    emit matrixValid();
    return mat;
  }

  CrystalBuilderDialog::UnitCellParameters
  CrystalBuilderDialog::checkParametersWidget()
  {
    UnitCellParameters p;
    p.a = ui.spin_a->value();
    p.b = ui.spin_b->value();
    p.c = ui.spin_c->value();
    p.alpha = ui.spin_alpha->value();
    p.beta  = ui.spin_beta->value();
    p.gamma = ui.spin_gamma->value();
    emit parametersValid();
    return p;
  }

  void CrystalBuilderDialog::checkCoordsWidget()
  {
    QStringList list;
    QString text = ui.edit_coords->document()->toPlainText();
    QStringList lines = text.split("\n",
                                   QString::SkipEmptyParts);
    // Clean up lines
    for (QStringList::iterator
           it = lines.begin(),
           it_end = lines.end();
         it != it_end; ++it) {
      *it = it->trimmed();
    }
    lines.removeAll("");

    QStringList row;

    for (QStringList::const_iterator
           it = lines.constBegin(),
           it_end = lines.constEnd();
         it != it_end;
         ++it) {
      row = it->split(QRegExp("\\s+|,|;"),
                      QString::SkipEmptyParts);
      if (row.size() != 4) {
        emit coordsInvalid();
        return;
      }
      bool ok;
      row.at(1).toDouble(&ok);
      if (ok) {
        row.at(2).toDouble(&ok);
      }
      if (ok) {
        row.at(3).toDouble(&ok);
      }
      if (!ok) {
        emit coordsInvalid();
        return;
      }
    }

    emit coordsValid();
  }

  void CrystalBuilderDialog::updateMatrix()
  {
    Eigen::Matrix3d mat = checkMatrixWidget();
    if (!mat.isZero()) {
      setCurrentCellMatrix(mat);
    }
  }

  void CrystalBuilderDialog::updateParameters()
  {
    UnitCellParameters p = checkParametersWidget();
    // TODO there is no parameter validation at the moment
    //if (p.isValid()) {
    setCurrentCellParameters(p);
    //}
  }

  void CrystalBuilderDialog::updateCoords()
  {
    QStringList list;
    QString text = ui.edit_coords->document()->toPlainText();
    QStringList lines = text.split("\n",
                                   QString::SkipEmptyParts);

    // Clean up lines
    for (QStringList::iterator
           it = lines.begin(),
           it_end = lines.end();
         it != it_end; ++it) {
      *it = it->trimmed();
    }
    lines.removeAll("");

    double x,y,z;
    QStringList ids;
    QStringList row;
    QList<Eigen::Vector3d> coords;
    ids.reserve(lines.size());
    coords.reserve(lines.size());

    for (QStringList::const_iterator
           it = lines.constBegin(),
           it_end = lines.constEnd();
         it != it_end;
         ++it) {
      row = it->split(QRegExp("\\s+|,|;"),
                      QString::SkipEmptyParts);
      if (row.size() != 4) {
        emit coordsInvalid();
        return;
      }
      ids.append(row.at(0));
      bool ok;
      x = row.at(1).toDouble(&ok);
      if (ok) {
        y = row.at(2).toDouble(&ok);
      }
      if (ok) {
        z = row.at(3).toDouble(&ok);
      }
      if (!ok) {
        emit coordsInvalid();
        return;
      }
      coords.append(Eigen::Vector3d(x,y,z));
    }

    if (coordsCartFrac() == Fractional) {
      setCurrentFractionalCoords(ids, coords);
    }
    else {
      setCurrentCartesianCoords(ids, coords);
    }

    emit coordsValid();
  }

  void CrystalBuilderDialog::markMatrixAsInvalid()
  {
    QTextCursor tc (ui.edit_matrix->document());
    QTextCharFormat redFormat;
    redFormat.setForeground(QBrush(Qt::red));
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_matrix->blockSignals(true);
    tc.mergeCharFormat(redFormat);
    ui.edit_matrix->blockSignals(false);
    ui.edit_matrix->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));
  }

  void CrystalBuilderDialog::markMatrixAsValid()
  {
    QTextCursor tc (ui.edit_matrix->document());
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_matrix->blockSignals(true);
    tc.setCharFormat(m_matrixCharFormat);
    ui.edit_matrix->blockSignals(false);
    ui.edit_matrix->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));
  }

  void CrystalBuilderDialog::markParametersAsInvalid()
  {
  }

  void CrystalBuilderDialog::markParametersAsValid()
  {
  }

  void CrystalBuilderDialog::markCoordsAsInvalid()
  {
    QTextCursor tc (ui.edit_coords->document());
    QTextCharFormat redFormat;
    redFormat.setForeground(QBrush(Qt::red));
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_coords->blockSignals(true);
    tc.mergeCharFormat(redFormat);
    ui.edit_coords->blockSignals(false);
    ui.edit_coords->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));
  }

  void CrystalBuilderDialog::markCoordsAsValid()
  {
    QTextCursor tc (ui.edit_coords->document());
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::End,
                    QTextCursor::KeepAnchor);
    ui.edit_coords->blockSignals(true);
    tc.setCharFormat(m_matrixCharFormat);
    ui.edit_coords->blockSignals(false);
    ui.edit_coords->setCurrentFont(QFont("Monospace",
                                         CB_FONTSIZE));
  }

  void CrystalBuilderDialog::closeEvent(QCloseEvent *event)
  {
    writeSettings();
    QMainWindow::closeEvent(event);
    this->deleteLater();
  }

  double CrystalBuilderDialog::convertLength(double length)
  {
    return length * lengthConversionFactor();
  }

  Eigen::Vector3d  CrystalBuilderDialog::convertLength
  (const Eigen::Vector3d& length)
  {
    return length * lengthConversionFactor();
  }

  Eigen::Matrix3d CrystalBuilderDialog::convertLength
  (const Eigen::Matrix3d& length)
  {
    return length * lengthConversionFactor();
  }

  double CrystalBuilderDialog::convertAngle(double angle)
  {
    return angle * angleConversionFactor();
  }

  //  display -> storage
  double CrystalBuilderDialog::unconvertLength(double length)
  {
    return length * (1.0 / lengthConversionFactor());
  }

  Eigen::Vector3d CrystalBuilderDialog::unconvertLength(const Eigen::Vector3d& length)
  {
    return length * (1.0 / lengthConversionFactor());
  }

  Eigen::Matrix3d CrystalBuilderDialog::unconvertLength(const Eigen::Matrix3d& length)
  {
    return length * (1.0 / lengthConversionFactor());
  }

  double CrystalBuilderDialog::unconvertAngle(double angle)
  {
    return angle * (1.0 / angleConversionFactor());
  }

  // retreival functions
  Eigen::Matrix3d CrystalBuilderDialog::currentCellMatrix()
  {
    if (!currentCell()) {
      return Eigen::Matrix3d::Zero();
    }

    return convertLength(OB2Eigen(currentCell()->GetCellMatrix()));
  }

  Eigen::Matrix3d
  CrystalBuilderDialog::currentFractionalMatrix()
  {
    if (!currentCell()) {
      return Eigen::Matrix3d::Zero();
    }

    return OB2Eigen(currentCell()->GetFractionalMatrix());
  }

  CrystalBuilderDialog::UnitCellParameters
    CrystalBuilderDialog::currentCellParameters()
  {
    UnitCellParameters params (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    OpenBabel::OBUnitCell *cell = currentCell();
    if (!cell) {
      return params;
    }

    params.a = convertLength(cell->GetA());
    params.b = convertLength(cell->GetB());
    params.c = convertLength(cell->GetC());
    params.alpha = convertAngle(cell->GetAlpha());
    params.beta  = convertAngle(cell->GetBeta());
    params.gamma = convertAngle(cell->GetGamma());

    return params;
  }

  QList<Eigen::Vector3d>
  CrystalBuilderDialog::currentCartesianCoords()
  {
    QList<Eigen::Vector3d> result;
    QList<Avogadro::Atom*> atoms = m_molecule->atoms();

    for (QList<Avogadro::Atom*>::const_iterator
           it = atoms.constBegin(),
           it_end = atoms.constEnd();
         it != it_end;
         ++it) {
      result << convertLength(*((*it)->pos()));
    }
    return result;
  }

  QList<Eigen::Vector3d>
  CrystalBuilderDialog::currentFractionalCoords()
  {
    OpenBabel::OBUnitCell *cell = currentCell();
    if (!cell) {
      return QList<Eigen::Vector3d>();
    }

    Eigen::Matrix3d frac = currentFractionalMatrix();
    QList<Eigen::Vector3d> result = currentCartesianCoords();
    for (QList<Eigen::Vector3d>::iterator
           it = result.begin(),
           it_end = result.end();
         it != it_end;
         ++it) {
      // Convert cartesian coords back to storage units
      (*it) = unconvertLength(*it);
      // Cartesian to fractional units
      (*it) = OB2Eigen(cell->CartesianToFractional
                       (Eigen2OB((*it))));
    }
    return result;
  }

  QList<int> CrystalBuilderDialog::currentAtomicNumbers()
  {
    QList<int> result;
    QList<Avogadro::Atom*> atoms = m_molecule->atoms();

    for (QList<Avogadro::Atom*>::const_iterator
           it = atoms.constBegin(),
           it_end = atoms.constEnd();
         it != it_end;
         ++it) {
      result << (*it)->atomicNumber();
    }

    return result;
  }

  QList<QString> CrystalBuilderDialog::currentAtomicSymbols()
  {
    QList<QString> result;
    QList<Avogadro::Atom*> atoms = m_molecule->atoms();

    for (QList<Avogadro::Atom*>::const_iterator
           it = atoms.constBegin(),
           it_end = atoms.constEnd();
         it != it_end;
         ++it) {
      result <<
        OpenBabel::etab.GetSymbol((*it)->atomicNumber());
    }
    return result;
  }

  double CrystalBuilderDialog::currentVolume()
  {
    OpenBabel::OBUnitCell *cell = currentCell();

    double volume;
    if (!cell) {
      volume = 0.0;
    }
    else {
      volume = cell->GetCellVolume();
      double factor = lengthConversionFactor();
      factor = factor*factor*factor;
      volume *= factor;
    }

    return volume;
  }

  // Adapted from unitcellextension:
  void CrystalBuilderDialog::fillUnitCell()
  {
    OpenBabel::OBUnitCell *cell = currentCell();

    const SpaceGroup *sg = cell->GetSpaceGroup();
    if (!sg) {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::information
        (this, CB_DIALOG_TITLE,
         tr("There is not a spacegroup set for this "
            "document.\n\nWould you like to set a "
            "spacegroup now?"),
         QMessageBox::Yes | QMessageBox::No,
         QMessageBox::Yes);
      if (reply == QMessageBox::Yes) {
        actionSetSpacegroup();
        // Tail recurse to restart
        return fillUnitCell();
      }
      else {
        return;
      }
    }

    wrapAtomsToCell();

    QList<Eigen::Vector3d> origFCoords =
      currentFractionalCoords();
    QList<Eigen::Vector3d> newFCoords;

    QList<QString> origIds = currentAtomicSymbols();
    QList<QString> newIds;

    // Duplicate tolerance squared
    const double dupTolSquared = 1e-8; // 1e-4 ^2

    // Non-fatal assert -- if the number of atoms have
    // changed, just tail-recurse and try again.
    if (origIds.size() != origFCoords.size()) {
      return fillUnitCell();
    }

    const QString *curId;
    const Eigen::Vector3d *curVec;
    std::list<OpenBabel::vector3> obxformed;
    std::list<OpenBabel::vector3>::const_iterator obxit;
    std::list<OpenBabel::vector3>::const_iterator obxit_end;
    QList<Eigen::Vector3d> xformed;
    QList<Eigen::Vector3d>::const_iterator xit, xit_end;
    QList<Eigen::Vector3d>::const_iterator newit, newit_end;
    for (int i = 0; i < origIds.size(); ++i) {
      curId = &origIds[i];
      curVec = &origFCoords[i];

      // Round off to remove floating point math errors
      double x = StableComp::round(curVec->x(), 6);
      double y = StableComp::round(curVec->y(), 6);
      double z = StableComp::round(curVec->z(), 6);

      // Get tranformed OB vectors
      obxformed = sg->Transform(OpenBabel::vector3(x,y,z));

      // Convert to Eigen, wrap to cell
      xformed.clear();
      Eigen::Vector3d tmp;
      obxit_end = obxformed.end();
      for (obxit = obxformed.begin();
           obxit != obxit_end; ++obxit) {
        tmp = OB2Eigen(*obxit);
        // Pseudo-modulus
        tmp.x() -= static_cast<int>(tmp.x());
        tmp.y() -= static_cast<int>(tmp.y());
        tmp.z() -= static_cast<int>(tmp.z());
        // Correct negative values
        if (tmp.x() < 0.0) ++tmp.x();
        if (tmp.y() < 0.0) ++tmp.y();
        if (tmp.z() < 0.0) ++tmp.z();
        // Add a fudge factor for cell edges
        if (tmp.x() >= 1.0 - 1e-4) tmp.x() = 0.0;
        if (tmp.y() >= 1.0 - 1e-4) tmp.y() = 0.0;
        if (tmp.z() >= 1.0 - 1e-4) tmp.z() = 0.0;
        xformed.append(tmp);
      }

      // Check all xformed vectors against the coords
      // already added. if they match, skip this atom.
      bool duplicate;
      xit_end = xformed.constEnd();
      for (xit = xformed.constBegin();
           xit != xit_end; ++xit) {
        newit_end = newFCoords.constEnd();
        duplicate = false;
        for (newit = newFCoords.constBegin();
             newit != newit_end; ++newit) {
          if (fabs((*newit - *xit).squaredNorm())
              < dupTolSquared) {
            duplicate = true;
            break;
          }
        }

        if (duplicate) {
          continue;
        }

        // Add transformed atom
        newFCoords.append(*xit);
        newIds.append(*curId);
      }
    }

    setCurrentFractionalCoords(newIds, newFCoords);
  }

  void CrystalBuilderDialog::wrapAtomsToCell()
  {
    QList<Eigen::Vector3d> fcoords =
      currentFractionalCoords();

    for (QList<Eigen::Vector3d>::iterator
           it = fcoords.begin(),
           it_end = fcoords.end();
         it != it_end; ++it) {
      // Pseudo-modulus
      it->x() -= static_cast<int>(it->x());
      it->y() -= static_cast<int>(it->y());
      it->z() -= static_cast<int>(it->z());
      // Correct negative values
      if (it->x() < 0) ++(it->x());
      if (it->y() < 0) ++(it->y());
      if (it->z() < 0) ++(it->z());
      // Add a fudge factor for cell edges
      if (it->x() >= 1.0 - 1e-4) it->x() = 0.0;
      if (it->y() >= 1.0 - 1e-4) it->y() = 0.0;
      if (it->z() >= 1.0 - 1e-4) it->z() = 0.0;
    }
    setCurrentFractionalCoords(currentAtomicSymbols(),
                               fcoords);
  }

  // Stable sort both ids and coords together to group all entries in
  // ids together. Entries are not sorted in any particular order,
  // just grouped. uniqIds and idCounts will contain a unique list of
  // all ids in same order as in the sorted ids and a list containing
  // how many of each id is in ids, respectively.
  inline void poscarSort(QList<QString> *ids,
                         QList<Eigen::Vector3d> *coords,
                         QList<QString> *uniqueIds,
                         QList<unsigned int> *idCounts)
  {
    Q_ASSERT(ids->size() == coords->size());
    // Get unique list of ids and count them
    uniqueIds->clear();
    idCounts->clear();
    for (QStringList::const_iterator
           it = ids->constBegin(),
           it_end = ids->constEnd();
         it != it_end; ++it) {
      int ind = uniqueIds->indexOf(*it);
      if (ind != -1) {
        ++((*idCounts)[ind]);
      }
      else {
        uniqueIds->append(*it);
        idCounts->append(1);
      }
    }

    // Sort lists
    QString curId;
    QStringList::iterator idit;
    QStringList::iterator idit_end = ids->end();
    QList<Eigen::Vector3d>::iterator coordit;
    QList<Eigen::Vector3d>::iterator coordit_end = coords->end();
    unsigned int sorted = 0;
    for (int uniqInd = 0; uniqInd < uniqueIds->size();
         ++uniqInd) {
      curId = (*uniqueIds)[uniqInd];
      unsigned int found = 0;
      unsigned int count = idCounts->at(uniqInd);
      idit = ids->begin() + sorted;
      coordit = coords->begin() + sorted;
      while (found < count) {
        // Should never reach the end
        Q_ASSERT(idit != idit_end);
        Q_ASSERT(coordit != coordit_end);
        if (idit->compare(curId) == 0) {
          qSwap(*idit, (*ids)[sorted]);
          qSwap(*coordit, (*coords)[sorted]);
          ++found;
          ++sorted;
        }
        ++idit;
        ++coordit;
      }
    }
  }

  void CrystalBuilderDialog::copyToClipboard()
  {
    QStringList ids = currentAtomicSymbols();
    QList<Eigen::Vector3d> coords = currentFractionalCoords();
    QStringList uniqueIds;
    QList<unsigned int> idCounts;

    // non-fatal assert:
    if (ids.size() != coords.size()) {
      return copyToClipboard();
    }

    poscarSort(&ids, &coords, &uniqueIds, &idCounts);

    Q_ASSERT(uniqueIds.size() == idCounts.size());

    QString poscar;

    // Comment line: composition
    for (unsigned int i = 0;
         i < static_cast<unsigned int>(uniqueIds.size());
         ++i) {
      poscar += QString("%1%2 ").arg(uniqueIds[i]).arg(idCounts[i]);
    }
    poscar += "\n";
    // Scaling factor. Just 1.0
    poscar += QString::number(1.0);
    poscar += "\n";
    // Unit Cell Vectors
    std::vector< vector3 > vecs = currentCell()->GetCellVectors();
    for (unsigned int i = 0; i < vecs.size(); i++) {
      poscar += QString("  %1 %2 %3\n")
        .arg(vecs[i].x(), 12, 'f', 8)
        .arg(vecs[i].y(), 12, 'f', 8)
        .arg(vecs[i].z(), 12, 'f', 8);
    }
    // Number of each type of atom
    for (int i = 0; i < idCounts.size(); i++) {
      poscar += QString::number(idCounts.at(i)) + " ";
    }
    poscar += "\n";
    // Use fractional coordinates:
    poscar += "Direct\n";
    // Coordinates of each atom (sorted alphabetically by symbol)
    for (int i = 0; i < coords.size(); i++) {
      poscar += QString("  %1 %2 %3\n")
        .arg(coords[i].x(), 12, 'f', 8)
        .arg(coords[i].y(), 12, 'f', 8)
        .arg(coords[i].z(), 12, 'f', 8);
    }

    // Set to system clipboard
    QApplication::clipboard()->setText
      (poscar, QClipboard::Clipboard);
    // For middle-click on X11
    if (QApplication::clipboard()->supportsSelection()) {
      QApplication::clipboard()->setText
        (poscar, QClipboard::Selection);
    }
  }

  // Implements the niggli reduction algorithm detailed in:
  // Grosse-Kunstleve RW, Sauter NK, Adams PD. Numerically stable
  // algorithms for the computation of reduced unit cells. Acta
  // Crystallographica Section A Foundations of
  // Crystallography. 2003;60(1):1-6.
  void CrystalBuilderDialog::niggliReduce()
  {
    const unsigned int iterations = 1000;

    // Get cell parameters in storage units
    OpenBabel::OBUnitCell *cell = currentCell();

    // convert deg->rad
    double a     = cell->GetA();
    double b     = cell->GetB();
    double c     = cell->GetC();
    double alpha = cell->GetAlpha() * DEG_TO_RAD;
    double beta  = cell->GetBeta()  * DEG_TO_RAD;
    double gamma = cell->GetGamma() * DEG_TO_RAD;

    // Compute characteristic (step 0)
    double A    = a*a;
    double B    = b*b;
    double C    = c*c;
    double xi   = 2*b*c*cos(alpha);
    double eta  = 2*a*c*cos(beta);
    double zeta = 2*a*b*cos(gamma);

    // Return value
    bool ret = false;

    // comparison tolerance
    double tol = STABLE_COMP_TOL * pow(a * b * c, 1.0/3.0);

    unsigned int iter;
    for (iter = 0; iter < iterations; ++iter) {
      // Step 1:
      if (
          StableComp::gt(A, B, tol)
          || (
              StableComp::eq(A, B, tol)
              &&
              StableComp::gt(fabs(xi), fabs(eta), tol)
              )
          ) {
        qSwap(A, B);
        qSwap(xi, eta);
      }

      // Step 2:
      if (
          StableComp::gt(B, C, tol)
          || (
              StableComp::eq(B, C, tol)
              &&
              StableComp::gt(fabs(eta), fabs(zeta), tol)
              )
          ) {
        qSwap(B, C);
        qSwap(eta, zeta);
        continue;
      }

      double xiEtaZeta = xi*eta*zeta;
      // Step 3:
      if (StableComp::gt(xiEtaZeta, 0, tol)) {
        xi   = fabs(xi);
        eta  = fabs(eta);
        zeta = fabs(zeta);
      }

      // Step 4:
      if (StableComp::leq(xiEtaZeta, 0, tol)) {
        xi   = -fabs(xi);
        eta  = -fabs(eta);
        zeta = -fabs(zeta);
      }

      // Step 5:
      if (StableComp::gt(fabs(xi), B, tol)
          || (StableComp::eq(xi, B, tol)
              && StableComp::lt(2*eta, zeta, tol)
              )
          || (StableComp::eq(xi, -B, tol)
              && StableComp::lt(zeta, 0, tol)
              )
          ) {
        double signXi = StableComp::sign(xi);
        C    = B + C - xi*signXi;
        eta  = eta - zeta*signXi;
        xi   = xi -   2*B*signXi;
        continue;
      }

      // Step 6:
      if (StableComp::gt(fabs(eta), A, tol)
          || (StableComp::eq(eta, A, tol)
              && StableComp::lt(2*xi, zeta, tol)
              )
          || (StableComp::eq(eta, -A, tol)
              && StableComp::lt(zeta, 0, tol)
              )
          ) {
        double signEta = StableComp::sign(eta);
        C    = A + C - eta*signEta;
        xi   = xi - zeta*signEta;
        eta  = eta - 2*A*signEta;
        continue;
      }

      // Step 7:
      if (StableComp::gt(fabs(zeta), A, tol)
          || (StableComp::eq(zeta, A, tol)
              && StableComp::lt(2*xi, eta, tol)
              )
          || (StableComp::eq(zeta, -A, tol)
              && StableComp::lt(eta, 0, tol)
              )
          ) {
        double signZeta = StableComp::sign(zeta);
        B    = A + B - zeta*signZeta;
        xi   = xi - eta*signZeta;
        zeta = zeta - 2*A*signZeta;
        continue;
      }

      // Step 8:
      double sumAllButC = A + B + xi + eta + zeta;
      if (StableComp::lt(sumAllButC, 0, tol)
          || (StableComp::eq(sumAllButC, 0, tol)
              && StableComp::gt(2*(A+eta)+zeta, 0, tol)
              )
          ) {
        C    = sumAllButC + C;
        xi = 2*B + xi + zeta;
        eta  = 2*A + eta + zeta;
        continue;
      }

      // Done!
      ret = true;
      break;
    }

    // No change
    if (iter == 0) {
      wrapAtomsToCell();
      QMessageBox::information
        (this,
         CB_DIALOG_TITLE,
         tr("This unit cell is already reduced to "
            "its canonical Niggli representation."));
      return;
    }

    // Update values
    if (ret == true) {
      UnitCellParameters p;
      p.a = sqrt(A);
      p.b = sqrt(B);
      p.c = sqrt(C);
      p.alpha = acos(xi   / (2*(p.b)*(p.c))) * RAD_TO_DEG;
      p.beta  = acos(eta  / (2*(p.a)*(p.c))) * RAD_TO_DEG;
      p.gamma = acos(zeta / (2*(p.a)*(p.b))) * RAD_TO_DEG;
      p.a = convertLength(p.a);
      p.b = convertLength(p.b);
      p.c = convertLength(p.c);
      p.alpha = convertAngle(p.alpha);
      p.beta  = convertAngle(p.beta);
      p.gamma = convertAngle(p.gamma);

      // Do not preserve fractional coordinates on this change
      CartFrac preserve = coordsPreserveCartFrac();
      setCoordsPreserveCartFrac(Cartesian);
      setCurrentCellParameters(p);
      setCoordsPreserveCartFrac(preserve);
    }
    else {
      QMessageBox::warning
        (this,
         CB_DIALOG_TITLE,
         tr("Failed to reduce cell after 1000 iterations of "
            "the reduction algorithm. Stopping."));
      return;
    }
  }

  void CrystalBuilderDialog::setCurrentCell
  (OpenBabel::OBUnitCell *cell)
  {
    if (coordsPreserveCartFrac() == Fractional) {
      cacheFractionalCoordinates();
    }

    m_molecule->setOBUnitCell(cell);

    if (coordsPreserveCartFrac() == Fractional) {
      restoreFractionalCoordinates();
    }

    emit cellChanged();
  }

  void CrystalBuilderDialog::setCurrentCellMatrix
  (const Eigen::Matrix3d &mat)
  {
    Q_ASSERT_X(matrixCartFrac() != Fractional,
               Q_FUNC_INFO,
               "Modification of fractional matrix "
               "is not supported.");

    OpenBabel::OBUnitCell *cell = currentCell();
    if (coordsPreserveCartFrac() == Fractional) {
      cacheFractionalCoordinates();
    }
    cell->SetData(Eigen2OB(unconvertLength(mat)));
    if (coordsPreserveCartFrac() == Fractional) {
      restoreFractionalCoordinates();
    }
    emit cellChanged();
  }

  void CrystalBuilderDialog::setCurrentCellParameters
  (const UnitCellParameters &params)
  {
    OpenBabel::OBUnitCell *cell = currentCell();

    if (coordsPreserveCartFrac() == Fractional) {
      cacheFractionalCoordinates();
    }

    cell->SetData(unconvertLength(params.a),
                  unconvertLength(params.b),
                  unconvertLength(params.c),
                  unconvertAngle(params.alpha),
                  unconvertAngle(params.beta),
                  unconvertAngle(params.gamma));

    if (coordsPreserveCartFrac() == Fractional) {
      restoreFractionalCoordinates();
    }

    emit cellChanged();
  }

  inline void
  updateMolecule(Avogadro::Molecule *mol,
                 const QList<QString> &ids,
                 const QList<Eigen::Vector3d> &coords)
  {
    // Remove old atoms
    QWriteLocker locker (mol->lock());
    QList<Avogadro::Atom*> atoms = mol->atoms();
    for (QList<Avogadro::Atom*>::iterator
           it = atoms.begin(),
           it_end = atoms.end();
         it != it_end;
         ++it) {
      mol->removeAtom(*it);
    }

    // Add new atoms
    for (int i = 0; i < ids.size(); ++i) {
      Atom *atom = mol->addAtom();
      atom->setAtomicNumber(OpenBabel::etab.GetAtomicNum
                            (ids[i].toStdString().c_str()));
      atom->setPos(coords[i]);
    }
  }

  void CrystalBuilderDialog::setCurrentFractionalCoords
  (const QList<QString> &ids,
   const QList<Eigen::Vector3d> &fcoords)
  {
    OpenBabel::OBUnitCell *cell = currentCell();
    QList<Eigen::Vector3d> coords;
    coords.reserve(fcoords.size());

    for (QList<Eigen::Vector3d>::const_iterator
           it = fcoords.constBegin(),
           it_end = fcoords.constEnd();
         it != it_end;
         ++it) {
      // Convert to storage cartesian
      coords.append(OB2Eigen(cell->FractionalToCartesian
                             (Eigen2OB(*it))));
    }

    updateMolecule(m_molecule, ids, coords);
    emit cellChanged();
  }

  void CrystalBuilderDialog::setCurrentCartesianCoords
  (const QList<QString> &ids,
   const QList<Eigen::Vector3d> &coords)
  {
    Q_ASSERT(ids.size() == coords.size());

    QList<Eigen::Vector3d> scoords;
    scoords.reserve(coords.size());

    for (QList<Eigen::Vector3d>::const_iterator
           it = coords.constBegin(),
           it_end = coords.constEnd();
         it != it_end;
         ++it) {
      // Convert to storage cartesian
      scoords.append(unconvertLength(*it));
    }

    updateMolecule(m_molecule, ids, scoords);
    emit cellChanged();
  }

  void CrystalBuilderDialog::setCurrentVolume(double volume)
  {
    // Get scaling factor
    double factor = pow(volume/currentVolume(), 1.0/3.0);

    UnitCellParameters p = currentCellParameters();
    p.a *= factor;
    p.b *= factor;
    p.c *= factor;
    setCurrentCellParameters(p);
  }

  void CrystalBuilderDialog::actionSetSpacegroup()
  {
    QStringList spacegroups;
    const OpenBabel::SpaceGroup *sg;
    for (unsigned int i = 1; i <= 230; ++i) {
      sg = OpenBabel::SpaceGroup::GetSpaceGroup(i);
      spacegroups << QString("%1: %2")
        .arg(i)
        .arg(QString::fromStdString(sg->GetHMName()));
    }
    OpenBabel::OBUnitCell *cell = currentCell();
    sg = cell->GetSpaceGroup();
    bool ok;

    QString selection =
      QInputDialog::getItem(this,
                            CB_DIALOG_TITLE,
                            tr("Set Spacegroup:"),
                            spacegroups,
                            (sg) ? sg->GetId() : 0,
                            false,
                            &ok);

    if (!ok) {
      return;
    }
    unsigned int index = spacegroups.indexOf(selection) + 1;
    cell->SetSpaceGroup(index);
    emit cellChanged();
  }

  void CrystalBuilderDialog::actionFillCell()
  {
    fillUnitCell();
  }

  void CrystalBuilderDialog::actionWrapAtoms()
  {
    wrapAtomsToCell();
  }

  void CrystalBuilderDialog::actionCopyToClipboard()
  {
    copyToClipboard();
  }

  void CrystalBuilderDialog::actionNiggliReduce()
  {
    // Warn if there are atoms present
    if (m_molecule->numAtoms()) {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::warning
        (this, CB_DIALOG_TITLE,
         tr("The current document contains atoms, and the algorithm "
            "used to carry out the Niggli reduction algorithm may "
            "not preserve the atomic structure of this document "
            "correctly. Atomic positions should be examined "
            "afterwards to verify correctness.\n\nWould you like "
            "to continue the cell reduction?"),
         QMessageBox::Yes | QMessageBox::No,
         QMessageBox::Yes);
      if (reply == QMessageBox::Yes) {
        return niggliReduce();
      }
      else {
        return;
      }
    }
  }

  void CrystalBuilderDialog::actionScaleToVolume()
  {
    double curvol = currentVolume();

    bool ok;
    double newvol = QInputDialog::getDouble
      (this, CB_DIALOG_TITLE,
       tr("Enter new volume:", "Unit cell volume"),
       curvol, 0, 1e20, 5, &ok);
    if (!ok) {
      return;
    }

    if (newvol == curvol) {
      return;
    }

    setCurrentVolume(newvol);
  }

  void CrystalBuilderDialog::actionUnitsLengthAngstrom()
  {
    setLengthUnit(Angstrom);
  }

  void CrystalBuilderDialog::actionUnitsLengthBohr()
  {
    setLengthUnit(Bohr);
  }

  void CrystalBuilderDialog::actionUnitsAngleDegree()
  {
    setAngleUnit(Degree);
  }

  void CrystalBuilderDialog::actionUnitsAngleRadian()
  {
    setAngleUnit(Radian);
  }

  void CrystalBuilderDialog::actionCoordsCart()
  {
    setCoordsCartFrac(Cartesian);
  }

  void CrystalBuilderDialog::actionCoordsFrac()
  {
    setCoordsCartFrac(Fractional);
  }

  void CrystalBuilderDialog::actionCoordsPreserveCart()
  {
    setCoordsPreserveCartFrac(Cartesian);
  }

  void CrystalBuilderDialog::actionCoordsPreserveFrac()
  {
    setCoordsPreserveCartFrac(Fractional);
  }

  void CrystalBuilderDialog::actionMatrixCart()
  {
    setMatrixCartFrac(Cartesian);
  }

  void CrystalBuilderDialog::actionMatrixFrac()
  {
    setMatrixCartFrac(Fractional);
  }

  void CrystalBuilderDialog::actionMatrixRowVectors()
  {
    setMatrixVectorStyle(RowVectors);
  }

  void CrystalBuilderDialog::actionMatrixColumnVectors()
  {
    setMatrixVectorStyle(ColumnVectors);
  }

  void CrystalBuilderDialog::cacheFractionalCoordinates()
  {
    m_cachedFractionalCoords = currentFractionalCoords();
    m_cachedFractionalIds = currentAtomicSymbols();
  }

  void CrystalBuilderDialog::restoreFractionalCoordinates()
  {
    Q_ASSERT(m_cachedFractionalCoords.size() ==
             m_cachedFractionalIds.size());
    setCurrentFractionalCoords(m_cachedFractionalIds,
                               m_cachedFractionalCoords);
  }

} // End namespace Avogadro
