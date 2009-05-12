// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/periodictableview.h>

using namespace boost::python;
using namespace Avogadro;

void export_PeriodicTableView()
{ 
 
  class_<Avogadro::PeriodicTableView, boost::noncopyable>("PeriodicTableView")
    .def(init<QWidget*>())
    ;

}
