/******************************************************************************

  This source file is part of the OpenQube project.

  Copyright 2008-2010 Marcus D. Hanwell

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef OQ_CUBE_H
#define OQ_CUBE_H

#include "openqubeabi.h"

#include <vector>
#include <Eigen/Core>
#include <QtCore/QString>

// Forward declarations
class QReadWriteLock;

namespace OpenQube {

class OPENQUBE_EXPORT Cube
{
public:
  Cube();
  ~Cube();

  /**
   * @enum Different Cube types relating to the data
   */
  enum Type{
    VdW,
    ESP,
    ElectronDensity,
    MO,
    FromFile,
    None
  };

  /**
   * @return The minimum point in the cube.
   */
  Eigen::Vector3d min() const { return m_min; }

  /**
   * @return The maximum point in the cube.
   */
  Eigen::Vector3d max() const { return m_max; }

  /**
   * @return The spacing of the grid.
   */
  Eigen::Vector3d spacing() const { return m_spacing; }

  /**
   * @return The x, y and z dimensions of the cube.
   */
  Eigen::Vector3i dimensions() const { return m_points; }

  /**
   * Set the limits of the cube.
   * @param min The minimum point in the cube.
   * @param max The maximum point in the cube.
   * @param points The number of (integer) points in the cube.
   */
  bool setLimits(const Eigen::Vector3d &min, const Eigen::Vector3d &max,
                 const Eigen::Vector3i &points);

  /**
   * Set the limits of the cube.
   * @param min The minimum point in the cube.
   * @param max The maximum point in the cube.
   * @param spacing The interval between points in the cube.
   */
  bool setLimits(const Eigen::Vector3d &min, const Eigen::Vector3d &max,
                 double spacing);

  /**
   * Set the limits of the cube.
   * @param min The minimum point in the cube.
   * @param dim The integer dimensions of the cube in x, y and z.
   * @param spacing The interval between points in the cube.
   */
  bool setLimits(const Eigen::Vector3d &min, const Eigen::Vector3i &dim,
                 double spacing);

  /**
   * Set the limits of the cube - copy the limits of an existing Cube.
   * @param cube Existing Cube to copy the limits from.
   */
  bool setLimits(const Cube &cube);

  /**
   * @return Vector containing all the data in a one-dimensional array.
   */
  std::vector<double> * data();

  /**
   * Set the values in the cube to those passed in the vector.
   */
  bool setData(const std::vector<double> &values);

  /**
   * Adds the values in the cube to those passed in the vector.
   */
  bool addData(const std::vector<double> &values);

  /**
   * @return Index of the point closest to the position supplied.
   * @param pos Position to get closest index for.
   */
  unsigned int closestIndex(const Eigen::Vector3d &pos) const;

  /**
   * @param pos Position to get closest index for.
   * @return The i, j, k index closest to the position supplied.
   */
  Eigen::Vector3i indexVector(const Eigen::Vector3d &pos) const;

  /**
   * @param index Index to be translated to a position.
   * @return Position of the given index.
   */
  Eigen::Vector3d position(unsigned int index) const;

  /**
   * This function is very quick as it just returns the value at the point.
   * @return Cube value at the integer point i, j, k.
   */
  double value(int i, int j, int k) const;

  /**
   * This function is very quick as it just returns the value at the point.
   * @return Cube value at the integer point pos.
   */
  double value(const Eigen::Vector3i &pos) const;

  /**
   * This function uses trilinear interpolation to find the value at points
   * between those specified in the cube.
   * @return Cube value at the specified position.
   * @warning This function is quite computationally expensive and should be
   * avoided where possible.
   */
  float valuef(const Eigen::Vector3f &pos) const;

  /**
   * This function uses trilinear interpolation to find the value at points
   * between those specified in the cube.
   * @return Cube value at the specified position.
   * @warning This function is quite computationally expensive and should be
   * avoided where possible.
   */
  double value(const Eigen::Vector3d &pos) const;

  /**
   * Sets the value at the specified point in the cube.
   * @param i x compenent of the position.
   * @param j y compenent of the position.
   * @param k z compenent of the position.
   * @param value Value at the specified position.
   */
  bool setValue(int i, int j, int k, double value);

  /**
   * Sets the value at the specified index in the cube.
   * @param i 1-dimenional index of the point to set in the cube.
   */
  bool setValue(unsigned int i, double value);

  /**
   * @return The minimum  value at any point in the Cube.
   */
  double minValue() const { return m_minValue; }

  /**
   * @return The maximum  value at any point in the Cube.
   */
  double maxValue() const { return m_maxValue; }

  void setName(const QString &name) { m_name = name; }
  QString name() const { return m_name; }

  void setCubeType(Type type) { m_cubeType = type; }
  Type cubeType() const { return m_cubeType; }

  /**
   * Provides locking.
   */
  QReadWriteLock * lock() const;

protected:
  std::vector<double> m_data;
  Eigen::Vector3d m_min, m_max, m_spacing;
  Eigen::Vector3i m_points;
  double m_minValue, m_maxValue;
  QString m_name;
  Type    m_cubeType;
  QReadWriteLock *m_lock;
};

inline bool Cube::setValue(unsigned int i, double value)
{
  if (i < m_data.size()) {
    m_data[i] = value;
    if (value > m_maxValue)
      m_maxValue = value;
    if (value < m_minValue)
      m_minValue = value;
    return true;
  }
  else
    return false;
}

} // End namespace

#endif
