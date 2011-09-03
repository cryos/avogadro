/**********************************************************************
  crystallographyextension.cpp - Crystal Builder Plugin for Avogadro

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

/**
 * @todo "Don't work with crystals?"
 * @todo Create/display cuts along miller indices
 * @todo Cache list of spacegroups
 * @todo Select spacegroup name style (HM or Hall)
 * @todo CEUnitCellParameter::isValid()
 * @todo Read/Write locks
 * @todo Find portable method to ensure that the cell matrix widget is 3 lines of text high.
 */

#include "crystallographyextension.h"

#include "avospglib.h"
#include "crystalpastedialog.h"
#include "ceundo.h"
#include "obeigenconv.h"
#include "stablecomparison.h"
#include "ui/ceabstracteditor.h"
#include "ui/cecoordinateeditor.h"
#include "ui/cematrixeditor.h"
#include "ui/ceparametereditor.h"
#include "ui/cetranslatewidget.h"

#include <avogadro/atom.h>
#include <avogadro/glwidget.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <Eigen/LU>

#include <QtGui/QClipboard>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTimer>

namespace Avogadro
{

  CrystallographyExtension::CrystallographyExtension(QObject *parent)
    : Extension( parent ),
      m_mainwindow(0),
      m_translateWidget(0),
      m_molecule(0),
      m_displayProperties(false),
      m_latticeProperty(0),
      m_spacegroupProperty(0),
      m_volumeProperty(0),
      m_lengthUnit(Angstrom),
      m_angleUnit(Degree),
      m_coordsCartFrac(Cartesian),
      m_coordsPreserveCartFrac(Fractional),
      m_matrixCartFrac(Cartesian),
      m_matrixVectorStyle(RowVectors),
      m_editorRefreshPending(false)
  {
    if (!m_mainwindow) {
      // HACK: This might have unintended consequences if used in other
      // applications: Find the QMainWindow that owns this
      // extension. If none found, leave m_mainwindow 0.
      QObject *mainwindow = this;
      while (mainwindow && !qobject_cast<QMainWindow*>(mainwindow)) {
        mainwindow = qobject_cast<QWidget*>(mainwindow->parent());
      }
      m_mainwindow = qobject_cast<QMainWindow*>(mainwindow);
    }

    if (!m_mainwindow) {
      qDebug() << "Disabling the Crystallography editors: No"
               << "QMainWindow found.";
      return;
    }

    createActions();
    QSettings settings;
    readSettings(settings);
    refreshActions();

    connect(this, SIGNAL(cellChanged()),
            this, SLOT(repaintMolecule()));

    // Editor refresh
    connect(this, SIGNAL(lengthUnitChanged(LengthUnit)),
            this, SLOT(refreshEditors()));
    connect(this, SIGNAL(angleUnitChanged(AngleUnit)),
            this, SLOT(refreshEditors()));
    connect(this, SIGNAL(coordsCartFracChanged(CartFrac)),
            this, SLOT(refreshEditors()));
    connect(this, SIGNAL(coordsPreserveCartFracChanged(CartFrac)),
            this, SLOT(refreshEditors()));
    connect(this, SIGNAL(matrixCartFracChanged(CartFrac)),
            this, SLOT(refreshEditors()));
    connect(this, SIGNAL(matrixVectorStyleChanged(VectorStyle)),
            this, SLOT(refreshEditors()));

    // Properties refresh
    connect(this, SIGNAL(lengthUnitChanged(LengthUnit)),
            this, SLOT(refreshProperties()));
    connect(this, SIGNAL(angleUnitChanged(AngleUnit)),
            this, SLOT(refreshProperties()));
    connect(this, SIGNAL(coordsCartFracChanged(CartFrac)),
            this, SLOT(refreshProperties()));
    connect(this, SIGNAL(coordsPreserveCartFracChanged(CartFrac)),
            this, SLOT(refreshProperties()));
    connect(this, SIGNAL(matrixCartFracChanged(CartFrac)),
            this, SLOT(refreshProperties()));
    connect(this, SIGNAL(matrixVectorStyleChanged(VectorStyle)),
            this, SLOT(refreshProperties()));

  }

  CrystallographyExtension::~CrystallographyExtension()
  {
    QSettings settings;
    writeSettings(settings);
  }

  QList<QAction *> CrystallographyExtension::actions() const
  {
    return m_actions;
  }

  QString CrystallographyExtension::menuPath (QAction *action) const
  {
    switch (static_cast<ActionIndex>(action->data().toInt())) {
    case PerceiveSpacegroupIndex:
    case SetSpacegroupIndex:
    case FillUnitCellIndex:
    case SymmetrizeCrystalIndex:
      return tr("&Crystallography") + '>' + tr("Space&group");
    case PrimitiveReduceIndex:
    case NiggliReduceIndex:
      return tr("&Crystallography") + '>' + tr("&Reduce");
    case ToggleUnitCellIndex:
    case PasteCrystalIndex:
    case ToggleUnitCellSepIndex:
    case ToggleEditorsIndex:
    case TogglePropertiesIndex:
    case ToggleGUISepIndex:
    case WrapAtomsIndex:
    case TranslateAtomsIndex:
    case OrientStandardIndex:
    case ScaleToVolumeIndex:
    case LooseSepIndex:
    case SettingsMainSep1Index:
      return tr("&Crystallography");
    case UnitsLengthAngstromIndex:
    case UnitsLengthBohrIndex:
    case UnitsLengthNanometerIndex:
    case UnitsLengthPicometerIndex:
      return tr("&Crystallography") + '>' + tr("&Settings") + '>' + tr("&Length Unit");
    case UnitsAngleDegreeIndex:
    case UnitsAngleRadianIndex:
      return tr("&Crystallography") + '>' + tr("&Settings") + '>' + tr("&Angle Unit");
    case CoordsCartIndex:
    case CoordsFracIndex:
      return tr("&Crystallography") + '>' + tr("&Settings") + '>' + tr("&Coordinate Display");
    case CoordsPreserveCartIndex:
    case CoordsPreserveFracIndex:
      return tr("&Crystallography") + '>' + tr("&Settings") + '>' + tr("Coordinate &Preservation");
    case MatrixCartIndex:
    case MatrixFracIndex:
    case MatrixCoordDisplaySep1Index:
    case MatrixRowVectorsIndex:
    case MatrixColumnVectorsIndex:
      return tr("&Crystallography") + '>' + tr("&Settings") + '>' + tr("&Matrix Display");
    default:
      qDebug() << "Unknown action...";
      return "";
    }
  }

  void CrystallographyExtension::setMolecule(Molecule *molecule)
  {
    if (m_molecule) {
      m_molecule->disconnect(this);
    }

    m_molecule = molecule;

    refreshActions();

    if (!m_molecule || !m_molecule->OBUnitCell()) {
      hideEditors();
      hideProperties();
      return;
    }

    GLWidget::current()->setRenderUnitCellAxes(true);

    // Connect molecule
    connect(m_molecule, SIGNAL(moleculeChanged()),
            this, SLOT(refreshEditors()));
    connect(m_molecule, SIGNAL(atomAdded(Atom *)),
            this, SLOT(refreshEditors()));
    connect(m_molecule, SIGNAL(atomUpdated(Atom *)),
            this, SLOT(refreshEditors()));
    connect(m_molecule, SIGNAL(atomRemoved(Atom *)),
            this, SLOT(refreshEditors()));

    refreshEditors();
    refreshProperties();

    // If it's a PDB file, we won't show these by default
    if (m_molecule->numResidues() != 0)
      return;

    showEditors();
    showProperties();
  }

