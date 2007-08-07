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

#ifndef __SKELETONTREE_H
#define __SKELETONTREE_H

#include "quaternion.h"

#include <avogadro/primitive.h>

#include <QObject>
#include <QList>

namespace Avogadro {

  /**
   * @class Node
   * @brief A node of a tree used to represent a molecule in a skeletal-like structure.
   * @author Shahzad Ali, Ross Braithwaite, James Bunt
   *
   * This class represents the a node in a particular tree structure.  It holds
   * an Atom and pointers to any number of child nodes.
   */
  class Node : public QObject
  {
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
      QList<Node*> *nodes();

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
   * This class creates and provides methods to manipulate a Molecule using
   * skeletal math.
   */
  class SkeletonTree : public QObject
  {
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
       * @param dx The distance the skeleton should move in the x direction.
       * @param dy The distance the skeleton should move in the y direction.
       * @param dz The distance the skeleton should move in the z direction.
       */
      void skeletonTranslate(double dx, double dy, double dz);

      /**
       * Rotates the Atoms attached to root node skeleton, by the given angle.
       *
       * @param angle The angle the skeleton rotate in radians.
       * @param rotationVector The Vector3d the skeleton should rotate around.
       * @param centerVector The Vector3d of the center of rotation for the
       *                     skeleton.
       */
      void skeletonRotate(double angle, Eigen::Vector3d rotationVector,
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
      /*
       * <TODO>
       *
       * @param mol <TODO>
       * @param node <TODO>
       * @param bond <TODO>
       */
      void recursivePopulate(Molecule* mol, Node* node, Bond* bond);

      /*
       * <TODO>
       *
       * @param n <TODO>
       * @param x <TODO>
       * @param y <TODO>
       * @param z <TODO>
       */
      void recursiveTranslate(Node* n, double x, double y, double z);

      /*
       * <TODO>
       *
       * @param n <TODO>
       * @param left <TODO>
       * @param right <TODO>
       * @param centerVector <TODO>
       */
      void recursiveRotate(Node* n, Quaternion left, Quaternion right, Eigen::Vector3d centerVector);

      /* <TODO>
       * Performs a rotation on a vector.
       * @param angle The angle to rotate by in radians.
       * @param rotationVector The Vector3d to rotate around, must be a unit
       *                       vector.
       * @param centerVector The Vector3d postion around which to rotate
       * @param postionVector The Vector3d postion of the vector to rotate
       * @return A Vector3d with the final postion after the rotation is
       *         performed.
       */
      Eigen::Vector3d performRotation(Quaternion left, Quaternion right,
                                      Eigen::Vector3d centerVector,
                                      Eigen::Vector3d positionVector);
  };
} // End namespace Avogadro

#endif /*__SKELETONTREE_H */
