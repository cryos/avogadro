/* spg_database.c */
/* Copyright (C) 2010 Atsushi Togo */

#include "spg_database.h"

/* In Hall symbols (4th column), '=' is used instead of '"'. */
static const SpacegroupType spacegroup_types[] = {
  {  0, "      ", "                ", "                               ", "                   ", "          ",          }, /*  0*/
  {  1, "C1^1  ", "P 1             ", "P 1                            ", "P 1                ", "P1        ", NO_CENTER}, /*  1*/
  {  2, "Ci^1  ", "-P 1            ", "P -1                           ", "P -1               ", "P-1       ", NO_CENTER}, /*  2*/
  {  3, "C2^1  ", "P 2y            ", "P 2 = P 1 2 1                  ", "P 1 2 1            ", "P2        ", NO_CENTER}, /*  3*/
  {  3, "C2^1  ", "P 2             ", "P 2 = P 1 1 2                  ", "P 1 1 2            ", "P2        ", NO_CENTER}, /*  4*/
  {  3, "C2^1  ", "P 2x            ", "P 2 = P 2 1 1                  ", "P 2 1 1            ", "P2        ", NO_CENTER}, /*  5*/
  {  4, "C2^2  ", "P 2yb           ", "P 2_1 = P 1 2_1 1              ", "P 1 2_1 1          ", "P2_1      ", NO_CENTER}, /*  6*/
  {  4, "C2^2  ", "P 2c            ", "P 2_1 = P 1 1 2_1              ", "P 1 1 2_1          ", "P2_1      ", NO_CENTER}, /*  7*/
  {  4, "C2^2  ", "P 2xa           ", "P 2_1 = P 2_1 1 1              ", "P 2_1 1 1          ", "P2_1      ", NO_CENTER}, /*  8*/
  {  5, "C2^3  ", "C 2y            ", "C 2 = C 1 2 1                  ", "C 1 2 1            ", "C2        ",    C_FACE}, /*  9*/
  {  5, "C2^3  ", "A 2y            ", "C 2 = A 1 2 1                  ", "A 1 2 1            ", "C2        ",    C_FACE}, /* 10*/
  {  5, "C2^3  ", "I 2y            ", "C 2 = I 1 2 1                  ", "I 1 2 1            ", "C2        ",    C_FACE}, /* 11*/
  {  5, "C2^3  ", "A 2             ", "C 2 = A 1 1 2                  ", "A 1 1 2            ", "C2        ",    C_FACE}, /* 12*/
  {  5, "C2^3  ", "B 2             ", "C 2 = B 1 1 2 = B 2            ", "B 1 1 2            ", "C2        ",    C_FACE}, /* 13*/
  {  5, "C2^3  ", "I 2             ", "C 2 = I 1 1 2                  ", "I 1 1 2            ", "C2        ",    C_FACE}, /* 14*/
  {  5, "C2^3  ", "B 2x            ", "C 2 = B 2 1 1                  ", "B 2 1 1            ", "C2        ",    C_FACE}, /* 15*/
  {  5, "C2^3  ", "C 2x            ", "C 2 = C 2 1 1                  ", "C 2 1 1            ", "C2        ",    C_FACE}, /* 16*/
  {  5, "C2^3  ", "I 2x            ", "C 2 = I 2 1 1                  ", "I 2 1 1            ", "C2        ",    C_FACE}, /* 17*/
  {  6, "Cs^1  ", "P -2y           ", "P m = P 1 m 1                  ", "P 1 m 1            ", "Pm        ", NO_CENTER}, /* 18*/
  {  6, "Cs^1  ", "P -2            ", "P m = P 1 1 m                  ", "P 1 1 m            ", "Pm        ", NO_CENTER}, /* 19*/
  {  6, "Cs^1  ", "P -2x           ", "P m = P m 1 1                  ", "P m 1 1            ", "Pm        ", NO_CENTER}, /* 20*/
  {  7, "Cs^2  ", "P -2yc          ", "P c = P 1 c 1                  ", "P 1 c 1            ", "Pc        ", NO_CENTER}, /* 21*/
  {  7, "Cs^2  ", "P -2yac         ", "P c = P 1 n 1                  ", "P 1 n 1            ", "Pc        ", NO_CENTER}, /* 22*/
  {  7, "Cs^2  ", "P -2ya          ", "P c = P 1 a 1                  ", "P 1 a 1            ", "Pc        ", NO_CENTER}, /* 23*/
  {  7, "Cs^2  ", "P -2a           ", "P c = P 1 1 a                  ", "P 1 1 a            ", "Pc        ", NO_CENTER}, /* 24*/
  {  7, "Cs^2  ", "P -2ab          ", "P c = P 1 1 n                  ", "P 1 1 n            ", "Pc        ", NO_CENTER}, /* 25*/
  {  7, "Cs^2  ", "P -2b           ", "P c = P 1 1 b = P b            ", "P 1 1 b            ", "Pc        ", NO_CENTER}, /* 26*/
  {  7, "Cs^2  ", "P -2xb          ", "P c = P b 1 1                  ", "P b 1 1            ", "Pc        ", NO_CENTER}, /* 27*/
  {  7, "Cs^2  ", "P -2xbc         ", "P c = P n 1 1                  ", "P n 1 1            ", "Pc        ", NO_CENTER}, /* 28*/
  {  7, "Cs^2  ", "P -2xc          ", "P c = P c 1 1                  ", "P c 1 1            ", "Pc        ", NO_CENTER}, /* 29*/
  {  8, "Cs^3  ", "C -2y           ", "C m = C 1 m 1                  ", "C 1 m 1            ", "Cm        ",    C_FACE}, /* 30*/
  {  8, "Cs^3  ", "A -2y           ", "C m = A 1 m 1                  ", "A 1 m 1            ", "Cm        ",    C_FACE}, /* 31*/
  {  8, "Cs^3  ", "I -2y           ", "C m = I 1 m 1                  ", "I 1 m 1            ", "Cm        ",    C_FACE}, /* 32*/
  {  8, "Cs^3  ", "A -2            ", "C m = A 1 1 m                  ", "A 1 1 m            ", "Cm        ",    C_FACE}, /* 33*/
  {  8, "Cs^3  ", "B -2            ", "C m = B 1 1 m = B m            ", "B 1 1 m            ", "Cm        ",    C_FACE}, /* 34*/
  {  8, "Cs^3  ", "I -2            ", "C m = I 1 1 m                  ", "I 1 1 m            ", "Cm        ",    C_FACE}, /* 35*/
  {  8, "Cs^3  ", "B -2x           ", "C m = B m 1 1                  ", "B m 1 1            ", "Cm        ",    C_FACE}, /* 36*/
  {  8, "Cs^3  ", "C -2x           ", "C m = C m 1 1                  ", "C m 1 1            ", "Cm        ",    C_FACE}, /* 37*/
  {  8, "Cs^3  ", "I -2x           ", "C m = I m 1 1                  ", "I m 1 1            ", "Cm        ",    C_FACE}, /* 38*/
  {  9, "Cs^4  ", "C -2yc          ", "C c = C 1 c 1                  ", "C 1 c 1            ", "Cc        ",    C_FACE}, /* 39*/
  {  9, "Cs^4  ", "A -2yac         ", "C c = A 1 n 1                  ", "A 1 n 1            ", "Cc        ",    C_FACE}, /* 40*/
  {  9, "Cs^4  ", "I -2ya          ", "C c = I 1 a 1                  ", "I 1 a 1            ", "Cc        ",    C_FACE}, /* 41*/
  {  9, "Cs^4  ", "A -2ya          ", "C c = A 1 a 1                  ", "A 1 a 1            ", "Cc        ",    C_FACE}, /* 42*/
  {  9, "Cs^4  ", "C -2ybc         ", "C c = C 1 n 1                  ", "C 1 n 1            ", "Cc        ",    C_FACE}, /* 43*/
  {  9, "Cs^4  ", "I -2yc          ", "C c = I 1 c 1                  ", "I 1 c 1            ", "Cc        ",    C_FACE}, /* 44*/
  {  9, "Cs^4  ", "A -2a           ", "C c = A 1 1 a                  ", "A 1 1 a            ", "Cc        ",    C_FACE}, /* 45*/
  {  9, "Cs^4  ", "B -2bc          ", "C c = B 1 1 n                  ", "B 1 1 n            ", "Cc        ",    C_FACE}, /* 46*/
  {  9, "Cs^4  ", "I -2b           ", "C c = I 1 1 b                  ", "I 1 1 b            ", "Cc        ",    C_FACE}, /* 47*/
  {  9, "Cs^4  ", "B -2b           ", "C c = B 1 1 b = B b            ", "B 1 1 b            ", "Cc        ",    C_FACE}, /* 48*/
  {  9, "Cs^4  ", "A -2ac          ", "C c = A 1 1 n                  ", "A 1 1 n            ", "Cc        ",    C_FACE}, /* 49*/
  {  9, "Cs^4  ", "I -2a           ", "C c = I 1 1 a                  ", "I 1 1 a            ", "Cc        ",    C_FACE}, /* 50*/
  {  9, "Cs^4  ", "B -2xb          ", "C c = B b 1 1                  ", "B b 1 1            ", "Cc        ",    C_FACE}, /* 51*/
  {  9, "Cs^4  ", "C -2xbc         ", "C c = C n 1 1                  ", "C n 1 1            ", "Cc        ",    C_FACE}, /* 52*/
  {  9, "Cs^4  ", "I -2xc          ", "C c = I c 1 1                  ", "I c 1 1            ", "Cc        ",    C_FACE}, /* 53*/
  {  9, "Cs^4  ", "C -2xc          ", "C c = C c 1 1                  ", "C c 1 1            ", "Cc        ",    C_FACE}, /* 54*/
  {  9, "Cs^4  ", "B -2xbc         ", "C c = B n 1 1                  ", "B n 1 1            ", "Cc        ",    C_FACE}, /* 55*/
  {  9, "Cs^4  ", "I -2xb          ", "C c = I b 1 1                  ", "I b 1 1            ", "Cc        ",    C_FACE}, /* 56*/
  { 10, "C2h^1 ", "-P 2y           ", "P 2/m = P 1 2/m 1              ", "P 1 2/m 1          ", "P2/m      ", NO_CENTER}, /* 57*/
  { 10, "C2h^1 ", "-P 2            ", "P 2/m = P 1 1 2/m              ", "P 1 1 2/m          ", "P2/m      ", NO_CENTER}, /* 58*/
  { 10, "C2h^1 ", "-P 2x           ", "P 2/m = P 2/m 1 1              ", "P 2/m 1 1          ", "P2/m      ", NO_CENTER}, /* 59*/
  { 11, "C2h^2 ", "-P 2yb          ", "P 2_1/m = P 1 2_1/m 1          ", "P 1 2_1/m 1        ", "P2_1/m    ", NO_CENTER}, /* 60*/
  { 11, "C2h^2 ", "-P 2c           ", "P 2_1/m = P 1 1 2_1/m          ", "P 1 1 2_1/m        ", "P2_1/m    ", NO_CENTER}, /* 61*/
  { 11, "C2h^2 ", "-P 2xa          ", "P 2_1/m = P 2_1/m 1 1          ", "P 2_1/m 1 1        ", "P2_1/m    ", NO_CENTER}, /* 62*/
  { 12, "C2h^3 ", "-C 2y           ", "C 2/m = C 1 2/m 1              ", "C 1 2/m 1          ", "C2/m      ",    C_FACE}, /* 63*/
  { 12, "C2h^3 ", "-A 2y           ", "C 2/m = A 1 2/m 1              ", "A 1 2/m 1          ", "C2/m      ",    C_FACE}, /* 64*/
  { 12, "C2h^3 ", "-I 2y           ", "C 2/m = I 1 2/m 1              ", "I 1 2/m 1          ", "C2/m      ",    C_FACE}, /* 65*/
  { 12, "C2h^3 ", "-A 2            ", "C 2/m = A 1 1 2/m              ", "A 1 1 2/m          ", "C2/m      ",    C_FACE}, /* 66*/
  { 12, "C2h^3 ", "-B 2            ", "C 2/m = B 1 1 2/m = B 2/m      ", "B 1 1 2/m          ", "C2/m      ",    C_FACE}, /* 67*/
  { 12, "C2h^3 ", "-I 2            ", "C 2/m = I 1 1 2/m              ", "I 1 1 2/m          ", "C2/m      ",    C_FACE}, /* 68*/
  { 12, "C2h^3 ", "-B 2x           ", "C 2/m = B 2/m 1 1              ", "B 2/m 1 1          ", "C2/m      ",    C_FACE}, /* 69*/
  { 12, "C2h^3 ", "-C 2x           ", "C 2/m = C 2/m 1 1              ", "C 2/m 1 1          ", "C2/m      ",    C_FACE}, /* 70*/
  { 12, "C2h^3 ", "-I 2x           ", "C 2/m = I 2/m 1 1              ", "I 2/m 1 1          ", "C2/m      ",    C_FACE}, /* 71*/
  { 13, "C2h^4 ", "-P 2yc          ", "P 2/c = P 1 2/c 1              ", "P 1 2/c 1          ", "P2/c      ", NO_CENTER}, /* 72*/
  { 13, "C2h^4 ", "-P 2yac         ", "P 2/c = P 1 2/n 1              ", "P 1 2/n 1          ", "P2/c      ", NO_CENTER}, /* 73*/
  { 13, "C2h^4 ", "-P 2ya          ", "P 2/c = P 1 2/a 1              ", "P 1 2/a 1          ", "P2/c      ", NO_CENTER}, /* 74*/
  { 13, "C2h^4 ", "-P 2a           ", "P 2/c = P 1 1 2/a              ", "P 1 1 2/a          ", "P2/c      ", NO_CENTER}, /* 75*/
  { 13, "C2h^4 ", "-P 2ab          ", "P 2/c = P 1 1 2/n              ", "P 1 1 2/n          ", "P2/c      ", NO_CENTER}, /* 76*/
  { 13, "C2h^4 ", "-P 2b           ", "P 2/c = P 1 1 2/b = P 2/b      ", "P 1 1 2/b          ", "P2/c      ", NO_CENTER}, /* 77*/
  { 13, "C2h^4 ", "-P 2xb          ", "P 2/c = P 2/b 1 1              ", "P 2/b 1 1          ", "P2/c      ", NO_CENTER}, /* 78*/
  { 13, "C2h^4 ", "-P 2xbc         ", "P 2/c = P 2/n 1 1              ", "P 2/n 1 1          ", "P2/c      ", NO_CENTER}, /* 79*/
  { 13, "C2h^4 ", "-P 2xc          ", "P 2/c = P 2/c 1 1              ", "P 2/c 1 1          ", "P2/c      ", NO_CENTER}, /* 80*/
  { 14, "C2h^5 ", "-P 2ybc         ", "P 2_1/c = P 1 2_1/c 1          ", "P 1 2_1/c 1        ", "P2_1/c    ", NO_CENTER}, /* 81*/
  { 14, "C2h^5 ", "-P 2yn          ", "P 2_1/c = P 1 2_1/n 1          ", "P 1 2_1/n 1        ", "P2_1/c    ", NO_CENTER}, /* 82*/
  { 14, "C2h^5 ", "-P 2yab         ", "P 2_1/c = P 1 2_1/a 1          ", "P 1 2_1/a 1        ", "P2_1/c    ", NO_CENTER}, /* 83*/
  { 14, "C2h^5 ", "-P 2ac          ", "P 2_1/c = P 1 1 2_1/a          ", "P 1 1 2_1/a        ", "P2_1/c    ", NO_CENTER}, /* 84*/
  { 14, "C2h^5 ", "-P 2n           ", "P 2_1/c = P 1 1 2_1/n          ", "P 1 1 2_1/n        ", "P2_1/c    ", NO_CENTER}, /* 85*/
  { 14, "C2h^5 ", "-P 2bc          ", "P 2_1/c = P 1 1 2_1/b = P 2_1/b", "P 1 1 2_1/b        ", "P2_1/c    ", NO_CENTER}, /* 86*/
  { 14, "C2h^5 ", "-P 2xab         ", "P 2_1/c = P 2_1/b 1 1          ", "P 2_1/b 1 1        ", "P2_1/c    ", NO_CENTER}, /* 87*/
  { 14, "C2h^5 ", "-P 2xn          ", "P 2_1/c = P 2_1/n 1 1          ", "P 2_1/n 1 1        ", "P2_1/c    ", NO_CENTER}, /* 88*/
  { 14, "C2h^5 ", "-P 2xac         ", "P 2_1/c = P 2_1/c 1 1          ", "P 2_1/c 1 1        ", "P2_1/c    ", NO_CENTER}, /* 89*/
  { 15, "C2h^6 ", "-C 2yc          ", "C 2/c = C 1 2/c 1              ", "C 1 2/c 1          ", "C2/c      ",    C_FACE}, /* 90*/
  { 15, "C2h^6 ", "-A 2yac         ", "C 2/c = A 1 2/n 1              ", "A 1 2/n 1          ", "C2/c      ",    C_FACE}, /* 91*/
  { 15, "C2h^6 ", "-I 2ya          ", "C 2/c = I 1 2/a 1              ", "I 1 2/a 1          ", "C2/c      ",    C_FACE}, /* 92*/
  { 15, "C2h^6 ", "-A 2ya          ", "C 2/c = A 1 2/a 1              ", "A 1 2/a 1          ", "C2/c      ",    C_FACE}, /* 93*/
  { 15, "C2h^6 ", "-C 2ybc         ", "C 2/c = C 1 2/n 1              ", "C 1 2/n 1          ", "C2/c      ",    C_FACE}, /* 94*/
  { 15, "C2h^6 ", "-I 2yc          ", "C 2/c = I 1 2/c 1              ", "I 1 2/c 1          ", "C2/c      ",    C_FACE}, /* 95*/
  { 15, "C2h^6 ", "-A 2a           ", "C 2/c = A 1 1 2/a              ", "A 1 1 2/a          ", "C2/c      ",    C_FACE}, /* 96*/
  { 15, "C2h^6 ", "-B 2bc          ", "C 2/c = B 1 1 2/n              ", "B 1 1 2/n          ", "C2/c      ",    C_FACE}, /* 97*/
  { 15, "C2h^6 ", "-I 2b           ", "C 2/c = I 1 1 2/b              ", "I 1 1 2/b          ", "C2/c      ",    C_FACE}, /* 98*/
  { 15, "C2h^6 ", "-B 2b           ", "C 2/c = B 1 1 2/b = B 2/b      ", "B 1 1 2/b          ", "C2/c      ",    C_FACE}, /* 99*/
  { 15, "C2h^6 ", "-A 2ac          ", "C 2/c = A 1 1 2/n              ", "A 1 1 2/n          ", "C2/c      ",    C_FACE}, /*100*/
  { 15, "C2h^6 ", "-I 2a           ", "C 2/c = I 1 1 2/a              ", "I 1 1 2/a          ", "C2/c      ",    C_FACE}, /*101*/
  { 15, "C2h^6 ", "-B 2xb          ", "C 2/c = B 2/b 1 1              ", "B 2/b 1 1          ", "C2/c      ",    C_FACE}, /*102*/
  { 15, "C2h^6 ", "-C 2xbc         ", "C 2/c = C 2/n 1 1              ", "C 2/n 1 1          ", "C2/c      ",    C_FACE}, /*103*/
  { 15, "C2h^6 ", "-I 2xc          ", "C 2/c = I 2/c 1 1              ", "I 2/c 1 1          ", "C2/c      ",    C_FACE}, /*104*/
  { 15, "C2h^6 ", "-C 2xc          ", "C 2/c = C 2/c 1 1              ", "C 2/c 1 1          ", "C2/c      ",    C_FACE}, /*105*/
  { 15, "C2h^6 ", "-B 2xbc         ", "C 2/c = B 2/n 1 1              ", "B 2/n 1 1          ", "C2/c      ",    C_FACE}, /*106*/
  { 15, "C2h^6 ", "-I 2xb          ", "C 2/c = I 2/b 1 1              ", "I 2/b 1 1          ", "C2/c      ",    C_FACE}, /*107*/
  { 16, "D2^1  ", "P 2 2           ", "P 2 2 2                        ", "P 2 2 2            ", "P222      ", NO_CENTER}, /*108*/
  { 17, "D2^2  ", "P 2c 2          ", "P 2 2 2_1                      ", "P 2 2 2_1          ", "P222_1    ", NO_CENTER}, /*109*/
  { 17, "D2^2  ", "P 2a 2a         ", "P 2_1 2 2                      ", "P 2_1 2 2          ", "P2_122    ", NO_CENTER}, /*110*/
  { 17, "D2^2  ", "P 2 2b          ", "P 2 2_1 2                      ", "P 2 2_1 2          ", "P22_12    ", NO_CENTER}, /*111*/
  { 18, "D2^3  ", "P 2 2ab         ", "P 2_1 2_1 2                    ", "P 2_1 2_1 2        ", "P2_12_12  ", NO_CENTER}, /*112*/
  { 18, "D2^3  ", "P 2bc 2         ", "P 2 2_1 2_1                    ", "P 2 2_1 2_1        ", "P22_12_1  ", NO_CENTER}, /*113*/
  { 18, "D2^3  ", "P 2ac 2ac       ", "P 2_1 2 2_1                    ", "P 2_1 2 2_1        ", "P2_122_1  ", NO_CENTER}, /*114*/
  { 19, "D2^4  ", "P 2ac 2ab       ", "P 2_1 2_1 2_1                  ", "P 2_1 2_1 2_1      ", "P2_12_12_1", NO_CENTER}, /*115*/
  { 20, "D2^5  ", "C 2c 2          ", "C 2 2 2_1                      ", "C 2 2 2_1          ", "C222_1    ",    C_FACE}, /*116*/
  { 20, "D2^5  ", "A 2a 2a         ", "A 2_1 2 2                      ", "A 2_1 2 2          ", "A2_122    ",    A_FACE}, /*117*/
  { 20, "D2^5  ", "B 2 2b          ", "B 2 2_1 2                      ", "B 2 2_1 2          ", "B22_12    ",    B_FACE}, /*118*/
  { 21, "D2^6  ", "C 2 2           ", "C 2 2 2                        ", "C 2 2 2            ", "C222      ",    C_FACE}, /*119*/
  { 21, "D2^6  ", "A 2 2           ", "A 2 2 2                        ", "A 2 2 2            ", "A222      ",    A_FACE}, /*120*/
  { 21, "D2^6  ", "B 2 2           ", "B 2 2 2                        ", "B 2 2 2            ", "B222      ",    B_FACE}, /*121*/
  { 22, "D2^7  ", "F 2 2           ", "F 2 2 2                        ", "F 2 2 2            ", "F222      ",      FACE}, /*122*/
  { 23, "D2^8  ", "I 2 2           ", "I 2 2 2                        ", "I 2 2 2            ", "I222      ",      BODY}, /*123*/
  { 24, "D2^9  ", "I 2b 2c         ", "I 2_1 2_1 2_1                  ", "I 2_1 2_1 2_1      ", "I2_12_12_1",      BODY}, /*124*/
  { 25, "C2v^1 ", "P 2 -2          ", "P m m 2                        ", "P m m 2            ", "Pmm2      ", NO_CENTER}, /*125*/
  { 25, "C2v^1 ", "P -2 2          ", "P 2 m m                        ", "P 2 m m            ", "P2mm      ", NO_CENTER}, /*126*/
  { 25, "C2v^1 ", "P -2 -2         ", "P m 2 m                        ", "P m 2 m            ", "Pm2m      ", NO_CENTER}, /*127*/
  { 26, "C2v^2 ", "P 2c -2         ", "P m c 2_1                      ", "P m c 2_1          ", "Pmc2_1    ", NO_CENTER}, /*128*/
  { 26, "C2v^2 ", "P 2c -2c        ", "P c m 2_1                      ", "P c m 2_1          ", "Pcm2_1    ", NO_CENTER}, /*129*/
  { 26, "C2v^2 ", "P -2a 2a        ", "P 2_1 m a                      ", "P 2_1 m a          ", "P2_1ma    ", NO_CENTER}, /*130*/
  { 26, "C2v^2 ", "P -2 2a         ", "P 2_1 a m                      ", "P 2_1 a m          ", "P2_1am    ", NO_CENTER}, /*131*/
  { 26, "C2v^2 ", "P -2 -2b        ", "P b 2_1 m                      ", "P b 2_1 m          ", "Pb2_1m    ", NO_CENTER}, /*132*/
  { 26, "C2v^2 ", "P -2b -2        ", "P m 2_1 b                      ", "P m 2_1 b          ", "Pm2_1b    ", NO_CENTER}, /*133*/
  { 27, "C2v^3 ", "P 2 -2c         ", "P c c 2                        ", "P c c 2            ", "Pcc2      ", NO_CENTER}, /*134*/
  { 27, "C2v^3 ", "P -2a 2         ", "P 2 a a                        ", "P 2 a a            ", "P2aa      ", NO_CENTER}, /*135*/
  { 27, "C2v^3 ", "P -2b -2b       ", "P b 2 b                        ", "P b 2 b            ", "Pb2b      ", NO_CENTER}, /*136*/
  { 28, "C2v^4 ", "P 2 -2a         ", "P m a 2                        ", "P m a 2            ", "Pma2      ", NO_CENTER}, /*137*/
  { 28, "C2v^4 ", "P 2 -2b         ", "P b m 2                        ", "P b m 2            ", "Pbm2      ", NO_CENTER}, /*138*/
  { 28, "C2v^4 ", "P -2b 2         ", "P 2 m b                        ", "P 2 m b            ", "P2mb      ", NO_CENTER}, /*139*/
  { 28, "C2v^4 ", "P -2c 2         ", "P 2 c m                        ", "P 2 c m            ", "P2cm      ", NO_CENTER}, /*140*/
  { 28, "C2v^4 ", "P -2c -2c       ", "P c 2 m                        ", "P c 2 m            ", "Pc2m      ", NO_CENTER}, /*141*/
  { 28, "C2v^4 ", "P -2a -2a       ", "P m 2 a                        ", "P m 2 a            ", "Pm2a      ", NO_CENTER}, /*142*/
  { 29, "C2v^5 ", "P 2c -2ac       ", "P c a 2_1                      ", "P c a 2_1          ", "Pca2_1    ", NO_CENTER}, /*143*/
  { 29, "C2v^5 ", "P 2c -2b        ", "P b c 2_1                      ", "P b c 2_1          ", "Pbc2_1    ", NO_CENTER}, /*144*/
  { 29, "C2v^5 ", "P -2b 2a        ", "P 2_1 a b                      ", "P 2_1 a b          ", "P2_1ab    ", NO_CENTER}, /*145*/
  { 29, "C2v^5 ", "P -2ac 2a       ", "P 2_1 c a                      ", "P 2_1 c a          ", "P2_1ca    ", NO_CENTER}, /*146*/
  { 29, "C2v^5 ", "P -2bc -2c      ", "P c 2_1 b                      ", "P c 2_1 b          ", "Pc2_1b    ", NO_CENTER}, /*147*/
  { 29, "C2v^5 ", "P -2a -2ab      ", "P b 2_1 a                      ", "P b 2_1 a          ", "Pb2_1a    ", NO_CENTER}, /*148*/
  { 30, "C2v^6 ", "P 2 -2bc        ", "P n c 2                        ", "P n c 2            ", "Pnc2      ", NO_CENTER}, /*149*/
  { 30, "C2v^6 ", "P 2 -2ac        ", "P c n 2                        ", "P c n 2            ", "Pcn2      ", NO_CENTER}, /*150*/
  { 30, "C2v^6 ", "P -2ac 2        ", "P 2 n a                        ", "P 2 n a            ", "P2na      ", NO_CENTER}, /*151*/
  { 30, "C2v^6 ", "P -2ab 2        ", "P 2 a n                        ", "P 2 a n            ", "P2an      ", NO_CENTER}, /*152*/
  { 30, "C2v^6 ", "P -2ab -2ab     ", "P b 2 n                        ", "P b 2 n            ", "Pb2n      ", NO_CENTER}, /*153*/
  { 30, "C2v^6 ", "P -2bc -2bc     ", "P n 2 b                        ", "P n 2 b            ", "Pn2b      ", NO_CENTER}, /*154*/
  { 31, "C2v^7 ", "P 2ac -2        ", "P m n 2_1                      ", "P m n 2_1          ", "Pmn2_1    ", NO_CENTER}, /*155*/
  { 31, "C2v^7 ", "P 2bc -2bc      ", "P n m 2_1                      ", "P n m 2_1          ", "Pnm2_1    ", NO_CENTER}, /*156*/
  { 31, "C2v^7 ", "P -2ab 2ab      ", "P 2_1 m n                      ", "P 2_1 m n          ", "P2_1mn    ", NO_CENTER}, /*157*/
  { 31, "C2v^7 ", "P -2 2ac        ", "P 2_1 n m                      ", "P 2_1 n m          ", "P2_1nm    ", NO_CENTER}, /*158*/
  { 31, "C2v^7 ", "P -2 -2bc       ", "P n 2_1 m                      ", "P n 2_1 m          ", "Pn2_1m    ", NO_CENTER}, /*159*/
  { 31, "C2v^7 ", "P -2ab -2       ", "P m 2_1 n                      ", "P m 2_1 n          ", "Pm2_1n    ", NO_CENTER}, /*160*/
  { 32, "C2v^8 ", "P 2 -2ab        ", "P b a 2                        ", "P b a 2            ", "Pba2      ", NO_CENTER}, /*161*/
  { 32, "C2v^8 ", "P -2bc 2        ", "P 2 c b                        ", "P 2 c b            ", "P2cb      ", NO_CENTER}, /*162*/
  { 32, "C2v^8 ", "P -2ac -2ac     ", "P c 2 a                        ", "P c 2 a            ", "Pc2a      ", NO_CENTER}, /*163*/
  { 33, "C2v^9 ", "P 2c -2n        ", "P n a 2_1                      ", "P n a 2_1          ", "Pna2_1    ", NO_CENTER}, /*164*/
  { 33, "C2v^9 ", "P 2c -2ab       ", "P b n 2_1                      ", "P b n 2_1          ", "Pbn2_1    ", NO_CENTER}, /*165*/
  { 33, "C2v^9 ", "P -2bc 2a       ", "P 2_1 n b                      ", "P 2_1 n b          ", "P2_1nb    ", NO_CENTER}, /*166*/
  { 33, "C2v^9 ", "P -2n 2a        ", "P 2_1 c n                      ", "P 2_1 c n          ", "P2_1cn    ", NO_CENTER}, /*167*/
  { 33, "C2v^9 ", "P -2n -2ac      ", "P c 2_1 n                      ", "P c 2_1 n          ", "Pc2_1n    ", NO_CENTER}, /*168*/
  { 33, "C2v^9 ", "P -2ac -2n      ", "P n 2_1 a                      ", "P n 2_1 a          ", "Pn2_1a    ", NO_CENTER}, /*169*/
  { 34, "C2v^10", "P 2 -2n         ", "P n n 2                        ", "P n n 2            ", "Pnn2      ", NO_CENTER}, /*170*/
  { 34, "C2v^10", "P -2n 2         ", "P 2 n n                        ", "P 2 n n            ", "P2nn      ", NO_CENTER}, /*171*/
  { 34, "C2v^10", "P -2n -2n       ", "P n 2 n                        ", "P n 2 n            ", "Pn2n      ", NO_CENTER}, /*172*/
  { 35, "C2v^11", "C 2 -2          ", "C m m 2                        ", "C m m 2            ", "Cmm2      ",    C_FACE}, /*173*/
  { 35, "C2v^11", "A -2 2          ", "A 2 m m                        ", "A 2 m m            ", "A2mm      ",    A_FACE}, /*174*/
  { 35, "C2v^11", "B -2 -2         ", "B m 2 m                        ", "B m 2 m            ", "Bm2m      ",    B_FACE}, /*175*/
  { 36, "C2v^12", "C 2c -2         ", "C m c 2_1                      ", "C m c 2_1          ", "Cmc2_1    ",    C_FACE}, /*176*/
  { 36, "C2v^12", "C 2c -2c        ", "C c m 2_1                      ", "C c m 2_1          ", "Ccm2_1    ",    C_FACE}, /*177*/
  { 36, "C2v^12", "A -2a 2a        ", "A 2_1 m a                      ", "A 2_1 m a          ", "A2_1ma    ",    A_FACE}, /*178*/
  { 36, "C2v^12", "A -2 2a         ", "A 2_1 a m                      ", "A 2_1 a m          ", "A2_1am    ",    A_FACE}, /*179*/
  { 36, "C2v^12", "B -2 -2b        ", "B b 2_1 m                      ", "B b 2_1 m          ", "Bb2_1m    ",    B_FACE}, /*180*/
  { 36, "C2v^12", "B -2b -2        ", "B m 2_1 b                      ", "B m 2_1 b          ", "Bm2_1b    ",    B_FACE}, /*181*/
  { 37, "C2v^13", "C 2 -2c         ", "C c c 2                        ", "C c c 2            ", "Ccc2      ",    C_FACE}, /*182*/
  { 37, "C2v^13", "A -2a 2         ", "A 2 a a                        ", "A 2 a a            ", "A2aa      ",    A_FACE}, /*183*/
  { 37, "C2v^13", "B -2b -2b       ", "B b 2 b                        ", "B b 2 b            ", "Bb2b      ",    B_FACE}, /*184*/
  { 38, "C2v^14", "A 2 -2          ", "A m m 2                        ", "A m m 2            ", "Amm2      ",    A_FACE}, /*185*/
  { 38, "C2v^14", "B 2 -2          ", "B m m 2                        ", "B m m 2            ", "Bmm2      ",    B_FACE}, /*186*/
  { 38, "C2v^14", "B -2 2          ", "B 2 m m                        ", "B 2 m m            ", "B2mm      ",    B_FACE}, /*187*/
  { 38, "C2v^14", "C -2 2          ", "C 2 m m                        ", "C 2 m m            ", "C2mm      ",    C_FACE}, /*188*/
  { 38, "C2v^14", "C -2 -2         ", "C m 2 m                        ", "C m 2 m            ", "Cm2m      ",    C_FACE}, /*189*/
  { 38, "C2v^14", "A -2 -2         ", "A m 2 m                        ", "A m 2 m            ", "Am2m      ",    A_FACE}, /*190*/
  { 39, "C2v^15", "A 2 -2c         ", "A e m 2                        ", "A e m 2            ", "Aem2      ",    A_FACE}, /*191*/
  { 39, "C2v^15", "B 2 -2c         ", "B m e 2                        ", "B m e 2            ", "Bme2      ",    B_FACE}, /*192*/
  { 39, "C2v^15", "B -2c 2         ", "B 2 e m                        ", "B 2 e m            ", "B2em      ",    B_FACE}, /*193*/
  { 39, "C2v^15", "C -2b 2         ", "C 2 m e                        ", "C 2 m e            ", "C2me      ",    C_FACE}, /*194*/
  { 39, "C2v^15", "C -2b -2b       ", "C m 2 e                        ", "C m 2 e            ", "Cm2e      ",    C_FACE}, /*195*/
  { 39, "C2v^15", "A -2c -2c       ", "A e 2 m                        ", "A e 2 m            ", "Ae2m      ",    A_FACE}, /*196*/
  { 40, "C2v^16", "A 2 -2a         ", "A m a 2                        ", "A m a 2            ", "Ama2      ",    A_FACE}, /*197*/
  { 40, "C2v^16", "B 2 -2b         ", "B b m 2                        ", "B b m 2            ", "Bbm2      ",    B_FACE}, /*198*/
  { 40, "C2v^16", "B -2b 2         ", "B 2 m b                        ", "B 2 m b            ", "B2mb      ",    B_FACE}, /*199*/
  { 40, "C2v^16", "C -2c 2         ", "C 2 c m                        ", "C 2 c m            ", "C2cm      ",    C_FACE}, /*200*/
  { 40, "C2v^16", "C -2c -2c       ", "C c 2 m                        ", "C c 2 m            ", "Cc2m      ",    C_FACE}, /*201*/
  { 40, "C2v^16", "A -2a -2a       ", "A m 2 a                        ", "A m 2 a            ", "Am2a      ",    A_FACE}, /*202*/
  { 41, "C2v^17", "A 2 -2ac        ", "A e a 2                        ", "A e a 2            ", "Aea2      ",    A_FACE}, /*203*/
  { 41, "C2v^17", "B 2 -2bc        ", "B b e 2                        ", "B b e 2            ", "Bbe2      ",    B_FACE}, /*204*/
  { 41, "C2v^17", "B -2bc 2        ", "B 2 e b                        ", "B 2 e b            ", "B2eb      ",    B_FACE}, /*205*/
  { 41, "C2v^17", "C -2bc 2        ", "C 2 c e                        ", "C 2 c e            ", "C2ce      ",    C_FACE}, /*206*/
  { 41, "C2v^17", "C -2bc -2bc     ", "C c 2 e                        ", "C c 2 e            ", "Cc2e      ",    C_FACE}, /*207*/
  { 41, "C2v^17", "A -2ac -2ac     ", "A e 2 a                        ", "A e 2 a            ", "Ae2a      ",    A_FACE}, /*208*/
  { 42, "C2v^18", "F 2 -2          ", "F m m 2                        ", "F m m 2            ", "Fmm2      ",      FACE}, /*209*/
  { 42, "C2v^18", "F -2 2          ", "F 2 m m                        ", "F 2 m m            ", "F2mm      ",      FACE}, /*210*/
  { 42, "C2v^18", "F -2 -2         ", "F m 2 m                        ", "F m 2 m            ", "Fm2m      ",      FACE}, /*211*/
  { 43, "C2v^19", "F 2 -2d         ", "F d d 2                        ", "F d d 2            ", "Fdd2      ",      FACE}, /*212*/
  { 43, "C2v^19", "F -2d 2         ", "F 2 d d                        ", "F 2 d d            ", "F2dd      ",      FACE}, /*213*/
  { 43, "C2v^19", "F -2d -2d       ", "F d 2 d                        ", "F d 2 d            ", "Fd2d      ",      FACE}, /*214*/
  { 44, "C2v^20", "I 2 -2          ", "I m m 2                        ", "I m m 2            ", "Imm2      ",      BODY}, /*215*/
  { 44, "C2v^20", "I -2 2          ", "I 2 m m                        ", "I 2 m m            ", "I2mm      ",      BODY}, /*216*/
  { 44, "C2v^20", "I -2 -2         ", "I m 2 m                        ", "I m 2 m            ", "Im2m      ",      BODY}, /*217*/
  { 45, "C2v^21", "I 2 -2c         ", "I b a 2                        ", "I b a 2            ", "Iba2      ",      BODY}, /*218*/
  { 45, "C2v^21", "I -2a 2         ", "I 2 c b                        ", "I 2 c b            ", "I2cb      ",      BODY}, /*219*/
  { 45, "C2v^21", "I -2b -2b       ", "I c 2 a                        ", "I c 2 a            ", "Ic2a      ",      BODY}, /*220*/
  { 46, "C2v^22", "I 2 -2a         ", "I m a 2                        ", "I m a 2            ", "Ima2      ",      BODY}, /*221*/
  { 46, "C2v^22", "I 2 -2b         ", "I b m 2                        ", "I b m 2            ", "Ibm2      ",      BODY}, /*222*/
  { 46, "C2v^22", "I -2b 2         ", "I 2 m b                        ", "I 2 m b            ", "I2mb      ",      BODY}, /*223*/
  { 46, "C2v^22", "I -2c 2         ", "I 2 c m                        ", "I 2 c m            ", "I2cm      ",      BODY}, /*224*/
  { 46, "C2v^22", "I -2c -2c       ", "I c 2 m                        ", "I c 2 m            ", "Ic2m      ",      BODY}, /*225*/
  { 46, "C2v^22", "I -2a -2a       ", "I m 2 a                        ", "I m 2 a            ", "Im2a      ",      BODY}, /*226*/
  { 47, "D2h^1 ", "-P 2 2          ", "P m m m                        ", "P 2/m 2/m 2/m      ", "Pmmm      ", NO_CENTER}, /*227*/
  { 48, "D2h^2 ", "P 2 2 -1n       ", "P n n n                        ", "P 2/n 2/n 2/n      ", "Pnnn      ", NO_CENTER}, /*228*/
  { 48, "D2h^2 ", "-P 2ab 2bc      ", "P n n n                        ", "P 2/n 2/n 2/n      ", "Pnnn      ", NO_CENTER}, /*229*/
  { 49, "D2h^3 ", "-P 2 2c         ", "P c c m                        ", "P 2/c 2/c 2/m      ", "Pccm      ", NO_CENTER}, /*230*/
  { 49, "D2h^3 ", "-P 2a 2         ", "P m a a                        ", "P 2/m 2/a 2/a      ", "Pmaa      ", NO_CENTER}, /*231*/
  { 49, "D2h^3 ", "-P 2b 2b        ", "P b m b                        ", "P 2/b 2/m 2/b      ", "Pbmb      ", NO_CENTER}, /*232*/
  { 50, "D2h^4 ", "P 2 2 -1ab      ", "P b a n                        ", "P 2/b 2/a 2/n      ", "Pban      ", NO_CENTER}, /*233*/
  { 50, "D2h^4 ", "-P 2ab 2b       ", "P b a n                        ", "P 2/b 2/a 2/n      ", "Pban      ", NO_CENTER}, /*234*/
  { 50, "D2h^4 ", "P 2 2 -1bc      ", "P n c b                        ", "P 2/n 2/c 2/b      ", "Pncb      ", NO_CENTER}, /*235*/
  { 50, "D2h^4 ", "-P 2b 2bc       ", "P n c b                        ", "P 2/n 2/c 2/b      ", "Pncb      ", NO_CENTER}, /*236*/
  { 50, "D2h^4 ", "P 2 2 -1ac      ", "P c n a                        ", "P 2/c 2/n 2/a      ", "Pcna      ", NO_CENTER}, /*237*/
  { 50, "D2h^4 ", "-P 2a 2c        ", "P c n a                        ", "P 2/c 2/n 2/a      ", "Pcna      ", NO_CENTER}, /*238*/
  { 51, "D2h^5 ", "-P 2a 2a        ", "P m m a                        ", "P 2_1/m 2/m 2/a    ", "Pmma      ", NO_CENTER}, /*239*/
  { 51, "D2h^5 ", "-P 2b 2         ", "P m m b                        ", "P 2/m 2_1/m 2/b    ", "Pmmb      ", NO_CENTER}, /*240*/
  { 51, "D2h^5 ", "-P 2 2b         ", "P b m m                        ", "P 2/b 2_1/m 2/m    ", "Pbmm      ", NO_CENTER}, /*241*/
  { 51, "D2h^5 ", "-P 2c 2c        ", "P c m m                        ", "P 2/c 2/m 2_1/m    ", "Pcmm      ", NO_CENTER}, /*242*/
  { 51, "D2h^5 ", "-P 2c 2         ", "P m c m                        ", "P 2/m 2/c 2_1/m    ", "Pmcm      ", NO_CENTER}, /*243*/
  { 51, "D2h^5 ", "-P 2 2a         ", "P m a m                        ", "P 2_1/m 2/a 2/m    ", "Pmam      ", NO_CENTER}, /*244*/
  { 52, "D2h^6 ", "-P 2a 2bc       ", "P n n a                        ", "P 2/n 2_1/n 2/a    ", "Pnna      ", NO_CENTER}, /*245*/
  { 52, "D2h^6 ", "-P 2b 2n        ", "P n n b                        ", "P 2_1/n 2/n 2/b    ", "Pnnb      ", NO_CENTER}, /*246*/
  { 52, "D2h^6 ", "-P 2n 2b        ", "P b n n                        ", "P 2/b 2/n 2_1/n    ", "Pbnn      ", NO_CENTER}, /*247*/
  { 52, "D2h^6 ", "-P 2ab 2c       ", "P c n n                        ", "P 2/c 2_1/n 2/n    ", "Pcnn      ", NO_CENTER}, /*248*/
  { 52, "D2h^6 ", "-P 2ab 2n       ", "P n c n                        ", "P 2_1/n 2/c 2/n    ", "Pncn      ", NO_CENTER}, /*249*/
  { 52, "D2h^6 ", "-P 2n 2bc       ", "P n a n                        ", "P 2/n 2/a 2_1/n    ", "Pnan      ", NO_CENTER}, /*250*/
  { 53, "D2h^7 ", "-P 2ac 2        ", "P m n a                        ", "P 2/m 2/n 2_1/a    ", "Pmna      ", NO_CENTER}, /*251*/
  { 53, "D2h^7 ", "-P 2bc 2bc      ", "P n m b                        ", "P 2/n 2/m 2_1/b    ", "Pnmb      ", NO_CENTER}, /*252*/
  { 53, "D2h^7 ", "-P 2ab 2ab      ", "P b m n                        ", "P 2_1/b 2/m 2/n    ", "Pbmn      ", NO_CENTER}, /*253*/
  { 53, "D2h^7 ", "-P 2 2ac        ", "P c n m                        ", "P 2_1/c 2/n 2/m    ", "Pcnm      ", NO_CENTER}, /*254*/
  { 53, "D2h^7 ", "-P 2 2bc        ", "P n c m                        ", "P 2/n 2_1/c 2/m    ", "Pncm      ", NO_CENTER}, /*255*/
  { 53, "D2h^7 ", "-P 2ab 2        ", "P m a n                        ", "P 2/m 2_1/a 2/n    ", "Pman      ", NO_CENTER}, /*256*/
  { 54, "D2h^8 ", "-P 2a 2ac       ", "P c c a                        ", "P 2_1/c 2/c 2/a    ", "Pcca      ", NO_CENTER}, /*257*/
  { 54, "D2h^8 ", "-P 2b 2c        ", "P c c b                        ", "P 2/c 2_1/c 2/b    ", "Pccb      ", NO_CENTER}, /*258*/
  { 54, "D2h^8 ", "-P 2a 2b        ", "P b a a                        ", "P 2/b 2_1/a 2/a    ", "Pbaa      ", NO_CENTER}, /*259*/
  { 54, "D2h^8 ", "-P 2ac 2c       ", "P c a a                        ", "P 2/c 2/a 2_1/a    ", "Pcaa      ", NO_CENTER}, /*260*/
  { 54, "D2h^8 ", "-P 2bc 2b       ", "P b c b                        ", "P 2/b 2/c 2_1/b    ", "Pbcb      ", NO_CENTER}, /*261*/
  { 54, "D2h^8 ", "-P 2b 2ab       ", "P b a b                        ", "P 2_1/b 2/a 2/b    ", "Pbab      ", NO_CENTER}, /*262*/
  { 55, "D2h^9 ", "-P 2 2ab        ", "P b a m                        ", "P 2_1/b 2_1/a 2/m  ", "Pbam      ", NO_CENTER}, /*263*/
  { 55, "D2h^9 ", "-P 2bc 2        ", "P m c b                        ", "P 2/m 2_1/c 2_1/b  ", "Pmcb      ", NO_CENTER}, /*264*/
  { 55, "D2h^9 ", "-P 2ac 2ac      ", "P c m a                        ", "P 2_1/c 2/m 2_1/a  ", "Pcma      ", NO_CENTER}, /*265*/
  { 56, "D2h^10", "-P 2ab 2ac      ", "P c c n                        ", "P 2_1/c 2_1/c 2/n  ", "Pccn      ", NO_CENTER}, /*266*/
  { 56, "D2h^10", "-P 2ac 2bc      ", "P n a a                        ", "P 2/n 2_1/a 2_1/a  ", "Pnaa      ", NO_CENTER}, /*267*/
  { 56, "D2h^10", "-P 2bc 2ab      ", "P b n b                        ", "P 2_1/b 2/n 2_1/b  ", "Pbnb      ", NO_CENTER}, /*268*/
  { 57, "D2h^11", "-P 2c 2b        ", "P b c m                        ", "P 2/b 2_1/c 2_1/m  ", "Pbcm      ", NO_CENTER}, /*269*/
  { 57, "D2h^11", "-P 2c 2ac       ", "P c a m                        ", "P 2_1/c 2/a 2_1/m  ", "Pcam      ", NO_CENTER}, /*270*/
  { 57, "D2h^11", "-P 2ac 2a       ", "P m c a                        ", "P 2_1/m 2/c 2_1/a  ", "Pmca      ", NO_CENTER}, /*271*/
  { 57, "D2h^11", "-P 2b 2a        ", "P m a b                        ", "P 2_1/m 2_1/a 2/b  ", "Pmab      ", NO_CENTER}, /*272*/
  { 57, "D2h^11", "-P 2a 2ab       ", "P b m a                        ", "P 2_1/b 2_1/m 2/a  ", "Pbma      ", NO_CENTER}, /*273*/
  { 57, "D2h^11", "-P 2bc 2c       ", "P c m b                        ", "P 2/c 2_1/m 2_1/b  ", "Pcmb      ", NO_CENTER}, /*274*/
  { 58, "D2h^12", "-P 2 2n         ", "P n n m                        ", "P 2_1/n 2_1/n 2/m  ", "Pnnm      ", NO_CENTER}, /*275*/
  { 58, "D2h^12", "-P 2n 2         ", "P m n n                        ", "P 2/m 2_1/n 2_1/n  ", "Pmnn      ", NO_CENTER}, /*276*/
  { 58, "D2h^12", "-P 2n 2n        ", "P n m n                        ", "P 2_1/n 2/m 2_1/n  ", "Pnmn      ", NO_CENTER}, /*277*/
  { 59, "D2h^13", "P 2 2ab -1ab    ", "P m m n                        ", "P 2_1/m 2_1/m 2/n  ", "Pmmn      ", NO_CENTER}, /*278*/
  { 59, "D2h^13", "-P 2ab 2a       ", "P m m n                        ", "P 2_1/m 2_1/m 2/n  ", "Pmmn      ", NO_CENTER}, /*279*/
  { 59, "D2h^13", "P 2bc 2 -1bc    ", "P n m m                        ", "P 2/n 2_1/m 2_1/m  ", "Pnmm      ", NO_CENTER}, /*280*/
  { 59, "D2h^13", "-P 2c 2bc       ", "P n m m                        ", "P 2/n 2_1/m 2_1/m  ", "Pnmm      ", NO_CENTER}, /*281*/
  { 59, "D2h^13", "P 2ac 2ac -1ac  ", "P m n m                        ", "P 2_1/m 2/n 2_1/m  ", "Pmnm      ", NO_CENTER}, /*282*/
  { 59, "D2h^13", "-P 2c 2a        ", "P m n m                        ", "P 2_1/m 2/n 2_1/m  ", "Pmnm      ", NO_CENTER}, /*283*/
  { 60, "D2h^14", "-P 2n 2ab       ", "P b c n                        ", "P 2_1/b 2/c 2_1/n  ", "Pbcn      ", NO_CENTER}, /*284*/
  { 60, "D2h^14", "-P 2n 2c        ", "P c a n                        ", "P 2/c 2_1/a 2_1/n  ", "Pcan      ", NO_CENTER}, /*285*/
  { 60, "D2h^14", "-P 2a 2n        ", "P n c a                        ", "P 2_1/n 2_1/c 2/a  ", "Pnca      ", NO_CENTER}, /*286*/
  { 60, "D2h^14", "-P 2bc 2n       ", "P n a b                        ", "P 2_1/n 2/a 2_1/b  ", "Pnab      ", NO_CENTER}, /*287*/
  { 60, "D2h^14", "-P 2ac 2b       ", "P b n a                        ", "P 2/b 2_1/n 2_1/a  ", "Pbna      ", NO_CENTER}, /*288*/
  { 60, "D2h^14", "-P 2b 2ac       ", "P c n b                        ", "P 2_1/c 2_1/n 2/b  ", "Pcnb      ", NO_CENTER}, /*289*/
  { 61, "D2h^15", "-P 2ac 2ab      ", "P b c a                        ", "P 2_1/b 2_1/c 2_1/a", "Pbca      ", NO_CENTER}, /*290*/
  { 61, "D2h^15", "-P 2bc 2ac      ", "P c a b                        ", "P 2_1/c 2_1/a 2_1/b", "Pcab      ", NO_CENTER}, /*291*/
  { 62, "D2h^16", "-P 2ac 2n       ", "P n m a                        ", "P 2_1/n 2_1/m 2_1/a", "Pnma      ", NO_CENTER}, /*292*/
  { 62, "D2h^16", "-P 2bc 2a       ", "P m n b                        ", "P 2_1/m 2_1/n 2_1/b", "Pmnb      ", NO_CENTER}, /*293*/
  { 62, "D2h^16", "-P 2c 2ab       ", "P b n m                        ", "P 2_1/b 2_1/n 2_1/m", "Pbnm      ", NO_CENTER}, /*294*/
  { 62, "D2h^16", "-P 2n 2ac       ", "P c m n                        ", "P 2_1/c 2_1/m 2_1/n", "Pcmn      ", NO_CENTER}, /*295*/
  { 62, "D2h^16", "-P 2n 2a        ", "P m c n                        ", "P 2_1/m 2_1/c 2_1/n", "Pmcn      ", NO_CENTER}, /*296*/
  { 62, "D2h^16", "-P 2c 2n        ", "P n a m                        ", "P 2_1/n 2_1/a 2_1/m", "Pnam      ", NO_CENTER}, /*297*/
  { 63, "D2h^17", "-C 2c 2         ", "C m c m                        ", "C 2/m 2/c 2_1/m    ", "Cmcm      ",    C_FACE}, /*298*/
  { 63, "D2h^17", "-C 2c 2c        ", "C c m m                        ", "C 2/c 2/m 2_1/m    ", "Ccmm      ",    C_FACE}, /*299*/
  { 63, "D2h^17", "-A 2a 2a        ", "A m m a                        ", "A 2_1/m 2/m 2/a    ", "Amma      ",    A_FACE}, /*300*/
  { 63, "D2h^17", "-A 2 2a         ", "A m a m                        ", "A 2_1/m 2/a 2/m    ", "Amam      ",    A_FACE}, /*301*/
  { 63, "D2h^17", "-B 2 2b         ", "B b m m                        ", "B 2/b 2_1/m 2/m    ", "Bbmm      ",    B_FACE}, /*302*/
  { 63, "D2h^17", "-B 2b 2         ", "B m m b                        ", "B 2/m 2_1/m 2/b    ", "Bmmb      ",    B_FACE}, /*303*/
  { 64, "D2h^18", "-C 2bc 2        ", "C m c e                        ", "C 2/m 2/c 2_1/e    ", "Cmce      ",    C_FACE}, /*304*/
  { 64, "D2h^18", "-C 2bc 2bc      ", "C c m e                        ", "C 2/c 2/m 2_1/e    ", "Ccme      ",    C_FACE}, /*305*/
  { 64, "D2h^18", "-A 2ac 2ac      ", "A e m a                        ", "A 2_1/e 2/m 2/a    ", "Aema      ",    A_FACE}, /*306*/
  { 64, "D2h^18", "-A 2 2ac        ", "A e a m                        ", "A 2_1/e 2/a 2/m    ", "Aeam      ",    A_FACE}, /*307*/
  { 64, "D2h^18", "-B 2 2bc        ", "B b e m                        ", "B 2/b 2_1/e 2/m    ", "Bbem      ",    B_FACE}, /*308*/
  { 64, "D2h^18", "-B 2bc 2        ", "B m e b                        ", "B 2/m 2_1/e 2/b    ", "Bmeb      ",    B_FACE}, /*309*/
  { 65, "D2h^19", "-C 2 2          ", "C m m m                        ", "C 2/m 2/m 2/m      ", "Cmmm      ",    C_FACE}, /*310*/
  { 65, "D2h^19", "-A 2 2          ", "A m m m                        ", "A 2/m 2/m 2/m      ", "Ammm      ",    A_FACE}, /*311*/
  { 65, "D2h^19", "-B 2 2          ", "B m m m                        ", "B 2/m 2/m 2/m      ", "Bmmm      ",    B_FACE}, /*312*/
  { 66, "D2h^20", "-C 2 2c         ", "C c c m                        ", "C 2/c 2/c 2/m      ", "Cccm      ",    C_FACE}, /*313*/
  { 66, "D2h^20", "-A 2a 2         ", "A m a a                        ", "A 2/m 2/a 2/a      ", "Amaa      ",    A_FACE}, /*314*/
  { 66, "D2h^20", "-B 2b 2b        ", "B b m b                        ", "B 2/b 2/m 2/b      ", "Bbmb      ",    B_FACE}, /*315*/
  { 67, "D2h^21", "-C 2b 2         ", "C m m e                        ", "C 2/m 2/m 2/e      ", "Cmme      ",    C_FACE}, /*316*/
  { 67, "D2h^21", "-C 2b 2b        ", "C m m e                        ", "C 2/m 2/m 2/e      ", "Cmme      ",    C_FACE}, /*317*/
  { 67, "D2h^21", "-A 2c 2c        ", "A e m m                        ", "A 2/e 2/m 2/m      ", "Aemm      ",    A_FACE}, /*318*/
  { 67, "D2h^21", "-A 2 2c         ", "A e m m                        ", "A 2/e 2/m 2/m      ", "Aemm      ",    A_FACE}, /*319*/
  { 67, "D2h^21", "-B 2 2c         ", "B m e m                        ", "B 2/m 2/e 2/m      ", "Bmem      ",    B_FACE}, /*320*/
  { 67, "D2h^21", "-B 2c 2         ", "B m e m                        ", "B 2/m 2/e 2/m      ", "Bmem      ",    B_FACE}, /*321*/
  { 68, "D2h^22", "C 2 2 -1bc      ", "C c c e                        ", "C 2/c 2/c 2/e      ", "Ccce      ",    C_FACE}, /*322*/
  { 68, "D2h^22", "-C 2b 2bc       ", "C c c e                        ", "C 2/c 2/c 2/e      ", "Ccce      ",    C_FACE}, /*323*/
  { 68, "D2h^22", "C 2 2 -1bc      ", "C c c e                        ", "C 2/c 2/c 2/e      ", "Ccce      ",    C_FACE}, /*324*/
  { 68, "D2h^22", "-C 2b 2c        ", "C c c e                        ", "C 2/c 2/c 2/e      ", "Ccce      ",    C_FACE}, /*325*/
  { 68, "D2h^22", "A 2 2 -1ac      ", "A e a a                        ", "A 2/e 2/a 2/a      ", "Aeaa      ",    A_FACE}, /*326*/
  { 68, "D2h^22", "-A 2a 2c        ", "A e a a                        ", "A 2/e 2/a 2/a      ", "Aeaa      ",    A_FACE}, /*327*/
  { 68, "D2h^22", "A 2 2 -1ac      ", "A e a a                        ", "A 2/e 2/a 2/a      ", "Aeaa      ",    A_FACE}, /*328*/
  { 68, "D2h^22", "-A 2ac 2c       ", "A e a a                        ", "A 2/e 2/a 2/a      ", "Aeaa      ",    A_FACE}, /*329*/
  { 68, "D2h^22", "B 2 2 -1bc      ", "B b e b                        ", "B 2/b 2/e 2/b      ", "Bbeb      ",    B_FACE}, /*330*/
  { 68, "D2h^22", "-B 2bc 2b       ", "B b c b                        ", "B 2/b 2/e 2/b      ", "Bbcb      ",    B_FACE}, /*331*/
  { 68, "D2h^22", "B 2 2 -1bc      ", "B b e b                        ", "B 2/b 2/e 2/b      ", "Bbeb      ",    B_FACE}, /*332*/
  { 68, "D2h^22", "-B 2b 2bc       ", "B b e b                        ", "B 2/b 2/e 2/b      ", "Bbeb      ",    B_FACE}, /*333*/
  { 69, "D2h^23", "-F 2 2          ", "F m m m                        ", "F 2/m 2/m 2/m      ", "Fmmm      ",      FACE}, /*334*/
  { 70, "D2h^24", "F 2 2 -1d       ", "F d d d                        ", "F 2/d 2/d 2/d      ", "Fddd      ",      FACE}, /*335*/
  { 70, "D2h^24", "-F 2uv 2vw      ", "F d d d                        ", "F 2/d 2/d 2/d      ", "Fddd      ",      FACE}, /*336*/
  { 71, "D2h^25", "-I 2 2          ", "I m m m                        ", "I 2/m 2/m 2/m      ", "Immm      ",      BODY}, /*337*/
  { 72, "D2h^26", "-I 2 2c         ", "I b a m                        ", "I 2/b 2/a 2/m      ", "Ibam      ",      BODY}, /*338*/
  { 72, "D2h^26", "-I 2a 2         ", "I m c b                        ", "I 2/m 2/c 2/b      ", "Imcb      ",      BODY}, /*339*/
  { 72, "D2h^26", "-I 2b 2b        ", "I c m a                        ", "I 2/c 2/m 2/a      ", "Icma      ",      BODY}, /*340*/
  { 73, "D2h^27", "-I 2b 2c        ", "I b c a                        ", "I 2/b 2/c 2/a      ", "Ibca      ",      BODY}, /*341*/
  { 73, "D2h^27", "-I 2a 2b        ", "I c a b                        ", "I 2/c 2/a 2/b      ", "Icab      ",      BODY}, /*342*/
  { 74, "D2h^28", "-I 2b 2         ", "I m m a                        ", "I 2/m 2/m 2/a      ", "Imma      ",      BODY}, /*343*/
  { 74, "D2h^28", "-I 2a 2a        ", "I m m b                        ", "I 2/m 2/m 2/b      ", "Immb      ",      BODY}, /*344*/
  { 74, "D2h^28", "-I 2c 2c        ", "I b m m                        ", "I 2/b 2/m 2/m      ", "Ibmm      ",      BODY}, /*345*/
  { 74, "D2h^28", "-I 2 2b         ", "I c m m                        ", "I 2/c 2/m 2/m      ", "Icmm      ",      BODY}, /*346*/
  { 74, "D2h^28", "-I 2 2a         ", "I m c m                        ", "I 2/m 2/c 2/m      ", "Imcm      ",      BODY}, /*347*/
  { 74, "D2h^28", "-I 2c 2         ", "I m a m                        ", "I 2/m 2/a 2/m      ", "Imam      ",      BODY}, /*348*/
  { 75, "C4^1  ", "P 4             ", "P 4                            ", "P 4                ", "P4        ", NO_CENTER}, /*349*/
  { 76, "C4^2  ", "P 4w            ", "P 4_1                          ", "P 4_1              ", "P4_1      ", NO_CENTER}, /*350*/
  { 77, "C4^3  ", "P 4c            ", "P 4_2                          ", "P 4_2              ", "P4_2      ", NO_CENTER}, /*351*/
  { 78, "C4^4  ", "P 4cw           ", "P 4_3                          ", "P 4_3              ", "P4_3      ", NO_CENTER}, /*352*/
  { 79, "C4^5  ", "I 4             ", "I 4                            ", "I 4                ", "I4        ",      BODY}, /*353*/
  { 80, "C4^6  ", "I 4bw           ", "I 4_1                          ", "I 4_1              ", "I4_1      ",      BODY}, /*354*/
  { 81, "S4^1  ", "P -4            ", "P -4                           ", "P -4               ", "P-4       ", NO_CENTER}, /*355*/
  { 82, "S4^2  ", "I -4            ", "I -4                           ", "I -4               ", "I-4       ",      BODY}, /*356*/
  { 83, "C4h^1 ", "-P 4            ", "P 4/m                          ", "P 4/m              ", "P4/m      ", NO_CENTER}, /*357*/
  { 84, "C4h^2 ", "-P 4c           ", "P 4_2/m                        ", "P 4_2/m            ", "P4_2/m    ", NO_CENTER}, /*358*/
  { 85, "C4h^3 ", "P 4ab -1ab      ", "P 4/n                          ", "P 4/n              ", "P4/n      ", NO_CENTER}, /*359*/
  { 85, "C4h^3 ", "-P 4a           ", "P 4/n                          ", "P 4/n              ", "P4/n      ", NO_CENTER}, /*360*/
  { 86, "C4h^4 ", "P 4n -1n        ", "P 4_2/n                        ", "P 4_2/n            ", "P4_2/n    ", NO_CENTER}, /*361*/
  { 86, "C4h^4 ", "-P 4bc          ", "P 4_2/n                        ", "P 4_2/n            ", "P4_2/n    ", NO_CENTER}, /*362*/
  { 87, "C4h^5 ", "-I 4            ", "I 4/m                          ", "I 4/m              ", "I4/m      ",      BODY}, /*363*/
  { 88, "C4h^6 ", "I 4bw -1bw      ", "I 4_1/a                        ", "I 4_1/a            ", "I4_1/a    ",      BODY}, /*364*/
  { 88, "C4h^6 ", "-I 4ad          ", "I 4_1/a                        ", "I 4_1/a            ", "I4_1/a    ",      BODY}, /*365*/
  { 89, "D4^1  ", "P 4 2           ", "P 4 2 2                        ", "P 4 2 2            ", "P422      ", NO_CENTER}, /*366*/
  { 90, "D4^2  ", "P 4ab 2ab       ", "P 4 2_1 2                      ", "P 4 2_1 2          ", "P42_12    ", NO_CENTER}, /*367*/
  { 91, "D4^3  ", "P 4w 2c         ", "P 4_1 2 2                      ", "P 4_1 2 2          ", "P4_122    ", NO_CENTER}, /*368*/
  { 92, "D4^4  ", "P 4abw 2nw      ", "P 4_1 2_1 2                    ", "P 4_1 2_1 2        ", "P4_12_12  ", NO_CENTER}, /*369*/
  { 93, "D4^5  ", "P 4c 2          ", "P 4_2 2 2                      ", "P 4_2 2 2          ", "P4_222    ", NO_CENTER}, /*370*/
  { 94, "D4^6  ", "P 4n 2n         ", "P 4_2 2_1 2                    ", "P 4_2 2_1 2        ", "P4_22_12  ", NO_CENTER}, /*371*/
  { 95, "D4^7  ", "P 4cw 2c        ", "P 4_3 2 2                      ", "P 4_3 2 2          ", "P4_322    ", NO_CENTER}, /*372*/
  { 96, "D4^8  ", "P 4nw 2abw      ", "P 4_3 2_1 2                    ", "P 4_3 2_1 2        ", "P4_32_12  ", NO_CENTER}, /*373*/
  { 97, "D4^9  ", "I 4 2           ", "I 4 2 2                        ", "I 4 2 2            ", "I422      ",      BODY}, /*374*/
  { 98, "D4^10 ", "I 4bw 2bw       ", "I 4_1 2 2                      ", "I 4_1 2 2          ", "I4_122    ",      BODY}, /*375*/
  { 99, "C4v^1 ", "P 4 -2          ", "P 4 m m                        ", "P 4 m m            ", "P4mm      ", NO_CENTER}, /*376*/
  {100, "C4v^2 ", "P 4 -2ab        ", "P 4 b m                        ", "P 4 b m            ", "P4bm      ", NO_CENTER}, /*377*/
  {101, "C4v^3 ", "P 4c -2c        ", "P 4_2 c m                      ", "P 4_2 c m          ", "P4_2cm    ", NO_CENTER}, /*378*/
  {102, "C4v^4 ", "P 4n -2n        ", "P 4_2 n m                      ", "P 4_2 n m          ", "P4_2nm    ", NO_CENTER}, /*379*/
  {103, "C4v^5 ", "P 4 -2c         ", "P 4 c c                        ", "P 4 c c            ", "P4cc      ", NO_CENTER}, /*380*/
  {104, "C4v^6 ", "P 4 -2n         ", "P 4 n c                        ", "P 4 n c            ", "P4nc      ", NO_CENTER}, /*381*/
  {105, "C4v^7 ", "P 4c -2         ", "P 4_2 m c                      ", "P 4_2 m c          ", "P4_2mc    ", NO_CENTER}, /*382*/
  {106, "C4v^8 ", "P 4c -2ab       ", "P 4_2 b c                      ", "P 4_2 b c          ", "P4_2bc    ", NO_CENTER}, /*383*/
  {107, "C4v^9 ", "I 4 -2          ", "I 4 m m                        ", "I 4 m m            ", "I4mm      ",      BODY}, /*384*/
  {108, "C4v^10", "I 4 -2c         ", "I 4 c m                        ", "I 4 c m            ", "I4cm      ",      BODY}, /*385*/
  {109, "C4v^11", "I 4bw -2        ", "I 4_1 m d                      ", "I 4_1 m d          ", "I4_1md    ",      BODY}, /*386*/
  {110, "C4v^12", "I 4bw -2c       ", "I 4_1 c d                      ", "I 4_1 c d          ", "I4_1cd    ",      BODY}, /*387*/
  {111, "D2d^1 ", "P -4 2          ", "P -4 2 m                       ", "P -4 2 m           ", "P-42m     ", NO_CENTER}, /*388*/
  {112, "D2d^2 ", "P -4 2c         ", "P -4 2 c                       ", "P -4 2 c           ", "P-42c     ", NO_CENTER}, /*389*/
  {113, "D2d^3 ", "P -4 2ab        ", "P -4 2_1 m                     ", "P -4 2_1 m         ", "P-42_1m   ", NO_CENTER}, /*390*/
  {114, "D2d^4 ", "P -4 2n         ", "P -4 2_1 c                     ", "P -4 2_1 c         ", "P-42_1c   ", NO_CENTER}, /*391*/
  {115, "D2d^5 ", "P -4 -2         ", "P -4 m 2                       ", "P -4 m 2           ", "P-4m2     ", NO_CENTER}, /*392*/
  {116, "D2d^6 ", "P -4 -2c        ", "P -4 c 2                       ", "P -4 c 2           ", "P-4c2     ", NO_CENTER}, /*393*/
  {117, "D2d^7 ", "P -4 -2ab       ", "P -4 b 2                       ", "P -4 b 2           ", "P-4b2     ", NO_CENTER}, /*394*/
  {118, "D2d^8 ", "P -4 -2n        ", "P -4 n 2                       ", "P -4 n 2           ", "P-4n2     ", NO_CENTER}, /*395*/
  {119, "D2d^9 ", "I -4 -2         ", "I -4 m 2                       ", "I -4 m 2           ", "I-4m2     ",      BODY}, /*396*/
  {120, "D2d^10", "I -4 -2c        ", "I -4 c 2                       ", "I -4 c 2           ", "I-4c2     ",      BODY}, /*397*/
  {121, "D2d^11", "I -4 2          ", "I -4 2 m                       ", "I -4 2 m           ", "I-42m     ",      BODY}, /*398*/
  {122, "D2d^12", "I -4 2bw        ", "I -4 2 d                       ", "I -4 2 d           ", "I-42d     ",      BODY}, /*399*/
  {123, "D4h^1 ", "-P 4 2          ", "P 4/m m m                      ", "P 4/m 2/m 2/m      ", "P4/mmm    ", NO_CENTER}, /*400*/
  {124, "D4h^2 ", "-P 4 2c         ", "P 4/m c c                      ", "P 4/m 2/c 2/c      ", "P4/mcc    ", NO_CENTER}, /*401*/
  {125, "D4h^3 ", "P 4 2 -1ab      ", "P 4/n b m                      ", "P 4/n 2/b 2/m      ", "P4/nbm    ", NO_CENTER}, /*402*/
  {125, "D4h^3 ", "-P 4a 2b        ", "P 4/n b m                      ", "P 4/n 2/b 2/m      ", "P4/nbm    ", NO_CENTER}, /*403*/
  {126, "D4h^4 ", "P 4 2 -1n       ", "P 4/n n c                      ", "P 4/n 2/n 2/c      ", "P4/nnc    ", NO_CENTER}, /*404*/
  {126, "D4h^4 ", "-P 4a 2bc       ", "P 4/n n c                      ", "P 4/n 2/n 2/c      ", "P4/nnc    ", NO_CENTER}, /*405*/
  {127, "D4h^5 ", "-P 4 2ab        ", "P 4/m b m                      ", "P 4/m 2_1/b m      ", "P4/mbm    ", NO_CENTER}, /*406*/
  {128, "D4h^6 ", "-P 4 2n         ", "P 4/m n c                      ", "P 4/m 2_1/n c      ", "P4/mnc    ", NO_CENTER}, /*407*/
  {129, "D4h^7 ", "P 4ab 2ab -1ab  ", "P 4/n m m                      ", "P 4/n 2_1/m m      ", "P4/nmm    ", NO_CENTER}, /*408*/
  {129, "D4h^7 ", "-P 4a 2a        ", "P 4/n m m                      ", "P 4/n 2_1/m m      ", "P4/nmm    ", NO_CENTER}, /*409*/
  {130, "D4h^8 ", "P 4ab 2n -1ab   ", "P 4/n c c                      ", "P 4/n 2_1/c c      ", "P4/ncc    ", NO_CENTER}, /*410*/
  {130, "D4h^8 ", "-P 4a 2ac       ", "P 4/n c c                      ", "P 4/n 2_1/c c      ", "P4/ncc    ", NO_CENTER}, /*411*/
  {131, "D4h^9 ", "-P 4c 2         ", "P 4_2/m m c                    ", "P 4_2/m 2/m 2/c    ", "P4_2/mmc  ", NO_CENTER}, /*412*/
  {132, "D4h^10", "-P 4c 2c        ", "P 4_2/m c m                    ", "P 4_2/m 2/c 2/m    ", "P4_2/mcm  ", NO_CENTER}, /*413*/
  {133, "D4h^11", "P 4n 2c -1n     ", "P 4_2/n b c                    ", "P 4_2/n 2/b 2/c    ", "P4_2/nbc  ", NO_CENTER}, /*414*/
  {133, "D4h^11", "-P 4ac 2b       ", "P 4_2/n b c                    ", "P 4_2/n 2/b 2/c    ", "P4_2/nbc  ", NO_CENTER}, /*415*/
  {134, "D4h^12", "P 4n 2 -1n      ", "P 4_2/n n m                    ", "P 4_2/n 2/n 2/m    ", "P4_2/nnm  ", NO_CENTER}, /*416*/
  {134, "D4h^12", "-P 4ac 2bc      ", "P 4_2/n n m                    ", "P 4_2/n 2/n 2/m    ", "P4_2/nnm  ", NO_CENTER}, /*417*/
  {135, "D4h^13", "-P 4c 2ab       ", "P 4_2/m b c                    ", "P 4_2/m 2_1/b 2/c  ", "P4_2/mbc  ", NO_CENTER}, /*418*/
  {136, "D4h^14", "-P 4n 2n        ", "P 4_2/m n m                    ", "P 4_2/m 2_1/n 2/m  ", "P4_2/mnm  ", NO_CENTER}, /*419*/
  {137, "D4h^15", "P 4n 2n -1n     ", "P 4_2/n m c                    ", "P 4_2/n 2_1/m 2/c  ", "P4_2/nmc  ", NO_CENTER}, /*420*/
  {137, "D4h^15", "-P 4ac 2a       ", "P 4_2/n m c                    ", "P 4_2/n 2_1/m 2/c  ", "P4_2/nmc  ", NO_CENTER}, /*421*/
  {138, "D4h^16", "P 4n 2ab -1n    ", "P 4_2/n c m                    ", "P 4_2/n 2_1/c 2/m  ", "P4_2/ncm  ", NO_CENTER}, /*422*/
  {138, "D4h^16", "-P 4ac 2ac      ", "P 4_2/n c m                    ", "P 4_2/n 2_1/c 2/m  ", "P4_2/ncm  ", NO_CENTER}, /*423*/
  {139, "D4h^17", "-I 4 2          ", "I 4/m m m                      ", "I 4/m 2/m 2/m      ", "I4/mmm    ",      BODY}, /*424*/
  {140, "D4h^18", "-I 4 2c         ", "I 4/m c m                      ", "I 4/m 2/c 2/m      ", "I4/mcm    ",      BODY}, /*425*/
  {141, "D4h^19", "I 4bw 2bw -1bw  ", "I 4_1/a m d                    ", "I 4_1/a 2/m 2/d    ", "I4_1/amd  ",      BODY}, /*426*/
  {141, "D4h^19", "-I 4bd 2        ", "I 4_1/a m d                    ", "I 4_1/a 2/m 2/d    ", "I4_1/amd  ",      BODY}, /*427*/
  {142, "D4h^20", "I 4bw 2aw -1bw  ", "I 4_1/a c d                    ", "I 4_1/a 2/c 2/d    ", "I4_1/acd  ",      BODY}, /*428*/
  {142, "D4h^20", "-I 4bd 2c       ", "I 4_1/a c d                    ", "I 4_1/a 2/c 2/d    ", "I4_1/acd  ",      BODY}, /*429*/
  {143, "C3^1  ", "P 3             ", "P 3                            ", "P 3                ", "P3        ", NO_CENTER}, /*430*/
  {144, "C3^2  ", "P 31            ", "P 3_1                          ", "P 3_1              ", "P3_1      ", NO_CENTER}, /*431*/
  {145, "C3^3  ", "P 32            ", "P 3_2                          ", "P 3_2              ", "P3_2      ", NO_CENTER}, /*432*/
  {146, "C3^4  ", "R 3             ", "R 3                            ", "R 3                ", "R3        ", NO_CENTER}, /*433*/
  {146, "C3^4  ", "P 3*            ", "R 3                            ", "R 3                ", "R3        ", NO_CENTER}, /*434*/
  {147, "C3i^1 ", "-P 3            ", "P -3                           ", "P -3               ", "P-3       ", NO_CENTER}, /*435*/
  {148, "C3i^2 ", "-R 3            ", "R -3                           ", "R -3               ", "R-3       ", NO_CENTER}, /*436*/
  {148, "C3i^2 ", "-P 3*           ", "R -3                           ", "R -3               ", "R-3       ", NO_CENTER}, /*437*/
  {149, "D3^1  ", "P 3 2           ", "P 3 1 2                        ", "P 3 1 2            ", "P312      ", NO_CENTER}, /*438*/
  {150, "D3^2  ", "P 3 2=          ", "P 3 2 1                        ", "P 3 2 1            ", "P321      ", NO_CENTER}, /*439*/
  {151, "D3^3  ", "P 31 2c (0 0 1) ", "P 3_1 1 2                      ", "P 3_1 1 2          ", "P3_112    ", NO_CENTER}, /*440*/
  {152, "D3^4  ", "P 31 2=         ", "P 3_1 2 1                      ", "P 3_1 2 1          ", "P3_121    ", NO_CENTER}, /*441*/
  {153, "D3^5  ", "P 32 2c (0 0 -1)", "P 3_2 1 2                      ", "P 3_2 1 2          ", "P3_212    ", NO_CENTER}, /*442*/
  {154, "D3^6  ", "P 32 2=         ", "P 3_2 2 1                      ", "P 3_2 2 1          ", "P3_221    ", NO_CENTER}, /*443*/
  {155, "D3^7  ", "R 3 2=          ", "R 3 2                          ", "R 3 2              ", "R32       ", NO_CENTER}, /*444*/
  {155, "D3^7  ", "P 3* 2          ", "R 3 2                          ", "R 3 2              ", "R32       ", NO_CENTER}, /*445*/
  {156, "C3v^1 ", "P 3 -2=         ", "P 3 m 1                        ", "P 3 m 1            ", "P3m1      ", NO_CENTER}, /*446*/
  {157, "C3v^2 ", "P 3 -2          ", "P 3 1 m                        ", "P 3 1 m            ", "P31m      ", NO_CENTER}, /*447*/
  {158, "C3v^3 ", "P 3 -2=c        ", "P 3 c 1                        ", "P 3 c 1            ", "P3c1      ", NO_CENTER}, /*448*/
  {159, "C3v^4 ", "P 3 -2c         ", "P 3 1 c                        ", "P 3 1 c            ", "P31c      ", NO_CENTER}, /*449*/
  {160, "C3v^5 ", "R 3 -2=         ", "R 3 m                          ", "R 3 m              ", "R3m       ", NO_CENTER}, /*450*/
  {160, "C3v^5 ", "P 3* -2         ", "R 3 m                          ", "R 3 m              ", "R3m       ", NO_CENTER}, /*451*/
  {161, "C3v^6 ", "R 3 -2=c        ", "R 3 c                          ", "R 3 c              ", "R3c       ", NO_CENTER}, /*452*/
  {161, "C3v^6 ", "P 3* -2n        ", "R 3 c                          ", "R 3 c              ", "R3c       ", NO_CENTER}, /*453*/
  {162, "D3d^1 ", "-P 3 2          ", "P -3 1 m                       ", "P -3 1 2/m         ", "P-31m     ", NO_CENTER}, /*454*/
  {163, "D3d^2 ", "-P 3 2c         ", "P -3 1 c                       ", "P -3 1 2/c         ", "P-31c     ", NO_CENTER}, /*455*/
  {164, "D3d^3 ", "-P 3 2=         ", "P -3 m 1                       ", "P -3 2/m 1         ", "P-3m1     ", NO_CENTER}, /*456*/
  {165, "D3d^4 ", "-P 3 2=c        ", "P -3 c 1                       ", "P -3 2/c 1         ", "P-3c1     ", NO_CENTER}, /*457*/
  {166, "D3d^5 ", "-R 3 2=         ", "R -3 m                         ", "R -3 2/m           ", "R-3m      ", NO_CENTER}, /*458*/
  {166, "D3d^5 ", "-P 3* 2         ", "R -3 m                         ", "R -3 2/m           ", "R-3m      ", NO_CENTER}, /*459*/
  {167, "D3d^6 ", "-R 3 2=c        ", "R -3 c                         ", "R -3 2/c           ", "R-3c      ", NO_CENTER}, /*460*/
  {167, "D3d^6 ", "-P 3* 2n        ", "R -3 c                         ", "R -3 2/c           ", "R-3c      ", NO_CENTER}, /*461*/
  {168, "C6^1  ", "P 6             ", "P 6                            ", "P 6                ", "P6        ", NO_CENTER}, /*462*/
  {169, "C6^2  ", "P 61            ", "P 6_1                          ", "P 6_1              ", "P6_1      ", NO_CENTER}, /*463*/
  {170, "C6^3  ", "P 65            ", "P 6_5                          ", "P 6_5              ", "P6_5      ", NO_CENTER}, /*464*/
  {171, "C6^4  ", "P 62            ", "P 6_2                          ", "P 6_2              ", "P6_2      ", NO_CENTER}, /*465*/
  {172, "C6^5  ", "P 64            ", "P 6_4                          ", "P 6_4              ", "P6_4      ", NO_CENTER}, /*466*/
  {173, "C6^6  ", "P 6c            ", "P 6_3                          ", "P 6_3              ", "P6_3      ", NO_CENTER}, /*467*/
  {174, "C3h^1 ", "P -6            ", "P -6                           ", "P -6               ", "P-6       ", NO_CENTER}, /*468*/
  {175, "C6h^1 ", "-P 6            ", "P 6/m                          ", "P 6/m              ", "P6/m      ", NO_CENTER}, /*469*/
  {176, "C6h^2 ", "-P 6c           ", "P 6_3/m                        ", "P 6_3/m            ", "P6_3/m    ", NO_CENTER}, /*470*/
  {177, "D6^1  ", "P 6 2           ", "P 6 2 2                        ", "P 6 2 2            ", "P622      ", NO_CENTER}, /*471*/
  {178, "D6^2  ", "P 61 2 (0 0 -1) ", "P 6_1 2 2                      ", "P 6_1 2 2          ", "P6_122    ", NO_CENTER}, /*472*/
  {179, "D6^3  ", "P 65 2 (0 0 1)  ", "P 6_5 2 2                      ", "P 6_5 2 2          ", "P6_522    ", NO_CENTER}, /*473*/
  {180, "D6^4  ", "P 62 2c (0 0 1) ", "P 6_2 2 2                      ", "P 6_2 2 2          ", "P6_222    ", NO_CENTER}, /*474*/
  {181, "D6^5  ", "P 64 2c (0 0 -1)", "P 6_4 2 2                      ", "P 6_4 2 2          ", "P6_422    ", NO_CENTER}, /*475*/
  {182, "D6^6  ", "P 6c 2c         ", "P 6_3 2 2                      ", "P 6_3 2 2          ", "P6_322    ", NO_CENTER}, /*476*/
  {183, "C6v^1 ", "P 6 -2          ", "P 6 m m                        ", "P 6 m m            ", "P6mm      ", NO_CENTER}, /*477*/
  {184, "C6v^2 ", "P 6 -2c         ", "P 6 c c                        ", "P 6 c c            ", "P6cc      ", NO_CENTER}, /*478*/
  {185, "C6v^3 ", "P 6c -2         ", "P 6_3 c m                      ", "P 6_3 c m          ", "P6_3cm    ", NO_CENTER}, /*479*/
  {186, "C6v^4 ", "P 6c -2c        ", "P 6_3 m c                      ", "P 6_3 m c          ", "P6_3mc    ", NO_CENTER}, /*480*/
  {187, "D3h^1 ", "P -6 2          ", "P -6 m 2                       ", "P -6 m 2           ", "P-6m2     ", NO_CENTER}, /*481*/
  {188, "D3h^2 ", "P -6c 2         ", "P -6 c 2                       ", "P -6 c 2           ", "P-6c2     ", NO_CENTER}, /*482*/
  {189, "D3h^3 ", "P -6 -2         ", "P -6 2 m                       ", "P -6 2 m           ", "P-62m     ", NO_CENTER}, /*483*/
  {190, "D3h^4 ", "P -6c -2c       ", "P -6 2 c                       ", "P -6 2 c           ", "P-62c     ", NO_CENTER}, /*484*/
  {191, "D6h^1 ", "-P 6 2          ", "P 6/m m m                      ", "P 6/m 2/m 2/m      ", "P6/mmm    ", NO_CENTER}, /*485*/
  {192, "D6h^2 ", "-P 6 2c         ", "P 6/m c c                      ", "P 6/m 2/c 2/c      ", "P6/mcc    ", NO_CENTER}, /*486*/
  {193, "D6h^3 ", "-P 6c 2         ", "P 6_3/m c m                    ", "P 6_3/m 2/c 2/m    ", "P6_3/mcm  ", NO_CENTER}, /*487*/
  {194, "D6h^4 ", "-P 6c 2c        ", "P 6_3/m m c                    ", "P 6_3/m 2/m 2/c    ", "P6_3/mmc  ", NO_CENTER}, /*488*/
  {195, "T^1   ", "P 2 2 3         ", "P 2 3                          ", "P 2 3              ", "P23       ", NO_CENTER}, /*489*/
  {196, "T^2   ", "F 2 2 3         ", "F 2 3                          ", "F 2 3              ", "F23       ",      FACE}, /*490*/
  {197, "T^3   ", "I 2 2 3         ", "I 2 3                          ", "I 2 3              ", "I23       ",      BODY}, /*491*/
  {198, "T^4   ", "P 2ac 2ab 3     ", "P 2_1 3                        ", "P 2_1 3            ", "P2_13     ", NO_CENTER}, /*492*/
  {199, "T^5   ", "I 2b 2c 3       ", "I 2_1 3                        ", "I 2_1 3            ", "I2_13     ",      BODY}, /*493*/
  {200, "Th^1  ", "-P 2 2 3        ", "P m 3                          ", "P 2/m -3           ", "Pm3       ", NO_CENTER}, /*494*/
  {201, "Th^2  ", "P 2 2 3 -1n     ", "P n 3                          ", "P 2/n -3           ", "Pn3       ", NO_CENTER}, /*495*/
  {201, "Th^2  ", "-P 2ab 2bc 3    ", "P n 3                          ", "P 2/n -3           ", "Pn3       ", NO_CENTER}, /*496*/
  {202, "Th^3  ", "-F 2 2 3        ", "F m 3                          ", "F 2/m -3           ", "Fm3       ",      FACE}, /*497*/
  {203, "Th^4  ", "F 2 2 3 -1d     ", "F d 3                          ", "F 2/d -3           ", "Fd3       ",      FACE}, /*498*/
  {203, "Th^4  ", "-F 2uv 2vw 3    ", "F d 3                          ", "F 2/d -3           ", "Fd3       ",      FACE}, /*499*/
  {204, "Th^5  ", "-I 2 2 3        ", "I m 3                          ", "I 2/m -3           ", "Im3       ",      BODY}, /*500*/
  {205, "Th^6  ", "-P 2ac 2ab 3    ", "P a 3                          ", "P 2_1/a -3         ", "Pa3       ", NO_CENTER}, /*501*/
  {206, "Th^7  ", "-I 2b 2c 3      ", "I a 3                          ", "I 2_1/a -3         ", "Ia3       ",      BODY}, /*502*/
  {207, "O^1   ", "P 4 2 3         ", "P 4 3 2                        ", "P 4 3 2            ", "P432      ", NO_CENTER}, /*503*/
  {208, "O^2   ", "P 4n 2 3        ", "P 4_2 3 2                      ", "P 4_2 3 2          ", "P4_232    ", NO_CENTER}, /*504*/
  {209, "O^3   ", "F 4 2 3         ", "F 4 3 2                        ", "F 4 3 2            ", "F432      ",      FACE}, /*505*/
  {210, "O^4   ", "F 4d 2 3        ", "F 4_1 3 2                      ", "F 4_1 3 2          ", "F4_132    ",      FACE}, /*506*/
  {211, "O^5   ", "I 4 2 3         ", "I 4 3 2                        ", "I 4 3 2            ", "I432      ",      BODY}, /*507*/
  {212, "O^6   ", "P 4acd 2ab 3    ", "P 4_3 3 2                      ", "P 4_3 3 2          ", "P4_332    ", NO_CENTER}, /*508*/
  {213, "O^7   ", "P 4bd 2ab 3     ", "P 4_1 3 2                      ", "P 4_1 3 2          ", "P4_132    ", NO_CENTER}, /*509*/
  {214, "O^8   ", "I 4bd 2c 3      ", "I 4_1 3 2                      ", "I 4_1 3 2          ", "I4_132    ",      BODY}, /*510*/
  {215, "Td^1  ", "P -4 2 3        ", "P -4 3 m                       ", "P -4 3 m           ", "P-43m     ", NO_CENTER}, /*511*/
  {216, "Td^2  ", "F -4 2 3        ", "F -4 3 m                       ", "F -4 3 m           ", "F-43m     ",      FACE}, /*512*/
  {217, "Td^3  ", "I -4 2 3        ", "I -4 3 m                       ", "I -4 3 m           ", "I-43m     ",      BODY}, /*513*/
  {218, "Td^4  ", "P -4n 2 3       ", "P -4 3 n                       ", "P -4 3 n           ", "P-43n     ", NO_CENTER}, /*514*/
  {219, "Td^5  ", "F -4c 2 3       ", "F -4 3 c                       ", "F -4 3 c           ", "F-43c     ",      FACE}, /*515*/
  {220, "Td^6  ", "I -4bd 2c 3     ", "I -4 3 d                       ", "I -4 3 d           ", "I-43d     ",      BODY}, /*516*/
  {221, "Oh^1  ", "-P 4 2 3        ", "P m -3 m                       ", "P 4/m -3 2/m       ", "Pm-3m     ", NO_CENTER}, /*517*/
  {222, "Oh^2  ", "P 4 2 3 -1n     ", "P n -3 n                       ", "P 4/n -3 2/n       ", "Pn-3n     ", NO_CENTER}, /*518*/
  {222, "Oh^2  ", "-P 4a 2bc 3     ", "P n -3 n                       ", "P 4/n -3 2/n       ", "Pn-3n     ", NO_CENTER}, /*519*/
  {223, "Oh^3  ", "-P 4n 2 3       ", "P m -3 n                       ", "P 4_2/m -3 2/n     ", "Pm-3n     ", NO_CENTER}, /*520*/
  {224, "Oh^4  ", "P 4n 2 3 -1n    ", "P n -3 m                       ", "P 4_2/n -3 2/m     ", "Pn-3m     ", NO_CENTER}, /*521*/
  {224, "Oh^4  ", "-P 4bc 2bc 3    ", "P n -3 m                       ", "P 4_2/n -3 2/m     ", "Pn-3m     ", NO_CENTER}, /*522*/
  {225, "Oh^5  ", "-F 4 2 3        ", "F m -3 m                       ", "F 4/m -3 2/m       ", "Fm-3m     ",      FACE}, /*523*/
  {226, "Oh^6  ", "-F 4c 2 3       ", "F m -3 c                       ", "F 4/m -3 2/c       ", "Fm-3c     ",      FACE}, /*524*/
  {227, "Oh^7  ", "F 4d 2 3 -1d    ", "F d -3 m                       ", "F 4_1/d -3 2/m     ", "Fd-3m     ",      FACE}, /*525*/
  {227, "Oh^7  ", "-F 4vw 2vw 3    ", "F d -3 m                       ", "F 4_1/d -3 2/m     ", "Fd-3m     ",      FACE}, /*526*/
  {228, "Oh^8  ", "F 4d 2 3 -1cd   ", "F d -3 c                       ", "F 4_1/d -3 2/c     ", "Fd-3c     ",      FACE}, /*527*/
  {228, "Oh^8  ", "-F 4cvw 2vw 3   ", "F d -3 c                       ", "F 4_1/d -3 2/c     ", "Fd-3c     ",      FACE}, /*528*/
  {229, "Oh^9  ", "-I 4 2 3        ", "I m -3 m                       ", "I 4/m -3 2/m       ", "Im-3m     ",      BODY}, /*529*/
  {230, "Oh^10 ", "-I 4bd 2c 3     ", "I a -3 d                       ", "I 4_1/a -3 2/d     ", "Ia-3d     ",      BODY}, /*530*/
};

static const int symmetry_operations[][13] = { 
  {  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*    0 */
  {  1,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*    1 */
  {  2,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*    2 */
  {  2, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*    3 */
  {  3,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*    4 */
  {  3, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*    5 */
  {  4,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*    6 */
  {  4, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*    7 */
  {  5,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*    8 */
  {  5,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*    9 */
  {  6,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   10 */
  {  6, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*   11 */
  {  7,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   12 */
  {  7, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*   13 */
  {  8,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   14 */
  {  8,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*   15 */
  {  9,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   16 */
  {  9, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*   17 */
  {  9,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*   18 */
  {  9, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*   19 */
  { 10,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   20 */
  { 10, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*   21 */
  { 10,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*   22 */
  { 10, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*   23 */
  { 11,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   24 */
  { 11, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*   25 */
  { 11,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*   26 */
  { 11, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*   27 */
  { 12,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   28 */
  { 12, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*   29 */
  { 12,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*   30 */
  { 12, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*   31 */
  { 13,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   32 */
  { 13, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*   33 */
  { 13,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*   34 */
  { 13, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*   35 */
  { 14,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   36 */
  { 14, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*   37 */
  { 14,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*   38 */
  { 14, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*   39 */
  { 15,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   40 */
  { 15,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*   41 */
  { 15,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*   42 */
  { 15,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*   43 */
  { 16,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   44 */
  { 16,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*   45 */
  { 16,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*   46 */
  { 16,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*   47 */
  { 17,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   48 */
  { 17,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*   49 */
  { 17,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*   50 */
  { 17,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*   51 */
  { 18,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   52 */
  { 18,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*   53 */
  { 19,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   54 */
  { 19,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*   55 */
  { 20,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   56 */
  { 20, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   57 */
  { 21,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   58 */
  { 21,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*   59 */
  { 22,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   60 */
  { 22,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*   61 */
  { 23,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   62 */
  { 23,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*   63 */
  { 24,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   64 */
  { 24,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*   65 */
  { 25,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   66 */
  { 25,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*   67 */
  { 26,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   68 */
  { 26,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*   69 */
  { 27,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   70 */
  { 27, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*   71 */
  { 28,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   72 */
  { 28, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*   73 */
  { 29,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   74 */
  { 29, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*   75 */
  { 30,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   76 */
  { 30,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*   77 */
  { 30,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*   78 */
  { 30,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*   79 */
  { 31,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   80 */
  { 31,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*   81 */
  { 31,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*   82 */
  { 31,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*   83 */
  { 32,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   84 */
  { 32,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*   85 */
  { 32,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*   86 */
  { 32,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*   87 */
  { 33,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   88 */
  { 33,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*   89 */
  { 33,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*   90 */
  { 33,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*   91 */
  { 34,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   92 */
  { 34,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*   93 */
  { 34,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*   94 */
  { 34,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*   95 */
  { 35,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*   96 */
  { 35,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*   97 */
  { 35,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*   98 */
  { 35,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*   99 */
  { 36,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  100 */
  { 36, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  101 */
  { 36,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  102 */
  { 36, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  103 */
  { 37,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  104 */
  { 37, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  105 */
  { 37,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  106 */
  { 37, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  107 */
  { 38,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  108 */
  { 38, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  109 */
  { 38,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  110 */
  { 38, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  111 */
  { 39,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  112 */
  { 39,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  113 */
  { 39,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  114 */
  { 39,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  115 */
  { 40,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  116 */
  { 40,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  117 */
  { 40,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  118 */
  { 40,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  119 */
  { 41,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  120 */
  { 41,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  121 */
  { 41,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  122 */
  { 41,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  123 */
  { 42,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  124 */
  { 42,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  125 */
  { 42,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  126 */
  { 42,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  127 */
  { 43,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  128 */
  { 43,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  129 */
  { 43,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  130 */
  { 43,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  131 */
  { 44,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  132 */
  { 44,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  133 */
  { 44,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  134 */
  { 44,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  135 */
  { 45,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  136 */
  { 45,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  137 */
  { 45,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  138 */
  { 45,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  139 */
  { 46,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  140 */
  { 46,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  141 */
  { 46,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  142 */
  { 46,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  143 */
  { 47,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  144 */
  { 47,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  145 */
  { 47,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  146 */
  { 47,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  147 */
  { 48,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  148 */
  { 48,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  149 */
  { 48,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  150 */
  { 48,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  151 */
  { 49,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  152 */
  { 49,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  153 */
  { 49,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  154 */
  { 49,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  155 */
  { 50,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  156 */
  { 50,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  157 */
  { 50,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  158 */
  { 50,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  159 */
  { 51,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  160 */
  { 51, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  161 */
  { 51,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  162 */
  { 51, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  163 */
  { 52,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  164 */
  { 52, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  165 */
  { 52,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  166 */
  { 52, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  167 */
  { 53,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  168 */
  { 53, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  169 */
  { 53,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  170 */
  { 53, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  171 */
  { 54,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  172 */
  { 54, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  173 */
  { 54,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  174 */
  { 54, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  175 */
  { 55,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  176 */
  { 55, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  177 */
  { 55,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  178 */
  { 55, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  179 */
  { 56,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  180 */
  { 56, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  181 */
  { 56,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  182 */
  { 56, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  183 */
  { 57,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  184 */
  { 57, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  185 */
  { 57, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  186 */
  { 57,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  187 */
  { 58,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  188 */
  { 58, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  189 */
  { 58, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  190 */
  { 58,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  191 */
  { 59,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  192 */
  { 59, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  193 */
  { 59,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  194 */
  { 59, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  195 */
  { 60,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  196 */
  { 60, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  197 */
  { 60, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  198 */
  { 60,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  199 */
  { 61,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  200 */
  { 61, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  201 */
  { 61, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  202 */
  { 61,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  203 */
  { 62,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  204 */
  { 62, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  205 */
  { 62,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  206 */
  { 62, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /*  207 */
  { 63,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  208 */
  { 63, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  209 */
  { 63, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  210 */
  { 63,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  211 */
  { 63,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  212 */
  { 63, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  213 */
  { 63, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  214 */
  { 63,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  215 */
  { 64,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  216 */
  { 64, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  217 */
  { 64, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  218 */
  { 64,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  219 */
  { 64,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  220 */
  { 64, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  221 */
  { 64, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  222 */
  { 64,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  223 */
  { 65,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  224 */
  { 65, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  225 */
  { 65, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  226 */
  { 65,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  227 */
  { 65,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  228 */
  { 65, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  229 */
  { 65, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  230 */
  { 65,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  231 */
  { 66,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  232 */
  { 66, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  233 */
  { 66, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  234 */
  { 66,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  235 */
  { 66,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  236 */
  { 66, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  237 */
  { 66, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  238 */
  { 66,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  239 */
  { 67,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  240 */
  { 67, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  241 */
  { 67, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  242 */
  { 67,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  243 */
  { 67,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  244 */
  { 67, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  245 */
  { 67, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  246 */
  { 67,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  247 */
  { 68,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  248 */
  { 68, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  249 */
  { 68, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  250 */
  { 68,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  251 */
  { 68,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  252 */
  { 68, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  253 */
  { 68, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  254 */
  { 68,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  255 */
  { 69,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  256 */
  { 69, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  257 */
  { 69,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  258 */
  { 69, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  259 */
  { 69,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  260 */
  { 69, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  261 */
  { 69,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  262 */
  { 69, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  263 */
  { 70,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  264 */
  { 70, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  265 */
  { 70,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  266 */
  { 70, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  267 */
  { 70,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  268 */
  { 70, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  269 */
  { 70,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  270 */
  { 70, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  271 */
  { 71,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  272 */
  { 71, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  273 */
  { 71,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  274 */
  { 71, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  275 */
  { 71,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  276 */
  { 71, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  277 */
  { 71,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  278 */
  { 71, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  279 */
  { 72,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  280 */
  { 72, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  281 */
  { 72, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  282 */
  { 72,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  283 */
  { 73,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  284 */
  { 73, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  285 */
  { 73, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  286 */
  { 73,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  287 */
  { 74,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  288 */
  { 74, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  289 */
  { 74, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  290 */
  { 74,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  291 */
  { 75,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  292 */
  { 75, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  293 */
  { 75, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  294 */
  { 75,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  295 */
  { 76,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  296 */
  { 76, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  297 */
  { 76, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  298 */
  { 76,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  299 */
  { 77,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  300 */
  { 77, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  301 */
  { 77, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  302 */
  { 77,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  303 */
  { 78,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  304 */
  { 78, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  305 */
  { 78,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /*  306 */
  { 78, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  307 */
  { 79,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  308 */
  { 79, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  309 */
  { 79,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  310 */
  { 79, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  311 */
  { 80,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  312 */
  { 80, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  313 */
  { 80,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /*  314 */
  { 80, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  315 */
  { 81,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  316 */
  { 81, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  317 */
  { 81, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  318 */
  { 81,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  319 */
  { 82,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  320 */
  { 82, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  321 */
  { 82, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  322 */
  { 82,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  323 */
  { 83,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  324 */
  { 83, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  325 */
  { 83, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  326 */
  { 83,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  327 */
  { 84,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  328 */
  { 84, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  329 */
  { 84, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  330 */
  { 84,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  331 */
  { 85,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  332 */
  { 85, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  333 */
  { 85, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  334 */
  { 85,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  335 */
  { 86,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  336 */
  { 86, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  337 */
  { 86, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  338 */
  { 86,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  339 */
  { 87,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  340 */
  { 87, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  341 */
  { 87,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  342 */
  { 87, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  343 */
  { 88,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  344 */
  { 88, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  345 */
  { 88,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  346 */
  { 88, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  347 */
  { 89,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  348 */
  { 89, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  349 */
  { 89,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  350 */
  { 89, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  351 */
  { 90,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  352 */
  { 90, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  353 */
  { 90, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  354 */
  { 90,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  355 */
  { 90,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  356 */
  { 90, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  357 */
  { 90, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  358 */
  { 90,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  359 */
  { 91,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  360 */
  { 91, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  361 */
  { 91, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  362 */
  { 91,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  363 */
  { 91,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  364 */
  { 91, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  365 */
  { 91, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  366 */
  { 91,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  367 */
  { 92,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  368 */
  { 92, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  369 */
  { 92, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  370 */
  { 92,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  371 */
  { 92,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  372 */
  { 92, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  373 */
  { 92, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  374 */
  { 92,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  375 */
  { 93,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  376 */
  { 93, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  377 */
  { 93, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  378 */
  { 93,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  379 */
  { 93,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  380 */
  { 93, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  381 */
  { 93, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  382 */
  { 93,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  383 */
  { 94,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  384 */
  { 94, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  385 */
  { 94, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  386 */
  { 94,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  387 */
  { 94,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  388 */
  { 94, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  389 */
  { 94, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  390 */
  { 94,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  391 */
  { 95,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  392 */
  { 95, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  393 */
  { 95, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  394 */
  { 95,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  395 */
  { 95,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  396 */
  { 95, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  397 */
  { 95, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  398 */
  { 95,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  399 */
  { 96,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  400 */
  { 96, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  401 */
  { 96, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  402 */
  { 96,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  403 */
  { 96,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  404 */
  { 96, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  405 */
  { 96, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  406 */
  { 96,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  407 */
  { 97,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  408 */
  { 97, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  409 */
  { 97, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  410 */
  { 97,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  411 */
  { 97,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  412 */
  { 97, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  413 */
  { 97, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  414 */
  { 97,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  415 */
  { 98,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  416 */
  { 98, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  417 */
  { 98, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  418 */
  { 98,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  419 */
  { 98,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  420 */
  { 98, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  421 */
  { 98, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  422 */
  { 98,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  423 */
  { 99,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  424 */
  { 99, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  425 */
  { 99, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  426 */
  { 99,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  427 */
  { 99,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  428 */
  { 99, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  429 */
  { 99, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  430 */
  { 99,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  431 */
  {100,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  432 */
  {100, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  433 */
  {100, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  434 */
  {100,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  435 */
  {100,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  436 */
  {100, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  437 */
  {100, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  438 */
  {100,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  439 */
  {101,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  440 */
  {101, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  441 */
  {101, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  442 */
  {101,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  443 */
  {101,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  444 */
  {101, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  445 */
  {101, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  446 */
  {101,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  447 */
  {102,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  448 */
  {102, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  449 */
  {102,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /*  450 */
  {102, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  451 */
  {102,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  452 */
  {102, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  453 */
  {102,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  454 */
  {102, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  455 */
  {103,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  456 */
  {103, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  457 */
  {103,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  458 */
  {103, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  459 */
  {103,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  460 */
  {103, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  461 */
  {103,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  462 */
  {103, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  463 */
  {104,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  464 */
  {104, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  465 */
  {104,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /*  466 */
  {104, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  467 */
  {104,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  468 */
  {104, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  469 */
  {104,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  470 */
  {104, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  471 */
  {105,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  472 */
  {105, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  473 */
  {105,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /*  474 */
  {105, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  475 */
  {105,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  476 */
  {105, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  477 */
  {105,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  478 */
  {105, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  479 */
  {106,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  480 */
  {106, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  481 */
  {106,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  482 */
  {106, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  483 */
  {106,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  484 */
  {106, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  485 */
  {106,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  486 */
  {106, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  487 */
  {107,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  488 */
  {107, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  489 */
  {107,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /*  490 */
  {107, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  491 */
  {107,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  492 */
  {107, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  493 */
  {107,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  494 */
  {107, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  495 */
  {108,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  496 */
  {108, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  497 */
  {108,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  498 */
  {108, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  499 */
  {109,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  500 */
  {109, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  501 */
  {109,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  502 */
  {109, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  503 */
  {110,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  504 */
  {110, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  505 */
  {110,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  506 */
  {110, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  507 */
  {111,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  508 */
  {111, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  509 */
  {111,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /*  510 */
  {111, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  511 */
  {112,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  512 */
  {112, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  513 */
  {112,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  514 */
  {112, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  515 */
  {113,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  516 */
  {113, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  517 */
  {113,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  518 */
  {113, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  519 */
  {114,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  520 */
  {114, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  521 */
  {114,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  522 */
  {114, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  523 */
  {115,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  524 */
  {115, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  525 */
  {115,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  526 */
  {115, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  527 */
  {116,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  528 */
  {116, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  529 */
  {116,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  530 */
  {116, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  531 */
  {116,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  532 */
  {116, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  533 */
  {116,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  534 */
  {116, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  535 */
  {117,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  536 */
  {117, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  537 */
  {117,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  538 */
  {117, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  539 */
  {117,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  540 */
  {117, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  541 */
  {117,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  542 */
  {117, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  543 */
  {118,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  544 */
  {118, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  545 */
  {118,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /*  546 */
  {118, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  547 */
  {118,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  548 */
  {118, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  549 */
  {118,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  550 */
  {118, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  551 */
  {119,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  552 */
  {119, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  553 */
  {119,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  554 */
  {119, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  555 */
  {119,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  556 */
  {119, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  557 */
  {119,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  558 */
  {119, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  559 */
  {120,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  560 */
  {120, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  561 */
  {120,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  562 */
  {120, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  563 */
  {120,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  564 */
  {120, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  565 */
  {120,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  566 */
  {120, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  567 */
  {121,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  568 */
  {121, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  569 */
  {121,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  570 */
  {121, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  571 */
  {121,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  572 */
  {121, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  573 */
  {121,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  574 */
  {121, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  575 */
  {122,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  576 */
  {122, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  577 */
  {122,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  578 */
  {122, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  579 */
  {122,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  580 */
  {122, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  581 */
  {122,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  582 */
  {122, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  583 */
  {122,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  584 */
  {122, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  585 */
  {122,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  586 */
  {122, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  587 */
  {122,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  588 */
  {122, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  589 */
  {122,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  590 */
  {122, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  591 */
  {123,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  592 */
  {123, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  593 */
  {123,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  594 */
  {123, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  595 */
  {123,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  596 */
  {123, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  597 */
  {123,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  598 */
  {123, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  599 */
  {124,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  600 */
  {124, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  601 */
  {124,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /*  602 */
  {124, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  603 */
  {124,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  604 */
  {124, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  605 */
  {124,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  606 */
  {124, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  607 */
  {125,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  608 */
  {125, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  609 */
  {125, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  610 */
  {125,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  611 */
  {126,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  612 */
  {126,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  613 */
  {126,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  614 */
  {126,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  615 */
  {127,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  616 */
  {127,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  617 */
  {127, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  618 */
  {127, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  619 */
  {128,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  620 */
  {128, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  621 */
  {128, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  622 */
  {128,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  623 */
  {129,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  624 */
  {129, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  625 */
  {129, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  626 */
  {129,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  627 */
  {130,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  628 */
  {130,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  629 */
  {130,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  630 */
  {130,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  631 */
  {131,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  632 */
  {131,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  633 */
  {131,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  634 */
  {131,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  635 */
  {132,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  636 */
  {132,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  637 */
  {132, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  638 */
  {132, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  639 */
  {133,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  640 */
  {133,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  641 */
  {133, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  642 */
  {133, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  643 */
  {134,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  644 */
  {134, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  645 */
  {134, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  646 */
  {134,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  647 */
  {135,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  648 */
  {135,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  649 */
  {135,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  650 */
  {135,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  651 */
  {136,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  652 */
  {136,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  653 */
  {136, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  654 */
  {136, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  655 */
  {137,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  656 */
  {137, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  657 */
  {137, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /*  658 */
  {137,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  659 */
  {138,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  660 */
  {138, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  661 */
  {138, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  662 */
  {138,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  663 */
  {139,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  664 */
  {139,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  665 */
  {139,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  666 */
  {139,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  667 */
  {140,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  668 */
  {140,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  669 */
  {140,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  670 */
  {140,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  671 */
  {141,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  672 */
  {141,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  673 */
  {141, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  674 */
  {141, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  675 */
  {142,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  676 */
  {142,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  677 */
  {142, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /*  678 */
  {142, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  679 */
  {143,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  680 */
  {143, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  681 */
  {143, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  682 */
  {143,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  683 */
  {144,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  684 */
  {144, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  685 */
  {144, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  686 */
  {144,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  687 */
  {145,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  688 */
  {145,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  689 */
  {145,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  690 */
  {145,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  691 */
  {146,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  692 */
  {146,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  693 */
  {146,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  694 */
  {146,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  695 */
  {147,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  696 */
  {147,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  697 */
  {147, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  698 */
  {147, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  699 */
  {148,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  700 */
  {148,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  701 */
  {148, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  702 */
  {148, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  703 */
  {149,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  704 */
  {149, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  705 */
  {149, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  706 */
  {149,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  707 */
  {150,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  708 */
  {150, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  709 */
  {150, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  710 */
  {150,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  711 */
  {151,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  712 */
  {151,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  713 */
  {151,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  714 */
  {151,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  715 */
  {152,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  716 */
  {152,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  717 */
  {152,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  718 */
  {152,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  719 */
  {153,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  720 */
  {153,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  721 */
  {153, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  722 */
  {153, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  723 */
  {154,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  724 */
  {154,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  725 */
  {154, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  726 */
  {154, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  727 */
  {155,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  728 */
  {155, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  729 */
  {155, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  730 */
  {155,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  731 */
  {156,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  732 */
  {156, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  733 */
  {156, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  734 */
  {156,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  735 */
  {157,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  736 */
  {157,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  737 */
  {157,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  738 */
  {157,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  739 */
  {158,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  740 */
  {158,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  741 */
  {158,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  742 */
  {158,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  743 */
  {159,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  744 */
  {159,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  745 */
  {159, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  746 */
  {159, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  747 */
  {160,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  748 */
  {160,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  749 */
  {160, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  750 */
  {160, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  751 */
  {161,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  752 */
  {161, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  753 */
  {161, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  754 */
  {161,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  755 */
  {162,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  756 */
  {162,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  757 */
  {162,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  758 */
  {162,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  759 */
  {163,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  760 */
  {163,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  761 */
  {163, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  762 */
  {163, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  763 */
  {164,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  764 */
  {164, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  765 */
  {164, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  766 */
  {164,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  767 */
  {165,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  768 */
  {165, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  769 */
  {165, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  770 */
  {165,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  771 */
  {166,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  772 */
  {166,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  773 */
  {166,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  774 */
  {166,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  775 */
  {167,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  776 */
  {167,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  777 */
  {167,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  778 */
  {167,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  779 */
  {168,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  780 */
  {168,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  781 */
  {168, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  782 */
  {168, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  783 */
  {169,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  784 */
  {169,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  785 */
  {169, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  786 */
  {169, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  787 */
  {170,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  788 */
  {170, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  789 */
  {170, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  790 */
  {170,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  791 */
  {171,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  792 */
  {171,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  793 */
  {171,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  794 */
  {171,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  795 */
  {172,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  796 */
  {172,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  797 */
  {172, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  798 */
  {172, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  799 */
  {173,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  800 */
  {173, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  801 */
  {173, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  802 */
  {173,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  803 */
  {173,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  804 */
  {173, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  805 */
  {173, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  806 */
  {173,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  807 */
  {174,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  808 */
  {174,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  809 */
  {174,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  810 */
  {174,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  811 */
  {174,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  812 */
  {174,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  813 */
  {174,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  814 */
  {174,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  815 */
  {175,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  816 */
  {175,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  817 */
  {175, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  818 */
  {175, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  819 */
  {175,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  820 */
  {175,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  821 */
  {175, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  822 */
  {175, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  823 */
  {176,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  824 */
  {176, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  825 */
  {176, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  826 */
  {176,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  827 */
  {176,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  828 */
  {176, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  829 */
  {176, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  830 */
  {176,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  831 */
  {177,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  832 */
  {177, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  833 */
  {177, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  834 */
  {177,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  835 */
  {177,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  836 */
  {177, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  837 */
  {177, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  838 */
  {177,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  839 */
  {178,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  840 */
  {178,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  841 */
  {178,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  842 */
  {178,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  843 */
  {178,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  844 */
  {178,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  845 */
  {178,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  846 */
  {178,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  847 */
  {179,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  848 */
  {179,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  849 */
  {179,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /*  850 */
  {179,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  851 */
  {179,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  852 */
  {179,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  853 */
  {179,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /*  854 */
  {179,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  855 */
  {180,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  856 */
  {180,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  857 */
  {180, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  858 */
  {180, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  859 */
  {180,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  860 */
  {180,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  861 */
  {180, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  862 */
  {180, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  863 */
  {181,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  864 */
  {181,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  865 */
  {181, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  866 */
  {181, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  867 */
  {181,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  868 */
  {181,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  869 */
  {181, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  870 */
  {181, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  871 */
  {182,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  872 */
  {182, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  873 */
  {182, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  874 */
  {182,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  875 */
  {182,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  876 */
  {182, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  877 */
  {182, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  878 */
  {182,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  879 */
  {183,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  880 */
  {183,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  881 */
  {183,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  882 */
  {183,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  883 */
  {183,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  884 */
  {183,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  885 */
  {183,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /*  886 */
  {183,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  887 */
  {184,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  888 */
  {184,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  889 */
  {184, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  890 */
  {184, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  891 */
  {184,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  892 */
  {184,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /*  893 */
  {184, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  894 */
  {184, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  895 */
  {185,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  896 */
  {185, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  897 */
  {185, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  898 */
  {185,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  899 */
  {185,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  900 */
  {185, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  901 */
  {185, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  902 */
  {185,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  903 */
  {186,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  904 */
  {186, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  905 */
  {186, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  906 */
  {186,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  907 */
  {186,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  908 */
  {186, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  909 */
  {186, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  910 */
  {186,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  911 */
  {187,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  912 */
  {187,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  913 */
  {187,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  914 */
  {187,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  915 */
  {187,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  916 */
  {187,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /*  917 */
  {187,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  918 */
  {187,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  919 */
  {188,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  920 */
  {188,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  921 */
  {188,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  922 */
  {188,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  923 */
  {188,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  924 */
  {188,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  925 */
  {188,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  926 */
  {188,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /*  927 */
  {189,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  928 */
  {189,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  929 */
  {189, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  930 */
  {189, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  931 */
  {189,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  932 */
  {189,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  933 */
  {189, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  934 */
  {189, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  935 */
  {190,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  936 */
  {190,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  937 */
  {190, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  938 */
  {190, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  939 */
  {190,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  940 */
  {190,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  941 */
  {190, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  942 */
  {190, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  943 */
  {191,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  944 */
  {191, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  945 */
  {191, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  946 */
  {191,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  947 */
  {191,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  948 */
  {191, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  949 */
  {191, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  950 */
  {191,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  951 */
  {192,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  952 */
  {192, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  953 */
  {192, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  954 */
  {192,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  955 */
  {192,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  956 */
  {192, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /*  957 */
  {192, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /*  958 */
  {192,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  959 */
  {193,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  960 */
  {193,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  961 */
  {193,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  962 */
  {193,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /*  963 */
  {193,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /*  964 */
  {193,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  965 */
  {193,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /*  966 */
  {193,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  967 */
  {194,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  968 */
  {194,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  969 */
  {194,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /*  970 */
  {194,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /*  971 */
  {194,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  972 */
  {194,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  973 */
  {194,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /*  974 */
  {194,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  975 */
  {195,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  976 */
  {195,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  977 */
  {195, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  978 */
  {195, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  979 */
  {195,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /*  980 */
  {195,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /*  981 */
  {195, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /*  982 */
  {195, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /*  983 */
  {196,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  984 */
  {196,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /*  985 */
  {196, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /*  986 */
  {196, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /*  987 */
  {196,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  988 */
  {196,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /*  989 */
  {196, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /*  990 */
  {196, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /*  991 */
  {197,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /*  992 */
  {197, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /*  993 */
  {197, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /*  994 */
  {197,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /*  995 */
  {197,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /*  996 */
  {197, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /*  997 */
  {197, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /*  998 */
  {197,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /*  999 */
  {198,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1000 */
  {198, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1001 */
  {198, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1002 */
  {198,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1003 */
  {198,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1004 */
  {198, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1005 */
  {198, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1006 */
  {198,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1007 */
  {199,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1008 */
  {199,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1009 */
  {199,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1010 */
  {199,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1011 */
  {199,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1012 */
  {199,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1013 */
  {199,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1014 */
  {199,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1015 */
  {200,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1016 */
  {200,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1017 */
  {200,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1018 */
  {200,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1019 */
  {200,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1020 */
  {200,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1021 */
  {200,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1022 */
  {200,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1023 */
  {201,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1024 */
  {201,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1025 */
  {201, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1026 */
  {201, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1027 */
  {201,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1028 */
  {201,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1029 */
  {201, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1030 */
  {201, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1031 */
  {202,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1032 */
  {202,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1033 */
  {202, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1034 */
  {202, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1035 */
  {202,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1036 */
  {202,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1037 */
  {202, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1038 */
  {202, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1039 */
  {203,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1040 */
  {203, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1041 */
  {203, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1042 */
  {203,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1043 */
  {203,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1044 */
  {203, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1045 */
  {203, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1046 */
  {203,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1047 */
  {204,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1048 */
  {204, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1049 */
  {204, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1050 */
  {204,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1051 */
  {204,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1052 */
  {204, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1053 */
  {204, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1054 */
  {204,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1055 */
  {205,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1056 */
  {205,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1057 */
  {205,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1058 */
  {205,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1059 */
  {205,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1060 */
  {205,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1061 */
  {205,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1062 */
  {205,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1063 */
  {206,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1064 */
  {206,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1065 */
  {206,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1066 */
  {206,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1067 */
  {206,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1068 */
  {206,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1069 */
  {206,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1070 */
  {206,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1071 */
  {207,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1072 */
  {207,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1073 */
  {207, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1074 */
  {207, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1075 */
  {207,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1076 */
  {207,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1077 */
  {207, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1078 */
  {207, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1079 */
  {208,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1080 */
  {208,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1081 */
  {208, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1082 */
  {208, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1083 */
  {208,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1084 */
  {208,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1085 */
  {208, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1086 */
  {208, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1087 */
  {209,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1088 */
  {209, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1089 */
  {209, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1090 */
  {209,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1091 */
  {209,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1092 */
  {209, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1093 */
  {209, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1094 */
  {209,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1095 */
  {209,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1096 */
  {209, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1097 */
  {209, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1098 */
  {209,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1099 */
  {209,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1100 */
  {209, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1101 */
  {209, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1102 */
  {209,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1103 */
  {210,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1104 */
  {210,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1105 */
  {210,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1106 */
  {210,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1107 */
  {210,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1108 */
  {210,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1109 */
  {210,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1110 */
  {210,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1111 */
  {210,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1112 */
  {210,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1113 */
  {210,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1114 */
  {210,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1115 */
  {210,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1116 */
  {210,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1117 */
  {210,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1118 */
  {210,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1119 */
  {211,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1120 */
  {211,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1121 */
  {211, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1122 */
  {211, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1123 */
  {211,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1124 */
  {211,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1125 */
  {211, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1126 */
  {211, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1127 */
  {211,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1128 */
  {211,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1129 */
  {211, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1130 */
  {211, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1131 */
  {211,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1132 */
  {211,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1133 */
  {211, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1134 */
  {211, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1135 */
  {212,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1136 */
  {212, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1137 */
  {212, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 3 }, /* 1138 */
  {212,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 3 }, /* 1139 */
  {212,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1140 */
  {212, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1141 */
  {212, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 9, 9 }, /* 1142 */
  {212,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 9 }, /* 1143 */
  {212,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1144 */
  {212, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1145 */
  {212, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 3, 9 }, /* 1146 */
  {212,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 9 }, /* 1147 */
  {212,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1148 */
  {212, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1149 */
  {212, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 9, 3 }, /* 1150 */
  {212,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 3 }, /* 1151 */
  {213,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1152 */
  {213,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 3 }, /* 1153 */
  {213,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1154 */
  {213,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 9 }, /* 1155 */
  {213,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1156 */
  {213,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 9 }, /* 1157 */
  {213,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1158 */
  {213,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 3 }, /* 1159 */
  {213,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1160 */
  {213,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 9 }, /* 1161 */
  {213,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1162 */
  {213,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 3 }, /* 1163 */
  {213,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1164 */
  {213,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 3 }, /* 1165 */
  {213,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1166 */
  {213,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 9 }, /* 1167 */
  {214,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1168 */
  {214,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 9 }, /* 1169 */
  {214, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 9, 3 }, /* 1170 */
  {214, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1171 */
  {214,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1172 */
  {214,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 3 }, /* 1173 */
  {214, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 3, 9 }, /* 1174 */
  {214, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1175 */
  {214,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1176 */
  {214,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 3 }, /* 1177 */
  {214, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 9, 9 }, /* 1178 */
  {214, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1179 */
  {214,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1180 */
  {214,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 9 }, /* 1181 */
  {214, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 3 }, /* 1182 */
  {214, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1183 */
  {215,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1184 */
  {215, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1185 */
  {215, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1186 */
  {215,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1187 */
  {215,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1188 */
  {215, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1189 */
  {215, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1190 */
  {215,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1191 */
  {216,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1192 */
  {216,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1193 */
  {216,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1194 */
  {216,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1195 */
  {216,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1196 */
  {216,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1197 */
  {216,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1198 */
  {216,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1199 */
  {217,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1200 */
  {217,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1201 */
  {217, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1202 */
  {217, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1203 */
  {217,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1204 */
  {217,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1205 */
  {217, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1206 */
  {217, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1207 */
  {218,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1208 */
  {218, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1209 */
  {218, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1210 */
  {218,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1211 */
  {218,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1212 */
  {218, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1213 */
  {218, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1214 */
  {218,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1215 */
  {219,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1216 */
  {219,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1217 */
  {219,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1218 */
  {219,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1219 */
  {219,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1220 */
  {219,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1221 */
  {219,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1222 */
  {219,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1223 */
  {220,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1224 */
  {220,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1225 */
  {220, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1226 */
  {220, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1227 */
  {220,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1228 */
  {220,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1229 */
  {220, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1230 */
  {220, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1231 */
  {221,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1232 */
  {221, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1233 */
  {221, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1234 */
  {221,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1235 */
  {221,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1236 */
  {221, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1237 */
  {221, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1238 */
  {221,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1239 */
  {222,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1240 */
  {222, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1241 */
  {222, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1242 */
  {222,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1243 */
  {222,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1244 */
  {222, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1245 */
  {222, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1246 */
  {222,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1247 */
  {223,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1248 */
  {223,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1249 */
  {223,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1250 */
  {223,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1251 */
  {223,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1252 */
  {223,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1253 */
  {223,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1254 */
  {223,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1255 */
  {224,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1256 */
  {224,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1257 */
  {224,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1258 */
  {224,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1259 */
  {224,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1260 */
  {224,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1261 */
  {224,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1262 */
  {224,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1263 */
  {225,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1264 */
  {225,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1265 */
  {225, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1266 */
  {225, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1267 */
  {225,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1268 */
  {225,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1269 */
  {225, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1270 */
  {225, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1271 */
  {226,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1272 */
  {226,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1273 */
  {226, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1274 */
  {226, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1275 */
  {226,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1276 */
  {226,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1277 */
  {226, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1278 */
  {226, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1279 */
  {227,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1280 */
  {227, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1281 */
  {227, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1282 */
  {227,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1283 */
  {227,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1284 */
  {227, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1285 */
  {227, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1286 */
  {227,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1287 */
  {228,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1288 */
  {228, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1289 */
  {228,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1290 */
  {228, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1291 */
  {228, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1292 */
  {228,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1293 */
  {228, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1294 */
  {228,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1295 */
  {229,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1296 */
  {229, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1297 */
  {229, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1298 */
  {229,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1299 */
  {229,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1300 */
  {229, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1301 */
  {229, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1302 */
  {229,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1303 */
  {230,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1304 */
  {230, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1305 */
  {230, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1306 */
  {230,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1307 */
  {230,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1308 */
  {230, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1309 */
  {230, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1310 */
  {230,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1311 */
  {231,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1312 */
  {231, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1313 */
  {231, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1314 */
  {231,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1315 */
  {231,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1316 */
  {231, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1317 */
  {231, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1318 */
  {231,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1319 */
  {232,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1320 */
  {232, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1321 */
  {232, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1322 */
  {232,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1323 */
  {232,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1324 */
  {232, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1325 */
  {232, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1326 */
  {232,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1327 */
  {233,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1328 */
  {233, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1329 */
  {233,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1330 */
  {233, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1331 */
  {233, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1332 */
  {233,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1333 */
  {233, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1334 */
  {233,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1335 */
  {234,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1336 */
  {234, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1337 */
  {234, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1338 */
  {234,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1339 */
  {234,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1340 */
  {234, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1341 */
  {234, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1342 */
  {234,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1343 */
  {235,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1344 */
  {235, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1345 */
  {235,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1346 */
  {235, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1347 */
  {235, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1348 */
  {235,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1349 */
  {235, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1350 */
  {235,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1351 */
  {236,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1352 */
  {236, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1353 */
  {236, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1354 */
  {236,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1355 */
  {236,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1356 */
  {236, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1357 */
  {236, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1358 */
  {236,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1359 */
  {237,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1360 */
  {237, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1361 */
  {237,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1362 */
  {237, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1363 */
  {237, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1364 */
  {237,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1365 */
  {237, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1366 */
  {237,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1367 */
  {238,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1368 */
  {238, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1369 */
  {238, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1370 */
  {238,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1371 */
  {238,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1372 */
  {238, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1373 */
  {238, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1374 */
  {238,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1375 */
  {239,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1376 */
  {239, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1377 */
  {239, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1378 */
  {239,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1379 */
  {239,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1380 */
  {239, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1381 */
  {239, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1382 */
  {239,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1383 */
  {240,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1384 */
  {240, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1385 */
  {240, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1386 */
  {240,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1387 */
  {240,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1388 */
  {240, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1389 */
  {240, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1390 */
  {240,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1391 */
  {241,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1392 */
  {241, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1393 */
  {241, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1394 */
  {241,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1395 */
  {241,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1396 */
  {241, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1397 */
  {241, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1398 */
  {241,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1399 */
  {242,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1400 */
  {242, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1401 */
  {242, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1402 */
  {242,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1403 */
  {242,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1404 */
  {242, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1405 */
  {242, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1406 */
  {242,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1407 */
  {243,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1408 */
  {243, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1409 */
  {243, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1410 */
  {243,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1411 */
  {243,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1412 */
  {243, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1413 */
  {243, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1414 */
  {243,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1415 */
  {244,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1416 */
  {244, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1417 */
  {244, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1418 */
  {244,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1419 */
  {244,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1420 */
  {244, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1421 */
  {244, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1422 */
  {244,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1423 */
  {245,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1424 */
  {245, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1425 */
  {245, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1426 */
  {245,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1427 */
  {245,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1428 */
  {245, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1429 */
  {245, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1430 */
  {245,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1431 */
  {246,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1432 */
  {246, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1433 */
  {246, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1434 */
  {246,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1435 */
  {246,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1436 */
  {246, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1437 */
  {246, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1438 */
  {246,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1439 */
  {247,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1440 */
  {247, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1441 */
  {247, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1442 */
  {247,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1443 */
  {247,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1444 */
  {247, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1445 */
  {247, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1446 */
  {247,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1447 */
  {248,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1448 */
  {248, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1449 */
  {248, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1450 */
  {248,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1451 */
  {248,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1452 */
  {248, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1453 */
  {248, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1454 */
  {248,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1455 */
  {249,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1456 */
  {249, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1457 */
  {249, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1458 */
  {249,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1459 */
  {249,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1460 */
  {249, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1461 */
  {249, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1462 */
  {249,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1463 */
  {250,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1464 */
  {250, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1465 */
  {250, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1466 */
  {250,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1467 */
  {250,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1468 */
  {250, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1469 */
  {250, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1470 */
  {250,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1471 */
  {251,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1472 */
  {251, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1473 */
  {251, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1474 */
  {251,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1475 */
  {251,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1476 */
  {251, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1477 */
  {251, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1478 */
  {251,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1479 */
  {252,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1480 */
  {252, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1481 */
  {252, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1482 */
  {252,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1483 */
  {252,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1484 */
  {252, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1485 */
  {252, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1486 */
  {252,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1487 */
  {253,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1488 */
  {253, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1489 */
  {253, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1490 */
  {253,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1491 */
  {253,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1492 */
  {253, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1493 */
  {253, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1494 */
  {253,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1495 */
  {254,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1496 */
  {254, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1497 */
  {254, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1498 */
  {254,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1499 */
  {254,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1500 */
  {254, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1501 */
  {254, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1502 */
  {254,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1503 */
  {255,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1504 */
  {255, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1505 */
  {255, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1506 */
  {255,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1507 */
  {255,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1508 */
  {255, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1509 */
  {255, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1510 */
  {255,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1511 */
  {256,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1512 */
  {256, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1513 */
  {256, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1514 */
  {256,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1515 */
  {256,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1516 */
  {256, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1517 */
  {256, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1518 */
  {256,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1519 */
  {257,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1520 */
  {257, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1521 */
  {257, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1522 */
  {257,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1523 */
  {257,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1524 */
  {257, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1525 */
  {257, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1526 */
  {257,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1527 */
  {258,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1528 */
  {258, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1529 */
  {258, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1530 */
  {258,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1531 */
  {258,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1532 */
  {258, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1533 */
  {258, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1534 */
  {258,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1535 */
  {259,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1536 */
  {259, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1537 */
  {259, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1538 */
  {259,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1539 */
  {259,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1540 */
  {259, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1541 */
  {259, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1542 */
  {259,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1543 */
  {260,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1544 */
  {260, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1545 */
  {260, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1546 */
  {260,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1547 */
  {260,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1548 */
  {260, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1549 */
  {260, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1550 */
  {260,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1551 */
  {261,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1552 */
  {261, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1553 */
  {261, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1554 */
  {261,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1555 */
  {261,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1556 */
  {261, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1557 */
  {261, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1558 */
  {261,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1559 */
  {262,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1560 */
  {262, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1561 */
  {262, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1562 */
  {262,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1563 */
  {262,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1564 */
  {262, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1565 */
  {262, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1566 */
  {262,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1567 */
  {263,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1568 */
  {263, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1569 */
  {263, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1570 */
  {263,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1571 */
  {263,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1572 */
  {263, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1573 */
  {263, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1574 */
  {263,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1575 */
  {264,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1576 */
  {264, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1577 */
  {264, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1578 */
  {264,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1579 */
  {264,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1580 */
  {264, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1581 */
  {264, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1582 */
  {264,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1583 */
  {265,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1584 */
  {265, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1585 */
  {265, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1586 */
  {265,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1587 */
  {265,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1588 */
  {265, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1589 */
  {265, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1590 */
  {265,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1591 */
  {266,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1592 */
  {266, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1593 */
  {266, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1594 */
  {266,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1595 */
  {266,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1596 */
  {266, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1597 */
  {266, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1598 */
  {266,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1599 */
  {267,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1600 */
  {267, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1601 */
  {267, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1602 */
  {267,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1603 */
  {267,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1604 */
  {267, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1605 */
  {267, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1606 */
  {267,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1607 */
  {268,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1608 */
  {268, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1609 */
  {268, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1610 */
  {268,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1611 */
  {268,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1612 */
  {268, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1613 */
  {268, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1614 */
  {268,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1615 */
  {269,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1616 */
  {269, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1617 */
  {269, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1618 */
  {269,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1619 */
  {269,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1620 */
  {269, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1621 */
  {269, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1622 */
  {269,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1623 */
  {270,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1624 */
  {270, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1625 */
  {270, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1626 */
  {270,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1627 */
  {270,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1628 */
  {270, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1629 */
  {270, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1630 */
  {270,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1631 */
  {271,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1632 */
  {271, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1633 */
  {271, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1634 */
  {271,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1635 */
  {271,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1636 */
  {271, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1637 */
  {271, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1638 */
  {271,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1639 */
  {272,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1640 */
  {272, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1641 */
  {272, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1642 */
  {272,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1643 */
  {272,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1644 */
  {272, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1645 */
  {272, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1646 */
  {272,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1647 */
  {273,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1648 */
  {273, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1649 */
  {273, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1650 */
  {273,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1651 */
  {273,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1652 */
  {273, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1653 */
  {273, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1654 */
  {273,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1655 */
  {274,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1656 */
  {274, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1657 */
  {274, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1658 */
  {274,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1659 */
  {274,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1660 */
  {274, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1661 */
  {274, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1662 */
  {274,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1663 */
  {275,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1664 */
  {275, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1665 */
  {275, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1666 */
  {275,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1667 */
  {275,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1668 */
  {275, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1669 */
  {275, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1670 */
  {275,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1671 */
  {276,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1672 */
  {276, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1673 */
  {276, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1674 */
  {276,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1675 */
  {276,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1676 */
  {276, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1677 */
  {276, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1678 */
  {276,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1679 */
  {277,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1680 */
  {277, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1681 */
  {277, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1682 */
  {277,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1683 */
  {277,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1684 */
  {277, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1685 */
  {277, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1686 */
  {277,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1687 */
  {278,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1688 */
  {278, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1689 */
  {278,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1690 */
  {278, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1691 */
  {278, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1692 */
  {278,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1693 */
  {278, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1694 */
  {278,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1695 */
  {279,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1696 */
  {279, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1697 */
  {279, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1698 */
  {279,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1699 */
  {279,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1700 */
  {279, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1701 */
  {279, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1702 */
  {279,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1703 */
  {280,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1704 */
  {280, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1705 */
  {280,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1706 */
  {280, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1707 */
  {280, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1708 */
  {280,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1709 */
  {280, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1710 */
  {280,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1711 */
  {281,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1712 */
  {281, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1713 */
  {281, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1714 */
  {281,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1715 */
  {281,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1716 */
  {281, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1717 */
  {281, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1718 */
  {281,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1719 */
  {282,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1720 */
  {282, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1721 */
  {282,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1722 */
  {282, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1723 */
  {282, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1724 */
  {282,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1725 */
  {282, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1726 */
  {282,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1727 */
  {283,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1728 */
  {283, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1729 */
  {283, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1730 */
  {283,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1731 */
  {283,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1732 */
  {283, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1733 */
  {283, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1734 */
  {283,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1735 */
  {284,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1736 */
  {284, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1737 */
  {284, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1738 */
  {284,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1739 */
  {284,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1740 */
  {284, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1741 */
  {284, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1742 */
  {284,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1743 */
  {285,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1744 */
  {285, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1745 */
  {285, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1746 */
  {285,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1747 */
  {285,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1748 */
  {285, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1749 */
  {285, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1750 */
  {285,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1751 */
  {286,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1752 */
  {286, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1753 */
  {286, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1754 */
  {286,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1755 */
  {286,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1756 */
  {286, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1757 */
  {286, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1758 */
  {286,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1759 */
  {287,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1760 */
  {287, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1761 */
  {287, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1762 */
  {287,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1763 */
  {287,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1764 */
  {287, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1765 */
  {287, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1766 */
  {287,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1767 */
  {288,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1768 */
  {288, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1769 */
  {288, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1770 */
  {288,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1771 */
  {288,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1772 */
  {288, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1773 */
  {288, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1774 */
  {288,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1775 */
  {289,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1776 */
  {289, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1777 */
  {289, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1778 */
  {289,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1779 */
  {289,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1780 */
  {289, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1781 */
  {289, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1782 */
  {289,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1783 */
  {290,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1784 */
  {290, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1785 */
  {290, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1786 */
  {290,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1787 */
  {290,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1788 */
  {290, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1789 */
  {290, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1790 */
  {290,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1791 */
  {291,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1792 */
  {291, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1793 */
  {291, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1794 */
  {291,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1795 */
  {291,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1796 */
  {291, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1797 */
  {291, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1798 */
  {291,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1799 */
  {292,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1800 */
  {292, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1801 */
  {292, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1802 */
  {292,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1803 */
  {292,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1804 */
  {292, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1805 */
  {292, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1806 */
  {292,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1807 */
  {293,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1808 */
  {293, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1809 */
  {293, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1810 */
  {293,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1811 */
  {293,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1812 */
  {293, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1813 */
  {293, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1814 */
  {293,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1815 */
  {294,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1816 */
  {294, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1817 */
  {294, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1818 */
  {294,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1819 */
  {294,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1820 */
  {294, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1821 */
  {294, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1822 */
  {294,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1823 */
  {295,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1824 */
  {295, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1825 */
  {295, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1826 */
  {295,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1827 */
  {295,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1828 */
  {295, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1829 */
  {295, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1830 */
  {295,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1831 */
  {296,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1832 */
  {296, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1833 */
  {296, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1834 */
  {296,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1835 */
  {296,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1836 */
  {296, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1837 */
  {296, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1838 */
  {296,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1839 */
  {297,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1840 */
  {297, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1841 */
  {297, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1842 */
  {297,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1843 */
  {297,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1844 */
  {297, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1845 */
  {297, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1846 */
  {297,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1847 */
  {298,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1848 */
  {298, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1849 */
  {298, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1850 */
  {298,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1851 */
  {298,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1852 */
  {298, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1853 */
  {298, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1854 */
  {298,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1855 */
  {298,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1856 */
  {298, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1857 */
  {298, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1858 */
  {298,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1859 */
  {298,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1860 */
  {298, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1861 */
  {298, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1862 */
  {298,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1863 */
  {299,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1864 */
  {299, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1865 */
  {299, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 1866 */
  {299,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 1867 */
  {299,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 1868 */
  {299, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 1869 */
  {299, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1870 */
  {299,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1871 */
  {299,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1872 */
  {299, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1873 */
  {299, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1874 */
  {299,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1875 */
  {299,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1876 */
  {299, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1877 */
  {299, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1878 */
  {299,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1879 */
  {300,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1880 */
  {300, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1881 */
  {300, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1882 */
  {300,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1883 */
  {300,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1884 */
  {300, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1885 */
  {300, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1886 */
  {300,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1887 */
  {300,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1888 */
  {300, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1889 */
  {300, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1890 */
  {300,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1891 */
  {300,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1892 */
  {300, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1893 */
  {300, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1894 */
  {300,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1895 */
  {301,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1896 */
  {301, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1897 */
  {301, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1898 */
  {301,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1899 */
  {301,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 1900 */
  {301, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 1901 */
  {301, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 1902 */
  {301,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 1903 */
  {301,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1904 */
  {301, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1905 */
  {301, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1906 */
  {301,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1907 */
  {301,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1908 */
  {301, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1909 */
  {301, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1910 */
  {301,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1911 */
  {302,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1912 */
  {302, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1913 */
  {302, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1914 */
  {302,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1915 */
  {302,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 1916 */
  {302, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 1917 */
  {302, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1918 */
  {302,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1919 */
  {302,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1920 */
  {302, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1921 */
  {302, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1922 */
  {302,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1923 */
  {302,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 1924 */
  {302, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 1925 */
  {302, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1926 */
  {302,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1927 */
  {303,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1928 */
  {303, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1929 */
  {303, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1930 */
  {303,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1931 */
  {303,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1932 */
  {303, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1933 */
  {303, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 1934 */
  {303,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 1935 */
  {303,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1936 */
  {303, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1937 */
  {303, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1938 */
  {303,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1939 */
  {303,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1940 */
  {303, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1941 */
  {303, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 1942 */
  {303,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 1943 */
  {304,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1944 */
  {304, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1945 */
  {304, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1946 */
  {304,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1947 */
  {304,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1948 */
  {304, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1949 */
  {304, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1950 */
  {304,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1951 */
  {304,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1952 */
  {304, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1953 */
  {304, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1954 */
  {304,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1955 */
  {304,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1956 */
  {304, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1957 */
  {304, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1958 */
  {304,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1959 */
  {305,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1960 */
  {305, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1961 */
  {305, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1962 */
  {305,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1963 */
  {305,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1964 */
  {305, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1965 */
  {305, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1966 */
  {305,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1967 */
  {305,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1968 */
  {305, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1969 */
  {305, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1970 */
  {305,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1971 */
  {305,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1972 */
  {305, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1973 */
  {305, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1974 */
  {305,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1975 */
  {306,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1976 */
  {306, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1977 */
  {306, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1978 */
  {306,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1979 */
  {306,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1980 */
  {306, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1981 */
  {306, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1982 */
  {306,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1983 */
  {306,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 1984 */
  {306, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 1985 */
  {306, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 1986 */
  {306,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 1987 */
  {306,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 1988 */
  {306, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 1989 */
  {306, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 1990 */
  {306,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 1991 */
  {307,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 1992 */
  {307, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 1993 */
  {307, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 1994 */
  {307,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 1995 */
  {307,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 1996 */
  {307, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 1997 */
  {307, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 1998 */
  {307,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 1999 */
  {307,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2000 */
  {307, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2001 */
  {307, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2002 */
  {307,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2003 */
  {307,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2004 */
  {307, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2005 */
  {307, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2006 */
  {307,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2007 */
  {308,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2008 */
  {308, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2009 */
  {308, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2010 */
  {308,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2011 */
  {308,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2012 */
  {308, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2013 */
  {308, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2014 */
  {308,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2015 */
  {308,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2016 */
  {308, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2017 */
  {308, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2018 */
  {308,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2019 */
  {308,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2020 */
  {308, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2021 */
  {308, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2022 */
  {308,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2023 */
  {309,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2024 */
  {309, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2025 */
  {309, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2026 */
  {309,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2027 */
  {309,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2028 */
  {309, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2029 */
  {309, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2030 */
  {309,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2031 */
  {309,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2032 */
  {309, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2033 */
  {309, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2034 */
  {309,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2035 */
  {309,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2036 */
  {309, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2037 */
  {309, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2038 */
  {309,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2039 */
  {310,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2040 */
  {310, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2041 */
  {310, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2042 */
  {310,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2043 */
  {310,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2044 */
  {310, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2045 */
  {310, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2046 */
  {310,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2047 */
  {310,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2048 */
  {310, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2049 */
  {310, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2050 */
  {310,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2051 */
  {310,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2052 */
  {310, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2053 */
  {310, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2054 */
  {310,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2055 */
  {311,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2056 */
  {311, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2057 */
  {311, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2058 */
  {311,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2059 */
  {311,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2060 */
  {311, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2061 */
  {311, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2062 */
  {311,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2063 */
  {311,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2064 */
  {311, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2065 */
  {311, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2066 */
  {311,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2067 */
  {311,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2068 */
  {311, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2069 */
  {311, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2070 */
  {311,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2071 */
  {312,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2072 */
  {312, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2073 */
  {312, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2074 */
  {312,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2075 */
  {312,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2076 */
  {312, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2077 */
  {312, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2078 */
  {312,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2079 */
  {312,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2080 */
  {312, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2081 */
  {312, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2082 */
  {312,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2083 */
  {312,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2084 */
  {312, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2085 */
  {312, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2086 */
  {312,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2087 */
  {313,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2088 */
  {313, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2089 */
  {313, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2090 */
  {313,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2091 */
  {313,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2092 */
  {313, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2093 */
  {313, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2094 */
  {313,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2095 */
  {313,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2096 */
  {313, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2097 */
  {313, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2098 */
  {313,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2099 */
  {313,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2100 */
  {313, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2101 */
  {313, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2102 */
  {313,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2103 */
  {314,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2104 */
  {314, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2105 */
  {314, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2106 */
  {314,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2107 */
  {314,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2108 */
  {314, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2109 */
  {314, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2110 */
  {314,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2111 */
  {314,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2112 */
  {314, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2113 */
  {314, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2114 */
  {314,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2115 */
  {314,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2116 */
  {314, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2117 */
  {314, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2118 */
  {314,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2119 */
  {315,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2120 */
  {315, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2121 */
  {315, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2122 */
  {315,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2123 */
  {315,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2124 */
  {315, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2125 */
  {315, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2126 */
  {315,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2127 */
  {315,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2128 */
  {315, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2129 */
  {315, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2130 */
  {315,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2131 */
  {315,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2132 */
  {315, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2133 */
  {315, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2134 */
  {315,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2135 */
  {316,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2136 */
  {316, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2137 */
  {316, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2138 */
  {316,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2139 */
  {316,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2140 */
  {316, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2141 */
  {316, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2142 */
  {316,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2143 */
  {316,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2144 */
  {316, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2145 */
  {316, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2146 */
  {316,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2147 */
  {316,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2148 */
  {316, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2149 */
  {316, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2150 */
  {316,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2151 */
  {317,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2152 */
  {317, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2153 */
  {317, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2154 */
  {317,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2155 */
  {317,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2156 */
  {317, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2157 */
  {317, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2158 */
  {317,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2159 */
  {317,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2160 */
  {317, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2161 */
  {317, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2162 */
  {317,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2163 */
  {317,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 2164 */
  {317, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 2165 */
  {317, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2166 */
  {317,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2167 */
  {318,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2168 */
  {318, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2169 */
  {318, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2170 */
  {318,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2171 */
  {318,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2172 */
  {318, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2173 */
  {318, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2174 */
  {318,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2175 */
  {318,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2176 */
  {318, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2177 */
  {318, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2178 */
  {318,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2179 */
  {318,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2180 */
  {318, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2181 */
  {318, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2182 */
  {318,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2183 */
  {319,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2184 */
  {319, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2185 */
  {319, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2186 */
  {319,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2187 */
  {319,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2188 */
  {319, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2189 */
  {319, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2190 */
  {319,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2191 */
  {319,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2192 */
  {319, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2193 */
  {319, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2194 */
  {319,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2195 */
  {319,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2196 */
  {319, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2197 */
  {319, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2198 */
  {319,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2199 */
  {320,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2200 */
  {320, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2201 */
  {320, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2202 */
  {320,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2203 */
  {320,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2204 */
  {320, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2205 */
  {320, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2206 */
  {320,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2207 */
  {320,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2208 */
  {320, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2209 */
  {320, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2210 */
  {320,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2211 */
  {320,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 2212 */
  {320, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 2213 */
  {320, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2214 */
  {320,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2215 */
  {321,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2216 */
  {321, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2217 */
  {321, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2218 */
  {321,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2219 */
  {321,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2220 */
  {321, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2221 */
  {321, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2222 */
  {321,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2223 */
  {321,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2224 */
  {321, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2225 */
  {321, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2226 */
  {321,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2227 */
  {321,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2228 */
  {321, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2229 */
  {321, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2230 */
  {321,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2231 */
  {322,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2232 */
  {322, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2233 */
  {322,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2234 */
  {322, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2235 */
  {322, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2236 */
  {322,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2237 */
  {322, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2238 */
  {322,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2239 */
  {322,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2240 */
  {322, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2241 */
  {322,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2242 */
  {322, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2243 */
  {322, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2244 */
  {322,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2245 */
  {322, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2246 */
  {322,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2247 */
  {323,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2248 */
  {323, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2249 */
  {323, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2250 */
  {323,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2251 */
  {323,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2252 */
  {323, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2253 */
  {323, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2254 */
  {323,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2255 */
  {323,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2256 */
  {323, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2257 */
  {323, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2258 */
  {323,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2259 */
  {323,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2260 */
  {323, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2261 */
  {323, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2262 */
  {323,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2263 */
  {324,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2264 */
  {324, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2265 */
  {324,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2266 */
  {324, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2267 */
  {324, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2268 */
  {324,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2269 */
  {324, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2270 */
  {324,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2271 */
  {324,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2272 */
  {324, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2273 */
  {324,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2274 */
  {324, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2275 */
  {324, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2276 */
  {324,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2277 */
  {324, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2278 */
  {324,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2279 */
  {325,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2280 */
  {325, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2281 */
  {325, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2282 */
  {325,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2283 */
  {325,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2284 */
  {325, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2285 */
  {325, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2286 */
  {325,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2287 */
  {325,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2288 */
  {325, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2289 */
  {325, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2290 */
  {325,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2291 */
  {325,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2292 */
  {325, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2293 */
  {325, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2294 */
  {325,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2295 */
  {326,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2296 */
  {326, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2297 */
  {326,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2298 */
  {326, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2299 */
  {326, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2300 */
  {326,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2301 */
  {326, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2302 */
  {326,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2303 */
  {326,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2304 */
  {326, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2305 */
  {326,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2306 */
  {326, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2307 */
  {326, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2308 */
  {326,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2309 */
  {326, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2310 */
  {326,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2311 */
  {327,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2312 */
  {327, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2313 */
  {327, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2314 */
  {327,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2315 */
  {327,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2316 */
  {327, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2317 */
  {327, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2318 */
  {327,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2319 */
  {327,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2320 */
  {327, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2321 */
  {327, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2322 */
  {327,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2323 */
  {327,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2324 */
  {327, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2325 */
  {327, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2326 */
  {327,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2327 */
  {328,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2328 */
  {328, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2329 */
  {328,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2330 */
  {328, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2331 */
  {328, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2332 */
  {328,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2333 */
  {328, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2334 */
  {328,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2335 */
  {328,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2336 */
  {328, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2337 */
  {328,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2338 */
  {328, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2339 */
  {328, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2340 */
  {328,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2341 */
  {328, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2342 */
  {328,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2343 */
  {329,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2344 */
  {329, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2345 */
  {329, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2346 */
  {329,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2347 */
  {329,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2348 */
  {329, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2349 */
  {329, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2350 */
  {329,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2351 */
  {329,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2352 */
  {329, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2353 */
  {329, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2354 */
  {329,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2355 */
  {329,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2356 */
  {329, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2357 */
  {329, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2358 */
  {329,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2359 */
  {330,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2360 */
  {330, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2361 */
  {330,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2362 */
  {330, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2363 */
  {330, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2364 */
  {330,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2365 */
  {330, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2366 */
  {330,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2367 */
  {330,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2368 */
  {330, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2369 */
  {330,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2370 */
  {330, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2371 */
  {330, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2372 */
  {330,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2373 */
  {330, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2374 */
  {330,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2375 */
  {331,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2376 */
  {331, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2377 */
  {331, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2378 */
  {331,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2379 */
  {331,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2380 */
  {331, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2381 */
  {331, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2382 */
  {331,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2383 */
  {331,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2384 */
  {331, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2385 */
  {331, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2386 */
  {331,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2387 */
  {331,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2388 */
  {331, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2389 */
  {331, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2390 */
  {331,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2391 */
  {332,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2392 */
  {332, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2393 */
  {332,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2394 */
  {332, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2395 */
  {332, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2396 */
  {332,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2397 */
  {332, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2398 */
  {332,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2399 */
  {332,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2400 */
  {332, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2401 */
  {332,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2402 */
  {332, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2403 */
  {332, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2404 */
  {332,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2405 */
  {332, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2406 */
  {332,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2407 */
  {333,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2408 */
  {333, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2409 */
  {333, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2410 */
  {333,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2411 */
  {333,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2412 */
  {333, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2413 */
  {333, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2414 */
  {333,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2415 */
  {333,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2416 */
  {333, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2417 */
  {333, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2418 */
  {333,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2419 */
  {333,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2420 */
  {333, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2421 */
  {333, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2422 */
  {333,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2423 */
  {334,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2424 */
  {334, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2425 */
  {334, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2426 */
  {334,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2427 */
  {334,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2428 */
  {334, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2429 */
  {334, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2430 */
  {334,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2431 */
  {334,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2432 */
  {334, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2433 */
  {334, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2434 */
  {334,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2435 */
  {334,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2436 */
  {334, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2437 */
  {334, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2438 */
  {334,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2439 */
  {334,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2440 */
  {334, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2441 */
  {334, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2442 */
  {334,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2443 */
  {334,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2444 */
  {334, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2445 */
  {334, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2446 */
  {334,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2447 */
  {334,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2448 */
  {334, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2449 */
  {334, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2450 */
  {334,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2451 */
  {334,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2452 */
  {334, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2453 */
  {334, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2454 */
  {334,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2455 */
  {335,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2456 */
  {335, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2457 */
  {335,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2458 */
  {335, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2459 */
  {335, -1, 0, 0, 0,-1, 0, 0, 0,-1, 3, 3, 3 }, /* 2460 */
  {335,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 3 }, /* 2461 */
  {335, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 3 }, /* 2462 */
  {335,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 3 }, /* 2463 */
  {335,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2464 */
  {335, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2465 */
  {335,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2466 */
  {335, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2467 */
  {335, -1, 0, 0, 0,-1, 0, 0, 0,-1, 3, 9, 9 }, /* 2468 */
  {335,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 9 }, /* 2469 */
  {335, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 9, 9 }, /* 2470 */
  {335,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 9 }, /* 2471 */
  {335,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2472 */
  {335, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2473 */
  {335,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2474 */
  {335, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2475 */
  {335, -1, 0, 0, 0,-1, 0, 0, 0,-1, 9, 3, 9 }, /* 2476 */
  {335,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 9 }, /* 2477 */
  {335, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 3, 9 }, /* 2478 */
  {335,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 9 }, /* 2479 */
  {335,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2480 */
  {335, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2481 */
  {335,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2482 */
  {335, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2483 */
  {335, -1, 0, 0, 0,-1, 0, 0, 0,-1, 9, 9, 3 }, /* 2484 */
  {335,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 3 }, /* 2485 */
  {335, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 9, 3 }, /* 2486 */
  {335,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 3 }, /* 2487 */
  {336,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2488 */
  {336, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2489 */
  {336, -1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 0 }, /* 2490 */
  {336,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 0 }, /* 2491 */
  {336,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 3, 3 }, /* 2492 */
  {336, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 3, 3 }, /* 2493 */
  {336, -1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 0, 3 }, /* 2494 */
  {336,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 0, 3 }, /* 2495 */
  {336,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2496 */
  {336, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2497 */
  {336, -1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 6 }, /* 2498 */
  {336,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 6 }, /* 2499 */
  {336,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 9, 9 }, /* 2500 */
  {336, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 9, 9 }, /* 2501 */
  {336, -1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 6, 9 }, /* 2502 */
  {336,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 6, 9 }, /* 2503 */
  {336,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2504 */
  {336, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2505 */
  {336, -1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 6 }, /* 2506 */
  {336,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 6 }, /* 2507 */
  {336,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 3, 9 }, /* 2508 */
  {336, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 3, 9 }, /* 2509 */
  {336, -1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 0, 9 }, /* 2510 */
  {336,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 0, 9 }, /* 2511 */
  {336,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2512 */
  {336, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2513 */
  {336, -1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 0 }, /* 2514 */
  {336,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 0 }, /* 2515 */
  {336,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 9, 3 }, /* 2516 */
  {336, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 9, 3 }, /* 2517 */
  {336, -1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 6, 3 }, /* 2518 */
  {336,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 6, 3 }, /* 2519 */
  {337,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2520 */
  {337, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2521 */
  {337, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2522 */
  {337,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2523 */
  {337,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2524 */
  {337, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2525 */
  {337, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2526 */
  {337,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2527 */
  {337,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2528 */
  {337, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2529 */
  {337, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2530 */
  {337,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2531 */
  {337,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2532 */
  {337, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2533 */
  {337, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2534 */
  {337,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2535 */
  {338,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2536 */
  {338, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2537 */
  {338, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2538 */
  {338,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2539 */
  {338,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2540 */
  {338, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2541 */
  {338, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2542 */
  {338,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2543 */
  {338,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2544 */
  {338, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2545 */
  {338, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2546 */
  {338,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2547 */
  {338,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2548 */
  {338, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2549 */
  {338, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2550 */
  {338,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2551 */
  {339,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2552 */
  {339, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2553 */
  {339, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2554 */
  {339,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2555 */
  {339,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2556 */
  {339, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2557 */
  {339, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2558 */
  {339,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2559 */
  {339,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2560 */
  {339, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2561 */
  {339, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2562 */
  {339,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2563 */
  {339,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2564 */
  {339, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2565 */
  {339, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2566 */
  {339,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2567 */
  {340,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2568 */
  {340, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2569 */
  {340, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2570 */
  {340,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2571 */
  {340,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2572 */
  {340, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2573 */
  {340, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2574 */
  {340,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2575 */
  {340,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2576 */
  {340, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2577 */
  {340, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2578 */
  {340,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2579 */
  {340,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2580 */
  {340, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2581 */
  {340, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2582 */
  {340,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2583 */
  {341,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2584 */
  {341, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2585 */
  {341, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2586 */
  {341,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2587 */
  {341,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2588 */
  {341, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2589 */
  {341, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2590 */
  {341,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2591 */
  {341,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2592 */
  {341, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2593 */
  {341, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2594 */
  {341,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2595 */
  {341,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2596 */
  {341, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2597 */
  {341, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2598 */
  {341,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2599 */
  {342,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2600 */
  {342, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2601 */
  {342, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2602 */
  {342,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2603 */
  {342,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2604 */
  {342, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2605 */
  {342, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2606 */
  {342,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2607 */
  {342,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2608 */
  {342, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2609 */
  {342, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2610 */
  {342,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2611 */
  {342,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2612 */
  {342, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2613 */
  {342, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2614 */
  {342,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2615 */
  {343,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2616 */
  {343, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2617 */
  {343, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2618 */
  {343,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2619 */
  {343,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2620 */
  {343, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2621 */
  {343, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2622 */
  {343,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2623 */
  {343,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2624 */
  {343, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2625 */
  {343, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2626 */
  {343,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2627 */
  {343,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2628 */
  {343, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2629 */
  {343, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2630 */
  {343,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2631 */
  {344,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2632 */
  {344, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2633 */
  {344, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2634 */
  {344,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2635 */
  {344,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 2636 */
  {344, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 2637 */
  {344, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2638 */
  {344,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2639 */
  {344,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2640 */
  {344, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2641 */
  {344, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2642 */
  {344,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2643 */
  {344,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2644 */
  {344, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2645 */
  {344, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2646 */
  {344,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2647 */
  {345,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2648 */
  {345, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2649 */
  {345, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2650 */
  {345,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2651 */
  {345,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2652 */
  {345, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2653 */
  {345, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2654 */
  {345,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2655 */
  {345,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2656 */
  {345, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2657 */
  {345, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2658 */
  {345,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2659 */
  {345,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2660 */
  {345, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2661 */
  {345, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2662 */
  {345,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2663 */
  {346,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2664 */
  {346, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2665 */
  {346, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2666 */
  {346,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2667 */
  {346,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 2668 */
  {346, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 2669 */
  {346, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2670 */
  {346,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2671 */
  {346,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2672 */
  {346, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2673 */
  {346, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2674 */
  {346,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2675 */
  {346,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 2676 */
  {346, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 2677 */
  {346, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2678 */
  {346,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2679 */
  {347,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2680 */
  {347, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2681 */
  {347, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2682 */
  {347,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2683 */
  {347,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 2684 */
  {347, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 2685 */
  {347, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 2686 */
  {347,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 2687 */
  {347,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2688 */
  {347, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2689 */
  {347, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2690 */
  {347,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2691 */
  {347,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 2692 */
  {347, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 2693 */
  {347, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 2694 */
  {347,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 2695 */
  {348,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2696 */
  {348, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2697 */
  {348, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2698 */
  {348,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2699 */
  {348,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2700 */
  {348, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2701 */
  {348, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 2702 */
  {348,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2703 */
  {348,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2704 */
  {348, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2705 */
  {348, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2706 */
  {348,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2707 */
  {348,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2708 */
  {348, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2709 */
  {348, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2710 */
  {348,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2711 */
  {349,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2712 */
  {349,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2713 */
  {349, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2714 */
  {349,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2715 */
  {350,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2716 */
  {350,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 3 }, /* 2717 */
  {350, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2718 */
  {350,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 9 }, /* 2719 */
  {351,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2720 */
  {351,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2721 */
  {351, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2722 */
  {351,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2723 */
  {352,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2724 */
  {352,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 9 }, /* 2725 */
  {352, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2726 */
  {352,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 3 }, /* 2727 */
  {353,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2728 */
  {353,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2729 */
  {353, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2730 */
  {353,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2731 */
  {353,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2732 */
  {353,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2733 */
  {353, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2734 */
  {353,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2735 */
  {354,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2736 */
  {354,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 2737 */
  {354, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2738 */
  {354,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 2739 */
  {354,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2740 */
  {354,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 2741 */
  {354, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2742 */
  {354,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 2743 */
  {355,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2744 */
  {355,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2745 */
  {355, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2746 */
  {355,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2747 */
  {356,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2748 */
  {356,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2749 */
  {356, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2750 */
  {356,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2751 */
  {356,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2752 */
  {356,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2753 */
  {356, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2754 */
  {356,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2755 */
  {357,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2756 */
  {357, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2757 */
  {357,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2758 */
  {357,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2759 */
  {357, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2760 */
  {357,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2761 */
  {357,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2762 */
  {357,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2763 */
  {358,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2764 */
  {358, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2765 */
  {358,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2766 */
  {358,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 2767 */
  {358, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2768 */
  {358,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2769 */
  {358,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2770 */
  {358,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 2771 */
  {359,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2772 */
  {359,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 2773 */
  {359, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2774 */
  {359,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 2775 */
  {359, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2776 */
  {359,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2777 */
  {359,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2778 */
  {359,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2779 */
  {360,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2780 */
  {360, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2781 */
  {360,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 2782 */
  {360,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 2783 */
  {360, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2784 */
  {360,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2785 */
  {360,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 2786 */
  {360,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 2787 */
  {361,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2788 */
  {361,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2789 */
  {361, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2790 */
  {361,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2791 */
  {361, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2792 */
  {361,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2793 */
  {361,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2794 */
  {361,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2795 */
  {362,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2796 */
  {362, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2797 */
  {362,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 2798 */
  {362,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 2799 */
  {362, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2800 */
  {362,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2801 */
  {362,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 2802 */
  {362,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 2803 */
  {363,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2804 */
  {363, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2805 */
  {363,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2806 */
  {363,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2807 */
  {363, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2808 */
  {363,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2809 */
  {363,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2810 */
  {363,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2811 */
  {363,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2812 */
  {363, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2813 */
  {363,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2814 */
  {363,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2815 */
  {363, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2816 */
  {363,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2817 */
  {363,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2818 */
  {363,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2819 */
  {364,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2820 */
  {364,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 2821 */
  {364, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2822 */
  {364,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 2823 */
  {364, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 3 }, /* 2824 */
  {364,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2825 */
  {364,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 9 }, /* 2826 */
  {364,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2827 */
  {364,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2828 */
  {364,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 2829 */
  {364, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2830 */
  {364,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 2831 */
  {364, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 9 }, /* 2832 */
  {364,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2833 */
  {364,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 3 }, /* 2834 */
  {364,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2835 */
  {365,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2836 */
  {365, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2837 */
  {365,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 3 }, /* 2838 */
  {365,  0, 1, 0,-1, 0, 0, 0, 0,-1, 9, 3, 3 }, /* 2839 */
  {365, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 2840 */
  {365,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 2841 */
  {365,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 9 }, /* 2842 */
  {365,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 9 }, /* 2843 */
  {365,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2844 */
  {365, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2845 */
  {365,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 2846 */
  {365,  0, 1, 0,-1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 2847 */
  {365, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 2848 */
  {365,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 2849 */
  {365,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 2850 */
  {365,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 2851 */
  {366,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2852 */
  {366,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2853 */
  {366, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2854 */
  {366,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2855 */
  {366,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2856 */
  {366,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2857 */
  {366, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2858 */
  {366,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2859 */
  {367,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2860 */
  {367,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 2861 */
  {367, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2862 */
  {367,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 2863 */
  {367,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 2864 */
  {367,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2865 */
  {367, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 2866 */
  {367,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2867 */
  {368,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2868 */
  {368,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 3 }, /* 2869 */
  {368, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2870 */
  {368,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 9 }, /* 2871 */
  {368,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2872 */
  {368,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 3 }, /* 2873 */
  {368, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2874 */
  {368,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 9 }, /* 2875 */
  {369,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2876 */
  {369,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 3 }, /* 2877 */
  {369, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2878 */
  {369,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 9 }, /* 2879 */
  {369,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 9 }, /* 2880 */
  {369,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 2881 */
  {369, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 3 }, /* 2882 */
  {369,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2883 */
  {370,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2884 */
  {370,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2885 */
  {370, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2886 */
  {370,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2887 */
  {370,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2888 */
  {370,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 2889 */
  {370, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2890 */
  {370,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 2891 */
  {371,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2892 */
  {371,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2893 */
  {371, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2894 */
  {371,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2895 */
  {371,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2896 */
  {371,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2897 */
  {371, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2898 */
  {371,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2899 */
  {372,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2900 */
  {372,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 9 }, /* 2901 */
  {372, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2902 */
  {372,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 3 }, /* 2903 */
  {372,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 2904 */
  {372,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 9 }, /* 2905 */
  {372, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2906 */
  {372,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 3 }, /* 2907 */
  {373,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2908 */
  {373,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 9 }, /* 2909 */
  {373, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2910 */
  {373,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 3 }, /* 2911 */
  {373,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 3 }, /* 2912 */
  {373,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 2913 */
  {373, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 9 }, /* 2914 */
  {373,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2915 */
  {374,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2916 */
  {374,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2917 */
  {374, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2918 */
  {374,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2919 */
  {374,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 2920 */
  {374,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2921 */
  {374, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 2922 */
  {374,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2923 */
  {374,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2924 */
  {374,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2925 */
  {374, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2926 */
  {374,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2927 */
  {374,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 2928 */
  {374,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2929 */
  {374, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 2930 */
  {374,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2931 */
  {375,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2932 */
  {375,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 2933 */
  {375, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2934 */
  {375,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 2935 */
  {375,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 3 }, /* 2936 */
  {375,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2937 */
  {375, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 9 }, /* 2938 */
  {375,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2939 */
  {375,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2940 */
  {375,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 2941 */
  {375, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2942 */
  {375,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 2943 */
  {375,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 9 }, /* 2944 */
  {375,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 2945 */
  {375, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 3 }, /* 2946 */
  {375,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 2947 */
  {376,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2948 */
  {376,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2949 */
  {376, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2950 */
  {376,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2951 */
  {376, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2952 */
  {376,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2953 */
  {376,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2954 */
  {376,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2955 */
  {377,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2956 */
  {377,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2957 */
  {377, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2958 */
  {377,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2959 */
  {377, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 2960 */
  {377,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 2961 */
  {377,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 2962 */
  {377,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 2963 */
  {378,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2964 */
  {378,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2965 */
  {378, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2966 */
  {378,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2967 */
  {378, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2968 */
  {378,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2969 */
  {378,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2970 */
  {378,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2971 */
  {379,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2972 */
  {379,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2973 */
  {379, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2974 */
  {379,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2975 */
  {379, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2976 */
  {379,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2977 */
  {379,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2978 */
  {379,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2979 */
  {380,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2980 */
  {380,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2981 */
  {380, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2982 */
  {380,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2983 */
  {380, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 2984 */
  {380,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2985 */
  {380,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 2986 */
  {380,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2987 */
  {381,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2988 */
  {381,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2989 */
  {381, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2990 */
  {381,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 2991 */
  {381, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 2992 */
  {381,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2993 */
  {381,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 2994 */
  {381,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 2995 */
  {382,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 2996 */
  {382,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2997 */
  {382, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 2998 */
  {382,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 2999 */
  {382, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3000 */
  {382,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3001 */
  {382,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3002 */
  {382,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3003 */
  {383,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3004 */
  {383,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3005 */
  {383, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3006 */
  {383,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3007 */
  {383, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3008 */
  {383,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3009 */
  {383,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3010 */
  {383,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3011 */
  {384,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3012 */
  {384,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3013 */
  {384, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3014 */
  {384,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3015 */
  {384, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3016 */
  {384,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3017 */
  {384,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3018 */
  {384,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3019 */
  {384,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3020 */
  {384,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3021 */
  {384, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3022 */
  {384,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3023 */
  {384, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3024 */
  {384,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3025 */
  {384,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3026 */
  {384,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3027 */
  {385,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3028 */
  {385,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3029 */
  {385, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3030 */
  {385,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3031 */
  {385, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3032 */
  {385,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3033 */
  {385,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3034 */
  {385,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3035 */
  {385,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3036 */
  {385,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3037 */
  {385, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3038 */
  {385,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3039 */
  {385, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3040 */
  {385,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3041 */
  {385,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3042 */
  {385,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3043 */
  {386,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3044 */
  {386,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3045 */
  {386, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3046 */
  {386,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3047 */
  {386, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3048 */
  {386,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3049 */
  {386,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3050 */
  {386,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3051 */
  {386,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3052 */
  {386,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3053 */
  {386, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3054 */
  {386,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3055 */
  {386, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3056 */
  {386,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3057 */
  {386,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3058 */
  {386,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3059 */
  {387,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3060 */
  {387,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3061 */
  {387, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3062 */
  {387,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3063 */
  {387, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3064 */
  {387,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 9 }, /* 3065 */
  {387,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3066 */
  {387,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 3 }, /* 3067 */
  {387,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3068 */
  {387,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3069 */
  {387, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3070 */
  {387,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3071 */
  {387, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3072 */
  {387,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 3 }, /* 3073 */
  {387,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3074 */
  {387,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 9 }, /* 3075 */
  {388,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3076 */
  {388,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3077 */
  {388, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3078 */
  {388,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3079 */
  {388,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3080 */
  {388,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3081 */
  {388, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3082 */
  {388,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3083 */
  {389,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3084 */
  {389,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3085 */
  {389, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3086 */
  {389,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3087 */
  {389,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 3088 */
  {389,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3089 */
  {389, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 3090 */
  {389,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3091 */
  {390,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3092 */
  {390,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3093 */
  {390, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3094 */
  {390,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3095 */
  {390,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3096 */
  {390,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3097 */
  {390, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3098 */
  {390,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3099 */
  {391,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3100 */
  {391,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3101 */
  {391, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3102 */
  {391,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3103 */
  {391,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3104 */
  {391,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3105 */
  {391, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3106 */
  {391,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3107 */
  {392,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3108 */
  {392,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3109 */
  {392, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3110 */
  {392,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3111 */
  {392, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3112 */
  {392,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3113 */
  {392,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3114 */
  {392,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3115 */
  {393,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3116 */
  {393,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3117 */
  {393, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3118 */
  {393,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3119 */
  {393, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3120 */
  {393,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3121 */
  {393,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3122 */
  {393,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3123 */
  {394,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3124 */
  {394,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3125 */
  {394, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3126 */
  {394,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3127 */
  {394, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3128 */
  {394,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3129 */
  {394,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3130 */
  {394,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3131 */
  {395,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3132 */
  {395,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3133 */
  {395, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3134 */
  {395,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3135 */
  {395, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3136 */
  {395,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3137 */
  {395,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3138 */
  {395,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3139 */
  {396,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3140 */
  {396,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3141 */
  {396, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3142 */
  {396,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3143 */
  {396, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3144 */
  {396,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3145 */
  {396,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3146 */
  {396,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3147 */
  {396,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3148 */
  {396,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3149 */
  {396, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3150 */
  {396,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3151 */
  {396, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3152 */
  {396,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3153 */
  {396,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3154 */
  {396,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3155 */
  {397,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3156 */
  {397,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3157 */
  {397, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3158 */
  {397,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3159 */
  {397, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3160 */
  {397,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3161 */
  {397,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3162 */
  {397,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3163 */
  {397,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3164 */
  {397,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3165 */
  {397, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3166 */
  {397,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3167 */
  {397, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3168 */
  {397,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3169 */
  {397,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3170 */
  {397,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3171 */
  {398,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3172 */
  {398,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3173 */
  {398, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3174 */
  {398,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3175 */
  {398,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3176 */
  {398,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3177 */
  {398, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3178 */
  {398,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3179 */
  {398,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3180 */
  {398,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3181 */
  {398, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3182 */
  {398,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3183 */
  {398,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3184 */
  {398,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3185 */
  {398, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3186 */
  {398,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3187 */
  {399,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3188 */
  {399,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3189 */
  {399, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3190 */
  {399,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3191 */
  {399,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 3 }, /* 3192 */
  {399,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3193 */
  {399, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 3 }, /* 3194 */
  {399,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3195 */
  {399,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3196 */
  {399,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3197 */
  {399, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3198 */
  {399,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3199 */
  {399,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 9 }, /* 3200 */
  {399,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3201 */
  {399, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 9 }, /* 3202 */
  {399,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3203 */
  {400,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3204 */
  {400, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3205 */
  {400,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3206 */
  {400,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3207 */
  {400, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3208 */
  {400,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3209 */
  {400,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3210 */
  {400,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3211 */
  {400,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3212 */
  {400, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3213 */
  {400,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3214 */
  {400,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3215 */
  {400, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3216 */
  {400,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3217 */
  {400,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3218 */
  {400,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3219 */
  {401,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3220 */
  {401, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3221 */
  {401,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3222 */
  {401,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3223 */
  {401, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3224 */
  {401,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3225 */
  {401,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3226 */
  {401,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3227 */
  {401,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 3228 */
  {401, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3229 */
  {401,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3230 */
  {401,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3231 */
  {401, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 3232 */
  {401,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3233 */
  {401,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3234 */
  {401,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3235 */
  {402,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3236 */
  {402,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3237 */
  {402, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3238 */
  {402,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3239 */
  {402,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3240 */
  {402,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3241 */
  {402, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3242 */
  {402,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3243 */
  {402, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3244 */
  {402,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3245 */
  {402,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3246 */
  {402,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3247 */
  {402, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3248 */
  {402,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3249 */
  {402,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3250 */
  {402,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3251 */
  {403,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3252 */
  {403, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3253 */
  {403,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 3254 */
  {403,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 3255 */
  {403, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3256 */
  {403,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3257 */
  {403,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 3258 */
  {403,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 3259 */
  {403,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 3260 */
  {403, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 3261 */
  {403,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3262 */
  {403,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3263 */
  {403, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 3264 */
  {403,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 3265 */
  {403,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3266 */
  {403,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3267 */
  {404,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3268 */
  {404,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3269 */
  {404, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3270 */
  {404,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3271 */
  {404,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3272 */
  {404,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3273 */
  {404, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3274 */
  {404,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3275 */
  {404, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3276 */
  {404,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3277 */
  {404,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3278 */
  {404,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3279 */
  {404, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3280 */
  {404,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3281 */
  {404,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3282 */
  {404,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3283 */
  {405,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3284 */
  {405, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3285 */
  {405,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 3286 */
  {405,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 3287 */
  {405, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3288 */
  {405,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3289 */
  {405,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 3290 */
  {405,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 3291 */
  {405,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 3292 */
  {405, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 3293 */
  {405,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3294 */
  {405,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3295 */
  {405, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 3296 */
  {405,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 3297 */
  {405,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3298 */
  {405,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3299 */
  {406,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3300 */
  {406, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3301 */
  {406,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3302 */
  {406,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3303 */
  {406, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3304 */
  {406,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3305 */
  {406,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3306 */
  {406,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3307 */
  {406,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3308 */
  {406, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3309 */
  {406,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3310 */
  {406,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3311 */
  {406, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3312 */
  {406,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3313 */
  {406,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3314 */
  {406,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3315 */
  {407,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3316 */
  {407, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3317 */
  {407,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3318 */
  {407,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3319 */
  {407, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3320 */
  {407,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3321 */
  {407,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3322 */
  {407,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3323 */
  {407,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3324 */
  {407, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3325 */
  {407,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3326 */
  {407,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3327 */
  {407, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3328 */
  {407,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3329 */
  {407,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3330 */
  {407,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3331 */
  {408,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3332 */
  {408,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3333 */
  {408, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3334 */
  {408,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3335 */
  {408,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3336 */
  {408,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3337 */
  {408, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3338 */
  {408,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3339 */
  {408, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3340 */
  {408,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3341 */
  {408,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3342 */
  {408,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3343 */
  {408, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3344 */
  {408,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3345 */
  {408,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3346 */
  {408,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3347 */
  {409,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3348 */
  {409, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3349 */
  {409,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 3350 */
  {409,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 3351 */
  {409, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3352 */
  {409,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3353 */
  {409,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 3354 */
  {409,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 3355 */
  {409,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 3356 */
  {409, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 3357 */
  {409,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3358 */
  {409,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3359 */
  {409, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 3360 */
  {409,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 3361 */
  {409,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3362 */
  {409,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3363 */
  {410,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3364 */
  {410,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3365 */
  {410, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3366 */
  {410,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3367 */
  {410,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3368 */
  {410,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3369 */
  {410, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3370 */
  {410,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3371 */
  {410, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3372 */
  {410,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3373 */
  {410,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3374 */
  {410,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3375 */
  {410, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3376 */
  {410,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3377 */
  {410,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3378 */
  {410,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3379 */
  {411,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3380 */
  {411, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3381 */
  {411,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 3382 */
  {411,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 3383 */
  {411, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3384 */
  {411,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3385 */
  {411,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 3386 */
  {411,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 3387 */
  {411,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 3388 */
  {411, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 3389 */
  {411,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3390 */
  {411,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3391 */
  {411, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 3392 */
  {411,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 3393 */
  {411,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3394 */
  {411,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3395 */
  {412,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3396 */
  {412, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3397 */
  {412,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3398 */
  {412,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3399 */
  {412, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3400 */
  {412,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3401 */
  {412,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3402 */
  {412,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3403 */
  {412,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3404 */
  {412, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3405 */
  {412,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3406 */
  {412,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3407 */
  {412, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3408 */
  {412,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3409 */
  {412,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3410 */
  {412,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3411 */
  {413,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3412 */
  {413, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3413 */
  {413,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3414 */
  {413,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3415 */
  {413, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3416 */
  {413,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3417 */
  {413,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3418 */
  {413,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3419 */
  {413,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 3420 */
  {413, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3421 */
  {413,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3422 */
  {413,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3423 */
  {413, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 3424 */
  {413,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3425 */
  {413,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3426 */
  {413,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3427 */
  {414,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3428 */
  {414,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3429 */
  {414, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3430 */
  {414,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3431 */
  {414,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 3432 */
  {414,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3433 */
  {414, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 3434 */
  {414,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3435 */
  {414, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3436 */
  {414,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3437 */
  {414,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3438 */
  {414,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3439 */
  {414, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3440 */
  {414,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3441 */
  {414,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3442 */
  {414,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3443 */
  {415,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3444 */
  {415, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3445 */
  {415,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 3446 */
  {415,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 3447 */
  {415, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3448 */
  {415,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3449 */
  {415,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 3450 */
  {415,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 3451 */
  {415,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 0 }, /* 3452 */
  {415, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0 }, /* 3453 */
  {415,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3454 */
  {415,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3455 */
  {415, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 3456 */
  {415,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 3457 */
  {415,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3458 */
  {415,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3459 */
  {416,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3460 */
  {416,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3461 */
  {416, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3462 */
  {416,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3463 */
  {416,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3464 */
  {416,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3465 */
  {416, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3466 */
  {416,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3467 */
  {416, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3468 */
  {416,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3469 */
  {416,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3470 */
  {416,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3471 */
  {416, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3472 */
  {416,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3473 */
  {416,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3474 */
  {416,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3475 */
  {417,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3476 */
  {417, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3477 */
  {417,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 3478 */
  {417,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 3479 */
  {417, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3480 */
  {417,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3481 */
  {417,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 3482 */
  {417,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 3483 */
  {417,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 3484 */
  {417, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 3485 */
  {417,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3486 */
  {417,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3487 */
  {417, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 3488 */
  {417,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 3489 */
  {417,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3490 */
  {417,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3491 */
  {418,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3492 */
  {418, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3493 */
  {418,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3494 */
  {418,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3495 */
  {418, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3496 */
  {418,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3497 */
  {418,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3498 */
  {418,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3499 */
  {418,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3500 */
  {418, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3501 */
  {418,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3502 */
  {418,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3503 */
  {418, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3504 */
  {418,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3505 */
  {418,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3506 */
  {418,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3507 */
  {419,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3508 */
  {419, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3509 */
  {419,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3510 */
  {419,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3511 */
  {419, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3512 */
  {419,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3513 */
  {419,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3514 */
  {419,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3515 */
  {419,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3516 */
  {419, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3517 */
  {419,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3518 */
  {419,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3519 */
  {419, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3520 */
  {419,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3521 */
  {419,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3522 */
  {419,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3523 */
  {420,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3524 */
  {420,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3525 */
  {420, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3526 */
  {420,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3527 */
  {420,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3528 */
  {420,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3529 */
  {420, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3530 */
  {420,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3531 */
  {420, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3532 */
  {420,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3533 */
  {420,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3534 */
  {420,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3535 */
  {420, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3536 */
  {420,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3537 */
  {420,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3538 */
  {420,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3539 */
  {421,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3540 */
  {421, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3541 */
  {421,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 3542 */
  {421,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 3543 */
  {421, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3544 */
  {421,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3545 */
  {421,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 3546 */
  {421,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 3547 */
  {421,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 0 }, /* 3548 */
  {421, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 0 }, /* 3549 */
  {421,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3550 */
  {421,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3551 */
  {421, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 3552 */
  {421,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 3553 */
  {421,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3554 */
  {421,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3555 */
  {422,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3556 */
  {422,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3557 */
  {422, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3558 */
  {422,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3559 */
  {422,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3560 */
  {422,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3561 */
  {422, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3562 */
  {422,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3563 */
  {422, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3564 */
  {422,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3565 */
  {422,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3566 */
  {422,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3567 */
  {422, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3568 */
  {422,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3569 */
  {422,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3570 */
  {422,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3571 */
  {423,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3572 */
  {423, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3573 */
  {423,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 3574 */
  {423,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 3575 */
  {423, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3576 */
  {423,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3577 */
  {423,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 3578 */
  {423,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 3579 */
  {423,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 3580 */
  {423, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 3581 */
  {423,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3582 */
  {423,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3583 */
  {423, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 3584 */
  {423,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 3585 */
  {423,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3586 */
  {423,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3587 */
  {424,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3588 */
  {424, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3589 */
  {424,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3590 */
  {424,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3591 */
  {424, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3592 */
  {424,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3593 */
  {424,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3594 */
  {424,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3595 */
  {424,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3596 */
  {424, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3597 */
  {424,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3598 */
  {424,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3599 */
  {424, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3600 */
  {424,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3601 */
  {424,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3602 */
  {424,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3603 */
  {424,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3604 */
  {424, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3605 */
  {424,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3606 */
  {424,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3607 */
  {424, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3608 */
  {424,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3609 */
  {424,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3610 */
  {424,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3611 */
  {424,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3612 */
  {424, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3613 */
  {424,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3614 */
  {424,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3615 */
  {424, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3616 */
  {424,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3617 */
  {424,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3618 */
  {424,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3619 */
  {425,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3620 */
  {425, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3621 */
  {425,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3622 */
  {425,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3623 */
  {425, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3624 */
  {425,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3625 */
  {425,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3626 */
  {425,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3627 */
  {425,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 3628 */
  {425, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3629 */
  {425,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3630 */
  {425,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3631 */
  {425, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 3632 */
  {425,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3633 */
  {425,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3634 */
  {425,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3635 */
  {425,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3636 */
  {425, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3637 */
  {425,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3638 */
  {425,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3639 */
  {425, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3640 */
  {425,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 3641 */
  {425,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3642 */
  {425,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3643 */
  {425,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3644 */
  {425, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3645 */
  {425,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3646 */
  {425,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3647 */
  {425, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 3648 */
  {425,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3649 */
  {425,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3650 */
  {425,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 3651 */
  {426,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3652 */
  {426,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3653 */
  {426, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3654 */
  {426,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3655 */
  {426,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 3 }, /* 3656 */
  {426,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3657 */
  {426, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 9 }, /* 3658 */
  {426,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3659 */
  {426, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 3 }, /* 3660 */
  {426,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3661 */
  {426,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 9 }, /* 3662 */
  {426,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3663 */
  {426, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3664 */
  {426,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3665 */
  {426,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3666 */
  {426,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3667 */
  {426,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3668 */
  {426,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3669 */
  {426, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3670 */
  {426,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3671 */
  {426,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 9 }, /* 3672 */
  {426,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3673 */
  {426, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 3 }, /* 3674 */
  {426,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3675 */
  {426, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 9 }, /* 3676 */
  {426,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3677 */
  {426,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 3 }, /* 3678 */
  {426,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3679 */
  {426, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3680 */
  {426,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3681 */
  {426,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3682 */
  {426,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3683 */
  {427,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3684 */
  {427, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3685 */
  {427,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 3 }, /* 3686 */
  {427,  0, 1, 0,-1, 0, 0, 0, 0,-1, 3, 9, 3 }, /* 3687 */
  {427, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 3688 */
  {427,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 3689 */
  {427,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 9 }, /* 3690 */
  {427,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 9 }, /* 3691 */
  {427,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3692 */
  {427, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3693 */
  {427,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 3, 9 }, /* 3694 */
  {427,  0, 1, 0, 1, 0, 0, 0, 0, 1, 3, 3, 9 }, /* 3695 */
  {427, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 3696 */
  {427,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 3697 */
  {427,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 3 }, /* 3698 */
  {427,  0,-1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 3 }, /* 3699 */
  {427,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3700 */
  {427, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3701 */
  {427,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 3702 */
  {427,  0, 1, 0,-1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 3703 */
  {427, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 3704 */
  {427,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 3705 */
  {427,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 3706 */
  {427,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 3707 */
  {427,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3708 */
  {427, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3709 */
  {427,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 3710 */
  {427,  0, 1, 0, 1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 3711 */
  {427, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 3712 */
  {427,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 3713 */
  {427,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 3714 */
  {427,  0,-1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 3715 */
  {428,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3716 */
  {428,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3717 */
  {428, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 3718 */
  {428,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3719 */
  {428,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 3 }, /* 3720 */
  {428,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3721 */
  {428, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 9 }, /* 3722 */
  {428,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3723 */
  {428, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 3 }, /* 3724 */
  {428,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3725 */
  {428,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 9 }, /* 3726 */
  {428,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3727 */
  {428, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3728 */
  {428,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 3 }, /* 3729 */
  {428,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3730 */
  {428,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 9 }, /* 3731 */
  {428,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3732 */
  {428,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 9 }, /* 3733 */
  {428, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3734 */
  {428,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 3 }, /* 3735 */
  {428,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 9 }, /* 3736 */
  {428,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3737 */
  {428, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 3 }, /* 3738 */
  {428,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 3739 */
  {428, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 9 }, /* 3740 */
  {428,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 3741 */
  {428,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 3 }, /* 3742 */
  {428,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3743 */
  {428, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3744 */
  {428,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 9 }, /* 3745 */
  {428,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 3746 */
  {428,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 3 }, /* 3747 */
  {429,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3748 */
  {429, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3749 */
  {429,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 3 }, /* 3750 */
  {429,  0, 1, 0,-1, 0, 0, 0, 0,-1, 3, 9, 3 }, /* 3751 */
  {429, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 3752 */
  {429,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 3753 */
  {429,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 9 }, /* 3754 */
  {429,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 9 }, /* 3755 */
  {429,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 3756 */
  {429, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3757 */
  {429,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 3758 */
  {429,  0, 1, 0, 1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 3759 */
  {429, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 3760 */
  {429,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 3761 */
  {429,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 3762 */
  {429,  0,-1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 3763 */
  {429,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 3764 */
  {429, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 3765 */
  {429,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 3766 */
  {429,  0, 1, 0,-1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 3767 */
  {429, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 3768 */
  {429,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 3769 */
  {429,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 3770 */
  {429,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 3771 */
  {429,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 3772 */
  {429, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 3773 */
  {429,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 9, 9 }, /* 3774 */
  {429,  0, 1, 0, 1, 0, 0, 0, 0, 1, 9, 9, 9 }, /* 3775 */
  {429, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 3776 */
  {429,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 3777 */
  {429,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 3 }, /* 3778 */
  {429,  0,-1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 3 }, /* 3779 */
  {430,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3780 */
  {430,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3781 */
  {430, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3782 */
  {431,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3783 */
  {431,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 4 }, /* 3784 */
  {431, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 3785 */
  {432,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3786 */
  {432,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 8 }, /* 3787 */
  {432, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 3788 */
  {433,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3789 */
  {433,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3790 */
  {433, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3791 */
  {433,  1, 0, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 3792 */
  {433,  0,-1, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 3793 */
  {433, -1, 1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 3794 */
  {433,  1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 3795 */
  {433,  0,-1, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 3796 */
  {433, -1, 1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 3797 */
  {434,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3798 */
  {434,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 3799 */
  {434,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 3800 */
  {435,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3801 */
  {435, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3802 */
  {435,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3803 */
  {435,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3804 */
  {435, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3805 */
  {435,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3806 */
  {436,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3807 */
  {436, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3808 */
  {436,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3809 */
  {436,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3810 */
  {436, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3811 */
  {436,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3812 */
  {436,  1, 0, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 3813 */
  {436, -1, 0, 0, 0,-1, 0, 0, 0,-1, 8, 4, 4 }, /* 3814 */
  {436,  0,-1, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 3815 */
  {436,  0, 1, 0,-1, 1, 0, 0, 0,-1, 8, 4, 4 }, /* 3816 */
  {436, -1, 1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 3817 */
  {436,  1,-1, 0, 1, 0, 0, 0, 0,-1, 8, 4, 4 }, /* 3818 */
  {436,  1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 3819 */
  {436, -1, 0, 0, 0,-1, 0, 0, 0,-1, 4, 8, 8 }, /* 3820 */
  {436,  0,-1, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 3821 */
  {436,  0, 1, 0,-1, 1, 0, 0, 0,-1, 4, 8, 8 }, /* 3822 */
  {436, -1, 1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 3823 */
  {436,  1,-1, 0, 1, 0, 0, 0, 0,-1, 4, 8, 8 }, /* 3824 */
  {437,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3825 */
  {437, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3826 */
  {437,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 3827 */
  {437,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 3828 */
  {437,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 3829 */
  {437,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 3830 */
  {438,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3831 */
  {438,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3832 */
  {438, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3833 */
  {438,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3834 */
  {438, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3835 */
  {438,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3836 */
  {439,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3837 */
  {439,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3838 */
  {439, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3839 */
  {439,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3840 */
  {439,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3841 */
  {439, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3842 */
  {440,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3843 */
  {440,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 4 }, /* 3844 */
  {440, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 3845 */
  {440,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 8 }, /* 3846 */
  {440, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 4 }, /* 3847 */
  {440,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3848 */
  {441,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3849 */
  {441,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 4 }, /* 3850 */
  {441, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 3851 */
  {441,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3852 */
  {441,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 8 }, /* 3853 */
  {441, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 4 }, /* 3854 */
  {442,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3855 */
  {442,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 8 }, /* 3856 */
  {442, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 3857 */
  {442,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 4 }, /* 3858 */
  {442, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 8 }, /* 3859 */
  {442,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3860 */
  {443,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3861 */
  {443,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 8 }, /* 3862 */
  {443, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 3863 */
  {443,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3864 */
  {443,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 4 }, /* 3865 */
  {443, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 8 }, /* 3866 */
  {444,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3867 */
  {444,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3868 */
  {444, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3869 */
  {444,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3870 */
  {444,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3871 */
  {444, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3872 */
  {444,  1, 0, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 3873 */
  {444,  0,-1, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 3874 */
  {444, -1, 1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 3875 */
  {444,  0, 1, 0, 1, 0, 0, 0, 0,-1, 8, 4, 4 }, /* 3876 */
  {444,  1,-1, 0, 0,-1, 0, 0, 0,-1, 8, 4, 4 }, /* 3877 */
  {444, -1, 0, 0,-1, 1, 0, 0, 0,-1, 8, 4, 4 }, /* 3878 */
  {444,  1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 3879 */
  {444,  0,-1, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 3880 */
  {444, -1, 1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 3881 */
  {444,  0, 1, 0, 1, 0, 0, 0, 0,-1, 4, 8, 8 }, /* 3882 */
  {444,  1,-1, 0, 0,-1, 0, 0, 0,-1, 4, 8, 8 }, /* 3883 */
  {444, -1, 0, 0,-1, 1, 0, 0, 0,-1, 4, 8, 8 }, /* 3884 */
  {445,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3885 */
  {445,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 3886 */
  {445,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 3887 */
  {445,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3888 */
  {445, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 3889 */
  {445,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 3890 */
  {446,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3891 */
  {446,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3892 */
  {446, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3893 */
  {446,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3894 */
  {446, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3895 */
  {446,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3896 */
  {447,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3897 */
  {447,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3898 */
  {447, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3899 */
  {447,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3900 */
  {447,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3901 */
  {447, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3902 */
  {448,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3903 */
  {448,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3904 */
  {448, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3905 */
  {448,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3906 */
  {448, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3907 */
  {448,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3908 */
  {449,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3909 */
  {449,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3910 */
  {449, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3911 */
  {449,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3912 */
  {449,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3913 */
  {449, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3914 */
  {450,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3915 */
  {450,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3916 */
  {450, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3917 */
  {450,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3918 */
  {450, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3919 */
  {450,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3920 */
  {450,  1, 0, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 3921 */
  {450,  0,-1, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 3922 */
  {450, -1, 1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 3923 */
  {450,  0,-1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 3924 */
  {450, -1, 1, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 3925 */
  {450,  1, 0, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 3926 */
  {450,  1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 3927 */
  {450,  0,-1, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 3928 */
  {450, -1, 1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 3929 */
  {450,  0,-1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 3930 */
  {450, -1, 1, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 3931 */
  {450,  1, 0, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 3932 */
  {451,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3933 */
  {451,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 3934 */
  {451,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 3935 */
  {451,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3936 */
  {451,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 3937 */
  {451,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 3938 */
  {452,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3939 */
  {452,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3940 */
  {452, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3941 */
  {452,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3942 */
  {452, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3943 */
  {452,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3944 */
  {452,  1, 0, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 3945 */
  {452,  0,-1, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 3946 */
  {452, -1, 1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 3947 */
  {452,  0,-1, 0,-1, 0, 0, 0, 0, 1, 8, 4,10 }, /* 3948 */
  {452, -1, 1, 0, 0, 1, 0, 0, 0, 1, 8, 4,10 }, /* 3949 */
  {452,  1, 0, 0, 1,-1, 0, 0, 0, 1, 8, 4,10 }, /* 3950 */
  {452,  1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 3951 */
  {452,  0,-1, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 3952 */
  {452, -1, 1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 3953 */
  {452,  0,-1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 2 }, /* 3954 */
  {452, -1, 1, 0, 0, 1, 0, 0, 0, 1, 4, 8, 2 }, /* 3955 */
  {452,  1, 0, 0, 1,-1, 0, 0, 0, 1, 4, 8, 2 }, /* 3956 */
  {453,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3957 */
  {453,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 3958 */
  {453,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 3959 */
  {453,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 3960 */
  {453,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 3961 */
  {453,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 3962 */
  {454,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3963 */
  {454, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3964 */
  {454,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3965 */
  {454,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3966 */
  {454, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3967 */
  {454,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3968 */
  {454,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3969 */
  {454,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3970 */
  {454, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3971 */
  {454,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3972 */
  {454,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3973 */
  {454, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3974 */
  {455,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3975 */
  {455, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3976 */
  {455,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3977 */
  {455,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3978 */
  {455, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3979 */
  {455,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3980 */
  {455,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 3981 */
  {455,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 3982 */
  {455, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 3983 */
  {455,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 3984 */
  {455,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 3985 */
  {455, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 3986 */
  {456,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3987 */
  {456, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3988 */
  {456,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3989 */
  {456,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3990 */
  {456, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3991 */
  {456,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3992 */
  {456,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 3993 */
  {456,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 3994 */
  {456,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 3995 */
  {456, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3996 */
  {456, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 3997 */
  {456,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 3998 */
  {457,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 3999 */
  {457, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4000 */
  {457,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4001 */
  {457,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4002 */
  {457, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4003 */
  {457,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4004 */
  {457,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4005 */
  {457,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4006 */
  {457,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4007 */
  {457, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4008 */
  {457, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4009 */
  {457,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4010 */
  {458,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4011 */
  {458, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4012 */
  {458,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4013 */
  {458,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4014 */
  {458, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4015 */
  {458,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4016 */
  {458,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4017 */
  {458,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4018 */
  {458,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4019 */
  {458, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4020 */
  {458, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4021 */
  {458,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4022 */
  {458,  1, 0, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 4023 */
  {458, -1, 0, 0, 0,-1, 0, 0, 0,-1, 8, 4, 4 }, /* 4024 */
  {458,  0,-1, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 4025 */
  {458,  0, 1, 0,-1, 1, 0, 0, 0,-1, 8, 4, 4 }, /* 4026 */
  {458, -1, 1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 4027 */
  {458,  1,-1, 0, 1, 0, 0, 0, 0,-1, 8, 4, 4 }, /* 4028 */
  {458,  0, 1, 0, 1, 0, 0, 0, 0,-1, 8, 4, 4 }, /* 4029 */
  {458,  0,-1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 4030 */
  {458,  1,-1, 0, 0,-1, 0, 0, 0,-1, 8, 4, 4 }, /* 4031 */
  {458, -1, 1, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 4032 */
  {458, -1, 0, 0,-1, 1, 0, 0, 0,-1, 8, 4, 4 }, /* 4033 */
  {458,  1, 0, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 4034 */
  {458,  1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 4035 */
  {458, -1, 0, 0, 0,-1, 0, 0, 0,-1, 4, 8, 8 }, /* 4036 */
  {458,  0,-1, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 4037 */
  {458,  0, 1, 0,-1, 1, 0, 0, 0,-1, 4, 8, 8 }, /* 4038 */
  {458, -1, 1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 4039 */
  {458,  1,-1, 0, 1, 0, 0, 0, 0,-1, 4, 8, 8 }, /* 4040 */
  {458,  0, 1, 0, 1, 0, 0, 0, 0,-1, 4, 8, 8 }, /* 4041 */
  {458,  0,-1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 4042 */
  {458,  1,-1, 0, 0,-1, 0, 0, 0,-1, 4, 8, 8 }, /* 4043 */
  {458, -1, 1, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 4044 */
  {458, -1, 0, 0,-1, 1, 0, 0, 0,-1, 4, 8, 8 }, /* 4045 */
  {458,  1, 0, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 4046 */
  {459,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4047 */
  {459, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4048 */
  {459,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4049 */
  {459,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4050 */
  {459,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4051 */
  {459,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4052 */
  {459,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4053 */
  {459,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4054 */
  {459, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 4055 */
  {459,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 4056 */
  {459,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 4057 */
  {459,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 4058 */
  {460,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4059 */
  {460, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4060 */
  {460,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4061 */
  {460,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4062 */
  {460, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4063 */
  {460,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4064 */
  {460,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4065 */
  {460,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4066 */
  {460,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4067 */
  {460, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4068 */
  {460, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4069 */
  {460,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4070 */
  {460,  1, 0, 0, 0, 1, 0, 0, 0, 1, 8, 4, 4 }, /* 4071 */
  {460, -1, 0, 0, 0,-1, 0, 0, 0,-1, 8, 4, 4 }, /* 4072 */
  {460,  0,-1, 0, 1,-1, 0, 0, 0, 1, 8, 4, 4 }, /* 4073 */
  {460,  0, 1, 0,-1, 1, 0, 0, 0,-1, 8, 4, 4 }, /* 4074 */
  {460, -1, 1, 0,-1, 0, 0, 0, 0, 1, 8, 4, 4 }, /* 4075 */
  {460,  1,-1, 0, 1, 0, 0, 0, 0,-1, 8, 4, 4 }, /* 4076 */
  {460,  0, 1, 0, 1, 0, 0, 0, 0,-1, 8, 4,10 }, /* 4077 */
  {460,  0,-1, 0,-1, 0, 0, 0, 0, 1, 8, 4,10 }, /* 4078 */
  {460,  1,-1, 0, 0,-1, 0, 0, 0,-1, 8, 4,10 }, /* 4079 */
  {460, -1, 1, 0, 0, 1, 0, 0, 0, 1, 8, 4,10 }, /* 4080 */
  {460, -1, 0, 0,-1, 1, 0, 0, 0,-1, 8, 4,10 }, /* 4081 */
  {460,  1, 0, 0, 1,-1, 0, 0, 0, 1, 8, 4,10 }, /* 4082 */
  {460,  1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 8, 8 }, /* 4083 */
  {460, -1, 0, 0, 0,-1, 0, 0, 0,-1, 4, 8, 8 }, /* 4084 */
  {460,  0,-1, 0, 1,-1, 0, 0, 0, 1, 4, 8, 8 }, /* 4085 */
  {460,  0, 1, 0,-1, 1, 0, 0, 0,-1, 4, 8, 8 }, /* 4086 */
  {460, -1, 1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 8 }, /* 4087 */
  {460,  1,-1, 0, 1, 0, 0, 0, 0,-1, 4, 8, 8 }, /* 4088 */
  {460,  0, 1, 0, 1, 0, 0, 0, 0,-1, 4, 8, 2 }, /* 4089 */
  {460,  0,-1, 0,-1, 0, 0, 0, 0, 1, 4, 8, 2 }, /* 4090 */
  {460,  1,-1, 0, 0,-1, 0, 0, 0,-1, 4, 8, 2 }, /* 4091 */
  {460, -1, 1, 0, 0, 1, 0, 0, 0, 1, 4, 8, 2 }, /* 4092 */
  {460, -1, 0, 0,-1, 1, 0, 0, 0,-1, 4, 8, 2 }, /* 4093 */
  {460,  1, 0, 0, 1,-1, 0, 0, 0, 1, 4, 8, 2 }, /* 4094 */
  {461,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4095 */
  {461, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4096 */
  {461,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4097 */
  {461,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4098 */
  {461,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4099 */
  {461,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4100 */
  {461,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 4101 */
  {461,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 4102 */
  {461, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 4103 */
  {461,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 4104 */
  {461,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 4105 */
  {461,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 4106 */
  {462,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4107 */
  {462,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4108 */
  {462,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4109 */
  {462, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4110 */
  {462, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4111 */
  {462,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4112 */
  {463,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4113 */
  {463,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2 }, /* 4114 */
  {463,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 4 }, /* 4115 */
  {463, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4116 */
  {463, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 4117 */
  {463,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0,10 }, /* 4118 */
  {464,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4119 */
  {464,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0,10 }, /* 4120 */
  {464,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 8 }, /* 4121 */
  {464, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4122 */
  {464, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 4123 */
  {464,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 2 }, /* 4124 */
  {465,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4125 */
  {465,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 4126 */
  {465,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 8 }, /* 4127 */
  {465, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4128 */
  {465, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 4129 */
  {465,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 8 }, /* 4130 */
  {466,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4131 */
  {466,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 4132 */
  {466,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 4 }, /* 4133 */
  {466, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4134 */
  {466, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 4135 */
  {466,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 4 }, /* 4136 */
  {467,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4137 */
  {467,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4138 */
  {467,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4139 */
  {467, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4140 */
  {467, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4141 */
  {467,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4142 */
  {468,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4143 */
  {468, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4144 */
  {468,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4145 */
  {468,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4146 */
  {468, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4147 */
  {468,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4148 */
  {469,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4149 */
  {469, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4150 */
  {469,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4151 */
  {469, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4152 */
  {469,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4153 */
  {469,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4154 */
  {469, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4155 */
  {469,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4156 */
  {469, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4157 */
  {469,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4158 */
  {469,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4159 */
  {469,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4160 */
  {470,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4161 */
  {470, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4162 */
  {470,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4163 */
  {470, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4164 */
  {470,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4165 */
  {470,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4166 */
  {470, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4167 */
  {470,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4168 */
  {470, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4169 */
  {470,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4170 */
  {470,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4171 */
  {470,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4172 */
  {471,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4173 */
  {471,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4174 */
  {471,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4175 */
  {471, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4176 */
  {471, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4177 */
  {471,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4178 */
  {471,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4179 */
  {471, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4180 */
  {471, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4181 */
  {471,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4182 */
  {471,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4183 */
  {471,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4184 */
  {472,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4185 */
  {472,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2 }, /* 4186 */
  {472,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 4 }, /* 4187 */
  {472, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4188 */
  {472, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 4189 */
  {472,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0,10 }, /* 4190 */
  {472,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0,10 }, /* 4191 */
  {472, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 8 }, /* 4192 */
  {472, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4193 */
  {472,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 4 }, /* 4194 */
  {472,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 2 }, /* 4195 */
  {472,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4196 */
  {473,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4197 */
  {473,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0,10 }, /* 4198 */
  {473,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 8 }, /* 4199 */
  {473, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4200 */
  {473, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 4201 */
  {473,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 2 }, /* 4202 */
  {473,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 2 }, /* 4203 */
  {473, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 4 }, /* 4204 */
  {473, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4205 */
  {473,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 8 }, /* 4206 */
  {473,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0,10 }, /* 4207 */
  {473,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4208 */
  {474,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4209 */
  {474,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 4210 */
  {474,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 8 }, /* 4211 */
  {474, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4212 */
  {474, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 4 }, /* 4213 */
  {474,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 8 }, /* 4214 */
  {474,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 8 }, /* 4215 */
  {474, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 4 }, /* 4216 */
  {474, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4217 */
  {474,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 8 }, /* 4218 */
  {474,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 4 }, /* 4219 */
  {474,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4220 */
  {475,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4221 */
  {475,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 4222 */
  {475,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 4 }, /* 4223 */
  {475, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4224 */
  {475, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 8 }, /* 4225 */
  {475,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 4 }, /* 4226 */
  {475,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 4 }, /* 4227 */
  {475, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 8 }, /* 4228 */
  {475, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4229 */
  {475,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 4 }, /* 4230 */
  {475,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 8 }, /* 4231 */
  {475,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4232 */
  {476,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4233 */
  {476,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4234 */
  {476,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4235 */
  {476, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4236 */
  {476, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4237 */
  {476,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4238 */
  {476,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4239 */
  {476, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4240 */
  {476, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4241 */
  {476,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4242 */
  {476,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4243 */
  {476,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4244 */
  {477,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4245 */
  {477,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4246 */
  {477,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4247 */
  {477, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4248 */
  {477, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4249 */
  {477,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4250 */
  {477,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4251 */
  {477,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4252 */
  {477,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4253 */
  {477,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4254 */
  {477, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4255 */
  {477, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4256 */
  {478,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4257 */
  {478,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4258 */
  {478,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4259 */
  {478, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4260 */
  {478, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4261 */
  {478,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4262 */
  {478,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4263 */
  {478,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4264 */
  {478,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4265 */
  {478,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4266 */
  {478, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4267 */
  {478, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4268 */
  {479,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4269 */
  {479,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4270 */
  {479,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4271 */
  {479, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4272 */
  {479, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4273 */
  {479,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4274 */
  {479,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4275 */
  {479,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4276 */
  {479,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4277 */
  {479,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4278 */
  {479, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4279 */
  {479, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4280 */
  {480,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4281 */
  {480,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4282 */
  {480,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4283 */
  {480, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4284 */
  {480, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4285 */
  {480,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4286 */
  {480,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4287 */
  {480,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4288 */
  {480,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4289 */
  {480,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4290 */
  {480, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4291 */
  {480, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4292 */
  {481,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4293 */
  {481, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4294 */
  {481,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4295 */
  {481,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4296 */
  {481, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4297 */
  {481,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4298 */
  {481,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4299 */
  {481,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4300 */
  {481, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4301 */
  {481,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4302 */
  {481,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4303 */
  {481, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4304 */
  {482,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4305 */
  {482, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4306 */
  {482,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4307 */
  {482,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4308 */
  {482, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4309 */
  {482,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4310 */
  {482,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4311 */
  {482,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4312 */
  {482, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4313 */
  {482,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4314 */
  {482,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4315 */
  {482, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4316 */
  {483,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4317 */
  {483, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4318 */
  {483,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4319 */
  {483,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4320 */
  {483, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4321 */
  {483,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4322 */
  {483,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4323 */
  {483, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4324 */
  {483,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4325 */
  {483,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4326 */
  {483, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4327 */
  {483,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4328 */
  {484,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4329 */
  {484, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4330 */
  {484,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4331 */
  {484,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4332 */
  {484, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4333 */
  {484,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4334 */
  {484,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4335 */
  {484, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4336 */
  {484,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4337 */
  {484,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4338 */
  {484, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4339 */
  {484,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4340 */
  {485,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4341 */
  {485, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4342 */
  {485,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4343 */
  {485, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4344 */
  {485,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4345 */
  {485,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4346 */
  {485, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4347 */
  {485,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4348 */
  {485, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4349 */
  {485,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4350 */
  {485,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4351 */
  {485,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4352 */
  {485,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4353 */
  {485,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4354 */
  {485, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4355 */
  {485,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4356 */
  {485, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4357 */
  {485,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4358 */
  {485,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4359 */
  {485,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4360 */
  {485,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4361 */
  {485, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4362 */
  {485,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4363 */
  {485, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4364 */
  {486,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4365 */
  {486, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4366 */
  {486,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4367 */
  {486, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4368 */
  {486,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4369 */
  {486,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4370 */
  {486, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4371 */
  {486,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4372 */
  {486, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4373 */
  {486,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4374 */
  {486,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4375 */
  {486,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4376 */
  {486,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4377 */
  {486,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4378 */
  {486, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4379 */
  {486,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4380 */
  {486, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4381 */
  {486,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4382 */
  {486,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4383 */
  {486,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4384 */
  {486,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4385 */
  {486, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4386 */
  {486,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4387 */
  {486, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4388 */
  {487,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4389 */
  {487, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4390 */
  {487,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4391 */
  {487, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4392 */
  {487,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4393 */
  {487,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4394 */
  {487, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4395 */
  {487,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4396 */
  {487, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4397 */
  {487,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4398 */
  {487,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4399 */
  {487,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4400 */
  {487,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4401 */
  {487,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4402 */
  {487, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4403 */
  {487,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4404 */
  {487, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4405 */
  {487,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4406 */
  {487,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4407 */
  {487,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4408 */
  {487,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4409 */
  {487, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4410 */
  {487,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4411 */
  {487, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4412 */
  {488,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4413 */
  {488, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4414 */
  {488,  1,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4415 */
  {488, -1, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4416 */
  {488,  0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4417 */
  {488,  0, 1, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4418 */
  {488, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4419 */
  {488,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4420 */
  {488, -1, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4421 */
  {488,  1,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4422 */
  {488,  0, 1, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4423 */
  {488,  0,-1, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4424 */
  {488,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 4425 */
  {488,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 4426 */
  {488, -1, 0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4427 */
  {488,  1, 0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4428 */
  {488, -1, 1, 0, 0, 1, 0, 0, 0,-1, 0, 0, 6 }, /* 4429 */
  {488,  1,-1, 0, 0,-1, 0, 0, 0, 1, 0, 0, 6 }, /* 4430 */
  {488,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 4431 */
  {488,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 4432 */
  {488,  1, 0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4433 */
  {488, -1, 0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4434 */
  {488,  1,-1, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4435 */
  {488, -1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4436 */
  {489,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4437 */
  {489, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4438 */
  {489,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4439 */
  {489, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4440 */
  {489,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4441 */
  {489,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4442 */
  {489,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4443 */
  {489,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4444 */
  {489,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4445 */
  {489,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4446 */
  {489,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4447 */
  {489,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4448 */
  {490,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4449 */
  {490, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4450 */
  {490,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4451 */
  {490, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4452 */
  {490,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4453 */
  {490,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4454 */
  {490,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4455 */
  {490,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4456 */
  {490,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4457 */
  {490,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4458 */
  {490,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4459 */
  {490,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4460 */
  {490,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 4461 */
  {490, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 4462 */
  {490,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 4463 */
  {490, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 4464 */
  {490,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4465 */
  {490,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4466 */
  {490,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4467 */
  {490,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4468 */
  {490,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 4469 */
  {490,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 4470 */
  {490,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 4471 */
  {490,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 4472 */
  {490,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 4473 */
  {490, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 4474 */
  {490,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 4475 */
  {490, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 4476 */
  {490,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4477 */
  {490,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4478 */
  {490,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4479 */
  {490,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4480 */
  {490,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 4481 */
  {490,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 4482 */
  {490,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 4483 */
  {490,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 4484 */
  {490,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 4485 */
  {490, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 4486 */
  {490,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 4487 */
  {490, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 4488 */
  {490,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4489 */
  {490,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4490 */
  {490,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4491 */
  {490,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4492 */
  {490,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 4493 */
  {490,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 4494 */
  {490,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 4495 */
  {490,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 4496 */
  {491,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4497 */
  {491, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4498 */
  {491,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4499 */
  {491, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4500 */
  {491,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4501 */
  {491,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4502 */
  {491,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4503 */
  {491,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4504 */
  {491,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4505 */
  {491,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4506 */
  {491,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4507 */
  {491,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4508 */
  {491,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 4509 */
  {491, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 4510 */
  {491,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 4511 */
  {491, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 4512 */
  {491,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4513 */
  {491,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 4514 */
  {491,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 4515 */
  {491,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4516 */
  {491,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 4517 */
  {491,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 4518 */
  {491,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 4519 */
  {491,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 4520 */
  {492,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4521 */
  {492, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 4522 */
  {492,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 4523 */
  {492, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 4524 */
  {492,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4525 */
  {492,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4526 */
  {492,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4527 */
  {492,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4528 */
  {492,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4529 */
  {492,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 4530 */
  {492,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 4531 */
  {492,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 4532 */
  {493,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4533 */
  {493, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 4534 */
  {493,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4535 */
  {493, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 4536 */
  {493,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4537 */
  {493,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 6 }, /* 4538 */
  {493,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 0 }, /* 4539 */
  {493,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4540 */
  {493,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4541 */
  {493,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 0 }, /* 4542 */
  {493,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 0 }, /* 4543 */
  {493,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 4544 */
  {493,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 4545 */
  {493, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 4546 */
  {493,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 4547 */
  {493, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 4548 */
  {493,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4549 */
  {493,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4550 */
  {493,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4551 */
  {493,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 0 }, /* 4552 */
  {493,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 4553 */
  {493,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 4554 */
  {493,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 4555 */
  {493,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 6 }, /* 4556 */
  {494,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4557 */
  {494, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4558 */
  {494, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4559 */
  {494,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4560 */
  {494,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4561 */
  {494, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4562 */
  {494, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4563 */
  {494,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4564 */
  {494,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4565 */
  {494,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4566 */
  {494,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4567 */
  {494,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4568 */
  {494,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4569 */
  {494,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4570 */
  {494,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4571 */
  {494,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4572 */
  {494,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4573 */
  {494,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4574 */
  {494,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4575 */
  {494,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4576 */
  {494,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4577 */
  {494,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4578 */
  {494,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4579 */
  {494,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4580 */
  {495,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4581 */
  {495, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4582 */
  {495,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4583 */
  {495, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4584 */
  {495,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4585 */
  {495,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4586 */
  {495,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4587 */
  {495,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4588 */
  {495,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4589 */
  {495,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4590 */
  {495,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4591 */
  {495,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4592 */
  {495, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 4593 */
  {495,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 4594 */
  {495, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 4595 */
  {495,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 4596 */
  {495,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 4597 */
  {495,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4598 */
  {495,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4599 */
  {495,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 4600 */
  {495,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 4601 */
  {495,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 4602 */
  {495,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 4603 */
  {495,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 4604 */
  {496,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4605 */
  {496, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4606 */
  {496, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 4607 */
  {496,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 4608 */
  {496,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 4609 */
  {496, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 4610 */
  {496, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 4611 */
  {496,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 4612 */
  {496,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4613 */
  {496,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4614 */
  {496,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4615 */
  {496,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4616 */
  {496,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4617 */
  {496,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4618 */
  {496,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4619 */
  {496,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4620 */
  {496,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4621 */
  {496,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4622 */
  {496,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 4623 */
  {496,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 4624 */
  {496,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 4625 */
  {496,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 4626 */
  {496,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 4627 */
  {496,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 4628 */
  {497,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4629 */
  {497, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4630 */
  {497, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4631 */
  {497,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4632 */
  {497,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4633 */
  {497, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4634 */
  {497, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4635 */
  {497,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4636 */
  {497,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4637 */
  {497,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4638 */
  {497,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4639 */
  {497,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4640 */
  {497,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4641 */
  {497,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4642 */
  {497,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4643 */
  {497,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4644 */
  {497,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4645 */
  {497,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4646 */
  {497,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4647 */
  {497,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4648 */
  {497,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4649 */
  {497,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4650 */
  {497,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4651 */
  {497,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4652 */
  {497,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 4653 */
  {497, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 4654 */
  {497, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 4655 */
  {497,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 4656 */
  {497,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 4657 */
  {497, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 4658 */
  {497, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 4659 */
  {497,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 4660 */
  {497,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4661 */
  {497,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4662 */
  {497,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4663 */
  {497,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4664 */
  {497,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4665 */
  {497,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4666 */
  {497,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4667 */
  {497,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4668 */
  {497,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 4669 */
  {497,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 4670 */
  {497,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 4671 */
  {497,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 4672 */
  {497,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 4673 */
  {497,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 4674 */
  {497,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 4675 */
  {497,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 4676 */
  {497,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 4677 */
  {497, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 4678 */
  {497, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 4679 */
  {497,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 4680 */
  {497,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 4681 */
  {497, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 4682 */
  {497, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 4683 */
  {497,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 4684 */
  {497,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4685 */
  {497,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4686 */
  {497,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4687 */
  {497,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4688 */
  {497,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4689 */
  {497,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4690 */
  {497,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4691 */
  {497,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4692 */
  {497,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 4693 */
  {497,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 4694 */
  {497,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 4695 */
  {497,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 4696 */
  {497,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 4697 */
  {497,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 4698 */
  {497,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 4699 */
  {497,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 4700 */
  {497,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 4701 */
  {497, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 4702 */
  {497, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 4703 */
  {497,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 4704 */
  {497,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 4705 */
  {497, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 4706 */
  {497, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 4707 */
  {497,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 4708 */
  {497,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4709 */
  {497,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4710 */
  {497,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4711 */
  {497,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4712 */
  {497,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4713 */
  {497,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4714 */
  {497,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4715 */
  {497,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4716 */
  {497,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 4717 */
  {497,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 4718 */
  {497,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 4719 */
  {497,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 4720 */
  {497,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 4721 */
  {497,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 4722 */
  {497,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 4723 */
  {497,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 4724 */
  {498,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4725 */
  {498, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4726 */
  {498,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4727 */
  {498, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4728 */
  {498,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4729 */
  {498,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4730 */
  {498,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4731 */
  {498,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4732 */
  {498,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4733 */
  {498,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4734 */
  {498,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4735 */
  {498,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4736 */
  {498, -1, 0, 0, 0,-1, 0, 0, 0,-1, 3, 3, 3 }, /* 4737 */
  {498,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 3 }, /* 4738 */
  {498, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 3 }, /* 4739 */
  {498,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 3 }, /* 4740 */
  {498,  0, 0,-1,-1, 0, 0, 0,-1, 0, 3, 3, 3 }, /* 4741 */
  {498,  0, 0,-1, 1, 0, 0, 0, 1, 0, 3, 3, 3 }, /* 4742 */
  {498,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 3, 3 }, /* 4743 */
  {498,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 3, 3 }, /* 4744 */
  {498,  0,-1, 0, 0, 0,-1,-1, 0, 0, 3, 3, 3 }, /* 4745 */
  {498,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 3, 3 }, /* 4746 */
  {498,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 3, 3 }, /* 4747 */
  {498,  0,-1, 0, 0, 0, 1, 1, 0, 0, 3, 3, 3 }, /* 4748 */
  {498,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 4749 */
  {498, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 4750 */
  {498,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 4751 */
  {498, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 4752 */
  {498,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4753 */
  {498,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4754 */
  {498,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4755 */
  {498,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4756 */
  {498,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 4757 */
  {498,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 4758 */
  {498,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 4759 */
  {498,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 4760 */
  {498, -1, 0, 0, 0,-1, 0, 0, 0,-1, 3, 9, 9 }, /* 4761 */
  {498,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 9 }, /* 4762 */
  {498, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 9, 9 }, /* 4763 */
  {498,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 9 }, /* 4764 */
  {498,  0, 0,-1,-1, 0, 0, 0,-1, 0, 3, 9, 9 }, /* 4765 */
  {498,  0, 0,-1, 1, 0, 0, 0, 1, 0, 3, 9, 9 }, /* 4766 */
  {498,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 9, 9 }, /* 4767 */
  {498,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 9, 9 }, /* 4768 */
  {498,  0,-1, 0, 0, 0,-1,-1, 0, 0, 3, 9, 9 }, /* 4769 */
  {498,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 9, 9 }, /* 4770 */
  {498,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 9, 9 }, /* 4771 */
  {498,  0,-1, 0, 0, 0, 1, 1, 0, 0, 3, 9, 9 }, /* 4772 */
  {498,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 4773 */
  {498, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 4774 */
  {498,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 4775 */
  {498, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 4776 */
  {498,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4777 */
  {498,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4778 */
  {498,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4779 */
  {498,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4780 */
  {498,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 4781 */
  {498,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 4782 */
  {498,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 4783 */
  {498,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 4784 */
  {498, -1, 0, 0, 0,-1, 0, 0, 0,-1, 9, 3, 9 }, /* 4785 */
  {498,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 9 }, /* 4786 */
  {498, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 3, 9 }, /* 4787 */
  {498,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 9 }, /* 4788 */
  {498,  0, 0,-1,-1, 0, 0, 0,-1, 0, 9, 3, 9 }, /* 4789 */
  {498,  0, 0,-1, 1, 0, 0, 0, 1, 0, 9, 3, 9 }, /* 4790 */
  {498,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 3, 9 }, /* 4791 */
  {498,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 3, 9 }, /* 4792 */
  {498,  0,-1, 0, 0, 0,-1,-1, 0, 0, 9, 3, 9 }, /* 4793 */
  {498,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 3, 9 }, /* 4794 */
  {498,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 3, 9 }, /* 4795 */
  {498,  0,-1, 0, 0, 0, 1, 1, 0, 0, 9, 3, 9 }, /* 4796 */
  {498,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 4797 */
  {498, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 4798 */
  {498,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 4799 */
  {498, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 4800 */
  {498,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4801 */
  {498,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4802 */
  {498,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4803 */
  {498,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4804 */
  {498,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 4805 */
  {498,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 4806 */
  {498,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 4807 */
  {498,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 4808 */
  {498, -1, 0, 0, 0,-1, 0, 0, 0,-1, 9, 9, 3 }, /* 4809 */
  {498,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 3 }, /* 4810 */
  {498, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 9, 3 }, /* 4811 */
  {498,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 3 }, /* 4812 */
  {498,  0, 0,-1,-1, 0, 0, 0,-1, 0, 9, 9, 3 }, /* 4813 */
  {498,  0, 0,-1, 1, 0, 0, 0, 1, 0, 9, 9, 3 }, /* 4814 */
  {498,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 9, 3 }, /* 4815 */
  {498,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 9, 3 }, /* 4816 */
  {498,  0,-1, 0, 0, 0,-1,-1, 0, 0, 9, 9, 3 }, /* 4817 */
  {498,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 9, 3 }, /* 4818 */
  {498,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 9, 3 }, /* 4819 */
  {498,  0,-1, 0, 0, 0, 1, 1, 0, 0, 9, 9, 3 }, /* 4820 */
  {499,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4821 */
  {499, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4822 */
  {499, -1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 0 }, /* 4823 */
  {499,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 0 }, /* 4824 */
  {499,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 3, 3 }, /* 4825 */
  {499, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 3, 3 }, /* 4826 */
  {499, -1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 0, 3 }, /* 4827 */
  {499,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 0, 3 }, /* 4828 */
  {499,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4829 */
  {499,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4830 */
  {499,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 3, 3 }, /* 4831 */
  {499,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 3, 3 }, /* 4832 */
  {499,  0, 0,-1, 1, 0, 0, 0,-1, 0, 3, 0, 3 }, /* 4833 */
  {499,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 0, 3 }, /* 4834 */
  {499,  0, 0,-1,-1, 0, 0, 0, 1, 0, 3, 3, 0 }, /* 4835 */
  {499,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 3, 0 }, /* 4836 */
  {499,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4837 */
  {499,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4838 */
  {499,  0,-1, 0, 0, 0, 1,-1, 0, 0, 3, 0, 3 }, /* 4839 */
  {499,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 0, 3 }, /* 4840 */
  {499,  0,-1, 0, 0, 0,-1, 1, 0, 0, 3, 3, 0 }, /* 4841 */
  {499,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 3, 0 }, /* 4842 */
  {499,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 3, 3 }, /* 4843 */
  {499,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 3, 3 }, /* 4844 */
  {499,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 4845 */
  {499, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 4846 */
  {499, -1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 6 }, /* 4847 */
  {499,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 6 }, /* 4848 */
  {499,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 9, 9 }, /* 4849 */
  {499, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 9, 9 }, /* 4850 */
  {499, -1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 6, 9 }, /* 4851 */
  {499,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 6, 9 }, /* 4852 */
  {499,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4853 */
  {499,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4854 */
  {499,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 9, 9 }, /* 4855 */
  {499,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 9, 9 }, /* 4856 */
  {499,  0, 0,-1, 1, 0, 0, 0,-1, 0, 3, 6, 9 }, /* 4857 */
  {499,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 6, 9 }, /* 4858 */
  {499,  0, 0,-1,-1, 0, 0, 0, 1, 0, 3, 9, 6 }, /* 4859 */
  {499,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 9, 6 }, /* 4860 */
  {499,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 4861 */
  {499,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 4862 */
  {499,  0,-1, 0, 0, 0, 1,-1, 0, 0, 3, 6, 9 }, /* 4863 */
  {499,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 6, 9 }, /* 4864 */
  {499,  0,-1, 0, 0, 0,-1, 1, 0, 0, 3, 9, 6 }, /* 4865 */
  {499,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 9, 6 }, /* 4866 */
  {499,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 9, 9 }, /* 4867 */
  {499,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 9, 9 }, /* 4868 */
  {499,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 4869 */
  {499, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 4870 */
  {499, -1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 6 }, /* 4871 */
  {499,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 6 }, /* 4872 */
  {499,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 3, 9 }, /* 4873 */
  {499, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 3, 9 }, /* 4874 */
  {499, -1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 0, 9 }, /* 4875 */
  {499,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 0, 9 }, /* 4876 */
  {499,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4877 */
  {499,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4878 */
  {499,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 3, 9 }, /* 4879 */
  {499,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 3, 9 }, /* 4880 */
  {499,  0, 0,-1, 1, 0, 0, 0,-1, 0, 9, 0, 9 }, /* 4881 */
  {499,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 0, 9 }, /* 4882 */
  {499,  0, 0,-1,-1, 0, 0, 0, 1, 0, 9, 3, 6 }, /* 4883 */
  {499,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 3, 6 }, /* 4884 */
  {499,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 4885 */
  {499,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 4886 */
  {499,  0,-1, 0, 0, 0, 1,-1, 0, 0, 9, 0, 9 }, /* 4887 */
  {499,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 0, 9 }, /* 4888 */
  {499,  0,-1, 0, 0, 0,-1, 1, 0, 0, 9, 3, 6 }, /* 4889 */
  {499,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 3, 6 }, /* 4890 */
  {499,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 3, 9 }, /* 4891 */
  {499,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 3, 9 }, /* 4892 */
  {499,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 4893 */
  {499, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 4894 */
  {499, -1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 0 }, /* 4895 */
  {499,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 0 }, /* 4896 */
  {499,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 9, 3 }, /* 4897 */
  {499, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 9, 3 }, /* 4898 */
  {499, -1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 6, 3 }, /* 4899 */
  {499,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 6, 3 }, /* 4900 */
  {499,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4901 */
  {499,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4902 */
  {499,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 9, 3 }, /* 4903 */
  {499,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 9, 3 }, /* 4904 */
  {499,  0, 0,-1, 1, 0, 0, 0,-1, 0, 9, 6, 3 }, /* 4905 */
  {499,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 6, 3 }, /* 4906 */
  {499,  0, 0,-1,-1, 0, 0, 0, 1, 0, 9, 9, 0 }, /* 4907 */
  {499,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 9, 0 }, /* 4908 */
  {499,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 4909 */
  {499,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 4910 */
  {499,  0,-1, 0, 0, 0, 1,-1, 0, 0, 9, 6, 3 }, /* 4911 */
  {499,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 6, 3 }, /* 4912 */
  {499,  0,-1, 0, 0, 0,-1, 1, 0, 0, 9, 9, 0 }, /* 4913 */
  {499,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 9, 0 }, /* 4914 */
  {499,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 9, 3 }, /* 4915 */
  {499,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 9, 3 }, /* 4916 */
  {500,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4917 */
  {500, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4918 */
  {500, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4919 */
  {500,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4920 */
  {500,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4921 */
  {500, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4922 */
  {500, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 4923 */
  {500,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 4924 */
  {500,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4925 */
  {500,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4926 */
  {500,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4927 */
  {500,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4928 */
  {500,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4929 */
  {500,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4930 */
  {500,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4931 */
  {500,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4932 */
  {500,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4933 */
  {500,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4934 */
  {500,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4935 */
  {500,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4936 */
  {500,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 4937 */
  {500,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 4938 */
  {500,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4939 */
  {500,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4940 */
  {500,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 4941 */
  {500, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 4942 */
  {500, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 4943 */
  {500,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 4944 */
  {500,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 4945 */
  {500, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 4946 */
  {500, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 4947 */
  {500,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 4948 */
  {500,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4949 */
  {500,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 4950 */
  {500,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 4951 */
  {500,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4952 */
  {500,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 4953 */
  {500,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4954 */
  {500,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 4955 */
  {500,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 4956 */
  {500,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 4957 */
  {500,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 4958 */
  {500,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 4959 */
  {500,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 4960 */
  {500,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 4961 */
  {500,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 4962 */
  {500,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 4963 */
  {500,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 4964 */
  {501,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4965 */
  {501, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4966 */
  {501, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 4967 */
  {501,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 4968 */
  {501,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 4969 */
  {501, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 4970 */
  {501, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 4971 */
  {501,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 4972 */
  {501,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4973 */
  {501,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4974 */
  {501,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 4975 */
  {501,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 4976 */
  {501,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 4977 */
  {501,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 4978 */
  {501,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 4979 */
  {501,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 4980 */
  {501,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 4981 */
  {501,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 4982 */
  {501,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 4983 */
  {501,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 4984 */
  {501,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 4985 */
  {501,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 4986 */
  {501,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 4987 */
  {501,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 4988 */
  {502,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 4989 */
  {502, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 4990 */
  {502, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 4991 */
  {502,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 4992 */
  {502,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 4993 */
  {502, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 4994 */
  {502, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 4995 */
  {502,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 4996 */
  {502,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 4997 */
  {502,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 4998 */
  {502,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 6 }, /* 4999 */
  {502,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 6 }, /* 5000 */
  {502,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 0 }, /* 5001 */
  {502,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 0, 0 }, /* 5002 */
  {502,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5003 */
  {502,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5004 */
  {502,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5005 */
  {502,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5006 */
  {502,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 0 }, /* 5007 */
  {502,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 0 }, /* 5008 */
  {502,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 0 }, /* 5009 */
  {502,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 0 }, /* 5010 */
  {502,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5011 */
  {502,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 5012 */
  {502,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 5013 */
  {502, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 5014 */
  {502, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5015 */
  {502,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 5016 */
  {502,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5017 */
  {502, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 5018 */
  {502, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 5019 */
  {502,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 5020 */
  {502,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5021 */
  {502,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5022 */
  {502,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5023 */
  {502,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5024 */
  {502,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5025 */
  {502,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5026 */
  {502,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 0 }, /* 5027 */
  {502,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 6, 0 }, /* 5028 */
  {502,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 5029 */
  {502,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 5030 */
  {502,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5031 */
  {502,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 5032 */
  {502,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5033 */
  {502,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 5034 */
  {502,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 6 }, /* 5035 */
  {502,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 6 }, /* 5036 */
  {503,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5037 */
  {503,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5038 */
  {503, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5039 */
  {503,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5040 */
  {503,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5041 */
  {503,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5042 */
  {503, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5043 */
  {503,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5044 */
  {503,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5045 */
  {503,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5046 */
  {503,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5047 */
  {503,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5048 */
  {503,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5049 */
  {503,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5050 */
  {503,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5051 */
  {503,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5052 */
  {503,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5053 */
  {503,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5054 */
  {503,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5055 */
  {503, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5056 */
  {503,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5057 */
  {503, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5058 */
  {503,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5059 */
  {503,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5060 */
  {504,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5061 */
  {504,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5062 */
  {504, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5063 */
  {504,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5064 */
  {504,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5065 */
  {504,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5066 */
  {504, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5067 */
  {504,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5068 */
  {504,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5069 */
  {504,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5070 */
  {504,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5071 */
  {504,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5072 */
  {504,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5073 */
  {504,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5074 */
  {504,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5075 */
  {504,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5076 */
  {504,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5077 */
  {504,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5078 */
  {504,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5079 */
  {504, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5080 */
  {504,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5081 */
  {504, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5082 */
  {504,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5083 */
  {504,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5084 */
  {505,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5085 */
  {505,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5086 */
  {505, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5087 */
  {505,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5088 */
  {505,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5089 */
  {505,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5090 */
  {505, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5091 */
  {505,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5092 */
  {505,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5093 */
  {505,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5094 */
  {505,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5095 */
  {505,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5096 */
  {505,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5097 */
  {505,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5098 */
  {505,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5099 */
  {505,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5100 */
  {505,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5101 */
  {505,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5102 */
  {505,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5103 */
  {505, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5104 */
  {505,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5105 */
  {505, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5106 */
  {505,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5107 */
  {505,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5108 */
  {505,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 5109 */
  {505,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 5110 */
  {505, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 5111 */
  {505,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 5112 */
  {505,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 5113 */
  {505,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 5114 */
  {505, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 5115 */
  {505,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 5116 */
  {505,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5117 */
  {505,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 6, 6 }, /* 5118 */
  {505,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5119 */
  {505,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 6, 6 }, /* 5120 */
  {505,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5121 */
  {505,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 6, 6 }, /* 5122 */
  {505,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5123 */
  {505,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 6, 6 }, /* 5124 */
  {505,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 5125 */
  {505,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 6 }, /* 5126 */
  {505,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5127 */
  {505, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 6 }, /* 5128 */
  {505,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 5129 */
  {505, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 6 }, /* 5130 */
  {505,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 5131 */
  {505,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 6 }, /* 5132 */
  {505,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 5133 */
  {505,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 5134 */
  {505, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5135 */
  {505,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 5136 */
  {505,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 5137 */
  {505,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 5138 */
  {505, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 5139 */
  {505,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 5140 */
  {505,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5141 */
  {505,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 0, 6 }, /* 5142 */
  {505,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5143 */
  {505,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 0, 6 }, /* 5144 */
  {505,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5145 */
  {505,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 0, 6 }, /* 5146 */
  {505,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5147 */
  {505,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 0, 6 }, /* 5148 */
  {505,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 5149 */
  {505,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 6 }, /* 5150 */
  {505,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 5151 */
  {505, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 6 }, /* 5152 */
  {505,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5153 */
  {505, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 6 }, /* 5154 */
  {505,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 5155 */
  {505,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 6 }, /* 5156 */
  {505,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 5157 */
  {505,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 5158 */
  {505, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 5159 */
  {505,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 5160 */
  {505,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5161 */
  {505,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 5162 */
  {505, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 5163 */
  {505,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 5164 */
  {505,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5165 */
  {505,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 6, 0 }, /* 5166 */
  {505,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5167 */
  {505,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 0 }, /* 5168 */
  {505,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5169 */
  {505,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 0 }, /* 5170 */
  {505,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5171 */
  {505,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 6, 0 }, /* 5172 */
  {505,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 5173 */
  {505,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 0 }, /* 5174 */
  {505,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 5175 */
  {505, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 0 }, /* 5176 */
  {505,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 5177 */
  {505, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 0 }, /* 5178 */
  {505,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5179 */
  {505,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 0 }, /* 5180 */
  {506,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5181 */
  {506,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 5182 */
  {506, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 5183 */
  {506,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 5184 */
  {506,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5185 */
  {506,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 5186 */
  {506, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 5187 */
  {506,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 5188 */
  {506,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5189 */
  {506,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 3, 3 }, /* 5190 */
  {506,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5191 */
  {506,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 9, 3 }, /* 5192 */
  {506,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5193 */
  {506,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 3, 9 }, /* 5194 */
  {506,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5195 */
  {506,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 9, 9 }, /* 5196 */
  {506,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5197 */
  {506,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 3, 3 }, /* 5198 */
  {506,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 5199 */
  {506, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 9, 9 }, /* 5200 */
  {506,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5201 */
  {506, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 9, 3 }, /* 5202 */
  {506,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5203 */
  {506,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 3, 9 }, /* 5204 */
  {506,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 5205 */
  {506,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 5206 */
  {506, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5207 */
  {506,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 5208 */
  {506,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 5209 */
  {506,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 5210 */
  {506, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5211 */
  {506,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 5212 */
  {506,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5213 */
  {506,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 9, 9 }, /* 5214 */
  {506,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5215 */
  {506,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 3, 9 }, /* 5216 */
  {506,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5217 */
  {506,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 9, 3 }, /* 5218 */
  {506,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5219 */
  {506,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 3, 3 }, /* 5220 */
  {506,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 5221 */
  {506,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 9, 9 }, /* 5222 */
  {506,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 5223 */
  {506, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 3, 3 }, /* 5224 */
  {506,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 5225 */
  {506, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 3, 9 }, /* 5226 */
  {506,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 5227 */
  {506,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 9, 3 }, /* 5228 */
  {506,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 5229 */
  {506,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 5230 */
  {506, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 5231 */
  {506,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 5232 */
  {506,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 5233 */
  {506,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 5234 */
  {506, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 5235 */
  {506,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 5236 */
  {506,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5237 */
  {506,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 3, 9 }, /* 5238 */
  {506,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5239 */
  {506,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 9, 9 }, /* 5240 */
  {506,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5241 */
  {506,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 3, 3 }, /* 5242 */
  {506,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5243 */
  {506,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 9, 3 }, /* 5244 */
  {506,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 5245 */
  {506,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 3, 9 }, /* 5246 */
  {506,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5247 */
  {506, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 9, 3 }, /* 5248 */
  {506,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5249 */
  {506, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 9, 9 }, /* 5250 */
  {506,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 5251 */
  {506,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 3, 3 }, /* 5252 */
  {506,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 5253 */
  {506,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 5254 */
  {506, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5255 */
  {506,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 5256 */
  {506,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5257 */
  {506,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 5258 */
  {506, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 5259 */
  {506,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 5260 */
  {506,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5261 */
  {506,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 9, 3 }, /* 5262 */
  {506,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5263 */
  {506,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 3, 3 }, /* 5264 */
  {506,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5265 */
  {506,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 9, 9 }, /* 5266 */
  {506,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5267 */
  {506,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 3, 9 }, /* 5268 */
  {506,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 5269 */
  {506,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 9, 3 }, /* 5270 */
  {506,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5271 */
  {506, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 3, 9 }, /* 5272 */
  {506,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 5273 */
  {506, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 3, 3 }, /* 5274 */
  {506,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5275 */
  {506,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 9, 9 }, /* 5276 */
  {507,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5277 */
  {507,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5278 */
  {507, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5279 */
  {507,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5280 */
  {507,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5281 */
  {507,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5282 */
  {507, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5283 */
  {507,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5284 */
  {507,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5285 */
  {507,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5286 */
  {507,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5287 */
  {507,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5288 */
  {507,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5289 */
  {507,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5290 */
  {507,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5291 */
  {507,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5292 */
  {507,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5293 */
  {507,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5294 */
  {507,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5295 */
  {507, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5296 */
  {507,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5297 */
  {507, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5298 */
  {507,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5299 */
  {507,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5300 */
  {507,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 5301 */
  {507,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5302 */
  {507, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 5303 */
  {507,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5304 */
  {507,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 5305 */
  {507,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5306 */
  {507, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 5307 */
  {507,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5308 */
  {507,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5309 */
  {507,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5310 */
  {507,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5311 */
  {507,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5312 */
  {507,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5313 */
  {507,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5314 */
  {507,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5315 */
  {507,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5316 */
  {507,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 5317 */
  {507,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5318 */
  {507,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 5319 */
  {507, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5320 */
  {507,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 5321 */
  {507, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5322 */
  {507,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 5323 */
  {507,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5324 */
  {508,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5325 */
  {508,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 5326 */
  {508, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5327 */
  {508,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 5328 */
  {508,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5329 */
  {508,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 5330 */
  {508, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 5331 */
  {508,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 5332 */
  {508,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5333 */
  {508,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 9, 3 }, /* 5334 */
  {508,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5335 */
  {508,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 9, 9 }, /* 5336 */
  {508,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5337 */
  {508,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 3, 3 }, /* 5338 */
  {508,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5339 */
  {508,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 3, 9 }, /* 5340 */
  {508,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5341 */
  {508,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 9, 9 }, /* 5342 */
  {508,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5343 */
  {508, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 3, 9 }, /* 5344 */
  {508,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5345 */
  {508, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 3, 3 }, /* 5346 */
  {508,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5347 */
  {508,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 9, 3 }, /* 5348 */
  {509,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5349 */
  {509,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 3 }, /* 5350 */
  {509, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5351 */
  {509,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 9 }, /* 5352 */
  {509,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5353 */
  {509,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 9, 9 }, /* 5354 */
  {509, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 5355 */
  {509,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 3 }, /* 5356 */
  {509,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5357 */
  {509,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 3, 9 }, /* 5358 */
  {509,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5359 */
  {509,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 3, 3 }, /* 5360 */
  {509,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5361 */
  {509,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 9, 9 }, /* 5362 */
  {509,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5363 */
  {509,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 9, 3 }, /* 5364 */
  {509,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5365 */
  {509,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 3, 3 }, /* 5366 */
  {509,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5367 */
  {509, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 9, 3 }, /* 5368 */
  {509,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5369 */
  {509, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 9, 9 }, /* 5370 */
  {509,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5371 */
  {509,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 3, 9 }, /* 5372 */
  {510,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5373 */
  {510,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 3 }, /* 5374 */
  {510, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5375 */
  {510,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 9 }, /* 5376 */
  {510,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 5377 */
  {510,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 5378 */
  {510, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 5379 */
  {510,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 5380 */
  {510,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5381 */
  {510,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 3, 9 }, /* 5382 */
  {510,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5383 */
  {510,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 3, 3 }, /* 5384 */
  {510,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 0 }, /* 5385 */
  {510,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 3, 3 }, /* 5386 */
  {510,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 0 }, /* 5387 */
  {510,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 3, 9 }, /* 5388 */
  {510,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5389 */
  {510,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 3, 3 }, /* 5390 */
  {510,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5391 */
  {510, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 9, 3 }, /* 5392 */
  {510,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 0 }, /* 5393 */
  {510, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 3, 3 }, /* 5394 */
  {510,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 6 }, /* 5395 */
  {510,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 9, 3 }, /* 5396 */
  {510,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 5397 */
  {510,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 5398 */
  {510, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 5399 */
  {510,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 5400 */
  {510,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5401 */
  {510,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 9, 9 }, /* 5402 */
  {510, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 5403 */
  {510,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 3 }, /* 5404 */
  {510,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5405 */
  {510,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 9, 3 }, /* 5406 */
  {510,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 6 }, /* 5407 */
  {510,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 9, 9 }, /* 5408 */
  {510,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5409 */
  {510,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 9, 9 }, /* 5410 */
  {510,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5411 */
  {510,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 9, 3 }, /* 5412 */
  {510,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 5413 */
  {510,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 9, 9 }, /* 5414 */
  {510,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 0 }, /* 5415 */
  {510, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 3, 9 }, /* 5416 */
  {510,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5417 */
  {510, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 9, 9 }, /* 5418 */
  {510,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5419 */
  {510,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 3, 9 }, /* 5420 */
  {511,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5421 */
  {511,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5422 */
  {511, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5423 */
  {511,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5424 */
  {511,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5425 */
  {511,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5426 */
  {511, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5427 */
  {511,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5428 */
  {511,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5429 */
  {511,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5430 */
  {511,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5431 */
  {511,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5432 */
  {511,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5433 */
  {511,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5434 */
  {511,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5435 */
  {511,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5436 */
  {511,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5437 */
  {511, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5438 */
  {511,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5439 */
  {511,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5440 */
  {511,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5441 */
  {511,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5442 */
  {511,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5443 */
  {511, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5444 */
  {512,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5445 */
  {512,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5446 */
  {512, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5447 */
  {512,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5448 */
  {512,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5449 */
  {512,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5450 */
  {512, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5451 */
  {512,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5452 */
  {512,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5453 */
  {512,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5454 */
  {512,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5455 */
  {512,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5456 */
  {512,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5457 */
  {512,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5458 */
  {512,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5459 */
  {512,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5460 */
  {512,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5461 */
  {512, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5462 */
  {512,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5463 */
  {512,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5464 */
  {512,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5465 */
  {512,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5466 */
  {512,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5467 */
  {512, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5468 */
  {512,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 5469 */
  {512,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 5470 */
  {512, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 5471 */
  {512,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 5472 */
  {512,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 5473 */
  {512,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 5474 */
  {512, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 5475 */
  {512,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 5476 */
  {512,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5477 */
  {512,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 6, 6 }, /* 5478 */
  {512,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5479 */
  {512,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 6, 6 }, /* 5480 */
  {512,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5481 */
  {512,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 6, 6 }, /* 5482 */
  {512,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5483 */
  {512,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 6, 6 }, /* 5484 */
  {512,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 5485 */
  {512, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 6 }, /* 5486 */
  {512,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5487 */
  {512,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 6 }, /* 5488 */
  {512,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 5489 */
  {512,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 6 }, /* 5490 */
  {512,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 5491 */
  {512, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 6 }, /* 5492 */
  {512,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 5493 */
  {512,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 5494 */
  {512, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5495 */
  {512,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 5496 */
  {512,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 5497 */
  {512,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 5498 */
  {512, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 5499 */
  {512,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 5500 */
  {512,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5501 */
  {512,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 0, 6 }, /* 5502 */
  {512,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5503 */
  {512,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 0, 6 }, /* 5504 */
  {512,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5505 */
  {512,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 0, 6 }, /* 5506 */
  {512,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5507 */
  {512,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 0, 6 }, /* 5508 */
  {512,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 5509 */
  {512, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 6 }, /* 5510 */
  {512,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 5511 */
  {512,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 6 }, /* 5512 */
  {512,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5513 */
  {512,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 6 }, /* 5514 */
  {512,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 5515 */
  {512, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 6 }, /* 5516 */
  {512,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 5517 */
  {512,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 5518 */
  {512, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 5519 */
  {512,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 5520 */
  {512,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5521 */
  {512,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 5522 */
  {512, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 5523 */
  {512,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 5524 */
  {512,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5525 */
  {512,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 0 }, /* 5526 */
  {512,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5527 */
  {512,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 0 }, /* 5528 */
  {512,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5529 */
  {512,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 0 }, /* 5530 */
  {512,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5531 */
  {512,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 0 }, /* 5532 */
  {512,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 5533 */
  {512, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 0 }, /* 5534 */
  {512,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 5535 */
  {512,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 0 }, /* 5536 */
  {512,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 5537 */
  {512,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 0 }, /* 5538 */
  {512,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5539 */
  {512, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 0 }, /* 5540 */
  {513,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5541 */
  {513,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5542 */
  {513, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5543 */
  {513,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5544 */
  {513,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5545 */
  {513,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5546 */
  {513, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5547 */
  {513,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5548 */
  {513,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5549 */
  {513,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5550 */
  {513,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5551 */
  {513,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5552 */
  {513,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5553 */
  {513,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5554 */
  {513,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5555 */
  {513,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5556 */
  {513,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5557 */
  {513, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5558 */
  {513,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5559 */
  {513,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5560 */
  {513,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5561 */
  {513,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5562 */
  {513,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5563 */
  {513, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5564 */
  {513,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 5565 */
  {513,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5566 */
  {513, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 5567 */
  {513,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5568 */
  {513,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 5569 */
  {513,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5570 */
  {513, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 5571 */
  {513,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5572 */
  {513,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5573 */
  {513,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5574 */
  {513,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5575 */
  {513,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5576 */
  {513,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5577 */
  {513,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5578 */
  {513,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5579 */
  {513,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5580 */
  {513,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 5581 */
  {513, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5582 */
  {513,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 5583 */
  {513,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5584 */
  {513,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 5585 */
  {513,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5586 */
  {513,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 5587 */
  {513, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5588 */
  {514,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5589 */
  {514,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5590 */
  {514, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5591 */
  {514,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5592 */
  {514,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5593 */
  {514,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5594 */
  {514, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5595 */
  {514,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5596 */
  {514,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5597 */
  {514,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5598 */
  {514,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5599 */
  {514,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5600 */
  {514,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5601 */
  {514,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5602 */
  {514,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5603 */
  {514,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5604 */
  {514,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5605 */
  {514, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5606 */
  {514,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5607 */
  {514,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5608 */
  {514,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5609 */
  {514,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5610 */
  {514,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5611 */
  {514, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5612 */
  {515,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5613 */
  {515,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 5614 */
  {515, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5615 */
  {515,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 5616 */
  {515,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5617 */
  {515,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 5618 */
  {515, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5619 */
  {515,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 5620 */
  {515,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5621 */
  {515,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 0, 0 }, /* 5622 */
  {515,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5623 */
  {515,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 0, 0 }, /* 5624 */
  {515,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5625 */
  {515,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 0, 0 }, /* 5626 */
  {515,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5627 */
  {515,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 0, 0 }, /* 5628 */
  {515,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5629 */
  {515, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 0 }, /* 5630 */
  {515,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5631 */
  {515,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 0 }, /* 5632 */
  {515,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5633 */
  {515,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 0 }, /* 5634 */
  {515,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5635 */
  {515, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 0 }, /* 5636 */
  {515,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 5637 */
  {515,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 5638 */
  {515, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 5639 */
  {515,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 5640 */
  {515,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 5641 */
  {515,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 5642 */
  {515, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 5643 */
  {515,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 5644 */
  {515,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5645 */
  {515,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5646 */
  {515,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5647 */
  {515,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5648 */
  {515,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5649 */
  {515,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5650 */
  {515,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5651 */
  {515,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5652 */
  {515,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 5653 */
  {515, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 6 }, /* 5654 */
  {515,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5655 */
  {515,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 6 }, /* 5656 */
  {515,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 5657 */
  {515,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 6 }, /* 5658 */
  {515,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 5659 */
  {515, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 6 }, /* 5660 */
  {515,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 5661 */
  {515,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 5662 */
  {515, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5663 */
  {515,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 5664 */
  {515,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 5665 */
  {515,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 5666 */
  {515, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 5667 */
  {515,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 5668 */
  {515,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5669 */
  {515,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 6 }, /* 5670 */
  {515,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5671 */
  {515,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 6 }, /* 5672 */
  {515,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5673 */
  {515,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 6 }, /* 5674 */
  {515,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5675 */
  {515,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 6 }, /* 5676 */
  {515,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 5677 */
  {515, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5678 */
  {515,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 5679 */
  {515,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5680 */
  {515,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5681 */
  {515,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5682 */
  {515,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 5683 */
  {515, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5684 */
  {515,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 5685 */
  {515,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5686 */
  {515, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 5687 */
  {515,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5688 */
  {515,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5689 */
  {515,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5690 */
  {515, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 5691 */
  {515,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5692 */
  {515,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5693 */
  {515,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 6, 0 }, /* 5694 */
  {515,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5695 */
  {515,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 6, 0 }, /* 5696 */
  {515,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5697 */
  {515,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 6, 0 }, /* 5698 */
  {515,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5699 */
  {515,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 6, 0 }, /* 5700 */
  {515,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 5701 */
  {515, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 0 }, /* 5702 */
  {515,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 5703 */
  {515,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 0 }, /* 5704 */
  {515,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 5705 */
  {515,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 0 }, /* 5706 */
  {515,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5707 */
  {515, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 0 }, /* 5708 */
  {516,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5709 */
  {516,  0, 1, 0,-1, 0, 0, 0, 0,-1, 3, 9, 3 }, /* 5710 */
  {516, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 5711 */
  {516,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 5712 */
  {516,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 5713 */
  {516,  0, 1, 0, 1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 5714 */
  {516, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 5715 */
  {516,  0,-1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 3 }, /* 5716 */
  {516,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5717 */
  {516,  0, 0,-1, 0, 1, 0,-1, 0, 0, 3, 3, 9 }, /* 5718 */
  {516,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 6 }, /* 5719 */
  {516,  0, 0,-1, 0,-1, 0, 1, 0, 0, 3, 9, 9 }, /* 5720 */
  {516,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 0 }, /* 5721 */
  {516,  0, 0, 1, 0, 1, 0, 1, 0, 0, 3, 3, 3 }, /* 5722 */
  {516,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5723 */
  {516,  0, 0, 1, 0,-1, 0,-1, 0, 0, 3, 9, 3 }, /* 5724 */
  {516,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5725 */
  {516, -1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 3, 3 }, /* 5726 */
  {516,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 0 }, /* 5727 */
  {516,  1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 3, 9 }, /* 5728 */
  {516,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 0 }, /* 5729 */
  {516,  1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 3, 3 }, /* 5730 */
  {516,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 5731 */
  {516, -1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 3, 9 }, /* 5732 */
  {516,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 5733 */
  {516,  0, 1, 0,-1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 5734 */
  {516, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5735 */
  {516,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 9 }, /* 5736 */
  {516,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 5737 */
  {516,  0, 1, 0, 1, 0, 0, 0, 0, 1, 9, 9, 9 }, /* 5738 */
  {516, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 5739 */
  {516,  0,-1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 5740 */
  {516,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5741 */
  {516,  0, 0,-1, 0, 1, 0,-1, 0, 0, 9, 9, 3 }, /* 5742 */
  {516,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5743 */
  {516,  0, 0,-1, 0,-1, 0, 1, 0, 0, 9, 3, 3 }, /* 5744 */
  {516,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5745 */
  {516,  0, 0, 1, 0, 1, 0, 1, 0, 0, 9, 9, 9 }, /* 5746 */
  {516,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 0 }, /* 5747 */
  {516,  0, 0, 1, 0,-1, 0,-1, 0, 0, 9, 3, 9 }, /* 5748 */
  {516,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 5749 */
  {516, -1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 9, 9 }, /* 5750 */
  {516,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 5751 */
  {516,  1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 9, 3 }, /* 5752 */
  {516,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5753 */
  {516,  1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 9, 9 }, /* 5754 */
  {516,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 6 }, /* 5755 */
  {516, -1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 9, 3 }, /* 5756 */
  {517,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5757 */
  {517, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5758 */
  {517,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5759 */
  {517,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5760 */
  {517, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5761 */
  {517,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5762 */
  {517,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5763 */
  {517,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5764 */
  {517,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5765 */
  {517, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5766 */
  {517,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5767 */
  {517,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5768 */
  {517, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5769 */
  {517,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5770 */
  {517,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5771 */
  {517,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5772 */
  {517,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5773 */
  {517,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5774 */
  {517,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5775 */
  {517,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5776 */
  {517,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5777 */
  {517,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5778 */
  {517,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5779 */
  {517,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5780 */
  {517,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5781 */
  {517,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5782 */
  {517,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5783 */
  {517,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5784 */
  {517,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5785 */
  {517,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5786 */
  {517,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5787 */
  {517,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5788 */
  {517,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5789 */
  {517,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5790 */
  {517,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5791 */
  {517, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5792 */
  {517,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5793 */
  {517,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5794 */
  {517, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5795 */
  {517,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5796 */
  {517,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5797 */
  {517,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5798 */
  {517, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5799 */
  {517,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5800 */
  {517,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5801 */
  {517,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5802 */
  {517,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5803 */
  {517, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5804 */
  {518,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5805 */
  {518,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5806 */
  {518, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5807 */
  {518,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5808 */
  {518,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5809 */
  {518,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5810 */
  {518, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5811 */
  {518,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5812 */
  {518,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5813 */
  {518,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5814 */
  {518,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5815 */
  {518,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5816 */
  {518,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5817 */
  {518,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5818 */
  {518,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5819 */
  {518,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5820 */
  {518,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5821 */
  {518,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5822 */
  {518,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5823 */
  {518, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5824 */
  {518,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5825 */
  {518, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5826 */
  {518,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5827 */
  {518,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5828 */
  {518, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 5829 */
  {518,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5830 */
  {518,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 5831 */
  {518,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5832 */
  {518, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 5833 */
  {518,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5834 */
  {518,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 5835 */
  {518,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5836 */
  {518,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5837 */
  {518,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5838 */
  {518,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5839 */
  {518,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5840 */
  {518,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5841 */
  {518,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5842 */
  {518,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5843 */
  {518,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5844 */
  {518,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 5845 */
  {518, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5846 */
  {518,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 5847 */
  {518,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5848 */
  {518,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 5849 */
  {518,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5850 */
  {518,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 5851 */
  {518, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5852 */
  {519,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5853 */
  {519, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5854 */
  {519,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 5855 */
  {519,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 5856 */
  {519, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 5857 */
  {519,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 5858 */
  {519,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 5859 */
  {519,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 5860 */
  {519,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 5861 */
  {519, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 5862 */
  {519,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5863 */
  {519,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5864 */
  {519, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 5865 */
  {519,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 5866 */
  {519,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 5867 */
  {519,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 5868 */
  {519,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5869 */
  {519,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5870 */
  {519,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 6, 0 }, /* 5871 */
  {519,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 6, 0 }, /* 5872 */
  {519,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 5873 */
  {519,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 5874 */
  {519,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 6 }, /* 5875 */
  {519,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 6 }, /* 5876 */
  {519,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 5877 */
  {519,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 5878 */
  {519,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5879 */
  {519,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5880 */
  {519,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 5881 */
  {519,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 5882 */
  {519,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 0, 0 }, /* 5883 */
  {519,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 0, 0 }, /* 5884 */
  {519,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5885 */
  {519,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5886 */
  {519,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 6 }, /* 5887 */
  {519, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 6 }, /* 5888 */
  {519,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 5889 */
  {519,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 5890 */
  {519, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 0 }, /* 5891 */
  {519,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 0 }, /* 5892 */
  {519,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 5893 */
  {519,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 5894 */
  {519, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5895 */
  {519,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5896 */
  {519,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 5897 */
  {519,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 5898 */
  {519,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 0 }, /* 5899 */
  {519, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 0 }, /* 5900 */
  {520,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5901 */
  {520, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5902 */
  {520,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5903 */
  {520,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5904 */
  {520, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5905 */
  {520,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5906 */
  {520,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5907 */
  {520,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5908 */
  {520,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5909 */
  {520, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5910 */
  {520,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5911 */
  {520,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5912 */
  {520, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5913 */
  {520,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5914 */
  {520,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5915 */
  {520,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5916 */
  {520,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5917 */
  {520,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5918 */
  {520,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5919 */
  {520,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5920 */
  {520,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5921 */
  {520,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5922 */
  {520,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5923 */
  {520,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5924 */
  {520,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5925 */
  {520,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5926 */
  {520,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5927 */
  {520,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5928 */
  {520,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5929 */
  {520,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5930 */
  {520,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5931 */
  {520,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5932 */
  {520,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5933 */
  {520,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5934 */
  {520,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5935 */
  {520, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5936 */
  {520,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5937 */
  {520,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5938 */
  {520, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5939 */
  {520,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5940 */
  {520,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5941 */
  {520,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5942 */
  {520, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5943 */
  {520,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5944 */
  {520,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5945 */
  {520,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5946 */
  {520,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5947 */
  {520, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5948 */
  {521,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5949 */
  {521,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5950 */
  {521, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 5951 */
  {521,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 5952 */
  {521,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5953 */
  {521,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5954 */
  {521, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 5955 */
  {521,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 5956 */
  {521,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5957 */
  {521,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 5958 */
  {521,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5959 */
  {521,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 5960 */
  {521,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 5961 */
  {521,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 5962 */
  {521,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 5963 */
  {521,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 5964 */
  {521,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 5965 */
  {521,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 5966 */
  {521,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 5967 */
  {521, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 5968 */
  {521,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 5969 */
  {521, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 5970 */
  {521,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 5971 */
  {521,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 5972 */
  {521, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 5973 */
  {521,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5974 */
  {521,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 5975 */
  {521,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 5976 */
  {521, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 5977 */
  {521,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5978 */
  {521,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 5979 */
  {521,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 5980 */
  {521,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5981 */
  {521,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 5982 */
  {521,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5983 */
  {521,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 5984 */
  {521,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 5985 */
  {521,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 5986 */
  {521,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 5987 */
  {521,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 5988 */
  {521,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 5989 */
  {521, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 5990 */
  {521,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 5991 */
  {521,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 5992 */
  {521,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 5993 */
  {521,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 5994 */
  {521,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 5995 */
  {521, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 5996 */
  {522,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 5997 */
  {522, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 5998 */
  {522,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 5999 */
  {522,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 6000 */
  {522, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 6001 */
  {522,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 6002 */
  {522,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 6003 */
  {522,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 6004 */
  {522,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 6005 */
  {522, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 6006 */
  {522,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 6007 */
  {522,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 6008 */
  {522, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 6009 */
  {522,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 6010 */
  {522,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 6011 */
  {522,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 6012 */
  {522,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6013 */
  {522,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6014 */
  {522,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 0, 6 }, /* 6015 */
  {522,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 0, 6 }, /* 6016 */
  {522,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6017 */
  {522,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6018 */
  {522,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 0 }, /* 6019 */
  {522,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 0 }, /* 6020 */
  {522,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6021 */
  {522,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6022 */
  {522,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 6023 */
  {522,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 6024 */
  {522,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6025 */
  {522,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6026 */
  {522,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 6, 6 }, /* 6027 */
  {522,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 6, 6 }, /* 6028 */
  {522,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 6029 */
  {522,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 6030 */
  {522,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 0 }, /* 6031 */
  {522, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 0 }, /* 6032 */
  {522,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 6033 */
  {522,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 6034 */
  {522, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 6 }, /* 6035 */
  {522,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 6 }, /* 6036 */
  {522,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 6037 */
  {522,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 6038 */
  {522, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 6039 */
  {522,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 6040 */
  {522,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 6041 */
  {522,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 6042 */
  {522,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 6 }, /* 6043 */
  {522, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 6 }, /* 6044 */
  {523,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 6045 */
  {523, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 6046 */
  {523,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 6047 */
  {523,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 6048 */
  {523, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 6049 */
  {523,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 6050 */
  {523,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 6051 */
  {523,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 6052 */
  {523,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 6053 */
  {523, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 6054 */
  {523,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 6055 */
  {523,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 6056 */
  {523, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 6057 */
  {523,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 6058 */
  {523,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 6059 */
  {523,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 6060 */
  {523,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6061 */
  {523,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6062 */
  {523,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 6063 */
  {523,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 6064 */
  {523,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6065 */
  {523,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6066 */
  {523,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 6067 */
  {523,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 6068 */
  {523,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6069 */
  {523,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6070 */
  {523,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 6071 */
  {523,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 6072 */
  {523,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6073 */
  {523,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6074 */
  {523,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 6075 */
  {523,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 6076 */
  {523,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 6077 */
  {523,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 6078 */
  {523,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 6079 */
  {523, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 6080 */
  {523,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 6081 */
  {523,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 6082 */
  {523, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 6083 */
  {523,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 6084 */
  {523,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 6085 */
  {523,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 6086 */
  {523, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 6087 */
  {523,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 6088 */
  {523,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 6089 */
  {523,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 6090 */
  {523,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 6091 */
  {523, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 6092 */
  {523,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 6093 */
  {523, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 6094 */
  {523,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 6095 */
  {523,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 6096 */
  {523, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 6097 */
  {523,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 6098 */
  {523,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 6099 */
  {523,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 6100 */
  {523,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 6101 */
  {523, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 6102 */
  {523,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 6103 */
  {523,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 6104 */
  {523, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 6105 */
  {523,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 6106 */
  {523,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 6107 */
  {523,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 6108 */
  {523,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6109 */
  {523,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6110 */
  {523,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 6, 6 }, /* 6111 */
  {523,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 6, 6 }, /* 6112 */
  {523,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6113 */
  {523,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6114 */
  {523,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 6, 6 }, /* 6115 */
  {523,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 6, 6 }, /* 6116 */
  {523,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6117 */
  {523,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6118 */
  {523,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 6, 6 }, /* 6119 */
  {523,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 6, 6 }, /* 6120 */
  {523,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6121 */
  {523,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6122 */
  {523,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 6, 6 }, /* 6123 */
  {523,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 6, 6 }, /* 6124 */
  {523,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 6125 */
  {523,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 6126 */
  {523,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 6 }, /* 6127 */
  {523, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 6 }, /* 6128 */
  {523,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 6129 */
  {523,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 6130 */
  {523, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 6 }, /* 6131 */
  {523,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 6 }, /* 6132 */
  {523,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 6133 */
  {523,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 6134 */
  {523, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 6 }, /* 6135 */
  {523,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 6 }, /* 6136 */
  {523,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 6137 */
  {523,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 6138 */
  {523,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 6 }, /* 6139 */
  {523, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 6 }, /* 6140 */
  {523,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 6141 */
  {523, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 6142 */
  {523,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 6143 */
  {523,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 6144 */
  {523, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 6145 */
  {523,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 6146 */
  {523,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 6147 */
  {523,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 6148 */
  {523,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 6149 */
  {523, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 6150 */
  {523,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 6151 */
  {523,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 6152 */
  {523, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 6153 */
  {523,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 6154 */
  {523,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 6155 */
  {523,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 6156 */
  {523,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6157 */
  {523,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6158 */
  {523,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 0, 6 }, /* 6159 */
  {523,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 0, 6 }, /* 6160 */
  {523,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6161 */
  {523,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6162 */
  {523,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 0, 6 }, /* 6163 */
  {523,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 0, 6 }, /* 6164 */
  {523,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6165 */
  {523,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6166 */
  {523,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 0, 6 }, /* 6167 */
  {523,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 0, 6 }, /* 6168 */
  {523,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6169 */
  {523,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6170 */
  {523,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 0, 6 }, /* 6171 */
  {523,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 0, 6 }, /* 6172 */
  {523,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 6173 */
  {523,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 6174 */
  {523,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 6 }, /* 6175 */
  {523, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 6 }, /* 6176 */
  {523,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 6177 */
  {523,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 6178 */
  {523, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 6 }, /* 6179 */
  {523,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 6 }, /* 6180 */
  {523,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 6181 */
  {523,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 6182 */
  {523, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 6 }, /* 6183 */
  {523,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 6 }, /* 6184 */
  {523,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 6185 */
  {523,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 6186 */
  {523,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 6 }, /* 6187 */
  {523, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 6 }, /* 6188 */
  {523,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 6189 */
  {523, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 6190 */
  {523,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 6191 */
  {523,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 6192 */
  {523, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 6193 */
  {523,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 6194 */
  {523,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 6195 */
  {523,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 6196 */
  {523,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 6197 */
  {523, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 6198 */
  {523,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 6199 */
  {523,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 6200 */
  {523, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 6201 */
  {523,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 6202 */
  {523,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 6203 */
  {523,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 6204 */
  {523,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6205 */
  {523,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6206 */
  {523,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 6, 0 }, /* 6207 */
  {523,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 0 }, /* 6208 */
  {523,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6209 */
  {523,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6210 */
  {523,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 0 }, /* 6211 */
  {523,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 0 }, /* 6212 */
  {523,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6213 */
  {523,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6214 */
  {523,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 0 }, /* 6215 */
  {523,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 0 }, /* 6216 */
  {523,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6217 */
  {523,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6218 */
  {523,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 6, 0 }, /* 6219 */
  {523,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 0 }, /* 6220 */
  {523,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 6221 */
  {523,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 6222 */
  {523,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 0 }, /* 6223 */
  {523, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 0 }, /* 6224 */
  {523,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 6225 */
  {523,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 6226 */
  {523, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 0 }, /* 6227 */
  {523,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 0 }, /* 6228 */
  {523,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 6229 */
  {523,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 6230 */
  {523, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 0 }, /* 6231 */
  {523,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 0 }, /* 6232 */
  {523,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 6233 */
  {523,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 6234 */
  {523,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 0 }, /* 6235 */
  {523, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 0 }, /* 6236 */
  {524,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 6237 */
  {524, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 6238 */
  {524,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 6239 */
  {524,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 6240 */
  {524, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 6241 */
  {524,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 6242 */
  {524,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 6243 */
  {524,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 6244 */
  {524,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 6245 */
  {524, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 6246 */
  {524,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 6247 */
  {524,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 6248 */
  {524, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 6249 */
  {524,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 6250 */
  {524,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 6251 */
  {524,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 6252 */
  {524,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6253 */
  {524,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6254 */
  {524,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 0, 0 }, /* 6255 */
  {524,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 0, 0 }, /* 6256 */
  {524,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6257 */
  {524,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6258 */
  {524,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 0, 0 }, /* 6259 */
  {524,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 0, 0 }, /* 6260 */
  {524,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6261 */
  {524,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6262 */
  {524,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 0, 0 }, /* 6263 */
  {524,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 0, 0 }, /* 6264 */
  {524,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6265 */
  {524,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6266 */
  {524,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 0, 0 }, /* 6267 */
  {524,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 0, 0 }, /* 6268 */
  {524,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 6269 */
  {524,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 6270 */
  {524,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 0 }, /* 6271 */
  {524, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 0 }, /* 6272 */
  {524,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 6273 */
  {524,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 6274 */
  {524, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 0 }, /* 6275 */
  {524,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 0 }, /* 6276 */
  {524,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 6277 */
  {524,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 6278 */
  {524, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 0 }, /* 6279 */
  {524,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 0 }, /* 6280 */
  {524,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 6281 */
  {524,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 6282 */
  {524,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 0 }, /* 6283 */
  {524, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 0 }, /* 6284 */
  {524,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 6285 */
  {524, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 6286 */
  {524,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 6287 */
  {524,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 6288 */
  {524, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 6289 */
  {524,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 6290 */
  {524,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 6291 */
  {524,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 6292 */
  {524,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 6293 */
  {524, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 6294 */
  {524,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 6295 */
  {524,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 6296 */
  {524, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 6297 */
  {524,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 6298 */
  {524,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 6299 */
  {524,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 6300 */
  {524,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6301 */
  {524,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6302 */
  {524,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 6303 */
  {524,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 6304 */
  {524,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6305 */
  {524,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6306 */
  {524,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 6307 */
  {524,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 6308 */
  {524,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6309 */
  {524,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6310 */
  {524,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 6311 */
  {524,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 6312 */
  {524,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6313 */
  {524,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6314 */
  {524,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 6315 */
  {524,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 6316 */
  {524,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 6317 */
  {524,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 6318 */
  {524,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 6 }, /* 6319 */
  {524, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 6 }, /* 6320 */
  {524,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 6321 */
  {524,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 6322 */
  {524, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 6 }, /* 6323 */
  {524,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 6 }, /* 6324 */
  {524,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 6325 */
  {524,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 6326 */
  {524, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 6 }, /* 6327 */
  {524,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 6 }, /* 6328 */
  {524,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 6329 */
  {524,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 6330 */
  {524,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 6 }, /* 6331 */
  {524, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 6 }, /* 6332 */
  {524,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 6333 */
  {524, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 6334 */
  {524,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 6335 */
  {524,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 6336 */
  {524, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 6337 */
  {524,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 6338 */
  {524,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 6339 */
  {524,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 6340 */
  {524,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 6341 */
  {524, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 6342 */
  {524,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 6343 */
  {524,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 6344 */
  {524, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 6345 */
  {524,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 6346 */
  {524,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 6347 */
  {524,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 6348 */
  {524,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6349 */
  {524,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6350 */
  {524,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 0, 6 }, /* 6351 */
  {524,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 6 }, /* 6352 */
  {524,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6353 */
  {524,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6354 */
  {524,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 6 }, /* 6355 */
  {524,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 6 }, /* 6356 */
  {524,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6357 */
  {524,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6358 */
  {524,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 6 }, /* 6359 */
  {524,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 6 }, /* 6360 */
  {524,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6361 */
  {524,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6362 */
  {524,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 0, 6 }, /* 6363 */
  {524,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 6 }, /* 6364 */
  {524,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 6365 */
  {524,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 6366 */
  {524,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 6367 */
  {524, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 6368 */
  {524,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 6369 */
  {524,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 6370 */
  {524, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 6371 */
  {524,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 6372 */
  {524,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 6373 */
  {524,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 6374 */
  {524, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 6375 */
  {524,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 6376 */
  {524,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 6377 */
  {524,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 6378 */
  {524,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 6379 */
  {524, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 6380 */
  {524,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 6381 */
  {524, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 6382 */
  {524,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 6383 */
  {524,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 6384 */
  {524, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 6385 */
  {524,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 6386 */
  {524,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 6387 */
  {524,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 6388 */
  {524,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 6389 */
  {524, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 6390 */
  {524,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 6391 */
  {524,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 6392 */
  {524, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 6393 */
  {524,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 6394 */
  {524,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 6395 */
  {524,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 6396 */
  {524,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6397 */
  {524,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6398 */
  {524,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 6, 0 }, /* 6399 */
  {524,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 6, 0 }, /* 6400 */
  {524,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6401 */
  {524,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6402 */
  {524,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 6, 0 }, /* 6403 */
  {524,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 6, 0 }, /* 6404 */
  {524,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6405 */
  {524,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6406 */
  {524,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 6, 0 }, /* 6407 */
  {524,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 6, 0 }, /* 6408 */
  {524,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6409 */
  {524,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6410 */
  {524,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 6, 0 }, /* 6411 */
  {524,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 6, 0 }, /* 6412 */
  {524,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 6413 */
  {524,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 6414 */
  {524,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 0 }, /* 6415 */
  {524, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 0 }, /* 6416 */
  {524,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 6417 */
  {524,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 6418 */
  {524, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 0 }, /* 6419 */
  {524,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 0 }, /* 6420 */
  {524,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 6421 */
  {524,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 6422 */
  {524, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 0 }, /* 6423 */
  {524,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 0 }, /* 6424 */
  {524,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 6425 */
  {524,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 6426 */
  {524,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 0 }, /* 6427 */
  {524, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 0 }, /* 6428 */
  {525,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 6429 */
  {525,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 6430 */
  {525, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 6431 */
  {525,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 6432 */
  {525,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 6433 */
  {525,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 6434 */
  {525, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 6435 */
  {525,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 6436 */
  {525,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6437 */
  {525,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 3, 3 }, /* 6438 */
  {525,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6439 */
  {525,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 9, 3 }, /* 6440 */
  {525,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6441 */
  {525,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 3, 9 }, /* 6442 */
  {525,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6443 */
  {525,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 9, 9 }, /* 6444 */
  {525,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 6445 */
  {525,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 3, 3 }, /* 6446 */
  {525,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 6447 */
  {525, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 9, 9 }, /* 6448 */
  {525,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 6449 */
  {525, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 9, 3 }, /* 6450 */
  {525,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 6451 */
  {525,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 3, 9 }, /* 6452 */
  {525, -1, 0, 0, 0,-1, 0, 0, 0,-1, 3, 3, 3 }, /* 6453 */
  {525,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 6454 */
  {525,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 9 }, /* 6455 */
  {525,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 6456 */
  {525, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 3 }, /* 6457 */
  {525,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 6458 */
  {525,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 9 }, /* 6459 */
  {525,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 6460 */
  {525,  0, 0,-1,-1, 0, 0, 0,-1, 0, 3, 3, 3 }, /* 6461 */
  {525,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 6462 */
  {525,  0, 0,-1, 1, 0, 0, 0, 1, 0, 9, 3, 9 }, /* 6463 */
  {525,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 0 }, /* 6464 */
  {525,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 3, 3 }, /* 6465 */
  {525,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 0, 6 }, /* 6466 */
  {525,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 3, 9 }, /* 6467 */
  {525,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 6, 6 }, /* 6468 */
  {525,  0,-1, 0, 0, 0,-1,-1, 0, 0, 3, 3, 3 }, /* 6469 */
  {525, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 6470 */
  {525,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 9, 3 }, /* 6471 */
  {525,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 6 }, /* 6472 */
  {525,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 3, 3 }, /* 6473 */
  {525,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 0 }, /* 6474 */
  {525,  0,-1, 0, 0, 0, 1, 1, 0, 0, 9, 9, 3 }, /* 6475 */
  {525, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 6 }, /* 6476 */
  {525,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 6477 */
  {525,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 6478 */
  {525, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 6479 */
  {525,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 6480 */
  {525,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 6481 */
  {525,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 6482 */
  {525, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 6483 */
  {525,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 6484 */
  {525,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6485 */
  {525,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 9, 9 }, /* 6486 */
  {525,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6487 */
  {525,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 3, 9 }, /* 6488 */
  {525,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6489 */
  {525,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 9, 3 }, /* 6490 */
  {525,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6491 */
  {525,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 3, 3 }, /* 6492 */
  {525,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 6493 */
  {525,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 9, 9 }, /* 6494 */
  {525,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 6495 */
  {525, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 3, 3 }, /* 6496 */
  {525,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 6497 */
  {525, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 3, 9 }, /* 6498 */
  {525,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 6499 */
  {525,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 9, 3 }, /* 6500 */
  {525, -1, 0, 0, 0,-1, 0, 0, 0,-1, 3, 9, 9 }, /* 6501 */
  {525,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 6502 */
  {525,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 3 }, /* 6503 */
  {525,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 6504 */
  {525, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 9, 9 }, /* 6505 */
  {525,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 6506 */
  {525,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 3 }, /* 6507 */
  {525,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 6508 */
  {525,  0, 0,-1,-1, 0, 0, 0,-1, 0, 3, 9, 9 }, /* 6509 */
  {525,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 6, 6 }, /* 6510 */
  {525,  0, 0,-1, 1, 0, 0, 0, 1, 0, 9, 9, 3 }, /* 6511 */
  {525,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 0, 6 }, /* 6512 */
  {525,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 9, 9 }, /* 6513 */
  {525,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 0 }, /* 6514 */
  {525,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 9, 3 }, /* 6515 */
  {525,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 6516 */
  {525,  0,-1, 0, 0, 0,-1,-1, 0, 0, 3, 9, 9 }, /* 6517 */
  {525, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 6 }, /* 6518 */
  {525,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 3, 9 }, /* 6519 */
  {525,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 6520 */
  {525,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 9, 9 }, /* 6521 */
  {525,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 6 }, /* 6522 */
  {525,  0,-1, 0, 0, 0, 1, 1, 0, 0, 9, 3, 9 }, /* 6523 */
  {525, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 0 }, /* 6524 */
  {525,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 6525 */
  {525,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 6526 */
  {525, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 6527 */
  {525,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 6528 */
  {525,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 6529 */
  {525,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 6530 */
  {525, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 6531 */
  {525,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 6532 */
  {525,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6533 */
  {525,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 3, 9 }, /* 6534 */
  {525,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6535 */
  {525,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 9, 9 }, /* 6536 */
  {525,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6537 */
  {525,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 3, 3 }, /* 6538 */
  {525,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6539 */
  {525,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 9, 3 }, /* 6540 */
  {525,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 6541 */
  {525,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 3, 9 }, /* 6542 */
  {525,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 6543 */
  {525, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 9, 3 }, /* 6544 */
  {525,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 6545 */
  {525, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 9, 9 }, /* 6546 */
  {525,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 6547 */
  {525,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 3, 3 }, /* 6548 */
  {525, -1, 0, 0, 0,-1, 0, 0, 0,-1, 9, 3, 9 }, /* 6549 */
  {525,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 6550 */
  {525,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 3 }, /* 6551 */
  {525,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 6552 */
  {525, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 3, 9 }, /* 6553 */
  {525,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 6554 */
  {525,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 3 }, /* 6555 */
  {525,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 6556 */
  {525,  0, 0,-1,-1, 0, 0, 0,-1, 0, 9, 3, 9 }, /* 6557 */
  {525,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 0, 6 }, /* 6558 */
  {525,  0, 0,-1, 1, 0, 0, 0, 1, 0, 3, 3, 3 }, /* 6559 */
  {525,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 6, 6 }, /* 6560 */
  {525,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 3, 9 }, /* 6561 */
  {525,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 6562 */
  {525,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 3, 3 }, /* 6563 */
  {525,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 0 }, /* 6564 */
  {525,  0,-1, 0, 0, 0,-1,-1, 0, 0, 9, 3, 9 }, /* 6565 */
  {525, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 6 }, /* 6566 */
  {525,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 9, 9 }, /* 6567 */
  {525,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 0 }, /* 6568 */
  {525,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 3, 9 }, /* 6569 */
  {525,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 6 }, /* 6570 */
  {525,  0,-1, 0, 0, 0, 1, 1, 0, 0, 3, 9, 9 }, /* 6571 */
  {525, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 6572 */
  {525,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 6573 */
  {525,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 6574 */
  {525, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 6575 */
  {525,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 6576 */
  {525,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 6577 */
  {525,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 6578 */
  {525, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 6579 */
  {525,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 6580 */
  {525,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6581 */
  {525,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 9, 3 }, /* 6582 */
  {525,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6583 */
  {525,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 3, 3 }, /* 6584 */
  {525,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6585 */
  {525,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 9, 9 }, /* 6586 */
  {525,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6587 */
  {525,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 3, 9 }, /* 6588 */
  {525,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 6589 */
  {525,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 9, 3 }, /* 6590 */
  {525,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 6591 */
  {525, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 3, 9 }, /* 6592 */
  {525,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 6593 */
  {525, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 3, 3 }, /* 6594 */
  {525,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 6595 */
  {525,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 9, 9 }, /* 6596 */
  {525, -1, 0, 0, 0,-1, 0, 0, 0,-1, 9, 9, 3 }, /* 6597 */
  {525,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 6598 */
  {525,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 9 }, /* 6599 */
  {525,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 6600 */
  {525, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 9, 3 }, /* 6601 */
  {525,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 6602 */
  {525,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 9 }, /* 6603 */
  {525,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 6604 */
  {525,  0, 0,-1,-1, 0, 0, 0,-1, 0, 9, 9, 3 }, /* 6605 */
  {525,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 0 }, /* 6606 */
  {525,  0, 0,-1, 1, 0, 0, 0, 1, 0, 3, 9, 9 }, /* 6607 */
  {525,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 6608 */
  {525,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 9, 3 }, /* 6609 */
  {525,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 6, 6 }, /* 6610 */
  {525,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 9, 9 }, /* 6611 */
  {525,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 0, 6 }, /* 6612 */
  {525,  0,-1, 0, 0, 0,-1,-1, 0, 0, 9, 9, 3 }, /* 6613 */
  {525, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 0 }, /* 6614 */
  {525,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 3, 3 }, /* 6615 */
  {525,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 6 }, /* 6616 */
  {525,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 9, 3 }, /* 6617 */
  {525,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 6618 */
  {525,  0,-1, 0, 0, 0, 1, 1, 0, 0, 3, 3, 3 }, /* 6619 */
  {525, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 6 }, /* 6620 */
  {526,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 6621 */
  {526, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 6622 */
  {526,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 3, 3 }, /* 6623 */
  {526,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 3, 3 }, /* 6624 */
  {526, -1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 6 }, /* 6625 */
  {526,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 6 }, /* 6626 */
  {526,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 0, 9 }, /* 6627 */
  {526,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 0, 9 }, /* 6628 */
  {526,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 3, 3 }, /* 6629 */
  {526, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 3, 3 }, /* 6630 */
  {526,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 6631 */
  {526,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 6632 */
  {526, -1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 0, 9 }, /* 6633 */
  {526,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 0, 9 }, /* 6634 */
  {526,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 6 }, /* 6635 */
  {526,  0,-1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 6 }, /* 6636 */
  {526,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6637 */
  {526,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6638 */
  {526,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 0, 3 }, /* 6639 */
  {526,  0, 0,-1, 0, 1, 0,-1, 0, 0, 3, 0, 3 }, /* 6640 */
  {526,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 9, 3 }, /* 6641 */
  {526,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 9, 3 }, /* 6642 */
  {526,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 9, 0 }, /* 6643 */
  {526,  0, 0,-1, 0,-1, 0, 1, 0, 0, 9, 9, 0 }, /* 6644 */
  {526,  0, 0,-1, 1, 0, 0, 0,-1, 0, 3, 0, 3 }, /* 6645 */
  {526,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 0, 3 }, /* 6646 */
  {526,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 6647 */
  {526,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 6648 */
  {526,  0, 0,-1,-1, 0, 0, 0, 1, 0, 9, 9, 0 }, /* 6649 */
  {526,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 9, 0 }, /* 6650 */
  {526,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 9, 3 }, /* 6651 */
  {526,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 9, 3 }, /* 6652 */
  {526,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 6653 */
  {526,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 6654 */
  {526,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 3, 0 }, /* 6655 */
  {526, -1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 3, 0 }, /* 6656 */
  {526,  0,-1, 0, 0, 0, 1,-1, 0, 0, 3, 6, 9 }, /* 6657 */
  {526,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 6, 9 }, /* 6658 */
  {526, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 9, 9 }, /* 6659 */
  {526,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 9, 9 }, /* 6660 */
  {526,  0,-1, 0, 0, 0,-1, 1, 0, 0, 3, 3, 0 }, /* 6661 */
  {526,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 3, 0 }, /* 6662 */
  {526, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 6663 */
  {526,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 6664 */
  {526,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 9, 9 }, /* 6665 */
  {526,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 9, 9 }, /* 6666 */
  {526,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 6, 9 }, /* 6667 */
  {526, -1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 6, 9 }, /* 6668 */
  {526,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 6669 */
  {526, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 6670 */
  {526,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 9, 9 }, /* 6671 */
  {526,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 9, 9 }, /* 6672 */
  {526, -1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 0 }, /* 6673 */
  {526,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 0 }, /* 6674 */
  {526,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 6, 3 }, /* 6675 */
  {526,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 6, 3 }, /* 6676 */
  {526,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 9, 9 }, /* 6677 */
  {526, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 9, 9 }, /* 6678 */
  {526,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 6 }, /* 6679 */
  {526,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 6 }, /* 6680 */
  {526, -1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 6, 3 }, /* 6681 */
  {526,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 6, 3 }, /* 6682 */
  {526,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 0 }, /* 6683 */
  {526,  0,-1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 0 }, /* 6684 */
  {526,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6685 */
  {526,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6686 */
  {526,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 6, 9 }, /* 6687 */
  {526,  0, 0,-1, 0, 1, 0,-1, 0, 0, 3, 6, 9 }, /* 6688 */
  {526,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 3, 9 }, /* 6689 */
  {526,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 3, 9 }, /* 6690 */
  {526,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 3, 6 }, /* 6691 */
  {526,  0, 0,-1, 0,-1, 0, 1, 0, 0, 9, 3, 6 }, /* 6692 */
  {526,  0, 0,-1, 1, 0, 0, 0,-1, 0, 3, 6, 9 }, /* 6693 */
  {526,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 6, 9 }, /* 6694 */
  {526,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 6, 6 }, /* 6695 */
  {526,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 6, 6 }, /* 6696 */
  {526,  0, 0,-1,-1, 0, 0, 0, 1, 0, 9, 3, 6 }, /* 6697 */
  {526,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 3, 6 }, /* 6698 */
  {526,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 3, 9 }, /* 6699 */
  {526,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 3, 9 }, /* 6700 */
  {526,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 6701 */
  {526,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 6702 */
  {526,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 9, 6 }, /* 6703 */
  {526, -1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 9, 6 }, /* 6704 */
  {526,  0,-1, 0, 0, 0, 1,-1, 0, 0, 3, 0, 3 }, /* 6705 */
  {526,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 0, 3 }, /* 6706 */
  {526, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 3, 3 }, /* 6707 */
  {526,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 3, 3 }, /* 6708 */
  {526,  0,-1, 0, 0, 0,-1, 1, 0, 0, 3, 9, 6 }, /* 6709 */
  {526,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 9, 6 }, /* 6710 */
  {526, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 6 }, /* 6711 */
  {526,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 6 }, /* 6712 */
  {526,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 3, 3 }, /* 6713 */
  {526,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 3, 3 }, /* 6714 */
  {526,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 0, 3 }, /* 6715 */
  {526, -1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 0, 3 }, /* 6716 */
  {526,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 6717 */
  {526, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 6718 */
  {526,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 3, 9 }, /* 6719 */
  {526,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 3, 9 }, /* 6720 */
  {526, -1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 0 }, /* 6721 */
  {526,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 0 }, /* 6722 */
  {526,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 0, 3 }, /* 6723 */
  {526,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 0, 3 }, /* 6724 */
  {526,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 3, 9 }, /* 6725 */
  {526, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 3, 9 }, /* 6726 */
  {526,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 6 }, /* 6727 */
  {526,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 6 }, /* 6728 */
  {526, -1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 0, 3 }, /* 6729 */
  {526,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 0, 3 }, /* 6730 */
  {526,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 0 }, /* 6731 */
  {526,  0,-1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 0 }, /* 6732 */
  {526,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6733 */
  {526,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6734 */
  {526,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 0, 9 }, /* 6735 */
  {526,  0, 0,-1, 0, 1, 0,-1, 0, 0, 9, 0, 9 }, /* 6736 */
  {526,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 9, 9 }, /* 6737 */
  {526,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 9, 9 }, /* 6738 */
  {526,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 9, 6 }, /* 6739 */
  {526,  0, 0,-1, 0,-1, 0, 1, 0, 0, 3, 9, 6 }, /* 6740 */
  {526,  0, 0,-1, 1, 0, 0, 0,-1, 0, 9, 0, 9 }, /* 6741 */
  {526,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 0, 9 }, /* 6742 */
  {526,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 0, 6 }, /* 6743 */
  {526,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 0, 6 }, /* 6744 */
  {526,  0, 0,-1,-1, 0, 0, 0, 1, 0, 3, 9, 6 }, /* 6745 */
  {526,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 9, 6 }, /* 6746 */
  {526,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 9, 9 }, /* 6747 */
  {526,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 9, 9 }, /* 6748 */
  {526,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 6749 */
  {526,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 6750 */
  {526,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 3, 6 }, /* 6751 */
  {526, -1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 3, 6 }, /* 6752 */
  {526,  0,-1, 0, 0, 0, 1,-1, 0, 0, 9, 6, 3 }, /* 6753 */
  {526,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 6, 3 }, /* 6754 */
  {526, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 9, 3 }, /* 6755 */
  {526,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 9, 3 }, /* 6756 */
  {526,  0,-1, 0, 0, 0,-1, 1, 0, 0, 9, 3, 6 }, /* 6757 */
  {526,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 3, 6 }, /* 6758 */
  {526, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 6 }, /* 6759 */
  {526,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 6 }, /* 6760 */
  {526,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 9, 3 }, /* 6761 */
  {526,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 9, 3 }, /* 6762 */
  {526,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 6, 3 }, /* 6763 */
  {526, -1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 6, 3 }, /* 6764 */
  {526,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 6765 */
  {526, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 6766 */
  {526,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 9, 3 }, /* 6767 */
  {526,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 9, 3 }, /* 6768 */
  {526, -1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 6 }, /* 6769 */
  {526,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 6 }, /* 6770 */
  {526,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 6, 9 }, /* 6771 */
  {526,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 6, 9 }, /* 6772 */
  {526,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 9, 3 }, /* 6773 */
  {526, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 9, 3 }, /* 6774 */
  {526,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 0 }, /* 6775 */
  {526,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 0 }, /* 6776 */
  {526, -1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 6, 9 }, /* 6777 */
  {526,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 6, 9 }, /* 6778 */
  {526,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 6 }, /* 6779 */
  {526,  0,-1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 6 }, /* 6780 */
  {526,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6781 */
  {526,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6782 */
  {526,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 6, 3 }, /* 6783 */
  {526,  0, 0,-1, 0, 1, 0,-1, 0, 0, 9, 6, 3 }, /* 6784 */
  {526,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 3, 3 }, /* 6785 */
  {526,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 3, 3 }, /* 6786 */
  {526,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 3, 0 }, /* 6787 */
  {526,  0, 0,-1, 0,-1, 0, 1, 0, 0, 3, 3, 0 }, /* 6788 */
  {526,  0, 0,-1, 1, 0, 0, 0,-1, 0, 9, 6, 3 }, /* 6789 */
  {526,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 6, 3 }, /* 6790 */
  {526,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 0 }, /* 6791 */
  {526,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 0 }, /* 6792 */
  {526,  0, 0,-1,-1, 0, 0, 0, 1, 0, 3, 3, 0 }, /* 6793 */
  {526,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 3, 0 }, /* 6794 */
  {526,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 3, 3 }, /* 6795 */
  {526,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 3, 3 }, /* 6796 */
  {526,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 6797 */
  {526,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 6798 */
  {526,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 9, 0 }, /* 6799 */
  {526, -1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 9, 0 }, /* 6800 */
  {526,  0,-1, 0, 0, 0, 1,-1, 0, 0, 9, 0, 9 }, /* 6801 */
  {526,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 0, 9 }, /* 6802 */
  {526, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 3, 9 }, /* 6803 */
  {526,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 3, 9 }, /* 6804 */
  {526,  0,-1, 0, 0, 0,-1, 1, 0, 0, 9, 9, 0 }, /* 6805 */
  {526,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 9, 0 }, /* 6806 */
  {526, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 0 }, /* 6807 */
  {526,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 0 }, /* 6808 */
  {526,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 3, 9 }, /* 6809 */
  {526,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 3, 9 }, /* 6810 */
  {526,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 0, 9 }, /* 6811 */
  {526, -1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 0, 9 }, /* 6812 */
  {527,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 6813 */
  {527,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 6814 */
  {527, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 6815 */
  {527,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 6816 */
  {527,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 6817 */
  {527,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 6818 */
  {527, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 6819 */
  {527,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 6820 */
  {527,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6821 */
  {527,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 3, 3 }, /* 6822 */
  {527,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6823 */
  {527,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 9, 3 }, /* 6824 */
  {527,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6825 */
  {527,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 3, 9 }, /* 6826 */
  {527,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6827 */
  {527,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 9, 9 }, /* 6828 */
  {527,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 6829 */
  {527,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 3, 3 }, /* 6830 */
  {527,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 0 }, /* 6831 */
  {527, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 9, 9 }, /* 6832 */
  {527,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 6833 */
  {527, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 9, 3 }, /* 6834 */
  {527,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 6835 */
  {527,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 3, 9 }, /* 6836 */
  {527, -1, 0, 0, 0,-1, 0, 0, 0,-1, 3, 3, 9 }, /* 6837 */
  {527,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 6838 */
  {527,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 3 }, /* 6839 */
  {527,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 6840 */
  {527, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 9 }, /* 6841 */
  {527,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 6842 */
  {527,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 3 }, /* 6843 */
  {527,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 6844 */
  {527,  0, 0,-1,-1, 0, 0, 0,-1, 0, 3, 3, 9 }, /* 6845 */
  {527,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 6 }, /* 6846 */
  {527,  0, 0,-1, 1, 0, 0, 0, 1, 0, 9, 3, 3 }, /* 6847 */
  {527,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 6848 */
  {527,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 3, 9 }, /* 6849 */
  {527,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 0, 0 }, /* 6850 */
  {527,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 3, 3 }, /* 6851 */
  {527,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 6, 0 }, /* 6852 */
  {527,  0,-1, 0, 0, 0,-1,-1, 0, 0, 3, 3, 9 }, /* 6853 */
  {527, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 6 }, /* 6854 */
  {527,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 9, 9 }, /* 6855 */
  {527,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 0 }, /* 6856 */
  {527,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 3, 9 }, /* 6857 */
  {527,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 6858 */
  {527,  0,-1, 0, 0, 0, 1, 1, 0, 0, 9, 9, 9 }, /* 6859 */
  {527, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 0, 0 }, /* 6860 */
  {527,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 6861 */
  {527,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 6862 */
  {527, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 6863 */
  {527,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 6864 */
  {527,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 6865 */
  {527,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 6866 */
  {527, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 6867 */
  {527,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 6868 */
  {527,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6869 */
  {527,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 9, 9 }, /* 6870 */
  {527,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6871 */
  {527,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 3, 9 }, /* 6872 */
  {527,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6873 */
  {527,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 9, 3 }, /* 6874 */
  {527,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6875 */
  {527,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 3, 3 }, /* 6876 */
  {527,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 6877 */
  {527,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 9, 9 }, /* 6878 */
  {527,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 6879 */
  {527, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 3, 3 }, /* 6880 */
  {527,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 6881 */
  {527, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 3, 9 }, /* 6882 */
  {527,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 6883 */
  {527,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 9, 3 }, /* 6884 */
  {527, -1, 0, 0, 0,-1, 0, 0, 0,-1, 3, 9, 3 }, /* 6885 */
  {527,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 6886 */
  {527,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 9 }, /* 6887 */
  {527,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 6888 */
  {527, -1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 9, 3 }, /* 6889 */
  {527,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 6890 */
  {527,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 9 }, /* 6891 */
  {527,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 6892 */
  {527,  0, 0,-1,-1, 0, 0, 0,-1, 0, 3, 9, 3 }, /* 6893 */
  {527,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 6, 0 }, /* 6894 */
  {527,  0, 0,-1, 1, 0, 0, 0, 1, 0, 9, 9, 9 }, /* 6895 */
  {527,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 0, 0 }, /* 6896 */
  {527,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 9, 3 }, /* 6897 */
  {527,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 6898 */
  {527,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 9, 9 }, /* 6899 */
  {527,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 6 }, /* 6900 */
  {527,  0,-1, 0, 0, 0,-1,-1, 0, 0, 3, 9, 3 }, /* 6901 */
  {527, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 6, 0 }, /* 6902 */
  {527,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 3, 3 }, /* 6903 */
  {527,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 6 }, /* 6904 */
  {527,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 9, 3 }, /* 6905 */
  {527,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 0 }, /* 6906 */
  {527,  0,-1, 0, 0, 0, 1, 1, 0, 0, 9, 3, 3 }, /* 6907 */
  {527, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 6908 */
  {527,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 6909 */
  {527,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 6910 */
  {527, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 0 }, /* 6911 */
  {527,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 6912 */
  {527,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 6913 */
  {527,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 6914 */
  {527, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 0 }, /* 6915 */
  {527,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 6916 */
  {527,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 6917 */
  {527,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 3, 9 }, /* 6918 */
  {527,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 6919 */
  {527,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 9, 9 }, /* 6920 */
  {527,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 6921 */
  {527,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 3, 3 }, /* 6922 */
  {527,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 6923 */
  {527,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 9, 3 }, /* 6924 */
  {527,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 6925 */
  {527,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 3, 9 }, /* 6926 */
  {527,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 6927 */
  {527, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 9, 3 }, /* 6928 */
  {527,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 6929 */
  {527, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 9, 9 }, /* 6930 */
  {527,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 6931 */
  {527,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 3, 3 }, /* 6932 */
  {527, -1, 0, 0, 0,-1, 0, 0, 0,-1, 9, 3, 3 }, /* 6933 */
  {527,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 6934 */
  {527,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 9 }, /* 6935 */
  {527,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 6936 */
  {527, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 3, 3 }, /* 6937 */
  {527,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 6938 */
  {527,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 9 }, /* 6939 */
  {527,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 6940 */
  {527,  0, 0,-1,-1, 0, 0, 0,-1, 0, 9, 3, 3 }, /* 6941 */
  {527,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 0, 0 }, /* 6942 */
  {527,  0, 0,-1, 1, 0, 0, 0, 1, 0, 3, 3, 9 }, /* 6943 */
  {527,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 6, 0 }, /* 6944 */
  {527,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 3, 3 }, /* 6945 */
  {527,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 6 }, /* 6946 */
  {527,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 3, 9 }, /* 6947 */
  {527,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 6948 */
  {527,  0,-1, 0, 0, 0,-1,-1, 0, 0, 9, 3, 3 }, /* 6949 */
  {527, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 0, 0 }, /* 6950 */
  {527,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 9, 3 }, /* 6951 */
  {527,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 6952 */
  {527,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 3, 3 }, /* 6953 */
  {527,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 0 }, /* 6954 */
  {527,  0,-1, 0, 0, 0, 1, 1, 0, 0, 3, 9, 3 }, /* 6955 */
  {527, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 6 }, /* 6956 */
  {527,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 6957 */
  {527,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 6958 */
  {527, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 6959 */
  {527,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 6960 */
  {527,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 6961 */
  {527,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 6962 */
  {527, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 6963 */
  {527,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 6964 */
  {527,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 6965 */
  {527,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 9, 3 }, /* 6966 */
  {527,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 6967 */
  {527,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 3, 3 }, /* 6968 */
  {527,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 6969 */
  {527,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 9, 9 }, /* 6970 */
  {527,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 6971 */
  {527,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 3, 9 }, /* 6972 */
  {527,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 6973 */
  {527,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 9, 3 }, /* 6974 */
  {527,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 6975 */
  {527, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 3, 9 }, /* 6976 */
  {527,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 0 }, /* 6977 */
  {527, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 3, 3 }, /* 6978 */
  {527,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 6979 */
  {527,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 9, 9 }, /* 6980 */
  {527, -1, 0, 0, 0,-1, 0, 0, 0,-1, 9, 9, 9 }, /* 6981 */
  {527,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 6982 */
  {527,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 3 }, /* 6983 */
  {527,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 6984 */
  {527, -1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 9, 9 }, /* 6985 */
  {527,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 6986 */
  {527,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 3 }, /* 6987 */
  {527,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 6988 */
  {527,  0, 0,-1,-1, 0, 0, 0,-1, 0, 9, 9, 9 }, /* 6989 */
  {527,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 6990 */
  {527,  0, 0,-1, 1, 0, 0, 0, 1, 0, 3, 9, 3 }, /* 6991 */
  {527,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 6 }, /* 6992 */
  {527,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 9, 9 }, /* 6993 */
  {527,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 6, 0 }, /* 6994 */
  {527,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 9, 3 }, /* 6995 */
  {527,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 0, 0 }, /* 6996 */
  {527,  0,-1, 0, 0, 0,-1,-1, 0, 0, 9, 9, 9 }, /* 6997 */
  {527, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 6998 */
  {527,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 3, 9 }, /* 6999 */
  {527,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 0 }, /* 7000 */
  {527,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 9, 9 }, /* 7001 */
  {527,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 6 }, /* 7002 */
  {527,  0,-1, 0, 0, 0, 1, 1, 0, 0, 3, 3, 9 }, /* 7003 */
  {527, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 6, 0 }, /* 7004 */
  {528,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 7005 */
  {528, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 7006 */
  {528,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 3, 9 }, /* 7007 */
  {528,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 3, 9 }, /* 7008 */
  {528, -1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 3, 6 }, /* 7009 */
  {528,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 3, 6 }, /* 7010 */
  {528,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 0, 3 }, /* 7011 */
  {528,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 0, 3 }, /* 7012 */
  {528,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 3, 3 }, /* 7013 */
  {528, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 3, 3 }, /* 7014 */
  {528,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 6 }, /* 7015 */
  {528,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 6 }, /* 7016 */
  {528, -1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 0, 9 }, /* 7017 */
  {528,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 0, 9 }, /* 7018 */
  {528,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 0 }, /* 7019 */
  {528,  0,-1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 0 }, /* 7020 */
  {528,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 7021 */
  {528,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 7022 */
  {528,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 0, 3 }, /* 7023 */
  {528,  0, 0,-1, 0, 1, 0,-1, 0, 0, 9, 0, 3 }, /* 7024 */
  {528,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 9, 3 }, /* 7025 */
  {528,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 9, 3 }, /* 7026 */
  {528,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 9, 0 }, /* 7027 */
  {528,  0, 0,-1, 0,-1, 0, 1, 0, 0, 3, 9, 0 }, /* 7028 */
  {528,  0, 0,-1, 1, 0, 0, 0,-1, 0, 3, 0, 3 }, /* 7029 */
  {528,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 0, 3 }, /* 7030 */
  {528,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 0, 0 }, /* 7031 */
  {528,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 0, 0 }, /* 7032 */
  {528,  0, 0,-1,-1, 0, 0, 0, 1, 0, 9, 9, 0 }, /* 7033 */
  {528,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 9, 0 }, /* 7034 */
  {528,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 9, 3 }, /* 7035 */
  {528,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 9, 3 }, /* 7036 */
  {528,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 7037 */
  {528,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 7038 */
  {528,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 9, 0 }, /* 7039 */
  {528, -1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 9, 0 }, /* 7040 */
  {528,  0,-1, 0, 0, 0, 1,-1, 0, 0, 3, 6, 9 }, /* 7041 */
  {528,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 6, 9 }, /* 7042 */
  {528, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 3, 9 }, /* 7043 */
  {528,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 3, 9 }, /* 7044 */
  {528,  0,-1, 0, 0, 0,-1, 1, 0, 0, 3, 3, 0 }, /* 7045 */
  {528,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 3, 0 }, /* 7046 */
  {528, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 6, 0 }, /* 7047 */
  {528,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 0 }, /* 7048 */
  {528,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 9, 9 }, /* 7049 */
  {528,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 9, 9 }, /* 7050 */
  {528,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 0, 9 }, /* 7051 */
  {528, -1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 0, 9 }, /* 7052 */
  {528,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 6 }, /* 7053 */
  {528, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 6, 6 }, /* 7054 */
  {528,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 9, 3 }, /* 7055 */
  {528,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 9, 3 }, /* 7056 */
  {528, -1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 9, 0 }, /* 7057 */
  {528,  1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 9, 0 }, /* 7058 */
  {528,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 6, 9 }, /* 7059 */
  {528,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 6, 9 }, /* 7060 */
  {528,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 9, 9 }, /* 7061 */
  {528, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 9, 9 }, /* 7062 */
  {528,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 6, 0 }, /* 7063 */
  {528,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 6, 0 }, /* 7064 */
  {528, -1, 0, 0, 0, 1, 0, 0, 0,-1, 9, 6, 3 }, /* 7065 */
  {528,  1, 0, 0, 0,-1, 0, 0, 0, 1, 9, 6, 3 }, /* 7066 */
  {528,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 6 }, /* 7067 */
  {528,  0,-1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 6 }, /* 7068 */
  {528,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 7069 */
  {528,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 7070 */
  {528,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 6, 9 }, /* 7071 */
  {528,  0, 0,-1, 0, 1, 0,-1, 0, 0, 9, 6, 9 }, /* 7072 */
  {528,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 3, 9 }, /* 7073 */
  {528,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 3, 9 }, /* 7074 */
  {528,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 3, 6 }, /* 7075 */
  {528,  0, 0,-1, 0,-1, 0, 1, 0, 0, 3, 3, 6 }, /* 7076 */
  {528,  0, 0,-1, 1, 0, 0, 0,-1, 0, 3, 6, 9 }, /* 7077 */
  {528,  0, 0, 1,-1, 0, 0, 0, 1, 0, 3, 6, 9 }, /* 7078 */
  {528,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 7079 */
  {528,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 7080 */
  {528,  0, 0,-1,-1, 0, 0, 0, 1, 0, 9, 3, 6 }, /* 7081 */
  {528,  0, 0, 1, 1, 0, 0, 0,-1, 0, 9, 3, 6 }, /* 7082 */
  {528,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 3, 9 }, /* 7083 */
  {528,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 3, 9 }, /* 7084 */
  {528,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 6, 6 }, /* 7085 */
  {528,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 6, 6 }, /* 7086 */
  {528,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 3, 6 }, /* 7087 */
  {528, -1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 3, 6 }, /* 7088 */
  {528,  0,-1, 0, 0, 0, 1,-1, 0, 0, 3, 0, 3 }, /* 7089 */
  {528,  0, 1, 0, 0, 0,-1, 1, 0, 0, 3, 0, 3 }, /* 7090 */
  {528, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 9, 3 }, /* 7091 */
  {528,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 9, 3 }, /* 7092 */
  {528,  0,-1, 0, 0, 0,-1, 1, 0, 0, 3, 9, 6 }, /* 7093 */
  {528,  0, 1, 0, 0, 0, 1,-1, 0, 0, 3, 9, 6 }, /* 7094 */
  {528, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 6 }, /* 7095 */
  {528,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 6 }, /* 7096 */
  {528,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 3, 3 }, /* 7097 */
  {528,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 3, 3 }, /* 7098 */
  {528,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 6, 3 }, /* 7099 */
  {528, -1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 6, 3 }, /* 7100 */
  {528,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 0, 6 }, /* 7101 */
  {528, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 0, 6 }, /* 7102 */
  {528,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 3, 3 }, /* 7103 */
  {528,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 3, 3 }, /* 7104 */
  {528, -1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 3, 0 }, /* 7105 */
  {528,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 3, 0 }, /* 7106 */
  {528,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 0, 9 }, /* 7107 */
  {528,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 0, 9 }, /* 7108 */
  {528,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 3, 9 }, /* 7109 */
  {528, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 3, 9 }, /* 7110 */
  {528,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 0, 0 }, /* 7111 */
  {528,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 0, 0 }, /* 7112 */
  {528, -1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 0, 3 }, /* 7113 */
  {528,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 0, 3 }, /* 7114 */
  {528,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 6 }, /* 7115 */
  {528,  0,-1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 6 }, /* 7116 */
  {528,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 7117 */
  {528,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 7118 */
  {528,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 0, 9 }, /* 7119 */
  {528,  0, 0,-1, 0, 1, 0,-1, 0, 0, 3, 0, 9 }, /* 7120 */
  {528,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 9, 9 }, /* 7121 */
  {528,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 9, 9 }, /* 7122 */
  {528,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 9, 6 }, /* 7123 */
  {528,  0, 0,-1, 0,-1, 0, 1, 0, 0, 9, 9, 6 }, /* 7124 */
  {528,  0, 0,-1, 1, 0, 0, 0,-1, 0, 9, 0, 9 }, /* 7125 */
  {528,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 0, 9 }, /* 7126 */
  {528,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 6 }, /* 7127 */
  {528,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 6 }, /* 7128 */
  {528,  0, 0,-1,-1, 0, 0, 0, 1, 0, 3, 9, 6 }, /* 7129 */
  {528,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 9, 6 }, /* 7130 */
  {528,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 9, 9 }, /* 7131 */
  {528,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 9, 9 }, /* 7132 */
  {528,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 0, 6 }, /* 7133 */
  {528,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 0, 6 }, /* 7134 */
  {528,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 9, 6 }, /* 7135 */
  {528, -1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 9, 6 }, /* 7136 */
  {528,  0,-1, 0, 0, 0, 1,-1, 0, 0, 9, 6, 3 }, /* 7137 */
  {528,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 6, 3 }, /* 7138 */
  {528, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 3, 3 }, /* 7139 */
  {528,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 3, 3 }, /* 7140 */
  {528,  0,-1, 0, 0, 0,-1, 1, 0, 0, 9, 3, 6 }, /* 7141 */
  {528,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 3, 6 }, /* 7142 */
  {528, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 7143 */
  {528,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 7144 */
  {528,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 9, 3 }, /* 7145 */
  {528,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 9, 3 }, /* 7146 */
  {528,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 0, 3 }, /* 7147 */
  {528, -1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 0, 3 }, /* 7148 */
  {528,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 7149 */
  {528, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 7150 */
  {528,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 9, 9 }, /* 7151 */
  {528,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 9, 9 }, /* 7152 */
  {528, -1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 9, 6 }, /* 7153 */
  {528,  1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 9, 6 }, /* 7154 */
  {528,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 6, 3 }, /* 7155 */
  {528,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 6, 3 }, /* 7156 */
  {528,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 9, 3 }, /* 7157 */
  {528, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 9, 3 }, /* 7158 */
  {528,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 7159 */
  {528,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 7160 */
  {528, -1, 0, 0, 0, 1, 0, 0, 0,-1, 3, 6, 9 }, /* 7161 */
  {528,  1, 0, 0, 0,-1, 0, 0, 0, 1, 3, 6, 9 }, /* 7162 */
  {528,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 0 }, /* 7163 */
  {528,  0,-1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 0 }, /* 7164 */
  {528,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 7165 */
  {528,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 7166 */
  {528,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 6, 3 }, /* 7167 */
  {528,  0, 0,-1, 0, 1, 0,-1, 0, 0, 3, 6, 3 }, /* 7168 */
  {528,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 3, 3 }, /* 7169 */
  {528,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 3, 3 }, /* 7170 */
  {528,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 3, 0 }, /* 7171 */
  {528,  0, 0,-1, 0,-1, 0, 1, 0, 0, 9, 3, 0 }, /* 7172 */
  {528,  0, 0,-1, 1, 0, 0, 0,-1, 0, 9, 6, 3 }, /* 7173 */
  {528,  0, 0, 1,-1, 0, 0, 0, 1, 0, 9, 6, 3 }, /* 7174 */
  {528,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 6, 0 }, /* 7175 */
  {528,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 6, 0 }, /* 7176 */
  {528,  0, 0,-1,-1, 0, 0, 0, 1, 0, 3, 3, 0 }, /* 7177 */
  {528,  0, 0, 1, 1, 0, 0, 0,-1, 0, 3, 3, 0 }, /* 7178 */
  {528,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 3, 3 }, /* 7179 */
  {528,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 3, 3 }, /* 7180 */
  {528,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 7181 */
  {528,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 7182 */
  {528,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 3, 0 }, /* 7183 */
  {528, -1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 3, 0 }, /* 7184 */
  {528,  0,-1, 0, 0, 0, 1,-1, 0, 0, 9, 0, 9 }, /* 7185 */
  {528,  0, 1, 0, 0, 0,-1, 1, 0, 0, 9, 0, 9 }, /* 7186 */
  {528, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 9, 9 }, /* 7187 */
  {528,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 9, 9 }, /* 7188 */
  {528,  0,-1, 0, 0, 0,-1, 1, 0, 0, 9, 9, 0 }, /* 7189 */
  {528,  0, 1, 0, 0, 0, 1,-1, 0, 0, 9, 9, 0 }, /* 7190 */
  {528, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 0, 0 }, /* 7191 */
  {528,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 0, 0 }, /* 7192 */
  {528,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 3, 9 }, /* 7193 */
  {528,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 3, 9 }, /* 7194 */
  {528,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 6, 9 }, /* 7195 */
  {528, -1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 6, 9 }, /* 7196 */
  {529,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 7197 */
  {529, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 7198 */
  {529,  0,-1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 7199 */
  {529,  0, 1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 7200 */
  {529, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 7201 */
  {529,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 7202 */
  {529,  0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 7203 */
  {529,  0,-1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 7204 */
  {529,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 7205 */
  {529, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 7206 */
  {529,  0,-1, 0,-1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 7207 */
  {529,  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 7208 */
  {529, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0 }, /* 7209 */
  {529,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0 }, /* 7210 */
  {529,  0, 1, 0, 1, 0, 0, 0, 0,-1, 0, 0, 0 }, /* 7211 */
  {529,  0,-1, 0,-1, 0, 0, 0, 0, 1, 0, 0, 0 }, /* 7212 */
  {529,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 7213 */
  {529,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 7214 */
  {529,  0, 0, 1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 7215 */
  {529,  0, 0,-1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 7216 */
  {529,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 7217 */
  {529,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 7218 */
  {529,  0, 0, 1, 0, 1, 0,-1, 0, 0, 0, 0, 0 }, /* 7219 */
  {529,  0, 0,-1, 0,-1, 0, 1, 0, 0, 0, 0, 0 }, /* 7220 */
  {529,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 7221 */
  {529,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 7222 */
  {529,  0, 0,-1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 7223 */
  {529,  0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 7224 */
  {529,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 7225 */
  {529,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 7226 */
  {529,  0, 0,-1, 0, 1, 0, 1, 0, 0, 0, 0, 0 }, /* 7227 */
  {529,  0, 0, 1, 0,-1, 0,-1, 0, 0, 0, 0, 0 }, /* 7228 */
  {529,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 7229 */
  {529,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 7230 */
  {529,  1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 7231 */
  {529, -1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 7232 */
  {529,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 7233 */
  {529,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 7234 */
  {529, -1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 7235 */
  {529,  1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 7236 */
  {529,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 0, 0 }, /* 7237 */
  {529,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0 }, /* 7238 */
  {529, -1, 0, 0, 0, 0,-1, 0,-1, 0, 0, 0, 0 }, /* 7239 */
  {529,  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 }, /* 7240 */
  {529,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 7241 */
  {529,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 7242 */
  {529,  1, 0, 0, 0, 0,-1, 0, 1, 0, 0, 0, 0 }, /* 7243 */
  {529, -1, 0, 0, 0, 0, 1, 0,-1, 0, 0, 0, 0 }, /* 7244 */
  {529,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 7245 */
  {529, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 7246 */
  {529,  0,-1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 7247 */
  {529,  0, 1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 7248 */
  {529, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 7249 */
  {529,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 7250 */
  {529,  0, 1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 7251 */
  {529,  0,-1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 7252 */
  {529,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 7253 */
  {529, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 7254 */
  {529,  0,-1, 0,-1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 7255 */
  {529,  0, 1, 0, 1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 7256 */
  {529, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 6, 6 }, /* 7257 */
  {529,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 6, 6 }, /* 7258 */
  {529,  0, 1, 0, 1, 0, 0, 0, 0,-1, 6, 6, 6 }, /* 7259 */
  {529,  0,-1, 0,-1, 0, 0, 0, 0, 1, 6, 6, 6 }, /* 7260 */
  {529,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 7261 */
  {529,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 7262 */
  {529,  0, 0, 1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 7263 */
  {529,  0, 0,-1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 7264 */
  {529,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 7265 */
  {529,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 7266 */
  {529,  0, 0, 1, 0, 1, 0,-1, 0, 0, 6, 6, 6 }, /* 7267 */
  {529,  0, 0,-1, 0,-1, 0, 1, 0, 0, 6, 6, 6 }, /* 7268 */
  {529,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 7269 */
  {529,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 7270 */
  {529,  0, 0,-1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 7271 */
  {529,  0, 0, 1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 7272 */
  {529,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 7273 */
  {529,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 7274 */
  {529,  0, 0,-1, 0, 1, 0, 1, 0, 0, 6, 6, 6 }, /* 7275 */
  {529,  0, 0, 1, 0,-1, 0,-1, 0, 0, 6, 6, 6 }, /* 7276 */
  {529,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 7277 */
  {529,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 7278 */
  {529,  1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 7279 */
  {529, -1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 7280 */
  {529,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 7281 */
  {529,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 7282 */
  {529, -1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 7283 */
  {529,  1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 7284 */
  {529,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 6, 6 }, /* 7285 */
  {529,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 6, 6 }, /* 7286 */
  {529, -1, 0, 0, 0, 0,-1, 0,-1, 0, 6, 6, 6 }, /* 7287 */
  {529,  1, 0, 0, 0, 0, 1, 0, 1, 0, 6, 6, 6 }, /* 7288 */
  {529,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 7289 */
  {529,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 7290 */
  {529,  1, 0, 0, 0, 0,-1, 0, 1, 0, 6, 6, 6 }, /* 7291 */
  {529, -1, 0, 0, 0, 0, 1, 0,-1, 0, 6, 6, 6 }, /* 7292 */
  {530,  1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, /* 7293 */
  {530, -1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0 }, /* 7294 */
  {530,  0,-1, 0, 1, 0, 0, 0, 0, 1, 3, 9, 3 }, /* 7295 */
  {530,  0, 1, 0,-1, 0, 0, 0, 0,-1, 3, 9, 3 }, /* 7296 */
  {530, -1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 6 }, /* 7297 */
  {530,  1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 6 }, /* 7298 */
  {530,  0, 1, 0,-1, 0, 0, 0, 0, 1, 3, 3, 9 }, /* 7299 */
  {530,  0,-1, 0, 1, 0, 0, 0, 0,-1, 3, 3, 9 }, /* 7300 */
  {530,  1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 6 }, /* 7301 */
  {530, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 6 }, /* 7302 */
  {530,  0,-1, 0,-1, 0, 0, 0, 0,-1, 3, 3, 3 }, /* 7303 */
  {530,  0, 1, 0, 1, 0, 0, 0, 0, 1, 3, 3, 3 }, /* 7304 */
  {530, -1, 0, 0, 0, 1, 0, 0, 0,-1, 6, 0, 0 }, /* 7305 */
  {530,  1, 0, 0, 0,-1, 0, 0, 0, 1, 6, 0, 0 }, /* 7306 */
  {530,  0, 1, 0, 1, 0, 0, 0, 0,-1, 3, 9, 9 }, /* 7307 */
  {530,  0,-1, 0,-1, 0, 0, 0, 0, 1, 3, 9, 9 }, /* 7308 */
  {530,  0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, /* 7309 */
  {530,  0, 0,-1,-1, 0, 0, 0,-1, 0, 0, 0, 0 }, /* 7310 */
  {530,  0, 0, 1, 0,-1, 0, 1, 0, 0, 3, 3, 9 }, /* 7311 */
  {530,  0, 0,-1, 0, 1, 0,-1, 0, 0, 3, 3, 9 }, /* 7312 */
  {530,  0, 0, 1,-1, 0, 0, 0,-1, 0, 6, 6, 0 }, /* 7313 */
  {530,  0, 0,-1, 1, 0, 0, 0, 1, 0, 6, 6, 0 }, /* 7314 */
  {530,  0, 0, 1, 0, 1, 0,-1, 0, 0, 9, 3, 3 }, /* 7315 */
  {530,  0, 0,-1, 0,-1, 0, 1, 0, 0, 9, 3, 3 }, /* 7316 */
  {530,  0, 0,-1, 1, 0, 0, 0,-1, 0, 6, 0, 0 }, /* 7317 */
  {530,  0, 0, 1,-1, 0, 0, 0, 1, 0, 6, 0, 0 }, /* 7318 */
  {530,  0, 0,-1, 0,-1, 0,-1, 0, 0, 3, 3, 3 }, /* 7319 */
  {530,  0, 0, 1, 0, 1, 0, 1, 0, 0, 3, 3, 3 }, /* 7320 */
  {530,  0, 0,-1,-1, 0, 0, 0, 1, 0, 0, 6, 0 }, /* 7321 */
  {530,  0, 0, 1, 1, 0, 0, 0,-1, 0, 0, 6, 0 }, /* 7322 */
  {530,  0, 0,-1, 0, 1, 0, 1, 0, 0, 9, 3, 9 }, /* 7323 */
  {530,  0, 0, 1, 0,-1, 0,-1, 0, 0, 9, 3, 9 }, /* 7324 */
  {530,  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, /* 7325 */
  {530,  0,-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0 }, /* 7326 */
  {530,  1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 3, 3 }, /* 7327 */
  {530, -1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 3, 3 }, /* 7328 */
  {530,  0,-1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 6 }, /* 7329 */
  {530,  0, 1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 6 }, /* 7330 */
  {530, -1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 9, 3 }, /* 7331 */
  {530,  1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 9, 3 }, /* 7332 */
  {530,  0,-1, 0, 0, 0,-1, 1, 0, 0, 0, 6, 0 }, /* 7333 */
  {530,  0, 1, 0, 0, 0, 1,-1, 0, 0, 0, 6, 0 }, /* 7334 */
  {530, -1, 0, 0, 0, 0,-1, 0,-1, 0, 3, 3, 3 }, /* 7335 */
  {530,  1, 0, 0, 0, 0, 1, 0, 1, 0, 3, 3, 3 }, /* 7336 */
  {530,  0, 1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 6 }, /* 7337 */
  {530,  0,-1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 6 }, /* 7338 */
  {530,  1, 0, 0, 0, 0,-1, 0, 1, 0, 9, 9, 3 }, /* 7339 */
  {530, -1, 0, 0, 0, 0, 1, 0,-1, 0, 9, 9, 3 }, /* 7340 */
  {530,  1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 6 }, /* 7341 */
  {530, -1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 6 }, /* 7342 */
  {530,  0,-1, 0, 1, 0, 0, 0, 0, 1, 9, 3, 9 }, /* 7343 */
  {530,  0, 1, 0,-1, 0, 0, 0, 0,-1, 9, 3, 9 }, /* 7344 */
  {530, -1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 0 }, /* 7345 */
  {530,  1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 0 }, /* 7346 */
  {530,  0, 1, 0,-1, 0, 0, 0, 0, 1, 9, 9, 3 }, /* 7347 */
  {530,  0,-1, 0, 1, 0, 0, 0, 0,-1, 9, 9, 3 }, /* 7348 */
  {530,  1, 0, 0, 0,-1, 0, 0, 0,-1, 6, 6, 0 }, /* 7349 */
  {530, -1, 0, 0, 0, 1, 0, 0, 0, 1, 6, 6, 0 }, /* 7350 */
  {530,  0,-1, 0,-1, 0, 0, 0, 0,-1, 9, 9, 9 }, /* 7351 */
  {530,  0, 1, 0, 1, 0, 0, 0, 0, 1, 9, 9, 9 }, /* 7352 */
  {530, -1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 6, 6 }, /* 7353 */
  {530,  1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 6, 6 }, /* 7354 */
  {530,  0, 1, 0, 1, 0, 0, 0, 0,-1, 9, 3, 3 }, /* 7355 */
  {530,  0,-1, 0,-1, 0, 0, 0, 0, 1, 9, 3, 3 }, /* 7356 */
  {530,  0, 0, 1, 1, 0, 0, 0, 1, 0, 6, 6, 6 }, /* 7357 */
  {530,  0, 0,-1,-1, 0, 0, 0,-1, 0, 6, 6, 6 }, /* 7358 */
  {530,  0, 0, 1, 0,-1, 0, 1, 0, 0, 9, 9, 3 }, /* 7359 */
  {530,  0, 0,-1, 0, 1, 0,-1, 0, 0, 9, 9, 3 }, /* 7360 */
  {530,  0, 0, 1,-1, 0, 0, 0,-1, 0, 0, 0, 6 }, /* 7361 */
  {530,  0, 0,-1, 1, 0, 0, 0, 1, 0, 0, 0, 6 }, /* 7362 */
  {530,  0, 0, 1, 0, 1, 0,-1, 0, 0, 3, 9, 9 }, /* 7363 */
  {530,  0, 0,-1, 0,-1, 0, 1, 0, 0, 3, 9, 9 }, /* 7364 */
  {530,  0, 0,-1, 1, 0, 0, 0,-1, 0, 0, 6, 6 }, /* 7365 */
  {530,  0, 0, 1,-1, 0, 0, 0, 1, 0, 0, 6, 6 }, /* 7366 */
  {530,  0, 0,-1, 0,-1, 0,-1, 0, 0, 9, 9, 9 }, /* 7367 */
  {530,  0, 0, 1, 0, 1, 0, 1, 0, 0, 9, 9, 9 }, /* 7368 */
  {530,  0, 0,-1,-1, 0, 0, 0, 1, 0, 6, 0, 6 }, /* 7369 */
  {530,  0, 0, 1, 1, 0, 0, 0,-1, 0, 6, 0, 6 }, /* 7370 */
  {530,  0, 0,-1, 0, 1, 0, 1, 0, 0, 3, 9, 3 }, /* 7371 */
  {530,  0, 0, 1, 0,-1, 0,-1, 0, 0, 3, 9, 3 }, /* 7372 */
  {530,  0, 1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 6 }, /* 7373 */
  {530,  0,-1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 6 }, /* 7374 */
  {530,  1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 9, 9 }, /* 7375 */
  {530, -1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 9, 9 }, /* 7376 */
  {530,  0,-1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 0 }, /* 7377 */
  {530,  0, 1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 0 }, /* 7378 */
  {530, -1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 3, 9 }, /* 7379 */
  {530,  1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 3, 9 }, /* 7380 */
  {530,  0,-1, 0, 0, 0,-1, 1, 0, 0, 6, 0, 6 }, /* 7381 */
  {530,  0, 1, 0, 0, 0, 1,-1, 0, 0, 6, 0, 6 }, /* 7382 */
  {530, -1, 0, 0, 0, 0,-1, 0,-1, 0, 9, 9, 9 }, /* 7383 */
  {530,  1, 0, 0, 0, 0, 1, 0, 1, 0, 9, 9, 9 }, /* 7384 */
  {530,  0, 1, 0, 0, 0,-1,-1, 0, 0, 6, 6, 0 }, /* 7385 */
  {530,  0,-1, 0, 0, 0, 1, 1, 0, 0, 6, 6, 0 }, /* 7386 */
  {530,  1, 0, 0, 0, 0,-1, 0, 1, 0, 3, 3, 9 }, /* 7387 */
  {530, -1, 0, 0, 0, 0, 1, 0,-1, 0, 3, 3, 9 }, /* 7388 */
};

static const int symmetry_operation_index[][2] = {
  {   0,    0}, /*   0 */
  {   1,    1}, /*   1 */
  {   2,    2}, /*   2 */
  {   2,    4}, /*   3 */
  {   2,    6}, /*   4 */
  {   2,    8}, /*   5 */
  {   2,   10}, /*   6 */
  {   2,   12}, /*   7 */
  {   2,   14}, /*   8 */
  {   4,   16}, /*   9 */
  {   4,   20}, /*  10 */
  {   4,   24}, /*  11 */
  {   4,   28}, /*  12 */
  {   4,   32}, /*  13 */
  {   4,   36}, /*  14 */
  {   4,   40}, /*  15 */
  {   4,   44}, /*  16 */
  {   4,   48}, /*  17 */
  {   2,   52}, /*  18 */
  {   2,   54}, /*  19 */
  {   2,   56}, /*  20 */
  {   2,   58}, /*  21 */
  {   2,   60}, /*  22 */
  {   2,   62}, /*  23 */
  {   2,   64}, /*  24 */
  {   2,   66}, /*  25 */
  {   2,   68}, /*  26 */
  {   2,   70}, /*  27 */
  {   2,   72}, /*  28 */
  {   2,   74}, /*  29 */
  {   4,   76}, /*  30 */
  {   4,   80}, /*  31 */
  {   4,   84}, /*  32 */
  {   4,   88}, /*  33 */
  {   4,   92}, /*  34 */
  {   4,   96}, /*  35 */
  {   4,  100}, /*  36 */
  {   4,  104}, /*  37 */
  {   4,  108}, /*  38 */
  {   4,  112}, /*  39 */
  {   4,  116}, /*  40 */
  {   4,  120}, /*  41 */
  {   4,  124}, /*  42 */
  {   4,  128}, /*  43 */
  {   4,  132}, /*  44 */
  {   4,  136}, /*  45 */
  {   4,  140}, /*  46 */
  {   4,  144}, /*  47 */
  {   4,  148}, /*  48 */
  {   4,  152}, /*  49 */
  {   4,  156}, /*  50 */
  {   4,  160}, /*  51 */
  {   4,  164}, /*  52 */
  {   4,  168}, /*  53 */
  {   4,  172}, /*  54 */
  {   4,  176}, /*  55 */
  {   4,  180}, /*  56 */
  {   4,  184}, /*  57 */
  {   4,  188}, /*  58 */
  {   4,  192}, /*  59 */
  {   4,  196}, /*  60 */
  {   4,  200}, /*  61 */
  {   4,  204}, /*  62 */
  {   8,  208}, /*  63 */
  {   8,  216}, /*  64 */
  {   8,  224}, /*  65 */
  {   8,  232}, /*  66 */
  {   8,  240}, /*  67 */
  {   8,  248}, /*  68 */
  {   8,  256}, /*  69 */
  {   8,  264}, /*  70 */
  {   8,  272}, /*  71 */
  {   4,  280}, /*  72 */
  {   4,  284}, /*  73 */
  {   4,  288}, /*  74 */
  {   4,  292}, /*  75 */
  {   4,  296}, /*  76 */
  {   4,  300}, /*  77 */
  {   4,  304}, /*  78 */
  {   4,  308}, /*  79 */
  {   4,  312}, /*  80 */
  {   4,  316}, /*  81 */
  {   4,  320}, /*  82 */
  {   4,  324}, /*  83 */
  {   4,  328}, /*  84 */
  {   4,  332}, /*  85 */
  {   4,  336}, /*  86 */
  {   4,  340}, /*  87 */
  {   4,  344}, /*  88 */
  {   4,  348}, /*  89 */
  {   8,  352}, /*  90 */
  {   8,  360}, /*  91 */
  {   8,  368}, /*  92 */
  {   8,  376}, /*  93 */
  {   8,  384}, /*  94 */
  {   8,  392}, /*  95 */
  {   8,  400}, /*  96 */
  {   8,  408}, /*  97 */
  {   8,  416}, /*  98 */
  {   8,  424}, /*  99 */
  {   8,  432}, /* 100 */
  {   8,  440}, /* 101 */
  {   8,  448}, /* 102 */
  {   8,  456}, /* 103 */
  {   8,  464}, /* 104 */
  {   8,  472}, /* 105 */
  {   8,  480}, /* 106 */
  {   8,  488}, /* 107 */
  {   4,  496}, /* 108 */
  {   4,  500}, /* 109 */
  {   4,  504}, /* 110 */
  {   4,  508}, /* 111 */
  {   4,  512}, /* 112 */
  {   4,  516}, /* 113 */
  {   4,  520}, /* 114 */
  {   4,  524}, /* 115 */
  {   8,  528}, /* 116 */
  {   8,  536}, /* 117 */
  {   8,  544}, /* 118 */
  {   8,  552}, /* 119 */
  {   8,  560}, /* 120 */
  {   8,  568}, /* 121 */
  {  16,  576}, /* 122 */
  {   8,  592}, /* 123 */
  {   8,  600}, /* 124 */
  {   4,  608}, /* 125 */
  {   4,  612}, /* 126 */
  {   4,  616}, /* 127 */
  {   4,  620}, /* 128 */
  {   4,  624}, /* 129 */
  {   4,  628}, /* 130 */
  {   4,  632}, /* 131 */
  {   4,  636}, /* 132 */
  {   4,  640}, /* 133 */
  {   4,  644}, /* 134 */
  {   4,  648}, /* 135 */
  {   4,  652}, /* 136 */
  {   4,  656}, /* 137 */
  {   4,  660}, /* 138 */
  {   4,  664}, /* 139 */
  {   4,  668}, /* 140 */
  {   4,  672}, /* 141 */
  {   4,  676}, /* 142 */
  {   4,  680}, /* 143 */
  {   4,  684}, /* 144 */
  {   4,  688}, /* 145 */
  {   4,  692}, /* 146 */
  {   4,  696}, /* 147 */
  {   4,  700}, /* 148 */
  {   4,  704}, /* 149 */
  {   4,  708}, /* 150 */
  {   4,  712}, /* 151 */
  {   4,  716}, /* 152 */
  {   4,  720}, /* 153 */
  {   4,  724}, /* 154 */
  {   4,  728}, /* 155 */
  {   4,  732}, /* 156 */
  {   4,  736}, /* 157 */
  {   4,  740}, /* 158 */
  {   4,  744}, /* 159 */
  {   4,  748}, /* 160 */
  {   4,  752}, /* 161 */
  {   4,  756}, /* 162 */
  {   4,  760}, /* 163 */
  {   4,  764}, /* 164 */
  {   4,  768}, /* 165 */
  {   4,  772}, /* 166 */
  {   4,  776}, /* 167 */
  {   4,  780}, /* 168 */
  {   4,  784}, /* 169 */
  {   4,  788}, /* 170 */
  {   4,  792}, /* 171 */
  {   4,  796}, /* 172 */
  {   8,  800}, /* 173 */
  {   8,  808}, /* 174 */
  {   8,  816}, /* 175 */
  {   8,  824}, /* 176 */
  {   8,  832}, /* 177 */
  {   8,  840}, /* 178 */
  {   8,  848}, /* 179 */
  {   8,  856}, /* 180 */
  {   8,  864}, /* 181 */
  {   8,  872}, /* 182 */
  {   8,  880}, /* 183 */
  {   8,  888}, /* 184 */
  {   8,  896}, /* 185 */
  {   8,  904}, /* 186 */
  {   8,  912}, /* 187 */
  {   8,  920}, /* 188 */
  {   8,  928}, /* 189 */
  {   8,  936}, /* 190 */
  {   8,  944}, /* 191 */
  {   8,  952}, /* 192 */
  {   8,  960}, /* 193 */
  {   8,  968}, /* 194 */
  {   8,  976}, /* 195 */
  {   8,  984}, /* 196 */
  {   8,  992}, /* 197 */
  {   8, 1000}, /* 198 */
  {   8, 1008}, /* 199 */
  {   8, 1016}, /* 200 */
  {   8, 1024}, /* 201 */
  {   8, 1032}, /* 202 */
  {   8, 1040}, /* 203 */
  {   8, 1048}, /* 204 */
  {   8, 1056}, /* 205 */
  {   8, 1064}, /* 206 */
  {   8, 1072}, /* 207 */
  {   8, 1080}, /* 208 */
  {  16, 1088}, /* 209 */
  {  16, 1104}, /* 210 */
  {  16, 1120}, /* 211 */
  {  16, 1136}, /* 212 */
  {  16, 1152}, /* 213 */
  {  16, 1168}, /* 214 */
  {   8, 1184}, /* 215 */
  {   8, 1192}, /* 216 */
  {   8, 1200}, /* 217 */
  {   8, 1208}, /* 218 */
  {   8, 1216}, /* 219 */
  {   8, 1224}, /* 220 */
  {   8, 1232}, /* 221 */
  {   8, 1240}, /* 222 */
  {   8, 1248}, /* 223 */
  {   8, 1256}, /* 224 */
  {   8, 1264}, /* 225 */
  {   8, 1272}, /* 226 */
  {   8, 1280}, /* 227 */
  {   8, 1288}, /* 228 */
  {   8, 1296}, /* 229 */
  {   8, 1304}, /* 230 */
  {   8, 1312}, /* 231 */
  {   8, 1320}, /* 232 */
  {   8, 1328}, /* 233 */
  {   8, 1336}, /* 234 */
  {   8, 1344}, /* 235 */
  {   8, 1352}, /* 236 */
  {   8, 1360}, /* 237 */
  {   8, 1368}, /* 238 */
  {   8, 1376}, /* 239 */
  {   8, 1384}, /* 240 */
  {   8, 1392}, /* 241 */
  {   8, 1400}, /* 242 */
  {   8, 1408}, /* 243 */
  {   8, 1416}, /* 244 */
  {   8, 1424}, /* 245 */
  {   8, 1432}, /* 246 */
  {   8, 1440}, /* 247 */
  {   8, 1448}, /* 248 */
  {   8, 1456}, /* 249 */
  {   8, 1464}, /* 250 */
  {   8, 1472}, /* 251 */
  {   8, 1480}, /* 252 */
  {   8, 1488}, /* 253 */
  {   8, 1496}, /* 254 */
  {   8, 1504}, /* 255 */
  {   8, 1512}, /* 256 */
  {   8, 1520}, /* 257 */
  {   8, 1528}, /* 258 */
  {   8, 1536}, /* 259 */
  {   8, 1544}, /* 260 */
  {   8, 1552}, /* 261 */
  {   8, 1560}, /* 262 */
  {   8, 1568}, /* 263 */
  {   8, 1576}, /* 264 */
  {   8, 1584}, /* 265 */
  {   8, 1592}, /* 266 */
  {   8, 1600}, /* 267 */
  {   8, 1608}, /* 268 */
  {   8, 1616}, /* 269 */
  {   8, 1624}, /* 270 */
  {   8, 1632}, /* 271 */
  {   8, 1640}, /* 272 */
  {   8, 1648}, /* 273 */
  {   8, 1656}, /* 274 */
  {   8, 1664}, /* 275 */
  {   8, 1672}, /* 276 */
  {   8, 1680}, /* 277 */
  {   8, 1688}, /* 278 */
  {   8, 1696}, /* 279 */
  {   8, 1704}, /* 280 */
  {   8, 1712}, /* 281 */
  {   8, 1720}, /* 282 */
  {   8, 1728}, /* 283 */
  {   8, 1736}, /* 284 */
  {   8, 1744}, /* 285 */
  {   8, 1752}, /* 286 */
  {   8, 1760}, /* 287 */
  {   8, 1768}, /* 288 */
  {   8, 1776}, /* 289 */
  {   8, 1784}, /* 290 */
  {   8, 1792}, /* 291 */
  {   8, 1800}, /* 292 */
  {   8, 1808}, /* 293 */
  {   8, 1816}, /* 294 */
  {   8, 1824}, /* 295 */
  {   8, 1832}, /* 296 */
  {   8, 1840}, /* 297 */
  {  16, 1848}, /* 298 */
  {  16, 1864}, /* 299 */
  {  16, 1880}, /* 300 */
  {  16, 1896}, /* 301 */
  {  16, 1912}, /* 302 */
  {  16, 1928}, /* 303 */
  {  16, 1944}, /* 304 */
  {  16, 1960}, /* 305 */
  {  16, 1976}, /* 306 */
  {  16, 1992}, /* 307 */
  {  16, 2008}, /* 308 */
  {  16, 2024}, /* 309 */
  {  16, 2040}, /* 310 */
  {  16, 2056}, /* 311 */
  {  16, 2072}, /* 312 */
  {  16, 2088}, /* 313 */
  {  16, 2104}, /* 314 */
  {  16, 2120}, /* 315 */
  {  16, 2136}, /* 316 */
  {  16, 2152}, /* 317 */
  {  16, 2168}, /* 318 */
  {  16, 2184}, /* 319 */
  {  16, 2200}, /* 320 */
  {  16, 2216}, /* 321 */
  {  16, 2232}, /* 322 */
  {  16, 2248}, /* 323 */
  {  16, 2264}, /* 324 */
  {  16, 2280}, /* 325 */
  {  16, 2296}, /* 326 */
  {  16, 2312}, /* 327 */
  {  16, 2328}, /* 328 */
  {  16, 2344}, /* 329 */
  {  16, 2360}, /* 330 */
  {  16, 2376}, /* 331 */
  {  16, 2392}, /* 332 */
  {  16, 2408}, /* 333 */
  {  32, 2424}, /* 334 */
  {  32, 2456}, /* 335 */
  {  32, 2488}, /* 336 */
  {  16, 2520}, /* 337 */
  {  16, 2536}, /* 338 */
  {  16, 2552}, /* 339 */
  {  16, 2568}, /* 340 */
  {  16, 2584}, /* 341 */
  {  16, 2600}, /* 342 */
  {  16, 2616}, /* 343 */
  {  16, 2632}, /* 344 */
  {  16, 2648}, /* 345 */
  {  16, 2664}, /* 346 */
  {  16, 2680}, /* 347 */
  {  16, 2696}, /* 348 */
  {   4, 2712}, /* 349 */
  {   4, 2716}, /* 350 */
  {   4, 2720}, /* 351 */
  {   4, 2724}, /* 352 */
  {   8, 2728}, /* 353 */
  {   8, 2736}, /* 354 */
  {   4, 2744}, /* 355 */
  {   8, 2748}, /* 356 */
  {   8, 2756}, /* 357 */
  {   8, 2764}, /* 358 */
  {   8, 2772}, /* 359 */
  {   8, 2780}, /* 360 */
  {   8, 2788}, /* 361 */
  {   8, 2796}, /* 362 */
  {  16, 2804}, /* 363 */
  {  16, 2820}, /* 364 */
  {  16, 2836}, /* 365 */
  {   8, 2852}, /* 366 */
  {   8, 2860}, /* 367 */
  {   8, 2868}, /* 368 */
  {   8, 2876}, /* 369 */
  {   8, 2884}, /* 370 */
  {   8, 2892}, /* 371 */
  {   8, 2900}, /* 372 */
  {   8, 2908}, /* 373 */
  {  16, 2916}, /* 374 */
  {  16, 2932}, /* 375 */
  {   8, 2948}, /* 376 */
  {   8, 2956}, /* 377 */
  {   8, 2964}, /* 378 */
  {   8, 2972}, /* 379 */
  {   8, 2980}, /* 380 */
  {   8, 2988}, /* 381 */
  {   8, 2996}, /* 382 */
  {   8, 3004}, /* 383 */
  {  16, 3012}, /* 384 */
  {  16, 3028}, /* 385 */
  {  16, 3044}, /* 386 */
  {  16, 3060}, /* 387 */
  {   8, 3076}, /* 388 */
  {   8, 3084}, /* 389 */
  {   8, 3092}, /* 390 */
  {   8, 3100}, /* 391 */
  {   8, 3108}, /* 392 */
  {   8, 3116}, /* 393 */
  {   8, 3124}, /* 394 */
  {   8, 3132}, /* 395 */
  {  16, 3140}, /* 396 */
  {  16, 3156}, /* 397 */
  {  16, 3172}, /* 398 */
  {  16, 3188}, /* 399 */
  {  16, 3204}, /* 400 */
  {  16, 3220}, /* 401 */
  {  16, 3236}, /* 402 */
  {  16, 3252}, /* 403 */
  {  16, 3268}, /* 404 */
  {  16, 3284}, /* 405 */
  {  16, 3300}, /* 406 */
  {  16, 3316}, /* 407 */
  {  16, 3332}, /* 408 */
  {  16, 3348}, /* 409 */
  {  16, 3364}, /* 410 */
  {  16, 3380}, /* 411 */
  {  16, 3396}, /* 412 */
  {  16, 3412}, /* 413 */
  {  16, 3428}, /* 414 */
  {  16, 3444}, /* 415 */
  {  16, 3460}, /* 416 */
  {  16, 3476}, /* 417 */
  {  16, 3492}, /* 418 */
  {  16, 3508}, /* 419 */
  {  16, 3524}, /* 420 */
  {  16, 3540}, /* 421 */
  {  16, 3556}, /* 422 */
  {  16, 3572}, /* 423 */
  {  32, 3588}, /* 424 */
  {  32, 3620}, /* 425 */
  {  32, 3652}, /* 426 */
  {  32, 3684}, /* 427 */
  {  32, 3716}, /* 428 */
  {  32, 3748}, /* 429 */
  {   3, 3780}, /* 430 */
  {   3, 3783}, /* 431 */
  {   3, 3786}, /* 432 */
  {   9, 3789}, /* 433 */
  {   3, 3798}, /* 434 */
  {   6, 3801}, /* 435 */
  {  18, 3807}, /* 436 */
  {   6, 3825}, /* 437 */
  {   6, 3831}, /* 438 */
  {   6, 3837}, /* 439 */
  {   6, 3843}, /* 440 */
  {   6, 3849}, /* 441 */
  {   6, 3855}, /* 442 */
  {   6, 3861}, /* 443 */
  {  18, 3867}, /* 444 */
  {   6, 3885}, /* 445 */
  {   6, 3891}, /* 446 */
  {   6, 3897}, /* 447 */
  {   6, 3903}, /* 448 */
  {   6, 3909}, /* 449 */
  {  18, 3915}, /* 450 */
  {   6, 3933}, /* 451 */
  {  18, 3939}, /* 452 */
  {   6, 3957}, /* 453 */
  {  12, 3963}, /* 454 */
  {  12, 3975}, /* 455 */
  {  12, 3987}, /* 456 */
  {  12, 3999}, /* 457 */
  {  36, 4011}, /* 458 */
  {  12, 4047}, /* 459 */
  {  36, 4059}, /* 460 */
  {  12, 4095}, /* 461 */
  {   6, 4107}, /* 462 */
  {   6, 4113}, /* 463 */
  {   6, 4119}, /* 464 */
  {   6, 4125}, /* 465 */
  {   6, 4131}, /* 466 */
  {   6, 4137}, /* 467 */
  {   6, 4143}, /* 468 */
  {  12, 4149}, /* 469 */
  {  12, 4161}, /* 470 */
  {  12, 4173}, /* 471 */
  {  12, 4185}, /* 472 */
  {  12, 4197}, /* 473 */
  {  12, 4209}, /* 474 */
  {  12, 4221}, /* 475 */
  {  12, 4233}, /* 476 */
  {  12, 4245}, /* 477 */
  {  12, 4257}, /* 478 */
  {  12, 4269}, /* 479 */
  {  12, 4281}, /* 480 */
  {  12, 4293}, /* 481 */
  {  12, 4305}, /* 482 */
  {  12, 4317}, /* 483 */
  {  12, 4329}, /* 484 */
  {  24, 4341}, /* 485 */
  {  24, 4365}, /* 486 */
  {  24, 4389}, /* 487 */
  {  24, 4413}, /* 488 */
  {  12, 4437}, /* 489 */
  {  48, 4449}, /* 490 */
  {  24, 4497}, /* 491 */
  {  12, 4521}, /* 492 */
  {  24, 4533}, /* 493 */
  {  24, 4557}, /* 494 */
  {  24, 4581}, /* 495 */
  {  24, 4605}, /* 496 */
  {  96, 4629}, /* 497 */
  {  96, 4725}, /* 498 */
  {  96, 4821}, /* 499 */
  {  48, 4917}, /* 500 */
  {  24, 4965}, /* 501 */
  {  48, 4989}, /* 502 */
  {  24, 5037}, /* 503 */
  {  24, 5061}, /* 504 */
  {  96, 5085}, /* 505 */
  {  96, 5181}, /* 506 */
  {  48, 5277}, /* 507 */
  {  24, 5325}, /* 508 */
  {  24, 5349}, /* 509 */
  {  48, 5373}, /* 510 */
  {  24, 5421}, /* 511 */
  {  96, 5445}, /* 512 */
  {  48, 5541}, /* 513 */
  {  24, 5589}, /* 514 */
  {  96, 5613}, /* 515 */
  {  48, 5709}, /* 516 */
  {  48, 5757}, /* 517 */
  {  48, 5805}, /* 518 */
  {  48, 5853}, /* 519 */
  {  48, 5901}, /* 520 */
  {  48, 5949}, /* 521 */
  {  48, 5997}, /* 522 */
  { 192, 6045}, /* 523 */
  { 192, 6237}, /* 524 */
  { 192, 6429}, /* 525 */
  { 192, 6621}, /* 526 */
  { 192, 6813}, /* 527 */
  { 192, 7005}, /* 528 */
  {  96, 7197}, /* 529 */
  {  96, 7293}, /* 530 */
};

void spgdb_get_operation( int operation[13], const int index )
{
  int i;

  for ( i = 0; i < 13; i++ ) {
    operation[i] = symmetry_operations[index][i];
  }
}

void spgdb_get_operation_index( int indices[2], const int hall_number )
{
  indices[0] = symmetry_operation_index[ hall_number ][0];
  indices[1] = symmetry_operation_index[ hall_number ][1];
}

SpacegroupType spgdb_get_spacegroup_type( int index )
{
  return spacegroup_types[index];
}
