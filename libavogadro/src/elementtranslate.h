/**********************************************************************
  ElementTranslator - Class to handle translation/i18n of element names

  Copyright (C) 2007 Geoffrey R. Hutchison

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

#ifndef __ELEMENTTRANSLATE_H
#define __ELEMENTTRANSLATE_H

#include <avogadro/global.h>

namespace Avogadro {

  /**
   * This class provides for translation of element names: e.g., tr("Carbon")
   *
   * @author Geoff Hutchison
   */
  class A_EXPORT ElementTranslator: public QObject
  {
    Q_OBJECT

  public:
    ElementTranslator(): QObject() {}
    ~ElementTranslator() {}

    /**
     * Translate element names.
     * @param element The atomic number of the element to be translated
     * @return a QString with the appropriate translation for the current locale
     */
    QString name(int element) const;
  };

  A_DECL_EXPORT extern ElementTranslator elementTranslator;

}

#endif
