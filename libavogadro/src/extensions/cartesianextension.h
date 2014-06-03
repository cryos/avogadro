/**********************************************************************
  cartesianextension.h - Cartesian Editor for Avogadro

  Copyright (C) 2009 by Konstantin Tokarev
  Based on code written by Tim Vandermeersch and Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef CARTESIANEXTENSION_H
#define CARTESIANEXTENSION_H

#include <avogadro/extension.h>
#include <QtGui/QUndoCommand>
#include <openbabel/mol.h>
using namespace OpenBabel;

#include "ui_cartesianeditor.h"

#ifndef BUFF_SIZE
#define BUFF_SIZE 256
#endif

class QProgressDialog;
namespace Avogadro {

  class Molecule;
  class Atom;

  enum CoordinateUnit {
    ANGSTROM=0,
    BOHR,
    FRACTIONAL
  };

  enum CoordinateFormat {
    XYZ=0,
    XYZ_NUM,
    XYZ_ONLY,
    GAMESS,
    GAMESS2,
    TURBOMOLE,
    PRIRODA
  };

  enum SortingType {
    NONE=0,
    ELEMENT,
    X,
    Y,
    Z
  };

  class CartesianEditor : public QDialog, Ui::CartesianEditorDialog 
  {
    Q_OBJECT
      public:
    //! Constructor
    CartesianEditor(QWidget *parent=0);
    ~CartesianEditor();

    void setMolecule (Molecule *molecule);
    void writeSettings() const;
    void readSettings();

    public Q_SLOTS:
    //! Slots to take signals from Molecules, and GLWidget
    void updateCoordinates();
    void updateMolecule();
    void updateAtoms(Atom*);
    void moleculeChanged(Molecule *previous);
    void changeSort();
    void changeUnits();
    void changeFormat();
    void paste();
    void copy();
    void cut();
    void clear();
    void textChanged();

  private:
    bool parseText(OBMol *mol);

    Molecule *m_molecule;
    CoordinateUnit m_unit;
    CoordinateFormat m_format;
    SortingType m_sort;
    bool m_illegalInput;
    QColor m_defaultTextColor;
    QColor m_alternateTextColor;
    QColor m_defaultBackgroundColor;
    QColor m_alternateBackgroundColor;
  };

  class CartesianExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Cartesian editor", tr("Cartesian editor"),
                         tr("Cartesian coordinates editor"))

  public:
    //! Constructor
    CartesianExtension(QObject *parent=0);
    //! Deconstructor
    virtual ~CartesianExtension();

    /** @return a menu path for the extension's actions */
    virtual QString menuPath(QAction *action) const;

    //! Perform Action
    virtual QList<QAction *> actions() const;
    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
    //@}

    void setMolecule(Molecule *molecule);

  private:
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    CartesianEditor *m_dialog;
    GLWidget *m_widget;
  };

  class CartesianExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(CartesianExtension)
  };

} // end namespace Avogadro

#endif
