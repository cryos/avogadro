/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2016 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef AVOGADRO_SPECIALKPOINTS_H
#define AVOGADRO_SPECIALKPOINTS_H

#include <avogadro/molecule.h>

#include <QString>

namespace Avogadro {

  class SpecialKPoints {
  public:
    /* Get the special k points for a particular space group. It will
     * be returned like the following:
     *
     * GM 0 0 0
     * Y  0 0.5 0
     * etc.
     *
     * @param mol The molecule for which to get a special k point. The space
     *            group needs to be set. If not, it will be assumed to be P1.
     *
     * @return A QString containing the data for the special kpoint.
     */
    static QString getSpecialKPoints(Molecule* mol);

  private:
    /* Process a special k point with a condition. These special k points
     * are separated in specialkpointsdata.h by a '#' symbol. This function
     * has stored in it all of the cases for different conditions. It will
     * process and change the @param specialKPoints to be the correct
     * QString for the @param molecule.
     *
     */
    static void processConditionKPoints(QString& specialKPoints,
                                        Molecule* mol);
  };

}

#endif
