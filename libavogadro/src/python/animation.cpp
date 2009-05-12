// Last update: timvdm 12 May 2009

//#include <Python.h>
// http://www.boost.org/doc/libs/1_39_0/libs/python/doc/building.html#include-issues
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>

#include <avogadro/animation.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Animation()
{
  
  class_<Avogadro::Animation, boost::noncopyable>("Animation")
    .def("setFrames", &Animation::setFrames)
    .def("setMolecule", &Animation::setMolecule)
    .def("setDuration", &Animation::setDuration)
    .def("setLoopCount", &Animation::setLoopCount)
    .def("setFrame", &Animation::setFrame)
    .def("start", &Animation::start)
    .def("stop", &Animation::stop)
    ;

}
