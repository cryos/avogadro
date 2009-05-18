/**********************************************************************
  BondCentricTool - Bond Centric Manipulation Tool for Avogadro

  Copyright (C) 2007 by Shahzad Ali
  Copyright (C) 2007 by Ross Braithwaite
  Copyright (C) 2007 by James Bunt

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

using namespace Eigen;
using namespace std;

namespace Avogadro {

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

  QList<Node*> Node::nodes()
  {
    return m_nodes;
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

    foreach (Node *node, m_nodes)
    {
      if (node->containsAtom(atom))
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

    Atom* bAtom = m_rootBond->beginAtom();
    Atom* eAtom = m_rootBond->endAtom();

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

    foreach (Bond *b, mol->bonds()) {
      Atom* bAtom = b->beginAtom();
      Atom* eAtom = b->endAtom();

      if ((b != bond) && ((bAtom == atom) || (eAtom == atom))) {
        Atom* diffAtom = (bAtom == atom) ? eAtom : bAtom;

        //Check if this atom already exists, so not to form loops
        if ((!m_endNode->containsAtom(diffAtom)) &&
            (!m_rootNode->containsAtom(diffAtom))) {
          Node* newNode = new Node(diffAtom);
          node -> addNode(newNode);
          found++;
          recursivePopulate(mol, newNode, b);
        }
      }
    }
  }

  // ##########  skeletonTranslate  ##########

  void SkeletonTree::skeletonTranslate(const Eigen::Vector3d &translationVector)
  {
    if (m_rootNode) {
      //Translate skeleton
      recursiveTranslate(m_rootNode, translationVector);
    }
  }

  // ##########  skeletonRotate  ##########

  void SkeletonTree::skeletonRotate(double angle,
                                    const Eigen::Vector3d &rotationAxis,
                                    const Eigen::Vector3d &centerVector)
  {
    if (m_rootNode) {
      //Rotate skeleton around a particular axis and center point
      Eigen::Transform3d rotation;
      rotation = Eigen::AngleAxisd(angle, rotationAxis);
      rotation.pretranslate(centerVector);
      rotation.translate(-centerVector);

      recursiveRotate(m_rootNode, rotation);
    }
  }

  // ##########  recursiveTranslate  ##########

  void SkeletonTree::recursiveTranslate(Node* n,
                                        const Eigen::Vector3d &translationVector)
  {
    // Translate the root node, and then update any children
    Atom* a = n->atom();

    a->setPos(*(a->pos()) + translationVector);
    a->update();

    foreach (Node* node, n->nodes())
      recursiveTranslate(node, translationVector);
  }

  // ##########  recursiveRotate  ##########

  void SkeletonTree::recursiveRotate(Node* n,
                                     const Eigen::Transform3d &rotationMatrix)
  {
    // Update the root node with the new position
    Atom* a = n->atom();
    a->setPos(rotationMatrix * (*a->pos()));
    a->update();

    // Now update the children
    foreach (Node *node, n->nodes())
    {
      recursiveRotate(node, rotationMatrix);
    }
  }

  // ##########  printSkeleton  ##########

  void SkeletonTree::printSkeleton(Node* n)
  {
    foreach (Node* node, n->nodes())
      printSkeleton(node);

    Atom* a = n->atom();
    cout << a->pos()->x() << "," << a->pos()->y()<< ","<<a->pos()->z() << endl;

    if (!n->isLeaf()) {
      cout << "-------------" << endl;
    }
  }

  // ##########  containsAtom  ##########

  bool SkeletonTree::containsAtom(Atom *atom)
  {
    return m_rootNode ? m_rootNode->containsAtom(atom) : false;
  }

}

