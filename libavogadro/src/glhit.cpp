/**********************************************************************
  GLHit - detect mouse clicks and the objects hit

  Copyright (C) 2006-2009 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2007-2009 Marcus D. Hanwell

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

#include "glhit.h"

namespace Avogadro {

  class GLHitPrivate
  {
  public:
    GLHitPrivate() {}

    GLuint type;
    GLuint name;
    GLuint minZ;
    GLuint maxZ;
  };

  GLHit::GLHit() : d(new GLHitPrivate)
  {
  }

  GLHit::GLHit( const GLHit &other ) : d( new GLHitPrivate )
  {
    GLHitPrivate *e = other.d;
    d->type = e->type;
    d->name = e->name;
    d->minZ = e->minZ;
    d->maxZ = e->maxZ;
  }

  GLHit::GLHit( GLuint type, GLuint name, GLuint minZ, GLuint maxZ ) : d( new GLHitPrivate )
  {
    d->name = name;
    d->type = type;
    d->minZ = minZ;
    d->maxZ = maxZ;
  }

  GLHit &GLHit::operator=( const GLHit &other )
  {
    GLHitPrivate *e = other.d;
    d->type = e->type;
    d->name = e->name;
    d->minZ = e->minZ;
    d->maxZ = e->maxZ;
    return *this;
  }

  GLHit::~GLHit()
  {
    delete d;
  }

  bool GLHit::operator<( const GLHit &other ) const
  {
    GLHitPrivate *e = other.d;
    return d->minZ < e->minZ;
  }

  bool GLHit::operator==( const GLHit &other ) const
  {
    GLHitPrivate *e = other.d;
    return (( d->type == e->type ) && ( d->name == e->name ) );
  }

  GLuint GLHit::name() const { return d->name; }
  GLuint GLHit::type() const { return d->type; }
  GLuint GLHit::minZ() const { return d->minZ; }
  GLuint GLHit::maxZ() const { return d->maxZ; }

  void GLHit::setName( GLuint name ) { d->name = name; }
  void GLHit::setType( GLuint type ) { d->type = type; }
  void GLHit::setMinZ( GLuint minZ ) { d->minZ = minZ; }
  void GLHit::setMaxZ( GLuint maxZ ) { d->maxZ = maxZ; }

} // End namespace Avogadro
