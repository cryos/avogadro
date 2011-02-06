/**********************************************************************
  CrystalBuilderDialog - Dialog for Crystal Builder

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

#ifndef CRYSTALBUILDERDIALOG_H
#define CRYSTALBUILDERDIALOG_H

#include <openbabel/math/matrix3x3.h>

#include <avogadro/molecule.h>

#include <QMainWindow>

#include <Eigen/Core>

#include "ui_crystalbuilderdialog.h"

namespace OpenBabel {
  class OBUnitCell;
}

namespace Avogadro {
  class CrystalBuilderDialog : public QMainWindow
  {
    Q_OBJECT

  public:
    //! Constructor
    explicit CrystalBuilderDialog(QWidget *parent = 0,
                                  Qt::WindowFlags f = 0);
    //! Deconstructor
    ~CrystalBuilderDialog();

    struct UnitCellParameters {
      double a, b, c, alpha, beta, gamma;
      UnitCellParameters(double _a,double _b,double _c,
                         double _al,double _be,double _ga) {
        a=_a;b=_b;c=_c;alpha=_al;beta=_be;gamma=_ga;};
      UnitCellParameters() {
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

    LengthUnit lengthUnit() {return m_lengthUnit;};
    AngleUnit angleUnit() {return m_angleUnit;};

    CartFrac coordsCartFrac() {return m_coordsCartFrac;};
    CartFrac coordsPreserveCartFrac() {return m_coordsPreserveCartFrac;};

    CartFrac matrixCartFrac() {return m_matrixCartFrac;};
    VectorStyle matrixVectorStyle() {return m_matrixVectorStyle;};

  signals:
    void cellChanged();

    void lengthUnitChanged(LengthUnit);
    void angleUnitChanged(AngleUnit);
    void coordsCartFracChanged(CartFrac);
    void coordsPreserveCartFracChanged(CartFrac);
    void matrixCartFracChanged(CartFrac);
    void matrixVectorStyleChanged(VectorStyle);

    void matrixValid();
    void matrixInvalid();
    void parametersValid();
    void parametersInvalid();
    void coordsValid();
    void coordsInvalid();

  public slots:
    void setMolecule(Molecule *molecule);

    void writeSettings();
    void readSettings();

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

  protected slots:
    void setCurrentCell(OpenBabel::OBUnitCell*);
    void setCurrentCellMatrix(const Eigen::Matrix3d&);
    void setCurrentCellParameters(const UnitCellParameters&);
    void setCurrentFractionalCoords(const QList<QString> &ids,
                                    const QList<Eigen::Vector3d> &fcoords);
    void setCurrentCartesianCoords(const QList<QString> &ids,
                                   const QList<Eigen::Vector3d> &coords);
    void setCurrentVolume(double volume);

    void refreshGui();
    void refreshSettings();
    void refreshParameters();
    void refreshMatrix();
    void refreshCoords();
    void refreshProperties();
    void refreshVolume();
    void refreshLattice();
    void refreshSpacegroup();
    void refreshUnitIndicators();

    void updateGuiForMatrixEdit();
    void updateGuiForParametersEdit();
    void updateGuiForCoordsEdit();

    Eigen::Matrix3d checkMatrixWidget();
    UnitCellParameters checkParametersWidget();
    void checkCoordsWidget();

    void updateMatrix();
    void updateParameters();
    void updateCoords();

    void markMatrixAsInvalid();
    void markMatrixAsValid();
    void markParametersAsInvalid();
    void markParametersAsValid();
    void markCoordsAsInvalid();
    void markCoordsAsValid();

  protected:
    void closeEvent(QCloseEvent *event);

    //  Conversion factor (storages * [factor] = display)
    inline double lengthConversionFactor()
    {
      // Storage is in angstrom, so convert appropriately.
      switch (lengthUnit()) {
      default:
      case Angstrom: return 1.0;
      case Bohr:     return 1.8897162;
      }
    }
    inline double angleConversionFactor()
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

    // retreival functions
    inline OpenBabel::OBUnitCell* currentCell() {
      return m_molecule->OBUnitCell();};
    Eigen::Matrix3d currentCellMatrix();
    Eigen::Matrix3d currentFractionalMatrix();
    // Array length 6, see enum UnitCellParameter for ordering
    UnitCellParameters currentCellParameters();
    QList<Eigen::Vector3d> currentFractionalCoords();
    QList<Eigen::Vector3d> currentCartesianCoords();
    QList<int> currentAtomicNumbers();
    QList<QString> currentAtomicSymbols();
    double currentVolume();

    // Tool helpers/implementaions
    void fillUnitCell();
    void wrapAtomsToCell();
    void copyToClipboard();
    void niggliReduce();

    Molecule *m_molecule;

  private:
    Ui::CrystalBuilderDialog ui;

    void createActions();
    void createMenuBar();
    void initializeStatusBar();

    QAction *m_actionPerceiveSpacegroup;
    QAction *m_actionSetSpacegroup;
    QAction *m_actionFillCell;
    QAction *m_actionWrapAtoms;
    QAction *m_actionCopyToClipboard;
    QAction *m_actionNiggliReduce;
    QAction *m_actionScaleToVolume;

    QActionGroup *m_actionUnitsLengthGroup;
    QAction      *m_actionUnitsLengthAngstrom;
    QAction      *m_actionUnitsLengthBohr;

    QActionGroup *m_actionUnitsAngleGroup;
    QAction      *m_actionUnitsAngleDegree;
    QAction      *m_actionUnitsAngleRadian;

    QActionGroup *m_actionCoordsCartFracGroup;
    QAction      *m_actionCoordsCart;
    QAction      *m_actionCoordsFrac;

    QActionGroup *m_actionCoordsPreserveGroup;
    QAction      *m_actionCoordsPreserveCart;
    QAction      *m_actionCoordsPreserveFrac;

    QActionGroup *m_actionMatrixCartFracGroup;
    QAction      *m_actionMatrixCart;
    QAction      *m_actionMatrixFrac;

    QActionGroup *m_actionMatrixVectorGroup;
    QAction      *m_actionMatrixRowVectors;
    QAction      *m_actionMatrixColumnVectors;

    QMenu *m_menuTools;
    QMenu *m_menuSettings;
    QMenu  *m_menuSettingsUnits;
    QMenu   *m_menuSettingsUnitsLength;
    QMenu   *m_menuSettingsUnitsAngle;
    QMenu  *m_menuSettingsCoords;
    QMenu  *m_menuSettingsMatrix;

    LengthUnit m_lengthUnit;
    AngleUnit m_angleUnit;
    CartFrac m_coordsCartFrac;
    CartFrac m_coordsPreserveCartFrac;
    CartFrac m_matrixCartFrac;
    VectorStyle m_matrixVectorStyle;

    QLabel *m_statusVolumeLabel;
    QLabel *m_statusLatticeLabel;
    QLabel *m_statusSpaceGroupLabel;
    QLabel *m_statusLengthUnitLabel;
    QLabel *m_statusAngleUnitLabel;

    QTextCharFormat m_matrixCharFormat;
    QTextCharFormat m_parametersCharFormat;
    QTextCharFormat m_coordsCharFormat;

    // Coordinate preservation
    QList<QString> m_cachedFractionalIds;
    QList<Eigen::Vector3d> m_cachedFractionalCoords;

  private slots:
    // Actions
    void actionPerceiveSpacegroup();
    void actionSetSpacegroup();
    void actionFillCell();
    void actionWrapAtoms();
    void actionCopyToClipboard();
    void actionNiggliReduce();
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
}

#endif
