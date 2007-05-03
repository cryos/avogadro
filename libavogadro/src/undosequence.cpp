#include <avogadro/undosequence.h>

namespace Avogadro {

  class UndoSequencePrivate {
    public:
      UndoSequencePrivate()  {};

      QList<QUndoCommand *> commands;
  };

  UndoSequence::UndoSequence() : d(new UndoSequencePrivate)
  {
  }

  UndoSequence::~UndoSequence()
  {
    while(!d->commands.isEmpty()) {
      delete d->commands.takeFirst();
    }
    delete d;
  }

  void UndoSequence::undo()
  {
    // last in first to undo
    for(int i=d->commands.count()-1; i >= 0; i--)
    {
      d->commands.at(i)->undo();
    }
  }
  
  void UndoSequence::redo()
  {
    foreach(QUndoCommand *command, d->commands)
    {
      command->redo();
    }
  }

  void UndoSequence::append(QUndoCommand *command)
  {
    d->commands.append(command);
  }

} // end namespace Avogadro