  void CrystallographyExtension::writeSettings(QSettings &settings) const
  {
    settings.beginGroup("crystallographyextension");

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

    settings.endGroup(); // "crystallographyextension"
  }

  void CrystallographyExtension::readSettings(QSettings &settings)
  {
    settings.beginGroup("crystallographyextension");

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
                                     Fractional).toInt()));
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

    settings.endGroup(); // "crystallographyextension"

    refreshActions();
    refreshEditors();
  }

  QUndoCommand* CrystallographyExtension::performAction(QAction *action,
                                                       GLWidget *widget)
  {
    switch (static_cast<ActionIndex>(action->data().toInt())) {
    case PerceiveSpacegroupIndex:
      actionPerceiveSpacegroup();
      break;
    case SetSpacegroupIndex:
      actionSetSpacegroup();
      break;
    case FillUnitCellIndex:
      actionFillUnitCell();
      break;
    case SymmetrizeCrystalIndex:
      actionSymmetrizeCrystal();
      break;
    case ToggleUnitCellIndex:
      actionToggleUnitCell();
      break;
    case PasteCrystalIndex:
      actionPasteCrystal();
      break;
    case ToggleEditorsIndex:
      actionToggleEditors();
      break;
    case TogglePropertiesIndex:
      actionToggleProperties();
      break;
    case WrapAtomsIndex:
      actionWrapAtoms();
      break;
    case TranslateAtomsIndex:
      actionTranslateAtoms(widget);
      break;
    case OrientStandardIndex:
      actionOrientStandard();
      break;
    case PrimitiveReduceIndex:
      actionPrimitiveReduce();
      break;
    case NiggliReduceIndex:
      actionNiggliReduce();
      break;
    case ScaleToVolumeIndex:
      actionScaleToVolume();
      break;
    case UnitsLengthAngstromIndex:
      actionUnitsLengthAngstrom();
      break;
    case UnitsLengthBohrIndex:
      actionUnitsLengthBohr();
      break;
    case UnitsLengthNanometerIndex:
      actionUnitsLengthNanometer();
      break;
    case UnitsLengthPicometerIndex:
      actionUnitsLengthPicometer();
      break;
    case UnitsAngleDegreeIndex:
      actionUnitsAngleDegree();
      break;
    case UnitsAngleRadianIndex:
      actionUnitsAngleRadian();
      break;
    case CoordsCartIndex:
      actionCoordsCart();
      break;
    case CoordsFracIndex:
      actionCoordsFrac();
      break;
    case CoordsPreserveCartIndex:
      actionCoordsPreserveCart();
      break;
    case CoordsPreserveFracIndex:
      actionCoordsPreserveFrac();
      break;
    case MatrixCartIndex:
      actionMatrixCart();
      break;
    case MatrixFracIndex:
      actionMatrixFrac();
      break;
    case MatrixRowVectorsIndex:
      actionMatrixRowVectors();
      break;
    case MatrixColumnVectorsIndex:
      actionMatrixColumnVectors();
      break;
    case SettingsMainSep1Index:
    case MatrixCoordDisplaySep1Index:
      qDebug() << "Cannot perform action on a separator...";
      return 0;
    default:
      return 0;
    }

    return 0;
  }

  void CrystallographyExtension::pushUndo(QUndoCommand *comm)
  {
    GLWidget::current()->undoStack()->push(comm);
  }

  void CrystallographyExtension::initializeEditors()
  {
    if (m_editors.size()) {
      qDebug() << "Editors already initialized.";
      return;
    }
    m_editors.append(new CEParameterEditor(this, m_mainwindow));
    m_editors.append(new CEMatrixEditor(this, m_mainwindow));
    m_editors.append(new CECoordinateEditor(this, m_mainwindow));
    refreshEditors();
    for (QList<CEAbstractEditor*>::const_iterator
           it = m_editors.constBegin(),
           it_end = m_editors.constEnd();
         it != it_end; ++it) {
      m_mainwindow->addDockWidget((*it)->preferredDockWidgetArea(), *it);
      (*it)->setVisible(false);
    }
  }

  void CrystallographyExtension::showEditors()
  {
    if (!m_editors.size()) {
      initializeEditors();
    }
    for (QList<CEAbstractEditor*>::const_iterator
           it = m_editors.constBegin(),
           it_end = m_editors.constEnd();
         it != it_end; ++it) {
      (*it)->show();
    }
    getAction(ToggleEditorsIndex)->setText(tr("Hide &Editors"));
  }

  void CrystallographyExtension::hideEditors()
  {
    if (!m_editors.size()) {
      initializeEditors();
    }
    for (QList<CEAbstractEditor*>::const_iterator
           it = m_editors.constBegin(),
           it_end = m_editors.constEnd();
         it != it_end; ++it) {
      (*it)->hide();
    }
    getAction(ToggleEditorsIndex)->setText(tr("Show &Editors"));
  }


  void CrystallographyExtension::lockEditors()
  {
    // Lock all editors other than the sender
    if (!m_editors.size()) {
      initializeEditors();
    }
    for (QList<CEAbstractEditor*>::iterator
           it = m_editors.begin(),
           it_end = m_editors.end();
         it != it_end; ++it) {
      if (!(*it)->isLocked() && *it != sender()) {
        (*it)->lockEditor();
      }
    }
  }

  void CrystallographyExtension::unlockEditors()
  {
    // Unlock all editors
    if (!m_editors.size()) {
      initializeEditors();
    }
    for (QList<CEAbstractEditor*>::iterator
           it = m_editors.begin(),
           it_end = m_editors.end();
         it != it_end; ++it) {
      (*it)->unlockEditor();
    }
  }

  void CrystallographyExtension::refreshEditors()
  {
    // Limit refreshes
    if (!m_editorRefreshPending) {
      m_editorRefreshPending = true;
      QTimer::singleShot(100, this, SLOT(refreshEditors_()));
    }
  }

  void CrystallographyExtension::refreshEditors_()
  {
    // refresh all editors
    if (!m_editors.size()) {
      initializeEditors();
    }
    m_editorRefreshPending = false;
    for (QList<CEAbstractEditor*>::iterator
           it = m_editors.begin(),
           it_end = m_editors.end();
         it != it_end; ++it) {
      (*it)->refreshEditor();
    }
  }

  void CrystallographyExtension::showProperties()
  {
    if (m_displayProperties) {
      return;
    }
    m_displayProperties = true;

    // Create properties if needed
    if (!m_latticeProperty)
      m_latticeProperty = new QLabel;
    if (!m_spacegroupProperty)
      m_spacegroupProperty = new QLabel;
    if (!m_volumeProperty)
      m_volumeProperty = new QLabel;

    connect(this, SIGNAL(cellChanged()),
            this, SLOT(refreshProperties()));

    getAction(TogglePropertiesIndex)->setText
      (tr("Hide &Property Display"));

    // Set text
    refreshProperties();

    // Create list to ensure that labels are added consecutively
    QList<QLabel*> list;
    list.append(m_latticeProperty);
    list.append(m_spacegroupProperty);
    list.append(m_volumeProperty);

    // Add to GLWidget
    GLWidget::current()->addTextOverlay(list);

    // Trigger render event
    if (m_molecule) {
      m_molecule->update();
    }
  }

  void CrystallographyExtension::hideProperties()
  {
    if (!m_displayProperties) {
      return;
    }

    disconnect(this, SIGNAL(cellChanged()),
               this, SLOT(refreshProperties()));

    m_displayProperties = false;

    getAction(TogglePropertiesIndex)->setText
      (tr("Show &Property Display"));

    delete m_latticeProperty;
    m_latticeProperty = 0;

    delete m_spacegroupProperty;
    m_spacegroupProperty = 0;

    delete m_volumeProperty;
    m_volumeProperty = 0;

    // Trigger render event
    if (m_molecule) {
      m_molecule->update();
    }
  }

  void CrystallographyExtension::refreshProperties()
  {
    if (!m_displayProperties) {
      return;
    }

    Q_ASSERT_X(m_latticeProperty, Q_FUNC_INFO,
               "Property labels must be created before "
               "they can be refreshed");
    Q_ASSERT_X(m_spacegroupProperty, Q_FUNC_INFO,
               "Property labels must be created before "
               "they can be refreshed");
    Q_ASSERT_X(m_volumeProperty, Q_FUNC_INFO,
               "Property labels must be created before "
               "they can be refreshed");

    OpenBabel::OBUnitCell *cell = currentCell();

    // Find spacegroup name
    QString spacegroup = "Unknown";
    if (cell) {
      const OpenBabel::SpaceGroup *sg;
      sg = cell->GetSpaceGroup();
      if (sg) {
        spacegroup = QString("%1 (%2)")
          .arg(sg->GetHMName().c_str())
          .arg(sg->GetId());
      }
    }

    // Build volume unit suffix
    QString volumeSuffix;
    switch (lengthUnit()) {
    case Angstrom:
      volumeSuffix = " " + CE_ANGSTROM + CE_SUPER_THREE;
      break;
    case Bohr:
      volumeSuffix = " a" + CE_SUB_ZERO + CE_SUPER_THREE;
      break;
    case Nanometer:
      volumeSuffix = " nm" + CE_SUPER_THREE;
      break;
    case Picometer:
      volumeSuffix = " pm" + CE_SUPER_THREE;
      break;
    default:
      volumeSuffix = "";
      break;
    }


    // update text
    m_latticeProperty->setText(tr("Lattice Type: %1",
                                  "Unit cell lattice")
                               .arg(currentLatticeType()));
    m_spacegroupProperty->setText
      (tr("Spacegroup: %1").arg(spacegroup));
    m_volumeProperty->setText
      (tr("Unit cell volume: %L1%2")
       .arg(currentVolume(), 0, 'f', 5)
       .arg(volumeSuffix));

    // Trigger render event
    if (m_molecule) {
      m_molecule->update();
    }
  }

  void CrystallographyExtension::refreshActions()
  {
    // Unit cell toggle:
    bool hasCell = static_cast<bool>(currentCell());
    if (hasCell) {
      getAction(ToggleUnitCellIndex)->setText
        (tr("Remove &Unit Cell"));
    }
    else {
      getAction(ToggleUnitCellIndex)->setText
        (tr("Add &Unit Cell"));
    }

    // If there is a cell, enable all the actions. Disable them below
    // if needed, but not before actions are disabled when !hasCell
    if (hasCell) {
      for (QList<QAction*>::const_iterator
             it = m_actions.constBegin(),
             it_end = m_actions.constEnd();
           it != it_end; ++it) {
        (*it)->setEnabled(true);
      }
    }

    // Editors toggle:
    bool editorsVisible = false;
    if (hasCell && !m_editors.size()) {
      initializeEditors();
    }

    for (QList<CEAbstractEditor*>::const_iterator
           it = m_editors.constBegin(),
           it_end = m_editors.constEnd();
         it != it_end; ++it) {
      if ((*it)->isVisible()) {
        editorsVisible = true;
        break;
      }
    }

    if (editorsVisible) {
      getAction(ToggleEditorsIndex)->setText
        (tr("Hide &Editors"));
    }
    else {
      getAction(ToggleEditorsIndex)->setText
        (tr("Show &Editors"));
    }

    if (m_displayProperties) {
      getAction(TogglePropertiesIndex)->setText
        (tr("Hide &Property Display"));
    }
    else {
      getAction(TogglePropertiesIndex)->setText
        (tr("Show &Property Display"));
    }


    // Settings checkgroups
    switch (lengthUnit()) {
    case Angstrom:
      getAction(UnitsLengthAngstromIndex)->setChecked(true);
      break;
    case Bohr:
      getAction(UnitsLengthBohrIndex)->setChecked(true);
      break;
    case Nanometer:
      getAction(UnitsLengthNanometerIndex)->setChecked(true);
      break;
    case Picometer:
      getAction(UnitsLengthPicometerIndex)->setChecked(true);
      break;
    default:
      break;
    }

    switch (angleUnit()) {
    case Degree:
      getAction(UnitsAngleDegreeIndex)->setChecked(true);
      getAction(UnitsAngleRadianIndex)->setChecked(false);
      break;
    case Radian:
      getAction(UnitsAngleDegreeIndex)->setChecked(false);
      getAction(UnitsAngleRadianIndex)->setChecked(true);
      break;
    default:
      break;
    }

    switch (coordsCartFrac()) {
    case Cartesian:
      getAction(CoordsCartIndex)->setChecked(true);
      getAction(CoordsFracIndex)->setChecked(false);
      break;
    case Fractional:
      getAction(CoordsCartIndex)->setChecked(false);
      getAction(CoordsFracIndex)->setChecked(true);
      break;
    default:
      break;
    }

    switch (coordsPreserveCartFrac()) {
    case Cartesian:
      getAction(CoordsPreserveCartIndex)->setChecked(true);
      getAction(CoordsPreserveFracIndex)->setChecked(false);
      break;
    case Fractional:
      getAction(CoordsPreserveCartIndex)->setChecked(false);
      getAction(CoordsPreserveFracIndex)->setChecked(true);
      break;
    default:
      break;
    }

    switch (matrixCartFrac()) {
    case Cartesian:
      getAction(MatrixCartIndex)->setChecked(true);
      getAction(MatrixFracIndex)->setChecked(false);
      break;
    case Fractional:
      getAction(MatrixCartIndex)->setChecked(false);
      getAction(MatrixFracIndex)->setChecked(true);
      break;
    default:
      break;
    }

    switch (matrixVectorStyle()) {
    case RowVectors:
      getAction(MatrixRowVectorsIndex)->setChecked(true);
      getAction(MatrixColumnVectorsIndex)->setChecked(false);
      break;
    case ColumnVectors:
      getAction(MatrixRowVectorsIndex)->setChecked(false);
      getAction(MatrixColumnVectorsIndex)->setChecked(true);
      break;
    default:
      break;
    }

    // If there is not a cell, disable all the actions. Overrides can
    // be added in this loop.
    if (!hasCell) {
      for (QList<QAction*>::const_iterator
             it = m_actions.constBegin(),
             it_end = m_actions.constEnd();
           it != it_end; ++it) {
        if ((*it)->data().toInt() == ToggleUnitCellIndex ||
            (*it)->data().toInt() == PasteCrystalIndex) {
          continue;
        }
        (*it)->setEnabled(false);
      }
    }

  }

  double CrystallographyExtension::convertLength(double length) const
  {
    return length * lengthConversionFactor();
  }

  Eigen::Vector3d  CrystallographyExtension::convertLength
  (const Eigen::Vector3d& length) const
  {
    return length * lengthConversionFactor();
  }

  Eigen::Matrix3d CrystallographyExtension::convertLength
  (const Eigen::Matrix3d& length) const
  {
    return length * lengthConversionFactor();
  }

  double CrystallographyExtension::convertAngle(double angle) const
  {
    return angle * angleConversionFactor();
  }

  //  display -> storage
  double CrystallographyExtension::unconvertLength(double length) const
  {
    return length * (1.0 / lengthConversionFactor());
  }

  Eigen::Vector3d CrystallographyExtension::unconvertLength
  (const Eigen::Vector3d& length) const
  {
    return length * (1.0 / lengthConversionFactor());
  }

  Eigen::Matrix3d CrystallographyExtension::unconvertLength
  (const Eigen::Matrix3d& length) const
  {
    return length * (1.0 / lengthConversionFactor());
  }

  double CrystallographyExtension::unconvertAngle(double angle) const
  {
    return angle * (1.0 / angleConversionFactor());
  }

  Eigen::Matrix3d CrystallographyExtension::currentCellMatrix() const
  {
    if (!currentCell()) {
      return Eigen::Matrix3d::Zero();
    }

    return convertLength(OB2Eigen(currentCell()->GetCellMatrix()));
  }

  Eigen::Matrix3d CrystallographyExtension::currentFractionalMatrix() const
  {
    if (!currentCell()) {
      return Eigen::Matrix3d::Zero();
    }

    return OB2Eigen(currentCell()->GetFractionalMatrix());
  }

  CEUnitCellParameters CrystallographyExtension::currentCellParameters() const
  {
    CEUnitCellParameters params (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
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
  CrystallographyExtension::currentCartesianCoords() const
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
  CrystallographyExtension::currentFractionalCoords() const
  {
    OpenBabel::OBUnitCell *cell = currentCell();
    if (!cell) {
      return QList<Eigen::Vector3d>();
    }

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

  QList<int> CrystallographyExtension::currentAtomicNumbers() const
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

  QList<QString> CrystallographyExtension::currentAtomicSymbols() const
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

  QString CrystallographyExtension::currentLatticeType() const
  {
    OpenBabel::OBUnitCell *cell = currentCell();

    if (!cell) {
      return tr("Undefined");
    }

    switch (cell->GetLatticeType()) {
    default:
    case OpenBabel::OBUnitCell::Undefined:
      return tr("Undefined");
    case OpenBabel::OBUnitCell::Triclinic:
      return tr("Triclinic", "Lattice type");
    case OpenBabel::OBUnitCell::Monoclinic:
      return tr("Monoclinic", "Lattice type");
    case OpenBabel::OBUnitCell::Orthorhombic:
      return tr("Orthorhombic", "Lattice type");
    case OpenBabel::OBUnitCell::Tetragonal:
      return tr("Tetragonal", "Lattice type");
    case OpenBabel::OBUnitCell::Rhombohedral:
      return tr("Rhombohedral", "Lattice type");
    case OpenBabel::OBUnitCell::Hexagonal:
      return tr("Hexagonal", "Lattice type");
    case OpenBabel::OBUnitCell::Cubic:
      return tr("Cubic", "Lattice type");
    }
  }

  double CrystallographyExtension::currentVolume() const
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

  void CrystallographyExtension::setCurrentCell
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

  void CrystallographyExtension::setCurrentCellMatrix
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

  void CrystallographyExtension::setCurrentCellParameters
  (const CEUnitCellParameters &params)
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

  void CrystallographyExtension::setCurrentFractionalCoords
  (const QList<QString> &ids,
   const QList<Eigen::Vector3d> &fcoords)
  {
    OpenBabel::OBUnitCell *cell = currentCell();
    QList<Eigen::Vector3d> coords;
#if QT_VERSION >= 0x040700
    coords.reserve(fcoords.size());
#endif

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

  void CrystallographyExtension::setCurrentCartesianCoords
  (const QList<QString> &ids,
   const QList<Eigen::Vector3d> &coords)
  {
    Q_ASSERT(ids.size() == coords.size());

    QList<Eigen::Vector3d> scoords;
#if QT_VERSION >= 0x040700
    scoords.reserve(coords.size());
#endif

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

  void CrystallographyExtension::setCurrentVolume(double volume)
  {
    // Get scaling factor
    double factor = pow(volume/currentVolume(), 1.0/3.0);

    CEUnitCellParameters p = currentCellParameters();
    p.a *= factor;
    p.b *= factor;
    p.c *= factor;
    setCurrentCellParameters(p);
  }

  // Adapted from unitcellextension:
  void CrystallographyExtension::fillUnitCell()
  {
    OpenBabel::OBUnitCell *cell = currentCell();
    Q_ASSERT(cell);
    const OpenBabel::SpaceGroup *sg = cell->GetSpaceGroup();
    Q_ASSERT(sg);

    wrapAtomsToCell();

    QList<Eigen::Vector3d> origFCoords =
      currentFractionalCoords();
    QList<Eigen::Vector3d> newFCoords;

    QList<QString> origIds = currentAtomicSymbols();
    QList<QString> newIds;

    // Duplicate tolerance squared
    const double dupTolSquared = 1e-8; // 1e-4 ^2

    // Non-fatal assert -- if the number of atoms has
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

  void CrystallographyExtension::wrapAtomsToCell()
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

  void CrystallographyExtension::orientStandard()
  {
    // Let's be lazy here; Just pull out the parameters and reapply them.
    setCurrentCellParameters(currentCellParameters());
  }

  void CrystallographyExtension::showPasteDialog(const QString &text)
  {
    // Create cell if none exists.
    bool hasCell = static_cast<bool>(currentCell());
    if (!hasCell) {
      actionToggleUnitCell();
    }

    CEUndoState before (this);

    // Dialog will handle changing m_molecule as needed.
    CEPasteDialog d (m_mainwindow, text, m_molecule);
    if (!d.formatIsValid()) {
      // Error message has already informed user of error, so we can
      // just return now.
      // Skip undo, remove unit cell if we had to add one.
      if (!hasCell) {
        actionToggleUnitCell();
      }
      return;
    }
    if (d.exec() != QDialog::Accepted) {
      // Skip undo, remove unit cell if we had to add one.
      if (!hasCell) {
        actionToggleUnitCell();
      }
      return;
    }

    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Paste Crystal")));
    emit cellChanged();
  }

  /// @todo Expose this to the GUI, might be useful at some point. Add
  /// a plot widget for extra fancy-ness.
  ///
  /// @todo Clean this up, it has some performance issues.
  void getIADHistogram(Molecule *mol,
                       OpenBabel::OBUnitCell *cell,
                       QList<double> * distance,
                       QList<double> * frequency,
                       double min, double max, double step,
                       Atom *atom = 0)
  {
    Q_ASSERT(min < max);
    Q_ASSERT(step > 0);

    double halfstep = step/2.0;

    // Populate distance list
    distance->clear();
    frequency->clear();
    for (double val = min; val < max; val += step) {
      distance->append(val);
      frequency->append(0);
    }

    QList<Atom*> atomList = mol->atoms();
    int numAtoms = atomList.size();
    if (!numAtoms) {
      return;
    }

    QList<Eigen::Vector3d> atomPositions;
    for (int i = 0; i < numAtoms; i++)
      atomPositions.push_back(*(atomList.at(i)->pos()));

    // Initialize vars
    //  Atomic Positions
    Eigen::Vector3d v1;
    Eigen::Vector3d v2;
    //  Unit Cell Vectors
    //  First get OB matrix, extract vectors, then convert to Eigen::Vector3d's
    Eigen::Matrix3d cellMatrix = OB2Eigen(cell->GetCellMatrix());
    Eigen::Vector3d u1 (cellMatrix.row(0));
    Eigen::Vector3d u2 (cellMatrix.row(1));
    Eigen::Vector3d u3 (cellMatrix.row(2));
    //  Find all combinations of unit cell vectors to get 3x3 cell
    QList<Eigen::Vector3d> uVecs;
    int s_1, s_2, s_3; // will be -1, 0, +1 multipliers
    for (s_1 = -1; s_1 <= 1; s_1++) {
      for (s_2 = -1; s_2 <= 1; s_2++) {
        for (s_3 = -1; s_3 <= 1; s_3++) {
          uVecs.append(s_1*u1 + s_2*u2 + s_3*u3);
        }
      }
    }

    // build histogram
    double diff;
    // Loop over all atoms
    if (atom == 0) {
      for (int i = 0; i < atomList.size(); i++) {
        v1 = atomPositions.at(i);
        for (int j = i+1; j < atomList.size(); j++) {
          v2 = atomPositions.at(j);
          // Intercell
          diff = fabs((v1-v2).norm());
          for (int k = 0; k < distance->size(); k++) {
            if (fabs(diff - distance->at(k)) < halfstep) {
              (*frequency)[k]++;
            }
          }
          // Intracell
          for (int vecInd = 0; vecInd < uVecs.size(); vecInd++) {
            diff = fabs(((v1+uVecs.at(vecInd))-v2).norm());
            for (int k = 0; k < distance->size(); k++) {
              if (fabs(diff - distance->at(k)) < halfstep) {
                (*frequency)[k]++;
              }
            }
          }
        }
      }
    }
    // Or, just the one requested
    else {
      v1 = *atom->pos();
      for (int j = 0; j < atomList.size(); j++) {
        v2 = atomPositions.at(j);
        // Intercell
        diff = fabs((v1-v2).norm());
        for (int k = 0; k < distance->size(); k++) {
          if (diff != 0 && fabs(diff - distance->at(k)) < halfstep) {
            (*frequency)[k]++;
          }
        }
        // Intracell
        for (int vecInd = 0; vecInd < uVecs.size(); vecInd++) {
          diff = fabs(((v1+uVecs.at(vecInd))-v2).norm());
          for (int k = 0; k < distance->size(); k++) {
            if (fabs(diff - distance->at(k)) < halfstep) {
              (*frequency)[k]++;
            }
          }
        }
      }
    }

    return;
  }

  // Implements the niggli reduction algorithm detailed in:
  // Grosse-Kunstleve RW, Sauter NK, Adams PD. Numerically stable
  // algorithms for the computation of reduced unit cells. Acta
  // Crystallographica Section A Foundations of
  // Crystallography. 2003;60(1):1-6.
  bool CrystallographyExtension::niggliReduce()
  {
    // Set maximum number of iterations
    const unsigned int iterations = 1000;

    // Get current unit cell
    OpenBabel::OBUnitCell *cell = currentCell();

    // For sanity checks:
    double origVolume = currentVolume();

    // Cache the current fractional coordinates for later.
    QList<Eigen::Vector3d> fcoords = currentFractionalCoords();

    // Get cell parameters in storage units, convert deg->rad
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

    // Initialize change of basis matrices:
    //
    // Although the reduction algorithm produces quantities directly
    // relatible to a,b,c,alpha,beta,gamma, we will calculate a change
    // of basis matrix to use instead, and discard A, B, C, xi, eta,
    // zeta. By multiplying the change of basis matrix against the
    // current cell matrix, we avoid the problem of handling the
    // orientation matrix already present in the cell. The inverse of
    // this matrix can also be used later to convert the atomic
    // positions.
    // tmpMat is used to build other matrices
    Eigen::Matrix3d tmpMat;

    // Cache static matrices:

    // Swap x,y (Used in Step 1). Negatives ensure proper sign of final
    // determinant.
    tmpMat << 0,-1,0, -1,0,0, 0,0,-1;
    const Eigen::Matrix3d C1(tmpMat);
    // Swap y,z (Used in Step 2). Negatives ensure proper sign of final
    // determinant
    tmpMat << -1,0,0, 0,0,-1, 0,-1,0;
    const Eigen::Matrix3d C2(tmpMat);
    // For step 8:
    tmpMat << 1,0,1, 0,1,1, 0,0,1;
    const Eigen::Matrix3d C8(tmpMat);

    // initial change of basis matrix
    tmpMat << 1,0,0, 0,1,0, 0,0,1;
    Eigen::Matrix3d cob(tmpMat);

    // Enable debugging output here:
//#define NIGGLI_DEBUG(step) qDebug() << iter << step << A << B << C << xi << eta << zeta;
#define NIGGLI_DEBUG(step)
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
        cob *= C1;
        qSwap(A, B);
        qSwap(xi, eta);
        NIGGLI_DEBUG(1);
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
        cob *= C2;
        qSwap(B, C);
        qSwap(eta, zeta);
        NIGGLI_DEBUG(2);
        continue;
      }

      // Step 3:
      // Use exact comparisons in steps 3 and 4.
      if (xi*eta*zeta > 0) {
        // Update change of basis matrix:
        tmpMat <<
          StableComp::sign(xi),0,0,
          0,StableComp::sign(eta),0,
          0,0,StableComp::sign(zeta);
        cob *= tmpMat;

        // Update characteristic
        xi   = fabs(xi);
        eta  = fabs(eta);
        zeta = fabs(zeta);
        NIGGLI_DEBUG(3);
        ++iter;
      }

      // Step 4:
      // Use exact comparisons for steps 3 and 4
      else { // either step 3 or 4 should run
        // Update change of basis matrix:
        double *p = NULL;
        double i = 1;
        double j = 1;
        double k = 1;
        if (xi > 0) {
          i = -1;
        }
        else if (!(xi < 0)) {
          p = &i;
        }
        if (eta > 0) {
          j = -1;
        }
        else if (!(eta < 0)) {
          p = &j;
        }
        if (zeta > 0) {
          k = -1;
        }
        else if (!(zeta < 0)) {
          p = &k;
        }
        if (i*j*k < 0) {
          if (!p) {
            QMessageBox::warning
                (m_mainwindow,
                 CE_DIALOG_TITLE,
                 tr("Niggli-reduction failed. The input structure's "
                    "lattice that is confusing the Niggli-reduction "
                    "algorithm. Try making a small perturbation (approx."
                    " 2 orders of magnitude smaller than the tolerance) "
                    "to the input lattices and try again."));
            return false;
          }
          *p = -1;
        }
        tmpMat << i,0,0, 0,j,0, 0,0,k;
        cob *= tmpMat;

        // Update characteristic
        xi   = -fabs(xi);
        eta  = -fabs(eta);
        zeta = -fabs(zeta);
        NIGGLI_DEBUG(4);
        ++iter;
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
        // Update change of basis matrix:
        tmpMat << 1,0,0, 0,1,-signXi, 0,0,1;
        cob *= tmpMat;

        // Update characteristic
        C    = B + C - xi*signXi;
        eta  = eta - zeta*signXi;
        xi   = xi -   2*B*signXi;
        NIGGLI_DEBUG(5);
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
        // Update change of basis matrix:
        tmpMat << 1,0,-signEta, 0,1,0, 0,0,1;
        cob *= tmpMat;

        // Update characteristic
        C    = A + C - eta*signEta;
        xi   = xi - zeta*signEta;
        eta  = eta - 2*A*signEta;
        NIGGLI_DEBUG(6);
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
        // Update change of basis matrix:
        tmpMat << 1,-signZeta,0, 0,1,0, 0,0,1;
        cob *= tmpMat;

        // Update characteristic
        B    = A + B - zeta*signZeta;
        xi   = xi - eta*signZeta;
        zeta = zeta - 2*A*signZeta;
        NIGGLI_DEBUG(7);
        continue;
      }

      // Step 8:
      double sumAllButC = A + B + xi + eta + zeta;
      if (StableComp::lt(sumAllButC, 0, tol)
          || (StableComp::eq(sumAllButC, 0, tol)
              && StableComp::gt(2*(A+eta)+zeta, 0, tol)
              )
          ) {
        // Update change of basis matrix:
        cob *= C8;

        // Update characteristic
        C    = sumAllButC + C;
        xi = 2*B + xi + zeta;
        eta  = 2*A + eta + zeta;
        NIGGLI_DEBUG(8);
        continue;
      }

      // Done!
      ret = true;
      break;
    }

    // No change
    if (iter == 0) {
      QMessageBox::information
        (m_mainwindow,
         CE_DIALOG_TITLE,
         tr("This unit cell is already reduced to "
            "its canonical Niggli representation."));
      return false;
    }

    if (!ret) {
      QMessageBox::warning
        (m_mainwindow,
         CE_DIALOG_TITLE,
         tr("Failed to reduce cell after 1000 iterations of "
            "the reduction algorithm. Stopping."));
      return false;
    }

    Q_ASSERT_X(cob.determinant() == 1, Q_FUNC_INFO,
               "Determinant of change of basis matrix must be 1.");

    // Update cell
    setCurrentCellMatrix(cob.transpose() * currentCellMatrix());

    Q_ASSERT_X(StableComp::eq(origVolume, currentVolume(), tol),
               Q_FUNC_INFO, "Cell volume changed during Niggli reduction.");

    // fix coordinates
    // Apply COB matrix:
    Eigen::Matrix3d invCob;
    cob.computeInverse(&invCob);
    for (QList<Eigen::Vector3d>::iterator
           it = fcoords.begin(),
           it_end = fcoords.end();
         it != it_end; ++it) {
      *it = invCob * (*it);
    }
    setCurrentFractionalCoords(currentAtomicSymbols(), fcoords);

    // wrap:
    wrapAtomsToCell();
    orientStandard();
    return true;
  }

  void CrystallographyExtension::createActions()
  {

#define CE_CACTION_ASSERT(index)                \
    Q_ASSERT(m_actions.size()-1 == index);      \
    Q_ASSERT(index == counter);

#define CE_CACTION_DEBUG(index)
    /*
#define CE_CACTION_DEBUG(index)               \
     qDebug() << "Ind:" << m_actions.size()-1 \
              << "Enum:" << index             \
              << "Counter:" << counter;
    */
    // Create in order of ActionIndex enum:
    QAction *a;
    QActionGroup *ag;
    // This is used to set the QAction data. It is the action's index
    // into m_actions and ActionIndex
    unsigned int counter = static_cast<unsigned int>(-1);

    // ToggleUnitCellIndex
    a = new QAction(tr("Add &Unit Cell"), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(ToggleUnitCellIndex);
    CE_CACTION_ASSERT(ToggleUnitCellIndex);

    // PasteCrystalIndex
    a = new QAction(tr("I&mport Crystal from Clipboard..."), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(PasteCrystalIndex);
    CE_CACTION_ASSERT(PasteCrystalIndex);

    // ToggleUnitCellSepIndex
    a = new QAction(this);
    a->setSeparator(true);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(ToggleUnitCellSepIndex);
    CE_CACTION_ASSERT(ToggleUnitCellSepIndex);

    // ToggleEditorsIndex
    a = new QAction(tr("Show &Editors"), this);
    a->setData(++counter);
    /// @todo figure out a shortcut key for this
    m_actions.append(a);
    CE_CACTION_DEBUG(ToggleEditorsIndex);
    CE_CACTION_ASSERT(ToggleEditorsIndex);

    // TogglePropertiesIndex
    a = new QAction(tr("Show &Property Display"), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(TogglePropertiesIndex);
    CE_CACTION_ASSERT(TogglePropertiesIndex);

    // ToggleGUISepIndex
    a = new QAction(this);
    a->setSeparator(true);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(ToggleGUISepIndex);
    CE_CACTION_ASSERT(ToggleGUISepIndex);

    // WrapAtomsIndex
    a = new QAction(tr("&Wrap Atoms to Cell"), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(WrapAtomsIndex);
    CE_CACTION_ASSERT(WrapAtomsIndex);

    // TranslateAtomsIndex
    a = new QAction(tr("&Translate Atoms..."), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(TranslateAtomsIndex);
    CE_CACTION_ASSERT(TranslateAtomsIndex);

    // OrientStandardIndex
    a = new QAction(tr("Rotate To Standard &Orientation"), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(OrientStandardIndex);
    CE_CACTION_ASSERT(OrientStandardIndex);

    // ScaleToVolumeIndex,
    a = new QAction(tr("Scale Cell To &Volume..."), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(ScaleToVolumeIndex);
    CE_CACTION_ASSERT(ScaleToVolumeIndex);

    // LooseSepIndex
    a = new QAction(this);
    a->setSeparator(true);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(LooseSepIndex);
    CE_CACTION_ASSERT(LooseSepIndex);

    // PerceiveSpacegroupIndex
    a = new QAction(tr("Perceive Space&group..."), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(PerceiveSpacegroupIndex);
    CE_CACTION_ASSERT(PerceiveSpacegroupIndex);

    // SetSpacegroupIndex
    a = new QAction(tr("Set &Spacegroup..."), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(SetSpacegroupIndex);
    CE_CACTION_ASSERT(SetSpacegroupIndex);

    // FillUnitCellIndex
    a = new QAction(tr("&Fill Unit Cell"), this);
    a->setStatusTip(tr("Fill the unit cell using current spacegroup."));
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(FillUnitCellIndex);
    CE_CACTION_ASSERT(FillUnitCellIndex);

    // SymmetrizeCrystalIndex
    a = new QAction(tr("Sy&mmetrize Crystal"), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(SymmetrizeCrystalIndex);
    CE_CACTION_ASSERT(SymmetrizeCrystalIndex);

    // PrimitiveReduceIndex,
    a = new QAction(tr("Reduce Cell (&Primitive)"), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(PrimitiveReduceIndex);
    CE_CACTION_ASSERT(PrimitiveReduceIndex);

    // NiggliReduceIndex,
    a = new QAction(tr("Reduce Cell (&Niggli)"), this);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(NiggliReduceIndex);
    CE_CACTION_ASSERT(NiggliReduceIndex);

    // SettingsMainSep1Index,
    a = new QAction(this);
    a->setSeparator(true);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(SettingsMainSep1Index);
    CE_CACTION_ASSERT(SettingsMainSep1Index);

    ///////////////////////////////////
    // UnitsLength action group:
    ag = new QActionGroup(this);
    // UnitsLengthAngstromIndex,
    a = new QAction(tr("&Angstrom"), this);
    a->setCheckable(true);
    a->setChecked(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(UnitsLengthAngstromIndex);
    CE_CACTION_ASSERT(UnitsLengthAngstromIndex);

    // UnitsLengthBohrIndex,
    a = new QAction(tr("&Bohr"), this);
    a->setCheckable(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(UnitsLengthBohrIndex);
    CE_CACTION_ASSERT(UnitsLengthBohrIndex);

    // UnitsLengthNanometerIndex,
    a = new QAction(tr("&Nanometer"), this);
    a->setCheckable(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(UnitsLengthNanometerIndex);
    CE_CACTION_ASSERT(UnitsLengthNanometerIndex);

    // UnitsLengthPicometerIndex,
    a = new QAction(tr("&Picometer"), this);
    a->setCheckable(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(UnitsLengthPicometerIndex);
    CE_CACTION_ASSERT(UnitsLengthPicometerIndex);

    ///////////////////////////////////
    // UnitsAngle action group:
    ag = new QActionGroup(this);
    // UnitsAngleDegreeIndex,
    a = new QAction(tr("&Degree"), this);
    a->setCheckable(true);
    a->setChecked(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(UnitsAngleDegreeIndex);
    CE_CACTION_ASSERT(UnitsAngleDegreeIndex);

    // UnitsAngleRadianIndex,
    a = new QAction(tr("&Radian"), this);
    a->setCheckable(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(UnitsAngleRadianIndex);
    CE_CACTION_ASSERT(UnitsAngleRadianIndex);

    ///////////////////////////////////
    // CoordsCartFrac action group:
    ag = new QActionGroup(this);
    // CoordsCartIndex,
    a = new QAction(tr("Display &cartesian coordinates"), this);
    a->setCheckable(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(CoordsCartIndex);
    CE_CACTION_ASSERT(CoordsCartIndex);

    // CoordsFracIndex,
    a = new QAction(tr("Display &fractional coordinates"), this);
    a->setCheckable(true);
    a->setChecked(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(CoordsFracIndex);
    CE_CACTION_ASSERT(CoordsFracIndex);

    ///////////////////////////////////
    // CoordsPreserveCartFrac action group:
    ag = new QActionGroup(this);
    // CoordsPreserveCartIndex,
    a = new QAction(tr("Preserve &Cartesian Coordinates During Cell "
                       "Modification"), this);
    a->setCheckable(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(CoordsPreserveCartIndex);
    CE_CACTION_ASSERT(CoordsPreserveCartIndex);

    // CoordsPreserveFracIndex,
    a = new QAction(tr("Preserve &Fractional Coordinates During Cell "
                       "Modification"), this);
    a->setCheckable(true);
    a->setChecked(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(CoordsPreserveFracIndex);
    CE_CACTION_ASSERT(CoordsPreserveFracIndex);

    ///////////////////////////////////
    // MatrixCartFrac action group:
    ag = new QActionGroup(this);
    // MatrixCartIndex,
    a =  new QAction(tr("Display &cartesian matrix"), this);
    a->setCheckable(true);
    a->setChecked(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(MatrixCartIndex);
    CE_CACTION_ASSERT(MatrixCartIndex);

    // MatrixFracIndex,
    a = new QAction(tr("Display &fractional matrix"), this);
    a->setCheckable(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(MatrixFracIndex);
    CE_CACTION_ASSERT(MatrixFracIndex);

    // MatrixCoordDisplaySep1Index,
    a = new QAction(this);
    a->setSeparator(true);
    a->setData(++counter);
    m_actions.append(a);
    CE_CACTION_DEBUG(MatrixCoordDisplaySep1Index);
    CE_CACTION_ASSERT(MatrixCoordDisplaySep1Index);

    ///////////////////////////////////
    // MatrixVectorStyle action group:
    ag = new QActionGroup(this);
    // MatrixRowVectorsIndex,
    a = new QAction(tr("Display as &row vectors"), this);
    a->setCheckable(true);
    a->setChecked(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(MatrixRowVectorsIndex);
    CE_CACTION_ASSERT(MatrixRowVectorsIndex);

    // MatrixColumnVectorsIndex
    a = new QAction(tr("Display as &column vectors"), this);
    a->setCheckable(true);
    a->setData(++counter);
    m_actions.append(a);
    ag->addAction(a);
    CE_CACTION_DEBUG(MatrixColumnVectorsIndex);
    CE_CACTION_ASSERT(MatrixColumnVectorsIndex);
  }

  void CrystallographyExtension::actionPerceiveSpacegroup()
  {
    QSettings settings;

    double tol = settings.value
      ("crystallographyextension/settings/spgTolAngstrom",
       0.1).toDouble();

    bool ok;
    tol = unconvertLength(QInputDialog::getDouble
                          (m_mainwindow,
                           CE_DIALOG_TITLE,
                           tr("Select tolerance in current cartesian units:"),
                           convertLength(tol), // initial
                           convertLength(1e-5), // min
                           convertLength(0.5), // max
                           5, &ok));

    if (!ok) {
      return;
    }

    settings.setValue
      ("crystallographyextension/settings/spgTolAngstrom", tol);
    settings.sync();

    OpenBabel::OBUnitCell *cell = currentCell();

    unsigned int spg = Spglib::getSpacegroup(m_molecule,
                                             cell, tol);

    if (spg == 0) {
      if (QMessageBox::question
          (m_mainwindow, CE_DIALOG_TITLE,
           tr("Spacegroup perception failed.\n\nWould you "
              "like to try again with a different tolerance?"),
           QMessageBox::Yes | QMessageBox::No,
           QMessageBox::Yes)
          == QMessageBox::Yes) {
        return actionPerceiveSpacegroup();
      }
      else {
        return;
      }
    }

    CEUndoState before (this);
    cell->SetSpaceGroup(spg);
    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Perceive Spacegroup")));
    emit cellChanged();
  }

  void CrystallographyExtension::actionSetSpacegroup()
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

    unsigned int spg;
    // Try to perceive the current group w/ default tolerance if no
    // spacegroup already set.
    sg = cell->GetSpaceGroup();
    if (!sg) {
      spg = Spglib::getSpacegroup(m_molecule,
                                  currentCell());
    }
    // Otherwise use current sg as default
    else {
      spg = sg->GetId();
    }

    bool ok;

    QString selection =
      QInputDialog::getItem(m_mainwindow,
                            CE_DIALOG_TITLE,
                            tr("Set Spacegroup:"),
                            spacegroups,
                            spg-1,
                            false,
                            &ok);

    if (!ok) {
      return;
    }
    unsigned int index = spacegroups.indexOf(selection);
    CEUndoState before (this);
    cell->SetSpaceGroup(index+1);
    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Set Spacegroup")));
    emit cellChanged();
  }

  void CrystallographyExtension::actionFillUnitCell()
  {
    OpenBabel::OBUnitCell *cell = currentCell();

    const OpenBabel::SpaceGroup *sg = cell->GetSpaceGroup();
    if (!sg) {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::information
        (m_mainwindow, CE_DIALOG_TITLE,
         tr("There is not a spacegroup set for this "
            "document.\n\nWould you like to set a "
            "spacegroup now?"),
         QMessageBox::Yes | QMessageBox::No,
         QMessageBox::Yes);
      if (reply == QMessageBox::Yes) {
        actionSetSpacegroup();
        // Tail recurse to restart
        return actionFillUnitCell();
      }
      else {
        return;
      }
    }

    CEUndoState before (this);
    fillUnitCell();
    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Fill Unit Cell")));
  }

  void CrystallographyExtension::actionSymmetrizeCrystal()
  {
    QSettings settings;
    double tol = settings.value
      ("crystallographyextension/settings/spgTolAngstrom",
       0.1).toDouble();

    bool ok;
    tol = unconvertLength(QInputDialog::getDouble
                          (m_mainwindow,
                           CE_DIALOG_TITLE,
                           tr("Select tolerance in current cartesian units:"),
                           convertLength(tol), // initial
                           convertLength(1e-5), // min
                           convertLength(0.5), // max
                           5, &ok));

    if (!ok) {
      return;
    }

    settings.setValue
      ("crystallographyextension/settings/spgTolAngstrom", tol);
    settings.sync();

    CEUndoState before (this);
    unsigned int spg = Spglib::refineCrystal(m_molecule, 0, tol);
    // spg == 0: Spacegroup perception failed
    if (spg == 0) {
      if (QMessageBox::question
          (m_mainwindow, CE_DIALOG_TITLE,
           tr("Spacegroup perception failed.\n\nWould you "
              "like to try again with a different tolerance?"),
           QMessageBox::Yes | QMessageBox::No,
           QMessageBox::Yes)
          == QMessageBox::Yes) {
        return actionSymmetrizeCrystal();
      }
      else {
        return;
      }
    }
    else if (spg == 1) {
      if (QMessageBox::question
          (m_mainwindow, CE_DIALOG_TITLE,
           tr("Spacegroup P1 detected -- cannot symmetrize to this "
              "spacegroup.\n\nWould you like to try again with a "
              "different tolerance?"),
           QMessageBox::Yes | QMessageBox::No,
           QMessageBox::Yes)
          == QMessageBox::Yes) {
        return actionSymmetrizeCrystal();
      }
      else {
        return;
      }
    }

    wrapAtomsToCell();
    orientStandard();
    currentCell()->SetSpaceGroup(spg);

    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Symmetrize Crystal")));

    emit cellChanged();
  }

  void CrystallographyExtension::actionToggleUnitCell()
  {
    bool hasCell = static_cast<bool>(currentCell());

    if (!hasCell) {
      OpenBabel::OBUnitCell *cell
        = new OpenBabel::OBUnitCell;
      cell->SetData(3.0, 3.0, 3.0,
                    90.0, 90.0, 90.0);

      pushUndo(new CEAddCellUndoCommand(m_molecule, cell, this));
      cell = 0; // Undo constructor takes ownership of cell.
      emit cellChanged();
      showEditors();
      GLWidget::current()->setRenderUnitCellAxes(true);
      refreshActions();
    }
    else {
      pushUndo(new CERemoveCellUndoCommand(m_molecule, this));
      emit cellChanged();
      hideEditors();
      GLWidget::current()->setRenderUnitCellAxes(false);
      refreshActions();
    }
  }

  void CrystallographyExtension::actionPasteCrystal()
  {
    showPasteDialog(QApplication::clipboard()->
                    text(QClipboard::Clipboard));
  }

  void CrystallographyExtension::actionToggleEditors()
  {
    // Are any currently shown?
    bool shown = false;
    for (QList<CEAbstractEditor*>::const_iterator
           it = m_editors.constBegin(),
           it_end = m_editors.constEnd();
         it != it_end; ++it) {
      if ((*it)->isVisible()) {
        shown = true;
        break;
      }
    }

    if (shown) {
      hideEditors();
    }
    else {
      showEditors();
    }
  }

  void CrystallographyExtension::actionToggleProperties()
  {
    if (m_displayProperties) {
      hideProperties();
    }
    else {
      showProperties();
    }
  }

  void CrystallographyExtension::actionWrapAtoms()
  {
    CEUndoState before (this);
    wrapAtomsToCell();
    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Wrap Atoms To Cell")));
  }

  void CrystallographyExtension::actionTranslateAtoms(GLWidget *gl)
  {
    if (!m_translateWidget) {
      m_translateWidget = new CETranslateWidget (this, m_mainwindow, gl);
      m_mainwindow->addDockWidget
        (m_translateWidget->preferredDockWidgetArea(),
         m_translateWidget);
    }
    m_translateWidget->show();
  }

  void CrystallographyExtension::actionOrientStandard()
  {
    CEUndoState before (this);
    orientStandard();
    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Rotate to Standard Orientation")));
  }

  void CrystallographyExtension::actionPrimitiveReduce()
  {
    QSettings settings;
    double tol = settings.value
      ("crystallographyextension/settings/spgTolAngstrom",
       0.1).toDouble();

    bool ok;
    tol = unconvertLength(QInputDialog::getDouble
                          (m_mainwindow,
                           CE_DIALOG_TITLE,
                           tr("Select tolerance in current cartesian units:"),
                           convertLength(tol), // initial
                           convertLength(1e-5), // min
                           convertLength(0.5), // max
                           5, &ok));

    if (!ok) {
      return;
    }

    settings.setValue
      ("crystallographyextension/settings/spgTolAngstrom", tol);
    settings.sync();

    CEUndoState before (this);
    unsigned int spg = Spglib::reduceToPrimitive(m_molecule, 0, tol);
    // spg == 0: Spacegroup perception failed
    if (spg == 0) {
      if (QMessageBox::question
          (m_mainwindow, CE_DIALOG_TITLE,
           tr("Spacegroup perception failed.\n\nWould you "
              "like to try again with a different tolerance?"),
           QMessageBox::Yes | QMessageBox::No,
           QMessageBox::Yes)
          == QMessageBox::Yes) {
        return actionPrimitiveReduce();
      }
      else {
        return;
      }
    }

    wrapAtomsToCell();
    orientStandard();
    currentCell()->SetSpaceGroup(spg);

    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Reduce to Primitive Cell")));

    emit cellChanged();
  }

  void CrystallographyExtension::actionNiggliReduce()
  {
    CEUndoState before (this);
    if (!niggliReduce()) {
      return;
    }
    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Reduce to Niggli Cell")));
  }

  void CrystallographyExtension::actionScaleToVolume()
  {
    double curvol = currentVolume();

    bool ok;
    double newvol = QInputDialog::getDouble
      (m_mainwindow, CE_DIALOG_TITLE,
       tr("Enter new volume:", "Unit cell volume"),
       curvol, 0, 1e20, 5, &ok);
    if (!ok) {
      return;
    }

    if (newvol == curvol) {
      return;
    }

    CEUndoState before (this);
    setCurrentVolume(newvol);
    CEUndoState after (this);
    pushUndo(new CEUndoCommand (before, after,
                                tr("Scale Unit Cell Volume")));
  }

  void CrystallographyExtension::actionUnitsLengthAngstrom()
  {
    setLengthUnit(Angstrom);
  }

  void CrystallographyExtension::actionUnitsLengthBohr()
  {
    setLengthUnit(Bohr);
  }

  void CrystallographyExtension::actionUnitsLengthNanometer()
  {
    setLengthUnit(Nanometer);
  }

  void CrystallographyExtension::actionUnitsLengthPicometer()
  {
    setLengthUnit(Picometer);
  }

  void CrystallographyExtension::actionUnitsAngleDegree()
  {
    setAngleUnit(Degree);
  }

  void CrystallographyExtension::actionUnitsAngleRadian()
  {
    setAngleUnit(Radian);
  }

  void CrystallographyExtension::actionCoordsCart()
  {
    setCoordsCartFrac(Cartesian);
  }

  void CrystallographyExtension::actionCoordsFrac()
  {
    setCoordsCartFrac(Fractional);
  }

  void CrystallographyExtension::actionCoordsPreserveCart()
  {
    setCoordsPreserveCartFrac(Cartesian);
  }

  void CrystallographyExtension::actionCoordsPreserveFrac()
  {
    setCoordsPreserveCartFrac(Fractional);
  }

  void CrystallographyExtension::actionMatrixCart()
  {
    setMatrixCartFrac(Cartesian);
  }

  void CrystallographyExtension::actionMatrixFrac()
  {
    setMatrixCartFrac(Fractional);
  }

  void CrystallographyExtension::actionMatrixRowVectors()
  {
    setMatrixVectorStyle(RowVectors);
  }

  void CrystallographyExtension::actionMatrixColumnVectors()
  {
    setMatrixVectorStyle(ColumnVectors);
  }

  void CrystallographyExtension::cacheFractionalCoordinates()
  {
    m_cachedFractionalCoords = currentFractionalCoords();
    m_cachedFractionalIds = currentAtomicSymbols();
  }

  void CrystallographyExtension::restoreFractionalCoordinates()
  {
    Q_ASSERT(m_cachedFractionalCoords.size() ==
             m_cachedFractionalIds.size());
    setCurrentFractionalCoords(m_cachedFractionalIds,
                               m_cachedFractionalCoords);
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(crystallographyextension,
                 Avogadro::CrystallographyExtensionFactory)

