#ifndef __DRAWCOMMAND_H
#define __DRAWCOMMAND_H

#include <QUndoCommand>
#include <eigen/vector.h>

namespace Avogadro {

  class GLWidget;
  class Atom;
  class Bond;

  class AddAtomDrawCommandPrivate;
  class AddAtomDrawCommand : public QUndoCommand
  {
    public:
      AddAtomDrawCommand(GLWidget *widget, const Eigen::Vector3d& pos, unsigned int element);
      AddAtomDrawCommand(GLWidget *widget, Atom *atom);
      ~AddAtomDrawCommand();

      virtual void undo();
      virtual void redo();

    private:
      AddAtomDrawCommandPrivate * const d;
  };

  class DeleteAtomDrawCommandPrivate;
  class DeleteAtomDrawCommand : public QUndoCommand
  {
    public:
      DeleteAtomDrawCommand(GLWidget *widget, int index);
      ~DeleteAtomDrawCommand();

      virtual void undo();
      virtual void redo();

    private:
      DeleteAtomDrawCommandPrivate * const d;
  };

  class AddBondDrawCommandPrivate;
  class AddBondDrawCommand : public QUndoCommand
  {
    public:
      AddBondDrawCommand(GLWidget *widget, Atom *beginAtom, Atom *endAtom, unsigned int order);
      AddBondDrawCommand(GLWidget *widget, Bond *bond);
      ~AddBondDrawCommand();

      virtual void undo();
      virtual void redo();

    private:
      AddBondDrawCommandPrivate * const d;
  };

} // end namespace Avogadro

#endif
