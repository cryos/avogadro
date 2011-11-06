
# Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
# Project managers: Eric Henon and Michael Krajecki
# Financial support: Region Champagne-Ardenne (Fr)

Compile/Install the WiiChem plugins for Avogadro on Linux.
info: The project name is WiiChem, all sources/directories using "wmavo" for "Wiimote for Avogadro".


An easy way to do it:

  
  $ cd [your_wmavo_dir]/
  $ ls 
    api   doc   extra   img   script    src   [...]
    
  $ cd script/
  
  # => You must have root rigths.
  $ sudo bash 1_dependency.sh
  # It contains all information about dependencies you must install (apt-get ...).
  # It compiles wiiuse/wiiusecpp librairies to interpret the Wiimote communication.
  # And it copies some cml files.
  
  # => You must have user rigths (NOT root).
  $ bash 2_compile_install.sh
  # After the previous etap, the plugins are compiled and
  # automatically copied in the Avogadro plugin directory.
