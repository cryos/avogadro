/**********************************************************************
  Sphere - Class for drawing spheres in OpenGL

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

#include "sphere.h"

using namespace Eigen;

namespace Avogadro {

class SpherePrivate
{
  public:
      SpherePrivate() : vertexBuffer(0), indexBuffer(0), displayList(0), isValid(false) {}

      /** Pointer to the buffer storing the vertex array */
      Eigen::Vector3f *vertexBuffer;
      /** Pointer to the buffer storing the indices */
      unsigned short *indexBuffer;
      /** The id of the OpenGL display list */
      GLuint displayList;
      /** the detail-level of the sphere. Must be at least 0.
       * If 0, the sphere is an octahedron. If >=1, this number is
       * interpreted as the number of sub-edges into which
       * each edge of the icosahedron must be split. So the
       * number of faces of the sphere is simply:
       * 20 * detail^2. When detail==1, the sphere is just the
       * icosahedron */
      int detail;

      bool isValid;
};

Sphere::Sphere(int detail) : d(new SpherePrivate)
{
  setup(detail);
}

Sphere::~Sphere()
{
  freeBuffers();
  if( d->displayList )
    glDeleteLists( d->displayList, 1 );
        delete d;
}

void Sphere::freeBuffers()
{
  if( d->indexBuffer )
  {
    delete [] d->indexBuffer;
    d->indexBuffer = 0;
  }
  if( d->vertexBuffer )
  {
    delete [] d->vertexBuffer;
    d->vertexBuffer = 0;
  }
}

void Sphere::draw(const Eigen::Vector3d &center, double radius) const
{
  glPushMatrix();
  glTranslated( center.x(), center.y(), center.z() );
  glScaled( radius, radius, radius );
  glCallList( d->displayList );
  glPopMatrix();
}

void Sphere::initialize()
{
  if( d->detail < 0 ) return;

  // deallocate any previously allocated buffer
  freeBuffers();
  d->isValid = false;
  int vertexCount = 0, indexCount = 0;

  if( d->detail == 0 )
  {
    if( ! d->displayList ) { d->displayList = glGenLists( 1 ); }
    if( ! d->displayList ) { return; }
    float octahedronVertices[6][3] = { { 1, 0, 0 } ,
                                       { 0, 1, 0 } ,
                                       { 0, 0, 1 } ,
                                       { 0, -1, 0 } ,
                                       { 0, 0, -1 } ,
                                       { -1, 0, 0 } };
#define USE_OCTAHEDRON_VERTEX(i) glNormal3fv(octahedronVertices[i]); \
                                 glVertex3fv(octahedronVertices[i]);
    glNewList( d->displayList, GL_COMPILE );
    glBegin(GL_TRIANGLE_FAN);
    USE_OCTAHEDRON_VERTEX(0);
    USE_OCTAHEDRON_VERTEX(1);
    USE_OCTAHEDRON_VERTEX(2);
    USE_OCTAHEDRON_VERTEX(3);
    USE_OCTAHEDRON_VERTEX(4);
    USE_OCTAHEDRON_VERTEX(1);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    USE_OCTAHEDRON_VERTEX(5);
    USE_OCTAHEDRON_VERTEX(1);
    USE_OCTAHEDRON_VERTEX(4);
    USE_OCTAHEDRON_VERTEX(3);
    USE_OCTAHEDRON_VERTEX(2);
    USE_OCTAHEDRON_VERTEX(1);
    glEnd();
    glEndList();
    d->isValid = true;
    return;
  }

  // compute number of vertices and indices
  vertexCount = ( 3 * d->detail + 1 ) * ( 5 * d->detail + 1 );
  indexCount = (2 * ( 2 * d->detail + 1 ) + 2 ) * 5 * d->detail;

  // allocate memory for buffers
  d->vertexBuffer = new Vector3f[vertexCount];
  if( ! d->vertexBuffer ) return;
  d->indexBuffer = new unsigned short[indexCount];
  if( ! d->indexBuffer ) 
  {
    delete [] d->vertexBuffer;
    d->vertexBuffer = 0;
    return;
  }

  // build vertex buffer
  for( int strip = 0; strip < 5; strip++ ) {
    for( int column = 1; column < d->detail; column++ ) {
      for( int row = column; row <= 2 * d->detail + column; row++ ) {
        computeVertex( strip, column, row );
      }
    }
  }

  for( int strip = 1; strip < 5; strip++ ) {
    for( int row = 0; row <= 3 * d->detail; row++ ) {
      computeVertex( strip, 0, row );
    }
  }

  for( int row = 0; row <= 2 * d->detail; row++ )
    computeVertex( 0, 0, row );

  for( int row = d->detail; row <= 3 * d->detail; row++ )
    computeVertex( 4, d->detail, row );

  // build index buffer
  unsigned int i = 0;
  for( int strip = 0; strip < 5; strip++ )
  for( int column = 0; column < d->detail; column++ )
  {
    int row = column;
    d->indexBuffer[i++] = indexOfVertex( strip, column, row );
    for( ; row <= 2 * d->detail + column; row++ )
    {
      d->indexBuffer[i++] =
        indexOfVertex( strip, column, row );
      d->indexBuffer[i++] =
        indexOfVertex( strip, column + 1, row + 1 );
    }
    d->indexBuffer[i++] = indexOfVertex( strip, column + 1,
      2 * d->detail + column + 1);
  }

  // compile display list and free buffers
  if( ! d->displayList ) { d->displayList = glGenLists( 1 ); }
  if( ! d->displayList ) { return; }
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
  glNewList( d->displayList, GL_COMPILE );
  glVertexPointer( 3, GL_FLOAT, 0, d->vertexBuffer );
  glNormalPointer( GL_FLOAT, 0, d->vertexBuffer );
  glDrawElements( GL_TRIANGLE_STRIP, indexCount,
      GL_UNSIGNED_SHORT, d->indexBuffer );
  glEndList();
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
  freeBuffers();
  d->isValid = true;
}

