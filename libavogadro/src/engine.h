/**********************************************************************
  Engine - Engine interface and plugin factory.

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Donald Ephraim Curtis

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

#ifndef __ENGINE_H
#define __ENGINE_H

#include <avogadro/global.h>
#include <avogadro/primitive.h>

#include <QString>
#include <QVector>
#include <QList>

namespace Avogadro {
  class GLWidget;

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
      //! constructor
      Engine(QObject *parent = 0);
      //! deconstructor
      virtual ~Engine(); 

      /** 
       * @return the short name of the engine as a QString
       * (ie. "FooBar")
       */
      virtual QString name() = 0;
      /**
       * @return a simple description of the engine
       * (ie. "Rendering Atoms and Bonds using FooBar style.")
       */
      virtual QString description() = 0;

      /**
       * Render a PrimitiveQueue.  This function is allowed to rendering 
       * whatever primitives it wishes.  There is no requirement that it
       * render every primitive.
       *
       * @param queue This parameter is of type PrimitiveQueue which
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
       * Engine::render(const PrimitiveQueue *queue)
       * {
       *   const QList<Primitive *> *list;
       *
       *   glPushName(Primitive::AtomType);
       *   list = q->primitiveList(Primitive::AtomType);
       *   for( int i=0; i<list->size(); i++ ) {
       *     Bond *atom = (*list)[i];
       *     glPushName(atom->GetIdx());
       *     *** render atom ***
       *     glPopName();
       *   }
       *
       *   glPushName(Primitive::BondType);
       *   list = q->primitiveList(Primitive::BondType);
       *   for( int i=0; i<list->size(); i++ ) {
       *     Bond *bond = (*list)[i];
       *     glPushName(bond->GetIdx());
       *     *** render bond ***
       *     glPopName();
       *   }
       *   glPopName();
       *
       *   return true;
       * }
       * @endcode
       *
       * @note To allow picking to happen you need to push the object type and name.
       * If objects cannot be picked this may be omitted.
       *
       * For more information on the various primitive lists available see
       * PrimitiveQueue.
       * 
       */
      virtual bool render(GLWidget *gl) = 0;

      const PrimitiveQueue& queue() const;

      /** Get the radius of the primitive referred to.
       * @param primitive is the Primitive to get the radius of
       * @return the radius of the Primitive
       */
      virtual double radius(const Primitive *primitive = 0);

      void clearQueue();
      bool isEnabled();
      void setEnabled(bool enabled);

    public Q_SLOTS:
      virtual void addPrimitive(Primitive *primitive);
      virtual void updatePrimitive(Primitive *primitive);
      virtual void removePrimitive(Primitive *primitive);

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

Q_DECLARE_METATYPE(Avogadro::Engine*);
Q_DECLARE_INTERFACE(Avogadro::EngineFactory, "net.sourceforge.avogadro.enginefactory/1.0");

#endif
