/**********************************************************************
  Application.h - main application events

  Copyright (C) 2007 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <QApplication>

namespace Avogadro {

  class Application : public QApplication
  {
    Q_OBJECT;

    public:
      Application(int &argc, char **argv);
      bool loadFile(const QString &fileName);

  protected:
      bool event(QEvent *event);

  private:
  };

} // end namespace Avogadro
#endif
