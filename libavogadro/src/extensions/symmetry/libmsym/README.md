# LIBrary for Molecular SYMmetry

libmsym is a C library dealing with point group symmetry in molecules. It can determine, symmetrize and generate molecules of any point group. It can also generate symmetry adapted linear combinations of atomic orbitals for a subset of all point groups and orbital angular momentum (l), and project orbitals into the irreducible representation with the larges component.

## Performance

120 (Ih) symmetry operations in C-720 fullerene found in ~30ms (2012 macbook air).
Detection, generation of permutation information and symmetrisation of above in ~70ms.
T point group protein with 15k+ elements detected and symmetrized in ~6s.
300 symmetry adapted orbitals of minimal basis C-60 buckminster fullerene generated in 0.18s

## Development

This was initially developed as part of a masters, and is now a hobby project.
There is no stable release yet, although the master branch will be kept as stable as possible.
API is still subject to change (especially changes related to basis functions and irreducible representations).

There are several additions/changes related to LCAO/SALC generation, orbital symmetrization, and vibrational symmetry under development.


