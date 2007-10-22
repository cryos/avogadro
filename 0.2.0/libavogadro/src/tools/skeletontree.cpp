/**********************************************************************
  BondCentricTool - Bond Centric Manipulation Tool for Avogadro

  Copyright (C) 2007 by Shahzad Ali
  Copyright (C) 2007 by Ross Braithwaite
  Copyright (C) 2007 by James Bunt

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "skeletontree.h"

using namespace Avogadro;
using namespace Eigen;
using namespace OpenBabel;
using namespace std;

// ################################## Node #####################################

// ##########  Constructor  ##########

Node::Node(Atom *atom)
{
  m_atom = atom;
}

// ##########  Destructor  ##########

Node::~Node() {}

// ##########  atom  ##########

Atom* Node::atom()
{
  return m_atom;
}

// ##########  nodes  ##########

QList<Node*> *Node::nodes()
{
  return &m_nodes;
}

// ##########  isLeaf  ##########

bool Node::isLeaf()
{
  return m_nodes.isEmpty();
}

// ##########  containsAtom  ##########

bool Node::containsAtom(Atom* atom)
{
  //"atom" exist in the children and grandchildren... of this node.?
  bool exists = false;
  if (m_atom == atom) {
    return true;
  }

  for (int i = 0; i < m_nodes.size(); i++)
  {
    Node* n = m_nodes.at(i);
    if (n->containsAtom(atom))
    {
      exists = true;
      break;
    }
  }

  return exists;
}

// ##########  addNode  ##########

void Node::addNode(Node* node)
{
  m_nodes.append(node);
}

// ##########  removeNode  ##########

void Node::removeNode(Node* node)
{
  int i = m_nodes.indexOf(node);

  if (i != -1) {
    m_nodes.removeAt(i);
  }
}

// ############################## SkeletonTree #################################

// ##########  Constructor  ##########

SkeletonTree::SkeletonTree() {}

// ##########  Destructor  ##########

SkeletonTree::~SkeletonTree() 
{
  delete m_rootNode;
}

// ##########  rootAtom  ##########

Atom* SkeletonTree::rootAtom()
{
  return m_rootNode->atom();
}

// ##########  rootBond  ##########

Bond* SkeletonTree::rootBond()
{
  return m_rootBond;
}

// ##########  populate  ##########

void SkeletonTree::populate(Atom *rootAtom, Bond *rootBond, Molecule* molecule)
{
  if (!m_rootNode) {
    delete m_rootNode;
  }

  m_rootNode = new Node(rootAtom);

  m_rootBond = rootBond;

  Atom* bAtom = static_cast<Atom*>(m_rootBond->GetBeginAtom());
  Atom* eAtom = static_cast<Atom*>(m_rootBond->GetEndAtom());

  if (bAtom != m_rootNode->atom() && eAtom != m_rootNode->atom()) {
    return;
  }

  Atom* diffAtom = (bAtom == m_rootNode->atom()) ? eAtom : bAtom;

  //A temproray tree to find loops
  m_endNode = new Node(diffAtom);

  //Recursively go through molecule and make a temproray tree.
  //starting from m_endNode
  recursivePopulate(molecule, m_endNode, m_rootBond);

  //Recursively go through molecule and make the tree.
  //starting from m_rootNode
  recursivePopulate(molecule, m_rootNode, m_rootBond);

  //delete the temporary tree
  delete m_endNode;

  //for debugging puposes
  //printSkeleton(m_rootNode);
}

// ##########  recursivePopulate  ##########

void SkeletonTree::recursivePopulate(Molecule* mol, Node* node, Bond* bond)
{
  Atom* atom = node->atom();
  int found = 0;

  for (unsigned int i=0; i < mol->NumBonds(); i++)
  {
    Bond* b = static_cast<Bond*>(mol->GetBond(i));
    Atom* bAtom = static_cast<Atom*>(b->GetBeginAtom());
    Atom* eAtom = static_cast<Atom*>(b->GetEndAtom());

    if ((b != bond) && ((bAtom == atom) || (eAtom == atom)))
    {
      Atom* diffAtom = (bAtom == atom) ? eAtom : bAtom;

      //Check if this atom already exists, so not to form loops
      if ((!m_endNode->containsAtom(diffAtom)) &&
          (!m_rootNode->containsAtom(diffAtom)))
      {
        Node* newNode = new Node(diffAtom);
        node -> addNode(newNode);
        found++;
        recursivePopulate(mol, newNode, b);
      }
    }
  }
}

// ##########  skeletonTranslate  ##########

void SkeletonTree::skeletonTranslate(double dx, double dy, double dz)
{
  if (m_rootNode) {
    //Translate skeleton
    recursiveTranslate(m_rootNode, dx, dy, dz);
  }
}

// ##########  skeletonRotate  ##########

void SkeletonTree::skeletonRotate(double angle, Eigen::Vector3d rotationVector,
                                  Eigen::Vector3d centerVector)
{
  if (m_rootNode) {
    //Rotate skeleton
    Quaternion qLeft = Quaternion::createRotationLeftHalf(angle, rotationVector);
    Quaternion qRight = qLeft.multiplicitiveInverse();
    recursiveRotate(m_rootNode, qLeft, qRight, centerVector);
  }
}

// ##########  recursiveTranslate  ##########

void SkeletonTree::recursiveTranslate(Node* n, double x, double y, double z)
{
  QList<Node*>* listNodes = n->nodes();
  Atom* a = n->atom();

  a->SetVector(a->x() + x, a->y() + y, a->z() + z);

  for (int i = 0; i < listNodes->size(); i++)
  {
    Node* node = listNodes->at(i);
    recursiveTranslate(node, x, y, z);
  }
}

// ##########  recursiveRotate  ##########

void SkeletonTree::recursiveRotate(Node* n, Quaternion left, Quaternion right, 
                                   Eigen::Vector3d centerVector)
{
  QList<Node*>* listNodes = n->nodes();
  Atom* a = n->atom();
  Vector3d final = performRotation(left, right, centerVector, a->pos());

  a->SetVector(final.x(), final.y(), final.z());

  for (int i = 0; i < listNodes->size(); i++)
  {
    Node* node = listNodes->at(i);
    recursiveRotate(node, left, right, centerVector);
  }
}

// ##########  printSkeleton  ##########

void SkeletonTree::printSkeleton(Node* n)
{
  QList<Node*>* listNodes = n->nodes();

  for (int i = 0; i < listNodes->size(); i++)
  {
    Node* n = listNodes->at(i);
    printSkeleton(n);
  }

  Atom* a = n->atom();
  cout << a->x() << "," << a->y()<< ","<<a->z() << endl;

  if (!n->isLeaf()) {
    cout << "-------------" << endl;
  }
}

// ##########  containsAtom  ##########

bool SkeletonTree::containsAtom(Atom *atom)
{
  return m_rootNode ? m_rootNode->containsAtom(atom) : false;
}

// ##########  performRotation  ##########

Eigen::Vector3d SkeletonTree::performRotation(Quaternion left, Quaternion right, 
                                              Eigen::Vector3d centerVector,
                                              Eigen::Vector3d positionVector)
{
  return Quaternion::performRotationMultiplication(left, positionVector -
                      centerVector, right) + centerVector;
}
