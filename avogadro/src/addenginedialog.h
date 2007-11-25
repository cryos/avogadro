/**********************************************************************
  AddEngineDialog - Settings Dialog

  Copyright (C) 2007 Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#ifndef __ADDENGINEDIALOG_H
#define __ADDENGINEDIALOG_H

#include "ui_addenginedialog.h"

class QAbstractButton;
namespace Avogadro {

  class Engine;
  class EngineFactory;
  class AddEngineDialog : public QDialog
  {
    Q_OBJECT

    public:
      AddEngineDialog(QWidget *parent=0);

      /**
       * \return a new engine object or 0
       */
      static Engine * getEngine(QWidget *parent, const QList<EngineFactory *> &engineFactories);

      void addType(const QString &type);
      int typeIndex();
      QString nameText();
      QString descriptionText();

    private:
      Ui::AddEngineDialog ui;

  };

} // end namespace Avogadro

#endif
