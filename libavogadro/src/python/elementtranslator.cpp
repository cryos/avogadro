// Last update: timvdm 18 June 2009
#include <boost/python.hpp>

#include <qperiodictable/elementtranslator.h>

using namespace boost::python;
using namespace Avogadro;

void export_ElementTranslator()
{
  
  class_<Avogadro::ElementTranslator, boost::noncopyable>("ElementTranslator", no_init)
    //
    // real functions
    //
    .def("name", 
        &ElementTranslator::name, 
        "Translate element names.")
    .staticmethod("name")
    ;

}
