/**********************************************************************
  ClippedCylinder - Render a cylinder clipped by a plane

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#include "clippedcylinder_p.h"

#include <QtGlobal> // For quint64
#include <QGLWidget> // For gl calls

#include <Eigen/Geometry>

using namespace Eigen;

//! todo document
//! todo mask limit!

namespace Avogadro
{

class ClippedCylinderPrivate {
public:

  ClippedCylinderPrivate()
    : end1(), end2(), normal(), ppoint(),
      faces(0), radius(0),
      cylinderIsSet(false), planeIsSet(false), facesIsSet(false) {}

  ClippedCylinderPrivate(const Vector3d &e1, const Vector3d &e2,
                         double r, const Vector3d &n,
                         const Vector3d &pp, int f)
    : end1(e1), end2(e2), normal(n), ppoint(pp),
      faces(f), radius(r),
      cylinderIsSet(true), planeIsSet(true), facesIsSet(true) {}

  //! First end of the cylinder. This is the end that will be rendered.
  Vector3d end1;
  //! Second end of the cylinder. This is the end that will be clipped out.
  Vector3d end2;
  //! Vector normal to the plane
  Vector3d normal;
  //! Point in the plane
  Vector3d ppoint;
  //! Number of faces on cylinder (resolution)
  int faces;
  //! Cylinder radius
  double radius;
  //! Is the cylinder geometry defined?
  bool cylinderIsSet;
  //! Is the plane geometry defined?
  bool planeIsSet;
  //! Is the faces geometry defined?
  bool facesIsSet;

};

ClippedCylinder::ClippedCylinder(
    const Vector3d &end1, const Vector3d &end2, double radius,
    const Vector3d &normal, const Vector3d &planePoint,
    int faces)
  : d(new ClippedCylinderPrivate(end1,end2,radius,normal,planePoint,faces))
{
}



ClippedCylinder::ClippedCylinder()
  : d(new ClippedCylinderPrivate())
{
}

ClippedCylinder::~ClippedCylinder()
{
  delete d;
}

void ClippedCylinder::setCylinder(const Vector3d &end1,
                                  const Vector3d &end2, double radius)
{
  d->end1 = end1;
  d->end2 = end2;
  d->radius = radius;
  d->cylinderIsSet = true;
}

void ClippedCylinder::setPlane(const Vector3d &normal,
                               const Vector3d &point)
{
  d->normal = normal;
  d->ppoint = point;
  d->planeIsSet = true;
}

void ClippedCylinder::setFaces(int faces)
{
  d->faces = faces;
  d->facesIsSet = true;
}

void ClippedCylinder::draw()
{
  // Is geometry specified?
  if (!d->cylinderIsSet || !d->planeIsSet || !d->facesIsSet) {
    return;
  }

  // Calculate and cache some lines, variables, etc
  const Vector3d cylinderVector (d->end2 - d->end1);
  const Vector3d normalizedCylinderVector (cylinderVector.normalized());
  const Vector3d cylinderOrthonormal (normalizedCylinderVector.unitOrthogonal());
  const AngleAxisd ringRotation ((2.0*M_PI) / static_cast<double>(d->faces),
                                 -normalizedCylinderVector);
  const double cylinderLength = cylinderVector.norm();

  const Hyperplane<double, 3> plane (d->normal, d->ppoint);

  // Can't be const because of intersection method
  ParametrizedLine<double, 3> cylinderLine (d->end1,
                                            normalizedCylinderVector);

  // Does the plane actually intersect the cylinder?
  const double cylinderIntParam = cylinderLine.intersection(plane);
  if (cylinderIntParam < 0.0 || cylinderIntParam > cylinderLength) {
    return;
  }

  // Cache the intersection point
  const Vector3d cylinderIntersection (d->end1 + (cylinderIntParam *
                                                  normalizedCylinderVector));

  // Generate ring of points around end1
  Vector3d *end1Ring = new Vector3d[d->faces];
  Vector3d *end1RingNormals = new Vector3d[d->faces];
  end1Ring[0] = d->end1 + (d->radius * cylinderOrthonormal);
  end1RingNormals[0] = cylinderOrthonormal;
  // If the plane intersects the line point_i--end1, mask point_i
  //! @todo what to do if d->faces > 64?
  quint64 endRingMask = Q_UINT64_C(0);
  quint64 one_64bit = Q_UINT64_C(1);
  for (int i = 1; i < d->faces; ++i) {
    Vector3d &p = end1Ring[i];
    Vector3d &n = end1RingNormals[i];

    n = ringRotation * end1RingNormals[i-1];
    p = d->end1 + (d->radius * n);

    //! @todo this will skip points > 64. Remove when mask limit removed
    if (i >= 64) {
      continue;
    }

    // Test for intersection and mask if needed
    const double intParam =
        ParametrizedLine<double, 3>::Through(d->end1, p).intersection(plane);
    if (intParam > 0.0 && intParam < d->radius) {
      endRingMask |= (one_64bit << i);
    }
  }

  // Find the ring of points at the intersection of the cylinder and cone
  // end2Ring will share normals with end1Ring.
  Vector3d *end2Ring = new Vector3d[d->faces];
  // For each point in end1Ring:
  for (int i = 0; i < d->faces; ++i) {
    // Check mask
    if (i < 64 && endRingMask & (one_64bit << i)) {
      continue;
    }

    // Setup references
    Vector3d &end1Point = end1Ring[i];
    Vector3d &end2Point = end2Ring[i];

    // Unclipped end2Point:
    end2Point = end1Point + cylinderVector;

    // Calculate the intersection of cylinder edge
    const double intParam = ParametrizedLine<double, 3>::Through(
          end1Point, end2Point).intersection(plane);

    // If the plane intersects the edge,
    if (intParam > 0.0 && intParam < cylinderLength) {
      // Replace end2point with the intersection point
      end2Point = end1Point + (intParam * normalizedCylinderVector);
    }
  }

  // Stitch a quad strip for the cylinder sides
  glBegin(GL_QUAD_STRIP);
  int firstFaceIndex = -1;
  for (int i = 0; i < d->faces; ++i) {
    // Check mask
    if (i < 64 && endRingMask & (one_64bit << i)) {
      continue;
    }

    // Used to stitch the cylinder closed
    if (firstFaceIndex == -1) {
      firstFaceIndex = i;
    }

    // Add this edge
    glNormal3dv(end1RingNormals[i].data());
    glVertex3dv(end1Ring[i].data());
    glVertex3dv(end2Ring[i].data());
  }
  // close the cylinder
  if (firstFaceIndex != -1) {
    glNormal3dv(end1RingNormals[firstFaceIndex].data());
    glVertex3dv(end1Ring[firstFaceIndex].data());
    glVertex3dv(end2Ring[firstFaceIndex].data());
  }
  glEnd(); // GL_QUAD_STRIP

  // A triangle fan for end1's cap
  glBegin(GL_TRIANGLE_FAN);
  glNormal3dv(Vector3d(-normalizedCylinderVector).data());
  glVertex3dv(d->end1.data());
  for (int i = 0; i < d->faces; ++i) {
    // Check mask
    if (i < 64 && endRingMask & (one_64bit << i)) {
      continue;
    }
    glVertex3dv(end1Ring[i].data());
  }
  glVertex3dv(end1Ring[firstFaceIndex].data());
  glEnd(); // GL_TRIANGLE_FAN

  // and another triangle fan for end2, reverse winding from end1
  glBegin(GL_TRIANGLE_FAN);
  glNormal3dv(normalizedCylinderVector.data()); // not correct approx, but eh.
  glVertex3dv(cylinderIntersection.data());
  glVertex3dv(end2Ring[firstFaceIndex].data());
  for (int i = d->faces - 1; i >= 0; --i) {
    // Check mask
    if (i < 64 && endRingMask & (one_64bit << i)) {
      continue;
    }
    glVertex3dv(end2Ring[i].data());
  }
  glEnd(); // GL_TRIANGLE_FAN

  // Clean up
  delete [] end1RingNormals;
  delete [] end1Ring;
  delete [] end2Ring;
}

} // namespace Avogadro
