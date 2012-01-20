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

#ifndef OQ_BASISSETLOADER_H
#define OQ_BASISSETLOADER_H

#include "openqubeabi.h"

// Forward declarations
class QString;

namespace OpenQube {

class BasisSet;

/**
 * @class BasisSetLoader basissetloader.h <openqube/basissetloader.h>
 * @brief BasisSetLoader chooses the correct parser, loads the file and returns
 * a basis set of the correct type.
 * @author Marcus D. Hanwell
 *
 * This class is very much subject to change. It removes the logic from the
 * individual classes, and takes care of choosing the correct parser before
 * loading a basis set and returning an object containing this data.
 */

class OPENQUBE_EXPORT BasisSetLoader
{
public:
  /**
   * Try to match the basis set to the supplied file path. This function will
   * search for a matching basis set file in the same directory.
   *
   * @return Proposed file that would be loaded. Empty if no file found.
   */
  static QString MatchBasisSet(const QString& filename);

  /**
   * Try to match the basis set to the supplied file path. This function will
   * search for a matching basis set file in the same directory.
   *
   * @param basisName char array that is overwritten with the corresponding
   * basis set file for @a filename. Ensure that this array is long enough to
   * contain the basis set name and path. Zero if no match found.
   */
  static void MatchBasisSet(const char *filename, char *basisName );

  /**
   * Load the supplied output file. The filename should be a valid quantum
   * output file.
   *
   * @return A BasisSet object populated with data file the file. Null on error.
   */
  static BasisSet * LoadBasisSet(const QString& filename);

  /**
   * Load the supplied output file. The filename should be a valid quantum
   * output file.
   *
   * @return A BasisSet object populated with data file the file. Null on error.
   */
  static BasisSet * LoadBasisSet(const char *filename);
};

} // End namespace

#endif
