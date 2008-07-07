namespace Avogadro {

class Engine : QObject
{
%TypeHeaderCode
#include <avogadro/engine.h>
%End

    public:
      /*
      enum EngineFlag {
        NoFlags = 0x00, /// no flags
        Transparent = 0x01, /// renders transparency
        Overlay = 0x02, /// renders overlay
        Bonds = 0x04, /// renders bonds
        Atoms = 0x08, /// renders atoms
        Molecules = 0x10 /// renders molecules
      };
      */

      Engine(QObject *parent = 0);
      virtual ~Engine();
      QString name() const;
      void setName(const QString &name);
      QString description() const;
      void setDescription(const QString &description);
      // virtual Avogadro::Engine::EngineFlags flags() const;
      virtual QString type() const = 0;
      virtual bool renderOpaque(Avogadro::PainterDevice *pd) = 0;
      virtual bool renderTransparent(Avogadro::PainterDevice *);
      virtual bool renderQuick(Avogadro::PainterDevice *pd);
      Avogadro::PrimitiveList primitives() const;
      virtual void setPrimitives(const Avogadro::PrimitiveList &primitives);
      virtual void clearPrimitives();
      virtual double radius(const Avogadro::PainterDevice *pd, const Avogadro::Primitive *primitive = 0) const;
      virtual double transparencyDepth() const;
      bool isEnabled() const;
      void setEnabled(bool enabled);
      virtual QWidget *settingsWidget();
      virtual Avogadro::Engine *clone() const = 0;
      virtual void writeSettings(QSettings &settings) const;
      virtual void readSettings(QSettings &settings);

    signals:
      void changed();

    public slots:
      virtual void addPrimitive(Avogadro::Primitive *primitive);
      virtual void updatePrimitive(Avogadro::Primitive *primitive);
      virtual void removePrimitive(Avogadro::Primitive *primitive);
      virtual void setColorMap(Avogadro::Color *map);
      virtual Avogadro::Color *colorMap();

};

/*
  class A_EXPORT EngineFactory
  {
    public:
      virtual ~EngineFactory() {}
      virtual Engine *createInstance(QObject *parent=0) = 0;
      virtual QString className() = 0;
      virtual QString type() = 0;

  };
*/

}; // end namespace Avogadro
