/**********************************************************************
  Cube - Primitive class to encapsulate volumetric data

  Copyright (C) 2008 Marcus D. Hanwell

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

#include "mesh.h"

#include <QColor>
#include <QReadWriteLock>
#include <QDebug>

using Eigen::Vector3f;
using std::vector;

namespace Avogadro {

  Mesh::Mesh(QObject *parent) : Primitive(MeshType, parent), m_vertices(0),
    m_normals(0), m_colors(0), m_stable(true), m_other(FALSE_ID), m_cube(0),
    m_lock(new QReadWriteLock)
  {
    m_vertices.reserve(100);
    m_normals.reserve(100);
    m_colors.reserve(1);
  }

  Mesh::~Mesh()
  {
    delete m_lock;
    m_lock = 0;
  }

  bool Mesh::reserve(unsigned int size, bool colors)
  {
    QWriteLocker lock(m_lock);
    m_vertices.reserve(size);
    m_normals.reserve(size);
    if (colors) m_colors.reserve(size);
    return true;
  }

  void Mesh::setStable(bool stable)
  {
    QWriteLocker lock(m_lock);
    m_stable = stable;
  }

  bool Mesh::stable()
  {
    QReadLocker lock(m_lock);
    return m_stable;
  }

  const vector<Vector3f> & Mesh::vertices() const
  {
    QReadLocker lock(m_lock);
    return m_vertices;
  }

  const Vector3f * Mesh::vertex(int n) const
  {
    QReadLocker lock(m_lock);
    return &(m_vertices[n]);
  }

  bool Mesh::setVertices(const vector<Vector3f> &values)
  {
    QWriteLocker lock(m_lock);
    m_vertices.clear();
    m_vertices = values;
    return true;
  }

  bool Mesh::addVertices(const vector<Vector3f> &values)
  {
    QWriteLocker lock(m_lock);
    if (m_vertices.capacity() < m_vertices.size() + values.size()) {
      m_vertices.reserve(m_vertices.capacity()*2);
    }
    if (values.size() % 3 == 0) {
      for (unsigned int i = 0; i < values.size(); ++i) {
        m_vertices.push_back(values.at(i));
      }
      return true;
    }
    else {
      qDebug() << "Error adding vertices." << values.size();
      return false;
    }
  }

  const vector<Vector3f> & Mesh::normals() const
  {
    QReadLocker lock(m_lock);
    return m_normals;
  }

  const Vector3f * Mesh::normal(int n) const
  {
    QReadLocker lock(m_lock);
    return &(m_normals[n*3]);
  }

  bool Mesh::setNormals(const vector<Vector3f> &values)
  {
    QWriteLocker lock(m_lock);
    m_normals.clear();
    m_normals = values;
    return true;
  }

  bool Mesh::addNormals(const vector<Vector3f> &values)
  {
    QWriteLocker lock(m_lock);
    if (m_normals.capacity() < m_normals.size() + values.size()) {
      m_normals.reserve(m_normals.capacity()*2);
    }
    if (values.size() % 3 == 0) {
      for (unsigned int i = 0; i < values.size(); ++i) {
        m_normals.push_back(values.at(i));
      }
      return true;
    }
    else {
      qDebug() << "Error adding normals." << values.size();
      return false;
    }
  }

  const vector<QColor> & Mesh::colors() const
  {
    QReadLocker lock(m_lock);
    return m_colors;
  }

  const QColor * Mesh::color(int n) const
  {
    QReadLocker lock(m_lock);
    // If there is only one color return that, otherwise colored by vertex
    if (m_colors.size() == 1) {
      return &(m_colors[0]);
    }
    else {
      return &(m_colors[n*3]);
    }
  }

  bool Mesh::setColors(const vector<QColor> &values)
  {
    QWriteLocker lock(m_lock);
    m_colors.clear();
    m_colors = values;
    return true;
  }

  bool Mesh::addColors(const vector<QColor> &values)
  {
    QWriteLocker lock(m_lock);
    if (m_colors.capacity() < m_colors.size() + values.size()) {
      m_colors.reserve(m_colors.capacity()*2);
    }
    if (values.size() % 3 == 0) {
      for (unsigned int i = 0; i < values.size(); ++i) {
        m_colors.push_back(values.at(i));
      }
      return true;
    }
    else {
      qDebug() << "Error adding colors." << values.size();
      return false;
    }
  }

  bool Mesh::valid() const
  {
    QWriteLocker lock(m_lock);
    if (m_vertices.size() == m_normals.size()) {
      if (m_colors.size() == 1 || m_colors.size() == m_vertices.size()) {
        return true;
      }
      else {
        return false;
      }
    }
    else {
      return false;
    }
  }

  bool Mesh::clear()
  {
    QWriteLocker lock(m_lock);
    m_vertices.clear();
    m_normals.clear();
    m_colors.clear();
    return true;
  }

  Mesh& Mesh::operator=(const Mesh& other)
  {
    QWriteLocker lock(m_lock);
    QReadLocker oLock(other.m_lock);
    m_vertices = other.m_vertices;
    m_normals = other.m_vertices;
    m_colors = other.m_colors;
    m_name = other.m_name;
    return *this;
  }

  QReadWriteLock * Mesh::lock() const
  {
    return m_lock;
  }

} // End namespace Avogadro

#include "mesh.moc"
