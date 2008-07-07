namespace Avogadro {

class Tool : QObject
{
%TypeHeaderCode
#include <avogadro/tool.h>
%End

    public:
      Tool(QObject *parent = 0);
      virtual ~Tool();
      virtual QString name() const = 0;
      virtual QString description() const;
      virtual QAction* activateAction() const;
      virtual QWidget* settingsWidget();
      virtual QUndoCommand* mousePress(Avogadro::GLWidget *widget, const QMouseEvent *event) = 0;
      virtual QUndoCommand* mouseRelease(Avogadro::GLWidget *widget, const QMouseEvent *event) = 0;
      virtual QUndoCommand* mouseMove(Avogadro::GLWidget *widget, const QMouseEvent *event) = 0;
      virtual QUndoCommand* wheel(Avogadro::GLWidget *widget, const QWheelEvent *event) = 0;
      virtual bool paint(Avogadro::GLWidget *widget);
      virtual int usefulness() const;
      bool operator<(const Avogadro::Tool &other) const;
      virtual void writeSettings(QSettings &settings) const;
      virtual void readSettings(QSettings &settings);

    signals:
      void message(const QString &m);

    public slots:
      virtual void setMolecule(Avogadro::Molecule *molecule);
};

/*
  class ToolFactory
  {
    public:
      virtual ~ToolFactory() {}
      virtual Tool *createInstance(QObject *parent=0) = 0;
  };
*/

}; // end namespace Avogadro

