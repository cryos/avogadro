#!/bin/bash

# Execution :
# Either :  $ bash 1_dependency.sh


# Information about dependency.
echo "info: You must to be root."
echo "info: This script compiles and installs the wiiuse/wiiusecpp librairies,"
echo "info: and, install bluez and libbluetooth-dev dependencies."
echo "info: (Press any button to continue, or press Ctl+C to abort.)"
read

# Install bluetooth librairies.
echo "info: Install bluetooth librairies."
sudo apt-get install bluez libbluetooth-dev
if [ $? -ne 0 ] ; then
  echo "error: An error is appeared during the bluetooth librairies install."
  echo "error: Instalation aborted."
  exit 1
fi
echo ""
  
#if wiiuse/wiiusecpp libraries existing
# search the files ...
if [ -r /usr/lib/libwiiuse.so ] ; then
  echo "info: libwiiuse.so existing and accessible in /usr/lib/"
fi

if [ -r /usr/lib/libwiiusecpp.so ] ; then
  echo "info: libwiiusecpp.so existing and accessible in /usr/lib/"
fi

wiiuse_dir=../libavogadro/src/extensions/wmavo/extra/wiiuse_wiiusecpp

if [ ! -d $wiiuse_dir ] ; then
  echo "error: The wiiuse/wiisecpp directory is not present !"
  echo "error: $wiiuse_dir"
  echo "error: Instalation aborted."
  exit 1
fi

echo "info: cd $wiiuse_dir"
cd $wiiuse_dir

echo ""
echo "info: Compilation and installation of wiiuse/wiiusecpp librairies (even if existing)"
echo ""

for dir in {wiiuse,wiiusecpp} ; do

  echo ""
  echo "info: In $dir directory"

  # if source directory noexisting ?
  if [ ! -d $dir ] ; then
    echo "error: The directory $dir is not present !"
    echo "error: Instalation aborted."
    exit 1
  fi

  cd $dir/src/

  echo ""
  echo "info: Clean up $dir before the compilation"
  make distclean
  
  echo ""
  echo "info: Compilation of $dir"
  make
  #if make error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  echo ""
  echo "info: Installation (copy) of $dir in : /usr/lib/lib$dir.so"
  sudo make install
  #if make install error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make install for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  cd ../../

done

cd ../../../../../../scripts/

echo "info: Process finished. SUCCESS!"
exit 0

