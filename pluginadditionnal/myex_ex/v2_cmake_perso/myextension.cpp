
#include "myextension.h"
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/primitivelist.h>

#include <openbabel/mol.h>
#include <openbabel/obiter.h>

#include <QAction>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  MyExtension::MyExtension(QObject *parent) : Extension(parent),
    m_molecule(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Nothing to nothing"));
    m_actions.append(action);

    action = new QAction( this );
    action->setSeparator(true);
    m_actions.append( action );
  }

  MyExtension::~MyExtension()
  {
  }

  QList<QAction *> MyExtension::actions() const
  {
    return m_actions;
  }

  QString MyExtension::menuPath(QAction *) const
  {
    return tr("&Construction");
  }

  QUndoCommand* MyExtension::performAction( QAction *, GLWidget *widget )
  {
    QUndoCommand *undo = 0;
		emit message( "Test signal !" ) ;

		printf( "printf ... ??? \n" ) ;
    return undo;
  }


} // end namespace Avogadro

//#include "myextension.moc"

Q_EXPORT_PLUGIN2(MyExtension, Avogadro::MyExtensionFactory)

