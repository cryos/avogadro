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

#ifndef ELEMENTTRANSLATE_H
#define ELEMENTTRANSLATE_H

#include <avogadro/global.h>

namespace Avogadro {

  /**
   * @class ElementTranslator elementtranslate.h <avogadro/elementtranslate.h>
   * @brief Internationalization of element names
   * @author Geoff Hutchison
   *
   * This class provides for translation of element names: e.g., tr("Carbon")
   * To use the global translator:
   * @code
   * #include <avogadro/elementtranslate.h>
   * ...
   * Avogadro::elementTranslator.name(6); // Return "carbon" in English
   * @endcode
   */
  class A_EXPORT ElementTranslator: public QObject
  {
    Q_OBJECT

  public:
    ElementTranslator();
    ~ElementTranslator();

    /**
     * Translate element names.
     * @param element The atomic number of the element to be translated
     * @return a QString with the appropriate translation for the current locale
     */
    static QString name(int element);
  };

}

#endif
