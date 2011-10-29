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

#ifndef CLIPPEDCYLINDER_P_H
#define CLIPPEDCYLINDER_P_H

#include <Eigen/Core>

namespace Avogadro
{
class ClippedCylinderPrivate;

class ClippedCylinder
{
public:
    ClippedCylinder(const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                    double radius, const Eigen::Vector3d &normal,
                    const Eigen::Vector3d &planePoint, int faces);
    ClippedCylinder();
    virtual ~ClippedCylinder();

    void setCylinder(const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                     double radius);
    void setPlane(const Eigen::Vector3d &normal,const Eigen::Vector3d &point);
    void setFaces(int faces);

    void draw();

    static void draw(const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                     double radius, const Eigen::Vector3d &normal,
                     const Eigen::Vector3d &planePoint, int faces)
    {
      ClippedCylinder c (end1, end2, radius, normal, planePoint, faces);
      c.draw();
    }

private:
    ClippedCylinderPrivate * const d;
};

} // namespace Avogadro

#endif // CLIPPEDCYLINDER_P_H
