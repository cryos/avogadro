namespace Avogadro {

class ToolGroup : QObject
{
%TypeHeaderCode
#include <avogadro/toolgroup.h>
%End
    
    public:
      ToolGroup(QObject *parent = 0);
      ~ToolGroup();
      //void load();
      void append(QList<Avogadro::Tool *> tools);
      void append(Avogadro::Tool *tool);


      Avogadro::Tool* activeTool() const;
      Avogadro::Tool* tool(int i) const;
      const QList<Avogadro::Tool *>& tools() const;
      const QActionGroup * activateActions() const;

    public slots:
      void setActiveTool(int i);
      void setActiveTool(const QString& name);
      void setActiveTool(Avogadro::Tool *tool);
      void setMolecule(Avogadro::Molecule *molecule);
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);

    signals:
      void toolActivated(Avogadro::Tool *tool);

};

}; // end namespace Avogadro
