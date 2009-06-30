/**********************************************************************
  OrbitalEngine - Engine for display of molecular orbitals

  Copyright (C) 2008-2009 Marcus D. Hanwell
  Copyright (C) 2008 Geoffrey R. Hutchison
  Copyright (C) 2008 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef ORBITALENGINE_H
#define ORBITALENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/color.h>

#include <QPointer>
#include <QList>

namespace Avogadro {

  class Mesh;
  class OrbitalSettingsWidget;

  //! Orbital Engine class.
  class OrbitalEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE("Orbitals", tr("Orbitals"),
                    tr("Renders molecular orbitals"))

    public:
      //! Constructor
      OrbitalEngine(QObject *parent=0);
      //! Destructor
      ~OrbitalEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      bool renderQuick(PainterDevice *pd);
      bool renderPick(PainterDevice *) { return false; }
      //@}

      double transparencyDepth() const;
      Layers layers() const;
      PrimitiveTypes primitiveTypes() const;
      ColorTypes colorTypes() const;

      Engine *clone() const;

      QWidget* settingsWidget();
      bool hasSettings() { return true; }

      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);

      void setPrimitives(const PrimitiveList &primitives);

    public slots:
      void addPrimitive(Primitive *primitive);
      void updatePrimitive(Primitive *primitive);
      void removePrimitive(Primitive *primitive);
      void setMolecule(const Molecule *molecule);

    protected:
      OrbitalSettingsWidget *m_settingsWidget;
      QPointer<Mesh> m_mesh1;
      QPointer<Mesh> m_mesh2;
      Eigen::Vector3d m_min, m_max;
      Color  m_posColor;
      Color  m_negColor;
      double m_alpha;
      int    m_renderMode;
      bool   m_drawBox;
      bool   m_colored;
      QList <unsigned long> m_meshes;

      bool renderBox(PainterDevice *pd);

    private slots:
      /**
       * Update the orbital combo box with new orbitals
       */
      void updateOrbitalCombo();

      void settingsWidgetDestroyed();
      /**
       * @param value orbital index
       */
      void setOrbital(int value);
      /**
       * @param value opacity of the surface / 20
       */
      void setOpacity(int value);
      /**
       * @param value render mode (0 = fill, 1 = wireframe, 2 = points)
       */
      void setRenderMode(int value);
      /**
       * @param value interpolate (0 = no, 1 = yes)
       */
      void setDrawBox(int value);

      /**
       * @param color render mode - 0 = selected colors, 1 = mapped colors
       */
      void setColorMode(int value);

      /**
       * @param color the color for the positive iso surface
       */
      void setPosColor(const QColor& color);
      /**
       * @param color the color for the negative iso surface
       */
      void setNegColor(const QColor& color);
  };

  //! Generates instances of our OrbitalEngine class
  class OrbitalEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(OrbitalEngine)
  };

} // end namespace Avogadro

#endif
