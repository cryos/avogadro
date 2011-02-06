/**********************************************************************
  CEPasteDialog - Dialog Crystal import

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

#ifndef CEPASTEDIALOG_H
#define CEPASTEDIALOG_H

#include <avogadro/molecule.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <QtCore/QString>

#include <QtGui/QDialog>

#include "ui_crystalpastedialog.h"

namespace Avogadro {

  class CEPasteDialog : public QDialog
  {
    Q_OBJECT
  public:
    CEPasteDialog(QWidget *parent,
                  const QString &text,
                  Molecule *mol);
    virtual ~CEPasteDialog() {}

    enum CEPFormat {
      InvalidFormat = 0,
      VaspFormat
    };

    bool formatIsValid();

  protected slots:
    void refresh();
    void accept();

  protected:
    Molecule *m_molecule;
    QString m_text;
    CEPFormat m_format;
    unsigned int m_numAtomTypes; // For Vasp

    CEPFormat guessFormat(const QString &text);

    bool isVaspFormat(const QString &text);

    void refreshInvalidFormat();
    void refreshVaspFormat();

    bool acceptVaspFormat();

  private:
    Ui::CEPasteDialog ui;
  };
}

#endif
