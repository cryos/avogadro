#include "drawcommand.h"
#include <avogadro/glwidget.h>
#include <avogadro/primitive.h>

namespace Avogadro {

  class AddAtomDrawCommandPrivate {
    public:
      AddAtomDrawCommandPrivate() : widget(0), index(0) {};

      GLWidget *widget;
      Eigen::Vector3d pos;
      int index;
      unsigned int element;
  };

  AddAtomDrawCommand::AddAtomDrawCommand(GLWidget *widget, const Eigen::Vector3d& pos, unsigned int element) : d(new AddAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Add Atom"));
    d->widget = widget;
    d->pos = pos;
    d->element = element;
  }

  AddAtomDrawCommand::AddAtomDrawCommand(GLWidget *widget, Atom *atom) : d(new AddAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Add Atom"));
    d->widget = widget;
    d->pos = atom->pos();
    d->element = atom->GetAtomicNum();
    d->index = atom->GetIdx();
  }

  AddAtomDrawCommand::~AddAtomDrawCommand()
  {
    delete d;
  }

  void AddAtomDrawCommand::undo()
  {
    Molecule *molecule = d->widget->molecule();
    if(!molecule) { 
      return;
    }

    OpenBabel::OBAtom *atom = molecule->GetAtom(d->index);

    if(atom)
    {
      molecule->BeginModify();
      molecule->DeleteAtom(atom);
      molecule->EndModify();
      d->widget->updateGeometry();
      d->widget->update();
      d->index = -1;
    }
  }

  void AddAtomDrawCommand::redo()
  {
    if(d->index >= 0) {
      return;
    }

    Molecule *molecule = d->widget->molecule();
    if(!molecule) { 
      return;
    }

    molecule->BeginModify();
    Atom *atom = static_cast<Atom*>(molecule->NewAtom());
    d->index = atom->GetIdx();
    atom->setPos(d->pos);
    atom->SetAtomicNum(d->element);
    molecule->EndModify();
    atom->update();
    d->widget->updateGeometry();
    d->widget->update();
  }

  class DeleteAtomDrawCommandPrivate {
    public:
      DeleteAtomDrawCommandPrivate() : index(-1) {};

      GLWidget *widget;
      Molecule molecule;
      int index;
  };

  DeleteAtomDrawCommand::DeleteAtomDrawCommand(GLWidget *widget, int index) : d(new DeleteAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Atom"));
    d->widget = widget;
    d->molecule = (*(widget->molecule()));
    d->index = index;
  }

  DeleteAtomDrawCommand::~DeleteAtomDrawCommand() 
  {
    delete d;
  }

  void DeleteAtomDrawCommand::undo()
  {
    Molecule *molecule = d->widget->molecule();
    *molecule = d->molecule;
    d->widget->updateGeometry();
    molecule->update();
  }

  void DeleteAtomDrawCommand::redo()
  {
    Molecule *molecule = d->widget->molecule();

    OpenBabel::OBAtom *atom = molecule->GetAtom(d->index);
    if(atom)
    {
      molecule->DeleteAtom(atom);
      d->widget->updateGeometry();
      molecule->update();
    }
  }

  class AddBondDrawCommandPrivate {
    public:
      AddBondDrawCommandPrivate() : widget(0), index(-1) {};

      GLWidget *widget;
      Eigen::Vector3d pos;
      int index;
      int beginAtomIndex;
      int endAtomIndex;
      unsigned int order;
  };

  AddBondDrawCommand::AddBondDrawCommand(GLWidget *widget, Atom *beginAtom, Atom *endAtom, unsigned int order) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->widget = widget;
    d->beginAtomIndex = beginAtom->GetIdx();
    d->endAtomIndex = endAtom->GetIdx();
    d->order = order;
  }

  AddBondDrawCommand::AddBondDrawCommand(GLWidget *widget, Bond *bond) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->widget = widget;
    d->beginAtomIndex = bond->GetBeginAtomIdx();
    d->endAtomIndex = bond->GetEndAtomIdx();
    d->order = bond->GetBondOrder();
    d->index = bond->GetIdx();
  }

  AddBondDrawCommand::~AddBondDrawCommand()
  {
    delete d;
  }

  void AddBondDrawCommand::undo()
  {
    Molecule *molecule = d->widget->molecule();
    if(!molecule) { 
      return;
    }

    OpenBabel::OBBond *bond = molecule->GetBond(d->index);

    if(bond)
    {
      molecule->BeginModify();
      molecule->DeleteBond(bond);
      molecule->EndModify();
      d->widget->updateGeometry();
      molecule->update();
      d->index = -1;
    }
  }

  void AddBondDrawCommand::redo()
  {
    if(d->index >= 0) {
      return;
    }

    Molecule *molecule = d->widget->molecule();
    if(!molecule) { 
      return;
    }

    OpenBabel::OBAtom *beginAtom = molecule->GetAtom(d->beginAtomIndex);
    OpenBabel::OBAtom *endAtom = molecule->GetAtom(d->endAtomIndex);
    if(!beginAtom || !endAtom) {
      return;
    }

    molecule->BeginModify();
    Bond *bond = static_cast<Bond *>(molecule->NewBond());
    d->index = bond->GetIdx();
    bond->SetBondOrder(d->order);
    bond->SetBegin(beginAtom);
    bond->SetEnd(endAtom);
    beginAtom->AddBond(bond);
    endAtom->AddBond(bond);
    molecule->EndModify();
    d->widget->updateGeometry();
    molecule->update();
  }

} // end namespace Avogadro
