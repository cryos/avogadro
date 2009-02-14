#ifndef PYTHON_MOLECULE_H
#define PYTHON_MOLECULE_H

#include <QObject>
#include <avogadro/molecule.h>

namespace Avogadro {

class MoleculeList : public QObject
{
  Q_OBJECT

  public:
    MoleculeList() : QObject(0)
    {
    }
    
    MoleculeList(const MoleculeList &other) : QObject(0)
    {
      m_molecules = other.m_molecules;
    }
    
    Molecule* addMolecule();

    Molecule* at(int index)
    {
      if ((index >= 0) && (index < m_molecules.size()))
        return m_molecules[index];
      return 0;
    }

    int numMolecules()
    {
      return m_molecules.size();
    }

    static MoleculeList* instance();
  public Q_SLOTS: 
    void moleculeDestroyed();

  private:
    QList<Molecule*> m_molecules;
};
   
}

#endif
