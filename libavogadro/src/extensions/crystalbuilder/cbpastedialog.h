/**********************************************************************
  CBPasteDialog - Dialog Crystal import

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

#ifndef CBPASTEDIALOG_H
#define CBPASTEDIALOG_H

#include <avogadro/molecule.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <QtCore/QString>

#include <QtGui/QDialog>

#include "ui_cbpastedialog.h"

namespace Avogadro {

  class CBPasteDialog : public QDialog
  {
    Q_OBJECT
  public:
    CBPasteDialog(QWidget *parent,
                  const QString &text,
                  Molecule *mol);
    virtual ~CBPasteDialog() {}

    enum CBPFormat {
      InvalidFormat = 0,
      VaspFormat
    };

  protected slots:
    void refresh();
    void accept();

  protected:
    Molecule *m_molecule;
    QString m_text;
    CBPFormat m_format;
    unsigned int m_numAtomTypes; // For Vasp

    CBPFormat guessFormat(const QString &text);

    bool isVaspFormat(const QString &text);

    void refreshInvalidFormat();
    void refreshVaspFormat();

    bool acceptVaspFormat();

  private:
    Ui::CBPasteDialog ui;
  };
}

#endif