unsigned short Sphere::indexOfVertex( int strip, int column, int row)
{
  return ( row + ( 3 * d->detail + 1 ) * ( column + d->detail * strip ) );
}

void Sphere::computeVertex( int strip, int column, int row)
{
  strip %= 5;
  int next_strip = (strip + 1) % 5;

  // the index of the vertex we want to store the result in
  unsigned short index = indexOfVertex( strip, column, row );

  // reference to the vertex we want to store the result in
  Vector3f & vertex = d->vertexBuffer[ index ];

  // the "golden ratio", useful to construct an icosahedron
  const float phi = ( 1 + sqrtf(5.0f) ) / 2;

  // the 12 vertices of the icosahedron
  const Vector3f northPole( 0, 1, phi );
  const Vector3f northVertices[5] = {
    Vector3f( 0, -1, phi ),
    Vector3f( phi, 0, 1 ),
    Vector3f( 1, phi, 0 ),
    Vector3f( -1, phi, 0 ),
    Vector3f( -phi, 0, 1 ) };
  const Vector3f southVertices[5] = {
    Vector3f( -1, -phi, 0 ),
    Vector3f( 1, -phi, 0 ),
    Vector3f( phi, 0, -1 ),
    Vector3f( 0, 1, -phi ),
    Vector3f( -phi, 0, -1 )
     };
  const Vector3f southPole( 0, -1, -phi );

  // pointers to the 3 vertices of the face of the icosahedron
  // in which we are
  const Vector3f *v0, *v1, *v2;

  // coordinates of our position inside this face.
  // range from 0 to d->detail.
  int  c1, c2;

  // first, normalize the global coords row, column
  if( row >= 2 * d->detail && column == 0 )
  {
    strip--;
    if( strip < 0 ) strip += 5;
    next_strip--;
    if( next_strip < 0 ) next_strip += 5;
    column = d->detail;
  }

  // next, determine in which face we are, and determine the coords
  // of our position inside this face
  if( row  <= d->detail )
  {
    v0 = &northVertices[strip];
    v1 = &northPole;
    v2 = &northVertices[next_strip];
    c1 = d->detail - row;
    c2 = column;
  }
  else if( row >= 2 * d->detail )
  {
    v0 = &southVertices[next_strip];
    v1 = &southPole;
    v2 = &southVertices[strip];
    c1 = row - 2 * d->detail;
    c2 = d->detail - column;
  }
  else if( row <= d->detail + column )
  {
    v0 = &northVertices[next_strip];
    v1 = &southVertices[next_strip];
    v2 = &northVertices[strip];
    c1 = row - d->detail;
    c2 = d->detail - column;
  }
  else
  {
    v0 = &southVertices[strip];
    v1 = &southVertices[next_strip];
    v2 = &northVertices[strip];
    c1 = column;
    c2 = 2 * d->detail - row;
  }

  // now, compute the actual coords of the vertex
  float u1 = static_cast<float>(c1) / d->detail;
  float u2 = static_cast<float>(c2) / d->detail;
  vertex = *v0 + u1 * ( *v1 - *v0 ) + u2 * ( *v2 - *v0 );

  // project the vertex onto the sphere
  vertex.normalize();
}

void Sphere::setup( int detail )
{
  if( d->isValid && detail == d->detail ) return;
  d->detail = detail;
  initialize();
}

}
