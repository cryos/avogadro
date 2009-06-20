// Last update: timvdm 18 June 2009
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>

#include <avogadro/animation.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Animation()
{
  
  class_<Avogadro::Animation, boost::noncopyable>("Animation", "Basic animation interface")
    .def("setMolecule", &Animation::setMolecule, "Set the molecule to animate.")
    .def("setFrames", &Animation::setFrames,
        "Set the frames for the animation. By default, the conformers in the "
        "molecule are used as animation frames. However, for trajectory files "
        "that don't contain any topology, it is needed to read in the the "
        "molecule topology before the trajectory. The trajectory frames can "
        "be used to call setFrames() later.") // @todo unit test conversion for argument
    .add_property("fps", &Animation::fps, &Animation::setFps, "The number of frames per second.")
    .add_property("loopCount", &Animation::loopCount, &Animation::setLoopCount, 
        "The number of loops (0 = repeat forever).")
    .add_property("numFrames", &Animation::numFrames, "The total number of frames in the animation.")
    .add_property("dynamicBonds", &Animation::dynamicBonds, &Animation::setDynamicBonds, 
        "True if dynamic bond detection is enabled.")
    .def("setFrame", &Animation::setFrame, "Set the current frame.")
    .def("start", &Animation::start, "Start the animation (at current frame).")
    .def("pause", &Animation::pause, "Pause the animation.")
    .def("stop", &Animation::stop, "Stop the animation (and return to the first frame).")
    ;

}
