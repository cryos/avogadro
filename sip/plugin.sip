namespace Avogadro {

class Plugin
{
%TypeHeaderCode
#include <avogadro/plugin.h>
%End

  public:
    enum Type 
    {
      EngineType = 0,
      ToolType,
      ExtensionType,
      ColorType,
      GradientType,
      OtherType,
      TypeCount // this needs to be last always
    };
 
  public:
    Plugin();
    virtual ~Plugin();
    virtual Avogadro::Plugin::Type type() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const;
};

class PluginFactory
{
%TypeHeaderCode
#include <avogadro/plugin.h>
%End

  public:
    virtual ~PluginFactory();
    virtual Avogadro::Plugin *createInstance(QObject *parent=0) = 0;
    virtual Avogadro::Plugin::Type type() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
};

}; // end namespace Avogadro

