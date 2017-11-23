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

#ifndef NUMVALENCEELECTRONS_H
#define NUMVALENCEELECTRONS_H

/* Returns the number of valence electrons for the neutral element with
 * atomic number @atomicNum. Full d and f shells DO count towards
 * the number of valence electrons. The numbers only go up to 90. They are
 * set to match that of YAeHMOP.
 *
 * @param atomicNum The atomic number of the element of interest.
 * @return The number of valence electrons for the element (assuming it is
 *         neutral).
 */
static unsigned char numValenceElectrons(unsigned char atomicNum)
{
  // This does not count full d and f shells towards the valence electrons
  switch (atomicNum) {

  // Hydrogen and alkali metals
  case 1:
  case 3:
  case 11:
  case 19:
  case 37:
  case 55:
  case 87:
    return 1;

  // Helium and alkaline earth metals
  case 2:
  case 4:
  case 12:
  case 20:
  case 38:
  case 56:
  case 88:
    return 2;

  // Boron group
  case 5:
  case 13:
  case 31:
  case 49:
  case 81:
  // Scandium group
  case 21:
  case 39:
  // Lanthanum group
  case 57:
  case 89:
    return 3;

  // Carbon group
  case 6:
  case 14:
  case 32:
  case 50:
  case 82:
  // Titanium group
  case 22:
  case 40:
  case 72:
  // Cerium group
  case 58:
    return 4;

  // Pnictogens
  case 7:
  case 15:
  case 33:
  case 51:
  case 83:
  // Vanadium group
  case 23:
  case 41:
  case 73:
  // Praseodymium group
  case 59:
    return 5;

  // Chalcogens
  case 8:
  case 16:
  case 34:
  case 52:
  case 84:
  // Chromium group
  case 24:
  case 42:
  case 74:
  // Neodymium group
  case 60:
    return 6;

  // Halogens
  case 9:
  case 17:
  case 35:
  case 53:
  case 85:
  // Manganese group
  case 25:
  case 43:
  case 75:
  // Promethium group
  case 61:
    return 7;

  // Noble gases (other than helium)
  case 10:
  case 18:
  case 36:
  case 54:
  case 86:
  // Iron group
  case 26:
  case 44:
  case 76:
  // Samarium group
  case 62:
    return 8;

  // Cobalt group
  case 27:
  case 45:
  case 77:
  // Europeum group
  case 63:
    return 9;

  // Nickel group
  case 28:
  case 46:
  case 78:
  // Gadolinium group
  case 64:
    return 10;

  // Copper group
  case 29:
  case 47:
  case 79:
  // Terbium group
  case 65:
    return 11;

  // Zinc group
  case 30:
  case 48:
  case 80:
  // Dysprosium group
  case 66:
    return 12;

  // Holmium group
  case 67:
    return 13;

  // Erbium group
  case 68:
    return 14;

  // Thulium group
  case 69:
    return 15;

  // Ytterbium group
  case 70:
    return 16;

  // Lutetium
  case 71:
    return 17;

  // Hopefully we won't get here
  default:
    return 0;
  }
}

/* Returns the total number of valence electrons for a vector of atomic
 * numbers.
 *
 * @param atomicNums The vector of atomic numbers.
 * @return The total number of valence electrons.
 */
static size_t numValenceElectrons(const std::vector<unsigned char>& atomicNums)
{
  size_t ret = 0;
  for (size_t i = 0; i < atomicNums.size(); ++i)
    ret += numValenceElectrons(atomicNums[i]);
  return ret;
}

#endif
