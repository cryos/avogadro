
#ifndef MYEXTENSION_H
#define MYEXTENSION_H

#include <avogadro/extension.h>
#include <avogadro/primitivelist.h>
#include <QUndoCommand>

namespace Avogadro {

 class MyExtension : public Extension
  {
    Q_OBJECT
      AVOGADRO_EXTENSION("MyExtension", tr("MyExtension"),
                         tr("Transform nothing to nothing"))

    public:
      //! Constructor
      MyExtension(QObject *parent=0);
      //! Destructor
      virtual ~MyExtension();

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      virtual QString menuPath(QAction *action) const;
      //@}

    private:
      QList<QAction *> m_actions;

      Molecule *m_molecule;
  };

  class MyExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(MyExtension)
  };


} // end namespace Avogadro

#endif
