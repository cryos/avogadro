
The wiiuse and wiiusecpp directories contains all sources, examples 
and makefile to use them.
The last update of this source are supplied only with the wmavo plugin of
the WiiChem project. Some modification change the use, so be careful if you
want to use it for you.

The wiiuse/wiiusecpp compiled librairies are copied in the lib_* directories.

wiiuse_wiiusecpp/lib_nix64/libwiiuse*         : release version for "linux 64 bits"
wiiuse_wiiusecpp/lib_nix64/debug/libwiiuse*   : debug version for "linux 64 bits"
...

So if you want to modify/compily the wiiuse/wiiusecpp libraries, do not forgot :
  - to copy them in the correct directory (rename/create if necessary) ;
  - to change the cmake which use them (initialy expected just for Windows32
    (compiled with MSVC), Linux32/64 (compiled with gcc).

