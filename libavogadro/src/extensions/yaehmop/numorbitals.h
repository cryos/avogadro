/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2017 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef NUMORBITALS_H
#define NUMORBITALS_H

static int _num_orbs[] = { 0, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 9, 4, 4, 4, 4, 4, 4, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 4, 4, 4, 4, 4, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 4, 4, 4, 4, 9, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 9, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 4, 4, 4, 4, 4, 9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

// Returns the number of orbitals that YAeHMOP uses for any given atomic number.
// If there is 1 orbital, it is just s. If there are 4 orbitals, it is s and p. If
// there are 9 orbitals, it is s, p, and d. If there are 16 orbitals, it is s, p, d,
// and f.
// Maximum atomic number is 104.

static int getNumYaehmopOrbitals(int atomicNum)
{
  if (atomicNum > 104)
    return 0;
  return _num_orbs[atomicNum];
}

#endif
