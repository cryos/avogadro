namespace Avogadro {

class PainterDevice
{
%TypeHeaderCode
#include <avogadro/painterdevice.h>
%End

  public:
    PainterDevice();
    virtual ~PainterDevice();

    virtual Avogadro::Painter *painter() const = 0;
    virtual Avogadro::Camera *camera() const = 0;
    virtual bool isSelected( const Avogadro::Primitive *p ) const = 0;
    virtual double radius( const Avogadro::Primitive *p ) const = 0;
    virtual const Avogadro::Molecule *molecule() const = 0;
    virtual Avogadro::Color* colorMap() const = 0;

    virtual int width() = 0;
    virtual int height() = 0;
};

}; // End namespace Avogadro
