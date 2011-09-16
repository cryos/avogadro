
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
  
  $ sudo bash 1_dependency.sh
  # => You must have root rigths.
  # It contains all information about dependencies you must install (apt-get ...).
  # It compiles wiiuse/wiiusecpp librairies to interpret the Wiimote communication.
  # And it copies some cml files.
  
  $ bash 2_compile_install.sh
  # => User rights are enough.
  # After the previous etap, you can compile the plugins
  # which automatically copy in the Avogadro plugin directory.
