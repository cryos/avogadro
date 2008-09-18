namespace Avogadro {

class PluginManager : QObject
{
%TypeHeaderCode
#include <avogadro/pluginmanager.h>
%End

  public:
    PluginManager(QObject *parent = 0);
    ~PluginManager();

    static void loadFactories();
    static QList<Avogadro::PluginFactory *> factories( int type );
    static Avogadro::PluginFactory *factory(const QString &name, Avogadro::Plugin::Type type);
    //QList<Avogadro::Extension *> extensions(QObject *parent=0) const;
    QList<Avogadro::Tool *> tools(QObject *parent=0) const;
    QList<Avogadro::Color *> colors(QObject *parent=0) const;
    static void writeSettings(QSettings &settings);
  
  public slots:
    void showDialog();
};


};
