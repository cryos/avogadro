/**********************************************************************
  BSDYEngine - Dynamic detail engine for "balls and sticks" display

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

#ifndef __BSDYENGINE_H
#define __BSDYENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

#include "ui_bsdysettingswidget.h"

namespace Avogadro {

  //! Ball and Stick Engine class.
  class BSDYSettingsWidget;
  class BSDYEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      BSDYEngine(QObject *parent=0);
      //! Deconstructor
      ~BSDYEngine();

      //! \name Description methods

      double transparencyDepth() const;
      EngineFlags flags() const;

      //! \name Render Methods
      //@{
      //! Render an Atom.
      bool renderOpaque(const Atom *a);
      //! Render a Bond.
      bool renderOpaque(const Bond *b);

      bool renderOpaque(GLWidget *gl);

      double radius(const Primitive *p = 0) const;
      //@}

      //! Display a window for the user to pick rendering options
      QWidget *settingsWidget();

    private:
      double radius(const Atom *atom) const;

      GLWidget *m_glwidget;
      BSDYSettingsWidget *m_settingsWidget;

      double m_atomRadiusPercentage;
      double m_bondRadius;

    private Q_SLOTS:
      void settingsWidgetDestroyed();

      /**
       * @param percent percentage of the VdwRad
       */
      void setAtomRadiusPercentage(int percent);

      /**
       * @param value radius of the bonds * 10
       */
      void setBondRadius(int value);

  };

  class BSDYSettingsWidget : public QWidget, public Ui::BSDYSettingsWidget
  {
    public:
      BSDYSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our BSDYEngine class
  class BSDYEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new BSDYEngine(parent); }
  };

} // end namespace Avogadro

#endif
