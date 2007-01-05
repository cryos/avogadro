/**********************************************************************
  Primitives - Wrapper class around the OpenBabel classes

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "primitives.moc"

using namespace Avogadro;

Atom * Molecule::CreateAtom()
{
  std::cout << "Molecule::CreateAtom" << std::endl;
  Atom *atom = new Atom();
  return(atom);
}

Bond * Molecule::CreateBond()
{
  Bond *bond = new Bond();
  return(bond);
}

Residue * Molecule::CreateResidue()
{
  Residue *residue = new Residue();
  return(residue);
}

//dc: Atom * Molecule::NewAtom()
//dc: {
//dc:   Atom *atom = (Atom *) OBMol::NewAtom();
//dc: //  emit atomAdded(atom);
//dc:   return(atom);
//dc: }
//dc: 
//dc: Bond * Molecule::NewBond()
//dc: {
//dc:   Bond *bond = (Bond *)OBMol::NewBond();
//dc: //  emit bondAdded(bond);
//dc:   return(bond);
//dc: }
//dc: 
//dc: Residue * Molecule::NewResidue()
//dc: {
//dc:   Residue *residue = (Residue *)OBMol::NewResidue();
//dc: // emit residueAdded(residue);
//dc:   return(residue);
//dc: }

//X void Primitive::setEngine(Engine *r)
//X {
//X   renderer = r; 
//X }
//X 
//X Engine *Primitive::getEngine()
//X {
//X   if( renderer == NULL )
//X   {
//X     return NULL;
//X   }
//X   else
//X   {
//X     return renderer;
//X   }
//X }
//X 
//X void Atom::render()
//X {
//X   MainWindow *w = ((Molecule *)GetParent())->getWindow();
//X   w->defaultEngine->renderAtom(*this);
//X }
//X 
//X void Bond::render()
//X {
//X   // MainWindow::defaultEngine->renderBond(*this);
//X }
//X 
//X void Molecule::render()
//X {
//X   vector<OpenBabel::OBNodeBase*>::iterator i;
//X   Atom *atom;
//X 
//X   for(atom = (Atom*)BeginAtom(i); atom; atom = (Atom*)NextAtom(i))
//X   {
//X     atom->render();
//X   }
//X }
