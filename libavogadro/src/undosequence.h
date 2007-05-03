#ifndef __UNDOSEQUENCE_H
#define __UNDOSEQUENCE_H

#include <QUndoCommand>

namespace Avogadro {

  class UndoSequencePrivate;
  class UndoSequence : public QUndoCommand
  {
    public:
      UndoSequence();
      ~UndoSequence();

      void append(QUndoCommand *command);

      virtual void undo();
      virtual void redo();

    private:
      UndoSequencePrivate * const d;
  };

} // end namespace Avogadro

#endif
