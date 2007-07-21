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

#ifndef __ENGINE_H
#define __ENGINE_H

#include <avogadro/global.h>
#include <avogadro/primitive.h>
#include <avogadro/primitivelist.h>
#include <avogadro/color.h>
#include "painterdevice.h"

#include <QString>
#include <QVector>
#include <QList>

namespace Avogadro {

  /**
   * @class Engine
   * @brief Engine plugin interface.
   * @author Donald Ephraim Curtis
   *
   * This class provides a an interface for our engines.
   * Subclasses of this class are loaded by the GLWidget and used to render
   * different parts of our project (Molecule, Atom, Bond, Residue) depending
   * on what style of engine we're implementing.
   * \sa render()
   */
  class EnginePrivate;
  class A_EXPORT Engine : public QObject
  {
    Q_OBJECT

    public:
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
      //! constructor
      Engine(QObject *parent = 0);
      //! deconstructor
      virtual ~Engine();

      /**
       * @return the default name for the engine
       */
      QString name() const;

      /**
       * @param name the new name for the engine instance
       */
      void setName(const QString &name);

      /**
       * @return engine description
       */
      QString description() const;

      /**
       * @param description the new description for this engine
       */
      void setDescription(const QString &description);

      /**
       * @return the flags for this engine
       */
      virtual EngineFlags flags() const;

      /**
       * Render a PrimitiveList.  This function is allowed to rendering
       * whatever primitives it wishes.  There is no requirement that it
       * render every primitive.
       *
       * @param queue This parameter is of type PrimitiveList which
       * provides an organized list of Primitive* objects.  During
       * generation of the GL view engines will have their render
       * function called at most once.  It is the responsibility
       * of the engine to render all of the objects in it's queue if
       * it can.
       *
       * @return @c true if the rendering was completed successfully, @c false otherwise
       *
       * Example
       * @code
       * FIXME!
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
      virtual bool renderTransparent(PainterDevice *) { return true; }

      PrimitiveList primitives() const;

      virtual void setPrimitives(const PrimitiveList &primitives);
      virtual void clearPrimitives();

      /** Get the radius of the primitive referred to.
       * @param primitive is the Primitive to get the radius of
       * @return the radius of the Primitive
       */
      virtual double radius(const PainterDevice *pd, const Primitive *primitive = 0) const;

      /**
       * @return transparency level, rendered low to high
       */
      virtual double transparencyDepth() const;

      bool isEnabled() const;
      void setEnabled(bool enabled);

      /**
       * @return a QWidget containing the engine settings or 0
       * if no settings widget is available
       */
      virtual QWidget *settingsWidget();

    Q_SIGNALS:
      void changed();

    public Q_SLOTS:
      virtual void addPrimitive(Primitive *primitive);
      virtual void updatePrimitive(Primitive *primitive);
      virtual void removePrimitive(Primitive *primitive);

      /** Set the color map to be used for this engine.
       * default is to color each atom by element
       * @param map is the new colors to be used
       */
      virtual void setColorMap(Color &map);

      /** @return the current color map used by this engine
       */
      virtual Color &colorMap();

    private:
      EnginePrivate *const d;
  };

  /**
   * @class EngineFactory
   * @warning This function needs to be looked at again.  It was originally designed
   * so that for a single thread we could have multiple rendering engines.  We have
   * decided that each molecule will have it's own window which will be it's own thread.
   * However, this style of plugin creation may still be needed as we might want to have
   * multiple views of the same molecule.
   *
   * This class is used to generate new instances of the Engine class for which
   * it is defined.
   */
  class A_EXPORT EngineFactory
  {
    public:
      /**
       * Engine factory deconstructor.
       */
      virtual ~EngineFactory() {}

      /**
       * @return pointer to a new instance of an Engine subclass object
       */
      virtual Engine *createInstance(QObject *parent=0) = 0;


  };


} // end namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Engine*)
Q_DECLARE_INTERFACE(Avogadro::EngineFactory, "net.sourceforge.avogadro.enginefactory/1.0")
Q_DECLARE_OPERATORS_FOR_FLAGS(Avogadro::Engine::EngineFlags)

#endif
