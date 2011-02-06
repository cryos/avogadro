/**********************************************************************
  crystallographyextension.h - Crystal Builder Plugin for Avogadro

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef CRYSTALLOGRAPHYEXTENSION_H
#define CRYSTALLOGRAPHYEXTENSION_H

#include <avogadro/extension.h>
#include <avogadro/molecule.h>

#include <openbabel/math/matrix3x3.h>

#include <Eigen/Core>

class QLabel;
class QMainWindow;
class QUndoCommand;

namespace OpenBabel {
  class OBUnitCell;
}

namespace Avogadro
{
  class CEAbstractEditor;

  struct CEUnitCellParameters {
    double a, b, c, alpha, beta, gamma;
    CEUnitCellParameters(double _a,double _b,double _c,
                       double _al,double _be,double _ga) {
      a=_a;b=_b;c=_c;alpha=_al;beta=_be;gamma=_ga;};
    CEUnitCellParameters() {
      a=b=c=alpha=beta=gamma=0;};
  };

  enum CartFrac {
    Cartesian = 0,
    Fractional
  };

  enum VectorStyle {
    RowVectors = 0,
    ColumnVectors
  };

  enum LengthUnit {
    Angstrom = 0,
    Bohr
  };

  enum AngleUnit {
    Degree = 0,
    Radian
  };

  const int CE_FONTSIZE = 10;

  const QString CE_DIALOG_TITLE =
    QT_TRANSLATE_NOOP("CrystallographyExtension", "Avogadro");

  const unsigned short CE_ANGSTROM_UTF16 = 0x212B;
  const QString CE_ANGSTROM =
    QString::fromUtf16(&CE_ANGSTROM_UTF16, 1);

  const unsigned short CE_SUB_ZERO_UTF16 = 0x2080;
  const QString CE_SUB_ZERO =
    QString::fromUtf16(&CE_SUB_ZERO_UTF16, 1);

  const unsigned short CE_DEGREE_UTF16 = 0x00B0;
  const QString CE_DEGREE =
    QString::fromUtf16(&CE_DEGREE_UTF16, 1);

  const unsigned short CE_SUPER_THREE_UTF16 = 0x00B3;
  const QString CE_SUPER_THREE =
    QString::fromUtf16(&CE_SUPER_THREE_UTF16, 1);

  class CrystallographyExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Crystallography", tr("Crystallography"),
                       tr("Construct and analyze periodic structures."));

  public:
    // Avogadro::Extension stuff:
    CrystallographyExtension(QObject *parent=0);
    virtual ~CrystallographyExtension();

    virtual QString menuPath(QAction *action) const;

    virtual QList<QAction *> actions() const;

    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    void setMolecule(Molecule *molecule);

    void writeSettings();
    void readSettings();

    // Settings access:
    LengthUnit lengthUnit() {return m_lengthUnit;};
    AngleUnit angleUnit() {return m_angleUnit;};

    CartFrac coordsCartFrac() {return m_coordsCartFrac;};
    CartFrac coordsPreserveCartFrac() {return m_coordsPreserveCartFrac;};

    CartFrac matrixCartFrac() {return m_matrixCartFrac;};
    VectorStyle matrixVectorStyle() {return m_matrixVectorStyle;};

    // Undo friends
    friend class CEUndoState;

  signals:
    void cellChanged();
    void lengthUnitChanged(LengthUnit);
    void angleUnitChanged(AngleUnit);
    void coordsCartFracChanged(CartFrac);
    void coordsPreserveCartFracChanged(CartFrac);
    void matrixCartFracChanged(CartFrac);
    void matrixVectorStyleChanged(VectorStyle);

  public slots:
    void repaintMolecule() {if (m_molecule) m_molecule->update();}

    // Push an undo command to the current undo stack
    void pushUndo(QUndoCommand*);

    // editor functions
    void initializeEditors();
    void showEditors();
    void hideEditors();
    void lockEditors();
    void unlockEditors();
    void refreshEditors();

    // Property display functions
    void showProperties();
    void hideProperties();
    void refreshProperties();

    // Actions (update checkboxes, labels, etc.)
    void refreshActions();

    // Settings modifiers:
    void setLengthUnit(LengthUnit l) {
      m_lengthUnit = l; emit lengthUnitChanged(l);};
    void setAngleUnit(AngleUnit a) {
      m_angleUnit = a; emit angleUnitChanged(a);};

    void setCoordsCartFrac(CartFrac c) {
      m_coordsCartFrac = c;
      emit coordsCartFracChanged(c);};
    void setCoordsPreserveCartFrac(CartFrac c) {
      m_coordsPreserveCartFrac = c;
      emit coordsPreserveCartFracChanged(c);};

    void setMatrixCartFrac(CartFrac c) {
      m_matrixCartFrac = c; emit matrixCartFracChanged(c);};
    void setMatrixVectorStyle(VectorStyle v) {
      m_matrixVectorStyle = v; matrixVectorStyleChanged(v);};

    //  Conversion factor (storage * [factor] = display)
    double lengthConversionFactor()
    {
      // Storage is in angstrom, so convert appropriately.
      switch (lengthUnit()) {
      default:
      case Angstrom: return 1.0;
      case Bohr:     return 1.8897162;
      }
    }
    double angleConversionFactor()
    {
      // Storage is in degree, so convert appropriately.
      switch (angleUnit()) {
      default:
      case Degree: return 1.0;
      case Radian: return 0.0174532925; // pi/180
      }
    }
    //  storage -> display
    double convertLength(double length);
    Eigen::Vector3d convertLength(const Eigen::Vector3d&);
    Eigen::Matrix3d convertLength(const Eigen::Matrix3d&);
    double convertAngle(double angle);
    //  display -> storage
    double unconvertLength(double length);
    Eigen::Vector3d unconvertLength(const Eigen::Vector3d&);
    Eigen::Matrix3d unconvertLength(const Eigen::Matrix3d&);
    double unconvertAngle(double angle);

    // Molecule access functions
    inline OpenBabel::OBUnitCell* currentCell() {
      return (m_molecule) ? m_molecule->OBUnitCell() : 0 ;}
    Eigen::Matrix3d currentCellMatrix();
    Eigen::Matrix3d currentFractionalMatrix();
    CEUnitCellParameters currentCellParameters();
    QList<Eigen::Vector3d> currentFractionalCoords();
    QList<Eigen::Vector3d> currentCartesianCoords();
    QList<int> currentAtomicNumbers();
    QList<QString> currentAtomicSymbols();
    QString currentLatticeType();
    double currentVolume();

    // Molecule modifiers
    void setCurrentCell(OpenBabel::OBUnitCell*);
    void setCurrentCellMatrix(const Eigen::Matrix3d&);
    void setCurrentCellParameters(const CEUnitCellParameters&);
    void setCurrentFractionalCoords(const QList<QString> &ids,
                                    const QList<Eigen::Vector3d> &fcoords);
    void setCurrentCartesianCoords(const QList<QString> &ids,
                                   const QList<Eigen::Vector3d> &coords);
    void setCurrentVolume(double volume);

    // Tool helpers/implementaions
    void fillUnitCell();
    void wrapAtomsToCell();
    void orientStandard();
    void showPasteDialog(const QString &text);
    bool niggliReduce();

  private:
    void createActions();

    enum ActionIndex {
      // Loose
      ToggleUnitCellIndex = 0,
      PasteCrystalIndex,
      ToggleUnitCellSepIndex,
      ToggleEditorsIndex,
      TogglePropertiesIndex,
      ToggleGUISepIndex,
      WrapAtomsIndex,
      OrientStandardIndex,
      ScaleToVolumeIndex,
      LooseSepIndex,
      // Spacegroup
      PerceiveSpacegroupIndex,
      SetSpacegroupIndex,
      FillUnitCellIndex,
      SymmetrizeCrystalIndex,
      // Reduce
      PrimitiveReduceIndex,
      NiggliReduceIndex,
      SettingsMainSep1Index,
      // Settings
      UnitsLengthAngstromIndex,
      UnitsLengthBohrIndex,
      UnitsAngleDegreeIndex,
      UnitsAngleRadianIndex,
      CoordsCartIndex,
      CoordsFracIndex,
      CoordsPreserveCartIndex,
      CoordsPreserveFracIndex,
      MatrixCartIndex,
      MatrixFracIndex,
      MatrixCoordDisplaySep1Index,
      MatrixRowVectorsIndex,
      MatrixColumnVectorsIndex
    };

    QAction* getAction(ActionIndex a) {
      return m_actions.at(static_cast<int>(a));
    }

    QMainWindow *m_mainwindow;
    QList<QAction*> m_actions;
    QList<CEAbstractEditor*> m_editors;
    Molecule *m_molecule;

    bool m_displayProperties;
    QLabel *m_latticeProperty;
    QLabel *m_spacegroupProperty;
    QLabel *m_volumeProperty;

    LengthUnit m_lengthUnit;
    AngleUnit m_angleUnit;
    CartFrac m_coordsCartFrac;
    CartFrac m_coordsPreserveCartFrac;
    CartFrac m_matrixCartFrac;
    VectorStyle m_matrixVectorStyle;

    // Coordinate preservation
    QList<QString> m_cachedFractionalIds;
    QList<Eigen::Vector3d> m_cachedFractionalCoords;

    // refresh limiting:
    bool m_editorRefreshPending;

  private slots:
    // Hidden functions
    void refreshEditors_();

    // Actions
    void actionPerceiveSpacegroup();
    void actionSetSpacegroup();
    void actionFillUnitCell();
    void actionSymmetrizeCrystal();
    void actionPrimitiveReduce();
    void actionNiggliReduce();
    void actionToggleUnitCell();
    void actionPasteCrystal();
    void actionToggleEditors();
    void actionToggleProperties();
    void actionWrapAtoms();
    void actionOrientStandard();
    void actionScaleToVolume();

    // Settings
    void actionUnitsLengthAngstrom();
    void actionUnitsLengthBohr();
    void actionUnitsAngleDegree();
    void actionUnitsAngleRadian();
    void actionCoordsCart();
    void actionCoordsFrac();
    void actionCoordsPreserveCart();
    void actionCoordsPreserveFrac();
    void actionMatrixCart();
    void actionMatrixFrac();
    void actionMatrixRowVectors();
    void actionMatrixColumnVectors();

    // Coordinate preservation
    void cacheFractionalCoordinates();
    void restoreFractionalCoordinates();
  };

  class CrystallographyExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(CrystallographyExtension);
  };

} // end namespace Avogadro

#endif

