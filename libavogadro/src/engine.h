/**********************************************************************
  Engine - Engine interface and plugin factory.

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

#ifndef ENGINE_H
#define ENGINE_H

#include <avogadro/global.h>
#include <avogadro/primitivelist.h>
#include "painterdevice.h"
#include "plugin.h"

#include <QSettings>
#include <QtPlugin>

#define AVOGADRO_ENGINE(t) \
  public: \
    static QString staticType() { return t; } \
    QString staticName() const { return staticType(); } \

#define AVOGADRO_ENGINE_FACTORY(c,d) \
  public: \
    Plugin *createInstance(QObject *parent = 0) { return new c(parent); } \
    int type() const { return Plugin::EngineType; }; \
    QString name() const { return c::staticType(); } \
    QString description() const { return d; }; 

namespace Avogadro {

  class PainterDevice;

  /**
   * @class Engine engine.h <avogadro/engine.h>
   * @brief Engine plugin interface.
   * @author Donald Ephraim Curtis
   * @author Marcus D. Hanwell
   *
   * This class provides an interface for our engines.
   * Subclasses of this class are loaded by the GLWidget and used to render
   * different parts of our project (Molecule, Atom, Bond, Residue) depending
   * on what style of engine we are implementing.
   * \sa GLWidget::render()
   */
  class EnginePrivate;
  class A_EXPORT Engine : public Plugin
  {
    Q_OBJECT

    public:
    /**
     * \enum EngineFlag
     * Different properties an engine can set which affect the rendering order.
     */
      enum EngineFlag {
        NoFlags = 0x00, /// no flags
        Transparent = 0x01, /// renders transparency
        Overlay = 0x02, /// renders overlay
        Bonds = 0x04, /// renders bonds
        Atoms = 0x08, /// renders atoms
        Molecules = 0x10 /// renders molecules
      };
      Q_DECLARE_FLAGS(EngineFlags, EngineFlag)

    public:
      /**
       * Constructor
       */
      Engine(QObject *parent = 0);
      /**
       * Destructor
       */
      virtual ~Engine();

      /** 
       * Plugin Type 
       */
      int type() const;
 
      /** 
       * Plugin Type Name (Engines)
       */
      QString typeName() const;
      /**
       * @return a string with the static name of the engine.
       */
      virtual QString staticName() const = 0;

      /**
       * @return the name of the engine.
       */
      QString name() const;

      /**
       * @param name the new name for the engine instance.
       */
      void setName(const QString &name);

      /**
       * @return engine description.
       */
      QString description() const;

      /**
       * @param description the new description of this engine.
       */
      void setDescription(const QString &description);

      /**
       * @return the flags for this engine.
       */
      virtual EngineFlags flags() const;

      /**
       * Render opaque elements.  This function is allowed to render
       * whatever opaque primitives it wishes.  There is no requirement that it
       * render every primitive.
       *
       * During generation of the GL view engines will have their render
       * functions called at most once.  It is the responsibility
       * of the engine to render all of the objects in it's queue if
       * it can.
       *
       * @return @c true if the rendering was completed successfully, @c false otherwise
       *
       * Example
       * @code
       * Color *map = colorMap(); // possible custom color map
       * if (!map) map = pd->colorMap(); // fall back to global color map
       *
       * // Get a list of bonds and render them
       * QList<Primitive *> list;
       * list = primitives().subList(Primitive::BondType);
       *
       * foreach(Primitive *p, list)
       *   render(static_cast<const Bond *>(p))
       *
       * return true;
       * @endcode
       *
       * @note To allow picking to happen you need to push the object type and name.
       * If objects cannot be picked this may be omitted.
       *
       * For more information on the various primitive lists available see
       * PrimitiveList.
       *
       */
      virtual bool renderOpaque(PainterDevice *pd) = 0;
      /**
       * Render transparent elements.  This function is allowed to render
       * whatever transparent primitives it wishes.  There is no requirement that it
       * render every primitive.
       *
       * During generation of the GL view engines will have their render
       * functions called at most once.  It is the responsibility
       * of the engine to render all of the objects in it's queue if
       * it can.
       *
       * @return @c true if the rendering was completed successfully, @c false otherwise
       *
       * Example
       * @code
       * Color *map = colorMap(); // possible custom color map
       * if (!map) map = pd->colorMap(); // fall back to global color map
       *
       * // Get a list of atoms and render the selected ones as transparent spheres
       * QList<Primitive *> list;
       * list = primitives().subList(Primitive::AtomType);
       *
       * foreach(Primitive *p, list) {
       *   const Atom *a = static_cast<const Atom *>(p);
       *
       *   // Render the selection highlight
       *   if (pd->isSelected(a)) {
       *     map->set(0.3, 0.6, 1.0, 0.7);
       *     glEnable(GL_BLEND);
       *     pd->painter()->setColor(map);
       *     pd->painter()->setName(a);
       *     pd->painter()->drawSphere(a->pos(), 0.1 + radius( a ));
       *     glDisable(GL_BLEND);
       *   }
       * }
       * @endcode
       *
       * @note To allow picking to happen you need to push the object type and name.
       * If objects cannot be picked this may be omitted.
       *
       * For more information on the various primitive lists available see
       * PrimitiveList.
       *
       */
      virtual bool renderTransparent(PainterDevice *) { return true; }

      /**
       * Render all elements the engine is responsible for quickly with an
       * emphasis on maintaining interactivity even with very large molecules.
       * By default this function calls renderOpaque but in most cases should
       * be implemented and tuned using large molecule test cases. The GLWidget
       * ensures dynamic scaling of geometric primitives is off before calling
       * this rendering function.
       */
      virtual bool renderQuick(PainterDevice *pd)
      {
        renderOpaque(pd);
        return true;
      }

      /**
       * @return the engine's PrimitiveList containing all primitives the engine
       * can render.
       */
      PrimitiveList primitives() const;

      /**
       * Set the primitives that the engine instance can render.
       * @param primitives the PrimitiveList the engine can render from.
       */
      virtual void setPrimitives(const PrimitiveList &primitives);

      /**
       * Clear the primitives of the engine instance.
       */
      virtual void clearPrimitives();

      /** Get the radius of the primitive referred to.
       * @param pd is the painter device used for rendering (e.g., if a primitive is selected)
       * @param primitive is the Primitive to get the radius of.
       * @return the radius of the Primitive.
       */
      virtual double radius(const PainterDevice *pd, const Primitive *primitive = 0) const;

      /**
       * @return transparency level, rendered low to high.
       */
      virtual double transparencyDepth() const;

      /**
       * @return true if the engine is enabled or false if it is not.
       */
      bool isEnabled() const;

      /**
       * Setter to enable or disable the engine instance.
       * @param enabled true to enable the egine, false to disable the engine.
       */
      void setEnabled(bool enabled);

      /**
       * @return a QWidget containing the engine settings or 0
       * if no settings widget is available.
       */
      virtual QWidget *settingsWidget();

      /**
       * @return a pointer to an identical engine or 0 if this fails
       */
      virtual Engine *clone() const = 0;

      /**
       * Write the engine settings so that they can be saved between sessions.
       */
      virtual void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the engine instance.
       */
      virtual void readSettings(QSettings &settings);

    Q_SIGNALS:
      /**
       * Signals that something has been changed and the engine needs to render
       * after these changes were made.
       */
      void changed();

    public Q_SLOTS:
      /**
       * Add the primitive to the engines PrimitiveList.
       * @param primitive to be added to the PrimitiveList.
       */
      virtual void addPrimitive(Primitive *primitive);

      /**
       * Update the primitive in the engines PrimitiveList.
       * @param primitive to be updated in the PrimitiveList.
       */
      virtual void updatePrimitive(Primitive *primitive);

      /**
       * Remove the primitive to from the engines PrimitiveList.
       * @param primitive to be removed from the PrimitiveList.
       */
      virtual void removePrimitive(Primitive *primitive);

      /** Set the color map to be used for this engine.
       * The default is to color each atom by element.
       * @param map is the new colors to be used
       */
      virtual void setColorMap(Color *map);

      /** @return the current color map used by this engine
       */
      virtual Color *colorMap();

    private:
      EnginePrivate *const d;
  };

} // end namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Engine*)
Q_DECLARE_OPERATORS_FOR_FLAGS(Avogadro::Engine::EngineFlags)

#endif
