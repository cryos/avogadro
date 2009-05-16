/**********************************************************************
  BondCentricTool - Bond Centric Manipulation Tool for Avogadro

  Copyright (C) 2007 by Shahzad Ali
  Copyright (C) 2007 by Ross Braithwaite
  Copyright (C) 2007 by James Bunt
  Some portions Copyright (C) 2009 by Geoffrey Hutchison

  Revisions to use Eigen/Geometry directly, rather than home-grown Quaternions

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

#ifndef SKELETONTREE_H
#define SKELETONTREE_H

#include <QObject>
#include <QList>

#include <Eigen/Geometry>

namespace Avogadro {

  class Atom;
  class Bond;
  class Molecule;

  /**
   * @class Node
   * @brief A node of a tree used to represent a molecule in a skeletal-like structure.
   * @author Shahzad Ali, Ross Braithwaite, James Bunt
   *
   * This class represents a node in a particular tree structure.  It holds
   * an Atom and pointers to any number of child nodes.
   */
  class Node : public QObject
  {
    Q_OBJECT

    protected:
      Atom* m_atom;
      QList<Node*> m_nodes;

    public:
      //! Constructor
      /**
       * Constructs a new Node.
       *
       * @param atom The Atom contained within this new Node.
       */
      Node(Atom *atom);

      //! Deconstructor
      virtual ~Node();

      /**
       * Retrieves a pointer to the Atom stored in this Node.
       *
       * @return A pointer to the Atom stored in this Node.
       */
      Atom *atom();

      /**
       * Gets all of this Node's children.
       *
       * @return A QList of pointers to all the children Nodes of this Node.
       */
      QList<Node*> nodes();

      /**
       * Determines whether or not this Node is a leaf in the SkeletonTree.
       *
       * @return True if this Node is a leaf in the tree, false otherwise.
       */
      bool isLeaf();

      /**
       * Determines whether or not this Node, or any of the Nodes in the (sub)tree
       * that this Node is the root of, contains the given Atom.
       *
       * @param atom The Atom being searched for in this Node's (sub)tree.
       *
       * @return True if the Atom was found in the (sub)tree, false otherwise.
       */
      bool containsAtom(Atom* atom);

      /**
       * Adds the given Node as a child of this Node.
       *
       * @param node The Node being made a child of this Node.
       */
      void addNode(Node* node);

      /**
       * Removes the given Node from this Node's children.
       *
       * @param node The Node being removed from this Node's children.
       */
      void removeNode(Node* node);
  };

  /**
   * @class SkeletonTree
   * @brief Skeletal representation and manipulation of a Molecule.
   * @author Shahzad Ali, Ross Braithwaite, James Bunt
   *
   * This class creates and provides methods to manipulate a Molecule 
   * recursively (e.g., change a bond length or angle)
   */
  class SkeletonTree : public QObject
  {
    Q_OBJECT

    public:
      //! Constructor
       SkeletonTree();
      //! Deconstructor
      virtual ~SkeletonTree();

      /**
       * Returns the root node Atom.
       *
       * @return The root node Atom at which the tree is made.
       */
      Atom *rootAtom();

      /**
       * Returns the Bond associated with the root Atom.
       *
       * @return The Bond associated with the root Atom.
       */
      Bond *rootBond();

      /**
       * Populates the tree from the Molecule, using the root node Atom.
       *
       * @param rootAtom The root node Atom.
       * @param rootBond The Bond at which the root node Atom is.
       * @param molecule The Molecule to make the tree.
       */
      void populate(Atom *rootAtom, Bond *rootBond, Molecule* molecule);

      /**
       * Translates the Atoms attached to root node skeleton by the given amount
       * in the 3 standard directions (x, y, and z).
       *
       * @param translationVector The translation vector for the skeleton.
       */
      void skeletonTranslate(Eigen::Vector3d translationVector);

      /**
       * Rotates the Atoms attached to root node skeleton, by the given angle.
       *
       * @param angle The angle the skeleton rotate in radians.
       * @param rotationAxis The axis of rotation for the skeleton
       * @param centerVector The position of the center of rotation for the
       *                     skeleton.
       */
      void skeletonRotate(double angle, Eigen::Vector3d rotationAxis,
                            Eigen::Vector3d centerVector);

      /**
       * Recusively prints the children of this Node and child Nodes.
       *
       * @param n The root Node of the tree to print.
       */
      void printSkeleton(Node* n);

      /**
       * Determines whether or not this SkeletonTree contains a Node with the
       * given Atom.
       *
       * @param atom The Atom being searched for in this SkeletonTree.
       *
       * @return True if the Atom was found in the tree, false otherwise.
       */
      bool containsAtom(Atom *atom);

    protected:
      Node *m_rootNode; //The root node, tree
      Bond *m_rootBond; //The bond at which root node atom is attached
      Node *m_endNode;  //A temporary tree.

    private:
      /**
       * Recursively populates the tree, where the Node node is the root
       * node to start from.
       * It makes sure not to form loops.
       *
       * @param mol The molecule to form the tree for.
       * @param node The root node to form the tree from.
       * @param bond The bond at which the root node atom exists.
       */
      void recursivePopulate(Molecule* mol, Node* node, Bond* bond);

      /**
       * Recursively translates the Atoms attached to Node n in the skeleton by
       * the given location.
       * @param n The node atom and the atoms attached to it to translates.
       * @param x New x location
       * @param y New y location
       * @param z New z location
       */
      void recursiveTranslate(Node* n, Eigen::Vector3d translationVector);

      /**
       * Recursivly rotates the Atoms attached to Node n in skeleton,
       * around the Vector centerVector.
       *
       * @param n Current node to rotate
       * @param rotationMatrix The rotation matrix for the transformation.
       * @param centerVector Center location to rotate around.
       */
      void recursiveRotate(Node* n,
                           Eigen::Transform3d rotationMatrix);

  };
} // End namespace Avogadro

#endif /*__SKELETONTREE_H */
