// Last update: timvdm 18 June 2009
#include <boost/python.hpp>

#include <Eigen/Core>

#include <avogadro/engine.h>
#include <avogadro/color.h>
#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

using namespace boost::python;
using namespace Avogadro;

void export_Engine()
{
  
  enum_<Engine::Layers>("Layers")
    // the Type enum
    .value("Opaque", Engine::Opaque)
    .value("Transparent", Engine::Transparent)
    .value("Overlay", Engine::Overlay)
    ;
    
  enum_<Engine::PrimitiveTypes>("PrimitiveTypes")
    .value("NoPrimitives", Engine::NoPrimitives)
    .value("Atoms", Engine::Atoms)
    .value("Bonds", Engine::Bonds)
    .value("Molecules", Engine::Molecules)
    .value("Surfaces", Engine::Surfaces)
    .value("Fragments", Engine::Fragments)
    ;
 
  enum_<Engine::ColorTypes>("ColorTypes")
    .value("NoColors", Engine::NoColors)
    .value("ColorPlugins", Engine::ColorPlugins)
    .value("IndexedColors", Engine::IndexedColors)
    .value("ColorGradients", Engine::ColorGradients)
    ;
      
  void (Engine::*setMolecule_ptr)(Molecule*molecule) = &Engine::setMolecule;
    
  class_<Avogadro::Engine, bases<Avogadro::Plugin>, boost::noncopyable>("Engine", no_init)
    //
    // read-only poperties 
    //
    .add_property("typeName", 
        &Engine::typeName, 
        "Type Name (Engines)")

    .add_property("layers", 
        &Engine::layers, 
        "The layers used by this engine.")

    .add_property("hasSettings", 
        &Engine::hasSettings,
        "This method can provide a much faster way of determining if an Engine "
        "has a configuration dialog, it should be reimplemented in classes to "
        "avoid the settings wigdet being constructed in order to check whether "
        "an engine has settings.")

    .add_property("primitiveTypes", 
        &Engine::primitiveTypes, 
        "The primitive types used by this engine.")

    .add_property("colorTypes", 
        &Engine::colorTypes, 
        "The color schemes used by this engine.")

    .add_property("transparencyDepth", 
        &Engine::transparencyDepth, 
        "Transparency level, rendered low to high.")

    .add_property("atoms", 
        &Engine::atoms, 
        "The engine's Atom list containing all atoms the engine can render.")

    .add_property("bonds", 
        &Engine::bonds, 
        "The engine's Bond list containing all bonds the engine can render.")

    // read/write properties
    .add_property("alias", 
        &Engine::alias, 
        &Engine::setAlias, 
        "The alias of the engine.")

    .add_property("shader", 
        &Engine::shader, 
        &Engine::setShader, 
        "The active GLSL shader for this engine, only used if GLSL is supported.")

    .add_property("molecule", 
        make_function(&Engine::molecule, return_value_policy<reference_existing_object>()),
        setMolecule_ptr, 
        "the Molecule for the engine.")

    .add_property("primitives", 
        &Engine::primitives, 
        &Engine::setPrimitives, 
        "The engine's PrimitiveList containing all primitives the engine "
        "can render.")

    .add_property("enabled", 
        &Engine::isEnabled, 
        &Engine::setEnabled,
        "True if the engine is enabled or false if it is not.")

    .add_property("colorMap", 
        make_function(&Engine::colorMap, return_value_policy<reference_existing_object>()),
        &Engine::setColorMap, 
        "The current color map used by this engine")

    //
    // real functions
    //
    .def("clearPrimitives", 
        &Engine::clearPrimitives, 
        "Clear the primitives of the engine instance.")

    .def("addPrimitive", 
        &Engine::addPrimitive, 
        "Add the primitive to the engines PrimitiveList.")

    .def("addAtom", 
        &Engine::addAtom, 
        "Add the Atom to the engines PrimitiveList.")

    .def("addBond", 
        &Engine::addBond, 
        "Add the Bond to the engines PrimitiveList.")

    .def("updatePrimitive", 
        &Engine::updatePrimitive, 
        "Update the primitive in the engines PrimitiveList.")

    .def("removePrimitive", 
        &Engine::removePrimitive, 
        "Remove the primitive from from the engines PrimitiveList.")

    .def("removeAtom", 
        &Engine::removeAtom, 
        "Remove the Atom from from the engines PrimitiveList.")

    .def("removeBond", 
        &Engine::removeBond, 
        "Remove the Bond from from the engines PrimitiveList.")

    .def("colorMapChanged", 
        &Engine::colorMapChanged, 
        "Accept that the color map changed and call for an update (e.g., the user modified the settings)")

    .def("clone", 
        &Engine::clone, return_value_policy<manage_new_object>(), 
        "Get an identical copy of this engine or None if this fails")
    ;
   
}
