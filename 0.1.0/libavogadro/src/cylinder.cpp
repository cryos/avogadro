/**********************************************************************
  Cylinder - OpenGL Cylinder drawing class.

  Copyright (C) 2006,2007 Benoit Jacob

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

#include "cylinder.h"

using namespace Eigen;

namespace Avogadro {

  class CylinderPrivate {
    public:
      CylinderPrivate() : vertexBuffer(0), normalBuffer(0), displayList(0), isValid(false) {}

      /** Pointer to the buffer storing the vertex array */
      Eigen::Vector3f *vertexBuffer;
      /** Pointer to the buffer storing the normal array */
      Eigen::Vector3f *normalBuffer;
      /** The id of the OpenGL display list */
      GLuint displayList;
      /** Equals true if the vertex array has been correctly initialized */
      bool isValid;

      /** the number of faces of the cylinder. This only
       * includes the lateral faces, as the base and top faces (the
       * two discs) are not rendered. */
      int faces;
  };

  Cylinder::Cylinder(int faces) : d(new CylinderPrivate)
  {
    setup(faces);
  }

  Cylinder::~Cylinder()
  {
    freeBuffers();
    if( d->displayList ) {
      glDeleteLists( d->displayList, 1 );
    }
    delete d;
  }

  void Cylinder::freeBuffers()
  {
    if( d->normalBuffer )
    {
      delete [] d->normalBuffer;
      d->normalBuffer = 0;
    }
    if( d->vertexBuffer )
    {
      delete [] d->vertexBuffer;
      d->vertexBuffer = 0;
    }
  }

  void Cylinder::setup( int faces )
  {
    if( d->isValid && faces == d->faces ) return;
    d->faces = faces;
    initialize();
  }

  void Cylinder::initialize()
  {
    d->isValid = false;
    if( d->faces < 0 ) return;

    // compile display list and free buffers
    if( ! d->displayList ) d->displayList = glGenLists( 1 );
    if( ! d->displayList ) return;
    
    if( d->faces < 3 ) 
    {
      glNewList( d->displayList, GL_COMPILE );
      glLineWidth(1.0);
      glBegin(GL_LINES);
      glVertex3f(0, 0, 0);
      glVertex3f(0, 0, 1);
      glEnd();
      glEndList();
    }
    else
    {
      // compute number of vertices
      int vertexCount = 2 * d->faces + 2;

      // deallocate any previously allocated buffer
      freeBuffers();

      // allocate memory for buffers
      d->vertexBuffer = new Vector3f[vertexCount];
      if( ! d->vertexBuffer ) return;
      d->normalBuffer = new Vector3f[vertexCount];
      if( ! d->normalBuffer ) return;

      float baseAngle = 2 * M_PI / d->faces;
      // build vertex and normal buffers
      for( int i = 0; i <= d->faces; i++ )
      {
        float angle = baseAngle * i;
        Vector3f v( cosf(angle), sinf(angle), 0.0f );
        d->normalBuffer[ 2 * i ] = v;
        d->normalBuffer[ 2 * i + 1 ] = v;
        d->vertexBuffer[ 2 * i ] = v;
        d->vertexBuffer[ 2 * i + 1 ] = v;
        d->vertexBuffer[ 2 * i ].z() = 1.0f;
      }
      glEnableClientState( GL_VERTEX_ARRAY );
      glEnableClientState( GL_NORMAL_ARRAY );
      glNewList( d->displayList, GL_COMPILE );
      glVertexPointer( 3, GL_FLOAT, 0, d->vertexBuffer );
      glNormalPointer( GL_FLOAT, 0, d->normalBuffer );
      glDrawArrays( GL_QUAD_STRIP, 0, vertexCount );
      glEndList();
      glDisableClientState( GL_VERTEX_ARRAY );
      glDisableClientState( GL_NORMAL_ARRAY );
    }
    freeBuffers();
    d->isValid = true;
  }

  void Cylinder::draw( const Vector3d &end1, const Vector3d &end2,
      double radius ) const
  {
    // the "axis vector" of the cylinder
    Vector3d axis = end2 - end1;

    // construct an orthogonal basis whose first vector is axis, and whose other vectors
    // have norm equal to 'radius'.
    Vector3d axisNormalized = axis / axis.norm();
    Vector3d ortho1, ortho2;
    ortho1.loadOrtho(axisNormalized);
    ortho1 *= radius;
    axisNormalized.cross( ortho1, &ortho2 );

    // construct the 4D transformation matrix
    Matrix4d matrix;

    matrix(0, 0) = ortho1(0);
    matrix(1, 0) = ortho1(1);
    matrix(2, 0) = ortho1(2);
    matrix(3, 0) = 0.0;

    matrix(0, 1) = ortho2(0);
    matrix(1, 1) = ortho2(1);
    matrix(2, 1) = ortho2(2);
    matrix(3, 1) = 0.0;

    matrix(0, 2) = axis(0);
    matrix(1, 2) = axis(1);
    matrix(2, 2) = axis(2);
    matrix(3, 2) = 0.0;

    matrix(0, 3) = end1(0);
    matrix(1, 3) = end1(1);
    matrix(2, 3) = end1(2);
    matrix(3, 3) = 1.0;

    //now we can do the actual drawing !
    glPushMatrix();
    glMultMatrixd( matrix.array() );
    glCallList( d->displayList );
    glPopMatrix();
  }

  void Cylinder::drawMulti( const Vector3d &end1, const Vector3d &end2,
      double radius, int order, double shift,
      const Vector3d &planeNormalVector ) const
  {

    // the "axis vector" of the cylinder
    Vector3d axis = end2 - end1;

    // now we want to construct an orthonormal basis whose first
    // vector is axis.normalized(). We don't use Eigen's loadOrthoBasis()
    // for that, because we want one more thing. The second vector in this
    // basis, which we call ortho1, should be approximately lying in the
    // z=0 plane if possible. This is to ensure double bonds don't look
    // like single bonds from the default point of view.
    double axisNorm = axis.norm();
    if( axisNorm == 0.0 ) return;
    Vector3d axisNormalized = axis / axisNorm;

    Vector3d ortho1 = axisNormalized.cross(planeNormalVector);
    double ortho1Norm = ortho1.norm();
    if( ortho1Norm > 0.001 ) ortho1 /= ortho1Norm;
    else ortho1 = axisNormalized.ortho();
    ortho1 *= radius;

    Vector3d ortho2 = cross( axisNormalized, ortho1 );       

    // construct the 4D transformation matrix
    Matrix4d matrix;

    matrix(0, 0) = ortho1(0);
    matrix(1, 0) = ortho1(1);
    matrix(2, 0) = ortho1(2);
    matrix(3, 0) = 0.0;

    matrix(0, 1) = ortho2(0);
    matrix(1, 1) = ortho2(1);
    matrix(2, 1) = ortho2(2);
    matrix(3, 1) = 0.0;

    matrix(0, 2) = axis(0);
    matrix(1, 2) = axis(1);
    matrix(2, 2) = axis(2);
    matrix(3, 2) = 0.0;

    matrix(0, 3) = end1(0);
    matrix(1, 3) = end1(1);
    matrix(2, 3) = end1(2);
    matrix(3, 3) = 1.0;

    //now we can do the actual drawing !
    glPushMatrix();
    glMultMatrixd( matrix.array() );
    if( order == 1 )
      glCallList( d->displayList );
    else
    {
      double angleOffset = 0.0;
      if( order >= 3 )
      {
        if( order == 3 ) angleOffset = 90.0;
        else angleOffset = 22.5;
      }

      double displacementFactor = shift / radius;
      for( int i = 0; i < order; i++)
      {
        glPushMatrix();
        glRotated( angleOffset + 360.0 * i / order,
            0.0, 0.0, 1.0 );
        glTranslated( displacementFactor, 0.0, 0.0 );
        glCallList( d->displayList );
        glPopMatrix();
      }
    }
    glPopMatrix();
  }

}
