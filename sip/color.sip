namespace Avogadro {

class Color
{
%TypeHeaderCode
#include <avogadro/color.h>
%End

  public:
    Color();
    virtual ~Color();
    Color( float red, float green, float blue, float alpha = 1.0 );
    Color( const Avogadro::Primitive *p);
    // Color& operator= (const QColor &color);
    virtual void set(float red, float green, float blue, float alpha = 1.0 );
    virtual void setToSelectionColor();
    virtual void set(const Avogadro::Primitive *p);
    virtual void setAlpha(double alpha);
    virtual void apply();
    virtual void applyAsMaterials();
    virtual void applyAsFlatMaterials();
    QColor color();
    float red();
    float green();
    float blue(); 
    float alpha();
    virtual void setName(const QString& name);
    virtual QString name() const;
    virtual QString type() const;
    virtual QWidget *settingsWidget();

};

};
