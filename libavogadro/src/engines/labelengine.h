/**********************************************************************
  LabelEngine - Engine for displaying labels.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2007 Marcus D. Hanwell

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

#ifndef __LABELENGINE_H
#define __LABELENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

#include "ui_labelsettingswidget.h"

namespace Avogadro {

  //! Label Engine class.
  class LabelSettingsWidget;
  class LabelEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Label"))

    public:
      //! Constructor
      LabelEngine(QObject *parent=0);
      //! Deconstructor
      ~LabelEngine() {}


      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderOpaque(PainterDevice *pd, const Atom *a);
      bool renderOpaque(PainterDevice *pd, const Bond *b);
      //@}

      EngineFlags flags() const;

      //! Display a window for the user to pick rendering options
      QWidget *settingsWidget();

    private:
      int m_atomType;  // Atom label type
      int m_bondType;  // Bond label type
      LabelSettingsWidget* m_settingsWidget;

    private Q_SLOTS:
      void setAtomType(int value);
      void setBondType(int value);
      void settingsWidgetDestroyed();

  };

  class LabelSettingsWidget : public QWidget, public Ui::LabelSettingsWidget
  {
    public:
      LabelSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our LabelEngine class
  class LabelEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    
    AVOGADRO_ENGINE_FACTORY(LabelEngine)
  };

} // end namespace Avogadro

#endif
