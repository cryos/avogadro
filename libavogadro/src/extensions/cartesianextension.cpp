/**********************************************************************
  cartesianextension.cpp - Cartesian Editor for Avogadro

  Copyright (C) 2009 by Konstantin Tokarev
  Based on code written by Tim Vandermeersch and Geoffrey R. Hutchison

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

#include "cartesianextension.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/primitivelist.h>

#include <QtGui/QAction>
#include <QtGui/QDialog>
#include <QtGui/QClipboard>
#include <QtGui/QMessageBox>

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  static const double BOHR_TO_ANGSTROM = 0.529177249;
  static const double ANGSTROM_TO_BOHR = 1.0 / 0.529177249;

#ifdef Q_WS_X11
  static const QString EDITOR_FONT = "Monospace";
#else
  // Windows and Mac
  static const QString EDITOR_FONT = "Courier";
#endif

  CartesianEditor::CartesianEditor(QWidget *parent) : QDialog(parent),
                                                      m_unit(CoordinateUnit(0)),
                                                      m_format(CoordinateFormat(0)),
                                                      m_illegalInput(false)
  {
    setupUi(this);
    readSettings();

    cartesianEdit->setTextColor(Qt::black);
    cartesianEdit->setCurrentFont(QFont(EDITOR_FONT,
                                        QApplication::font().pointSize() + 1));

    connect(sortBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSort()));
    connect(unitsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeUnits()));
    connect(formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFormat()));

    connect(pasteButton, SIGNAL(clicked()), this, SLOT(paste()));
    connect(copyButton, SIGNAL(clicked()), this, SLOT(copy()));
    connect(cutButton, SIGNAL(clicked()), this, SLOT(cut()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));

    connect(cartesianEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));

    connect(applyButton, SIGNAL(clicked()), this, SLOT(updateMolecule()));
    connect(revertButton, SIGNAL(clicked()), this, SLOT(updateCoordinates()));
  }

  CartesianEditor::~CartesianEditor()
  {
    writeSettings();
  }

  void CartesianEditor::changeSort()
  {
    m_sort = SortingType(sortBox->currentIndex());
    updateCoordinates();
  }

  void CartesianEditor::changeUnits()
  {
    if (unitsBox->currentIndex() == int(FRACTIONAL) &&
        m_molecule->OBUnitCell() == 0) {
      // no unit cell
      QMessageBox::warning(this,
                           tr("Cartesian Editor"),
                           tr("No unit cell defined for molecule -- cannot use fractional coordinates."));
      unitsBox->setCurrentIndex(ANGSTROM);
      return;
    }

    m_unit = CoordinateUnit(unitsBox->currentIndex());
    updateCoordinates();
  }

  void CartesianEditor::changeFormat()
  {
    m_format = CoordinateFormat(formatBox->currentIndex());
    updateCoordinates();
  }

  void CartesianEditor::paste()
  {
    QClipboard *clipboard = QApplication::clipboard();
    cartesianEdit->append(clipboard->text());
  }

  void CartesianEditor::copy()
  {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(cartesianEdit->toPlainText());
  }

  void CartesianEditor::cut()
  {
    copy();
    clear();
  }

  void CartesianEditor::clear()
  {
    cartesianEdit->setText("");
  }

  void CartesianEditor::textChanged()
  {
    if (m_illegalInput) {
      m_illegalInput = false;
      cartesianEdit->setTextColor(Qt::black);
      QString t = cartesianEdit->toPlainText();
      cartesianEdit->setText(t);
    }
  }

  void CartesianEditor::updateMolecule()
  {
    OBMol *tmpMol = new OBMol;
    OBUnitCell *cell = 0;
    if (m_molecule->OBUnitCell())
      cell = new OBUnitCell (*(m_molecule->OBUnitCell()));
    if (parseText(tmpMol)) {
      m_molecule->setOBMol(tmpMol);
      if (cell)
        m_molecule->setOBUnitCell(cell);
      m_molecule->update();
      updateCoordinates();
    } else {
      cartesianEdit->setTextColor(Qt::red);
      QString t = cartesianEdit->toPlainText();
      cartesianEdit->setText(t);
      m_illegalInput = true;
      if (cell)
        delete cell;
    }
    delete tmpMol;
  }

  bool CartesianEditor::parseText(OBMol *mol)
  {
    QString coord = cartesianEdit->toPlainText();
    QStringList coordStrings = coord.split(QRegExp("\n"));

    matrix3x3 xform;
    switch (m_unit) {
    case ANGSTROM:
      for (int i = 0; i < 3; i++)
        xform.Set(i,i,1.0);
      break;
    case BOHR:
      for (int i = 0; i < 3; i++)
        xform.Set(i,i,BOHR_TO_ANGSTROM);
      break;
    case FRACTIONAL:
      // Fractional coordinates -- convert below
      xform = matrix3x3(0.0);
      break;
    }

    // Guess format

    // split on any non-word symbol, except '.'
    QStringList data = coordStrings.at(0).trimmed().split(QRegExp("\\s+|,|;"));
    // Format definition, will be used for parsing
    int NameCol=-1, Xcol=-1, Ycol=-1, Zcol=-1;
    QString format("");
    int a;
    double b;
    bool ok;
    for (int i=0; i<data.size(); i++) {
      if (data.at(i) == "") {
        continue;
      }

      a = data.at(i).toInt(&ok);
      if (ok) {
        format += "i";
        continue;
      }

      b = data.at(i).toDouble(&ok);
      if (ok) {
        if ((int)b == b && b!=0)
          format += "i";    // non-zero integer found - not likely to be coordinate
        else
          format += "d";
      } else {
        format += "s";
      }
    }

    qDebug() << "Format is: " << format;

    if (format.length() < 3)
      return false; // invalid format

    if (format == "ddd") {
      Xcol=0;
      Ycol=1;
      Zcol=2;
    } else if (format == "iddd") { // special XYZ variant
      NameCol=0;
      Xcol=1;
      Ycol=2;
      Zcol=3;
    }
    else { // more columns
        for (int i=0; i<format.length(); i++) {

            if ((format.at(i)=='d') || (format.length()==4 && format.at(i)=='i')) {
              // double
              if (Xcol==-1) {
                Xcol=i;
                continue;
              }
              if (Ycol==-1) {
                Ycol=i;
                continue;
              }
              if (Zcol==-1) {
                Zcol=i;
                continue;
              }
              continue; // nothing valuable
            }

            if (format.at(i) == 's') {
              // string
              if (NameCol != -1)  // just found
                continue;

              // Try to find element name or symbol inside it
              int n,iso;
              QString s = data.at(i);
              while (s.length()!=0) { // recognize name with number
                iso = 0;
                n = OpenBabel::etab.GetAtomicNum(s.toStdString(), iso);
                if (iso != 0)
                  n = 1;

                if (n!=0) {
                  NameCol=i;
                  break;
                } else {
                  s.chop(1);
                }
              }
            }
            continue;
        }
    }

    if((Xcol==-1) || (Ycol==-1) || (Zcol==-1)) {
      return false;
    }

    if ((NameCol==-1) && format != "ddd") {
      return false;
    }

    // Read and apply coordinates
    mol->BeginModify();
    for (int N=0; N<coordStrings.size(); N++) {
      if (coordStrings.at(N).trimmed() == "") {
        continue;
      }
      double x=0, y=0, z=0;
      int _n=0,_iso=0;
      OBAtom *atom  = mol->NewAtom();
      QStringList s_data = coordStrings.at(N).trimmed().split(QRegExp("\\s+|,|;"));
      if (s_data.size() != data.size()) {
        return false;
      }
      for (int i=0; i<s_data.size(); i++) {
        bool ok = true;
        if (i == Xcol) {
            x = s_data.at(i).toDouble(&ok);
        } else if (i == Ycol) {
            y = s_data.at(i).toDouble(&ok);
        } else if (i == Zcol) {
            z = s_data.at(i).toDouble(&ok);
        } else if ((i == NameCol) && (format == "iddd")) {
            _n = s_data.at(i).toInt(&ok);
        } else if (i == NameCol) {

            // Try to find element name or symbol inside it

              QString _s = s_data.at(i);
              while (_s.length()!=0) { // recognize name with number
                _iso=0;
                _n = OpenBabel::etab.GetAtomicNum(_s.toStdString(), _iso);
                if (_iso != 0)
                  _n = 1;

                if (_n!=0)
                  break;
                else
                  _s.chop(1);
              }
              if (_n==0)
                return false;
        }
        if (!ok) return false;
      }

      vector3 pos (x, y, z);
      if (format == "ddd") {
        if (m_molecule) {
          if (N < static_cast<int>(m_molecule->numAtoms()))
            atom->SetAtomicNum(m_molecule->atom(N)->atomicNumber());
          else
            atom->SetAtomicNum(0);
        }
      } else {
          atom->SetAtomicNum(_n);
      }
      if (xform.determinant() == 0.0) { // fractional coordinates
        atom->SetVector(m_molecule->OBUnitCell()->FractionalToCartesian(pos));
      }
      else {
        atom->SetVector(xform * pos);
      }
    }
    mol->EndModify();
    mol->ConnectTheDots();
    mol->PerceiveBondOrders();

    return true;
  }

  void CartesianEditor::updateCoordinates()
  {
    m_illegalInput = false;
    cartesianEdit->setTextColor(Qt::black);
    QString t = cartesianEdit->toPlainText();
    cartesianEdit->setText(t);

    if (!m_molecule) {
        clear();
    } else {
        QString *coord = new QString;
        QTextStream coordStream(coord);
        coordStream.setRealNumberPrecision(10);

        // Do sorting
        // FIXME: add new function for it?
        QList<Atom *> localAtom;
        QMultiMap<double, Atom*> tmpMap;
        QMultiMap<double, Atom*>::const_iterator it;

        int n=0;
        foreach (Atom *a, m_molecule->atoms()) {
          double key;
          switch (m_sort) {
          case ELEMENT:
            key = static_cast<double>(-1*a->atomicNumber());
            break;
          case X:
            key = a->pos()->x();
            break;
          case Y:
            key = a->pos()->y();
            break;
          case Z:
            key = a->pos()->z();
            break;
          default:
            key = n;
            n++;
          }
          tmpMap.insert(key, a);
        }

        it=tmpMap.constBegin();
        for (int i=0; it !=tmpMap.constEnd(); i++,it++ )
          localAtom.push_back(it.value());

        matrix3x3 xform;
        switch (m_unit) {
        case ANGSTROM:
          for (int i = 0; i < 3; i++)
            xform.Set(i,i,1.0);
          break;
        case BOHR:
          for (int i = 0; i < 3; i++)
            xform.Set(i,i,ANGSTROM_TO_BOHR);
          break;
        case FRACTIONAL:
          xform = matrix3x3(0.0); // Check below and use proper conversions
          break;
        }

        vector3 pos;

        for (unsigned int i=0; i<m_molecule->numAtoms(); i++) {
          //Atom *atom = m_molecule->atom(i);
          Atom *atom = localAtom.at(i);
          if (xform.determinant() == 0.0) { // fractional coordinates
            pos = m_molecule->OBUnitCell()->CartesianToFractional(
                  atom->OBAtom().GetVector());
          }
          else {
            pos = xform * atom->OBAtom().GetVector();
          }

          switch (m_format) {
          case XYZ:
            coordStream.setFieldWidth(3);
            coordStream << left << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()));
            coordStream.setFieldWidth(18);
            coordStream << fixed << forcepoint << right << pos.x() << pos.y()
                << pos.z() << endl;
            break;

          case XYZ_ONLY:
            coordStream.setFieldWidth(18);
            coordStream << fixed << forcepoint << right << pos.x() << pos.y()
                << pos.z() << endl;
            break;

          case XYZ_NUM:
            coordStream.setFieldWidth(6);
            coordStream << left << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()))+
            QString::number(i+1);
            coordStream.setFieldWidth(18);
            coordStream << fixed << forcepoint << right << pos.x() << pos.y()
              << pos.z() << endl;
            break;

          case GAMESS:
            coordStream.setFieldWidth(3);
            coordStream << left << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber()));
            coordStream.setFieldWidth(3);
            coordStream << right << atom->atomicNumber();
            coordStream.setFieldWidth(2);
            coordStream << left << ".0";
            coordStream.setFieldWidth(18);
            coordStream << fixed << forcepoint << right << pos.x() << pos.y()
              << pos.z() << endl;
            break;

          case GAMESS2:
            coordStream.setFieldWidth(12);
            coordStream << left << QString(OpenBabel::etab.GetName(atom->atomicNumber()).c_str());
            coordStream.setFieldWidth(3);
            coordStream << right << atom->atomicNumber();
            coordStream.setFieldWidth(2);
            coordStream << left << ".0";
            coordStream.setFieldWidth(18);
            coordStream << fixed << forcepoint << right << pos.x() << pos.y()
              << pos.z() << endl;
            break;

          case TURBOMOLE:
            coordStream.setFieldWidth(14);
            coordStream << fixed << forcepoint << left << right << pos.x();
            coordStream.setFieldWidth(18);
            coordStream << pos.y()
              << pos.z();
            coordStream.setFieldWidth(5);
            coordStream << left << right << QString(OpenBabel::etab.GetSymbol(atom->atomicNumber())) << endl;
            break;

          case PRIRODA:
            coordStream.setFieldWidth(3);
            coordStream << left << atom->atomicNumber();
            coordStream.setFieldWidth(18);
            coordStream << fixed << forcepoint << right << pos.x() << pos.y()
              << pos.z() << endl;
          }
        }
        cartesianEdit->setText(*coord);
        delete coord;
    }
  }

  void CartesianEditor::updateAtoms(Atom*)
  {
    updateCoordinates();
  }

  void CartesianEditor::moleculeChanged(Molecule *)
  {
    updateCoordinates();
  }

  void CartesianEditor::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    connect(m_molecule, SIGNAL(atomUpdated(Atom*)), this, SLOT(updateAtoms(Atom*)));
    connect(m_molecule, SIGNAL(atomRemoved(Atom*)), this, SLOT(updateAtoms(Atom*)));
    connect(m_molecule, SIGNAL(moleculeChanged()), this, SLOT(updateCoordinates()));
    updateCoordinates();
  }

  void CartesianEditor::writeSettings() const
  {
    QSettings settings;
    settings.setValue("cartesian/sort", m_sort);
    settings.setValue("cartesian/format", m_format);
    if (m_unit != FRACTIONAL)
      settings.setValue("cartesian/unit", m_unit);
  }

  void CartesianEditor::readSettings()
  {
    QSettings settings;
    m_sort = SortingType(settings.value("cartesian/sort", NONE).toInt());
    sortBox->setCurrentIndex(m_sort);
    m_unit = CoordinateUnit(settings.value("cartesian/unit", ANGSTROM).toInt());
    unitsBox->setCurrentIndex(m_unit);
    m_format = CoordinateFormat(settings.value("cartesian/format", XYZ).toInt());
    formatBox->setCurrentIndex(m_format);
  }


  CartesianExtension::CartesianExtension( QObject *parent ) : Extension( parent ), m_molecule(0), m_dialog(0)
  {
    QAction *action;

    action = new QAction( this );
    action->setSeparator(true);
    action->setData(-1);
    m_actions.append(action);

    action = new QAction( this );
    action->setText( tr("Cartesian Editor..." ));
    m_actions.append( action );
  }

  CartesianExtension::~CartesianExtension()
  {}

  QList<QAction *> CartesianExtension::actions() const
  {
    return m_actions;
  }

  QString CartesianExtension::menuPath(QAction *action) const
  {
    Q_UNUSED(action)
    return tr("&Build");
  }

  void CartesianExtension::setMolecule(Molecule *molecule)
  {
    if (m_molecule)
      disconnect( m_molecule, 0, 0, 0 );

    m_molecule = molecule;

    if (m_dialog) {
      m_dialog->setMolecule(molecule);
    }
  }

  QUndoCommand* CartesianExtension::performAction(QAction *action,
                                                   GLWidget *widget)
  {
    Q_UNUSED(action)
    QUndoCommand *undo = 0;

    if (!m_molecule)
      return 0; // nothing we can do

    // Disconnect in case we're attached to a new widget
    if (m_widget) {
      disconnect( m_molecule, 0, 0, 0 );
      if (m_dialog) {
        m_dialog->setMolecule(m_molecule);
      }
    }

    if (widget)
      m_widget = widget;

    if (!m_dialog) {
      m_dialog = new CartesianEditor(m_widget);
      m_dialog->setMolecule(m_molecule);
    }

    m_dialog->show();
    m_dialog->updateCoordinates();

    return undo;
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2( cartesianextension, Avogadro::CartesianExtensionFactory )
