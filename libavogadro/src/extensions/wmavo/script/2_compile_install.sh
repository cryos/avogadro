#!/bin/bash

# Execution :
# Either :  $ bash 2_compile_install.sh


# Information about dependency.
echo "info: Verify the dependency with (1_dependency.sh) script"
echo "info:   before to continue the plugin compilation."
echo "info:   (Here the user account is enough.)"
echo "info:   (Press any button to continue, or press Ctl+C to abort.)"
read

which avogadro
if [ $? -ne 0 ] ; then
  echo "error: Avogadro seems not to exist ..."
  echo "error: Instalation aborted."
  exit 1
fi

version_avo=`avogadro --version | grep "^Avogadro:" | cut -d " " -f 2 | sed -re 's/\t//g'`

echo "info: Avogadro version:$version_avo"
version_avo_major=`echo $version_avo | cut -d "." -f 1`
version_avo_minor=`echo $version_avo | cut -d "." -f 2`
version_avo_patch=`echo $version_avo | cut -d "." -f 3`


login=`whoami`
echo "info: The current user is : $login."


if [ $login == "root" ] ; then

  if [[ $version_avo_major -le 0 ]] || [[ $version_avo_major -le 1  && $version_avo_minor -le 0 ]] ; then
    dir_lib="/usr/lib/avogadro/$version_avo_major""_$version_avo_minor/extensions/"
  else
    dir_lib="/usr/local/lib/avogadro/$version_avo_major""_$version_avo_minor/extensions/"
  fi
  
  echo "info: Be careful, you are root. The installation continues in"
  echo "info:   $dir_lib"
  echo "info:   (Press any button to continue, or press Ctl+C to abort.)"
  read
  
  if [ ! -d $dir_lib ] ; then
    echo "error: $dir_lib doesn't exist ..."
    echo "error: Instalation aborted."
    exit 1
  fi
  
else

  ## Copy (2nd version) the lib in user directory.
  dir_lib="/home/`whoami`/.avogadro"
  if [ ! -d $dir_lib ] ; then
    mkdir $dir_lib
  fi

  dir_lib="$dir_lib/$version_avo_major""_$version_avo_minor"
  if [ ! -d $dir_lib ] ; then
    mkdir $dir_lib
  fi

  dir_lib="$dir_lib/plugins"
  if [ ! -d $dir_lib ] ; then
    mkdir $dir_lib
  fi

  dir_lib="$dir_lib/extensions/"
  if [ ! -d $dir_lib ] ; then
    mkdir $dir_lib
  fi
fi

echo ""
echo "info: Part II: Compilation and installation of wiimote plugins"
echo ""

cd ../src/
build_dir="build_auto"

for dir in {en,ex,tool} ; do

  echo ""
  echo "info: Compiling src/$dir directory"

  # if source directory noexisting ?
  if [ ! -d $dir ] ; then
    echo "error: The directory $dir is not present !"
    echo "error: Instalation aborted."
    exit 1
  fi

  cd $dir

  # if build directory existing ?
  if [ -d $build_dir ] ; then
    echo ""
    echo "info: Clean up src/$dir/$build_dir directory"
    rm -r $build_dir
  fi

  echo ""
  echo "info: Create src/$dir/$build_dir directory"
  mkdir $build_dir
  cd $build_dir

  echo ""  
  echo "info: Cmake ($dir)"
  cmake -G"Unix Makefiles" ../
  #if cmake error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during cmake for src/$dir/$dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  echo ""
  echo "info: Make ($dir)"
  #sudo make clean
  make
  #if make error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  if [ $dir == "en" ] ; then
    dir_type="engines"
  fi
  if [ $dir == "ex" ] ; then
    dir_type="extensions"  
  fi
  if [ $dir == "tool" ] ; then
    dir_type="tools"
  fi
    

  
  ok=0
  while [ $ok -eq 0 ] ; do
  
    echo ""
    echo "info: The $dir_type plugin will be copied in:"
    echo $dir_lib
    echo "info: This repertory seems ok ? (y/n) (y:default value)"
    read a
    
    if [ "$a" != "" ] ; then
      # You don't just pressed Enter.
      
      if [ $a != "y" ] ; then
        echo "info: Enter your desired path:"
        read dir_lib
      fi
    fi
    
    cp libwm*.so $dir_lib
    #if cp error ...
    if [ $? -ne 0 ] ; then
      echo "error: Directory $dir_lib causes problem during copy"
    else
      ok=1
    fi
  done
  
  cd ../../

done

cd ../script
echo "info: Process finished with SUCCESS!"
exit 0
