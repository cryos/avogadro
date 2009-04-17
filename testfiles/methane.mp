*** Jobname.Temp
!file,2,INSERT WAVEFUNCTION FILE LOCATION HERE
!memory,INSERT MEMORY HERE
basis,6-31g

gprint,basis
gprint,orbitals

geomtyp=xyz
geometry={
5
Geometry specification:
  C,       -0.00092,       -0.00000,       -0.00289
  H,       -0.62971,       -0.93095,       -0.19667
  H,       -0.62951,        0.93099,       -0.19709
  H,        0.92146,       -0.00025,       -0.67283
  H,        0.34326,        0.00021,        1.08392
}

!INSERT QM METHODS HERE
hf
orbprint,12

frequencies
---
