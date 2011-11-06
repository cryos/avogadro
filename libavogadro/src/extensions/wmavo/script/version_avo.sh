#!/bin/bash

# Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
# Project managers: Eric Henon and Michael Krajecki
# Financial support: Region Champagne-Ardenne (Fr)


# Destined to test, not to use ...

which avog
if [ $? -ne 0 ] ; then
  echo "error: avog does not exist"
  echo ""
fi

version_avo=`avogadro --version | grep "^Avogadro:" | cut -d " " -f 2 | sed -re 's/\t//g'`
echo ""
echo "Avogadro version:$version_avo"

version_avo_major=`echo $version_avo | cut -d "." -f 1`
version_avo_minor=`echo $version_avo | cut -d "." -f 2`
version_avo_patch=`echo $version_avo | cut -d "." -f 3`

echo "Avogadro major version:$version_avo_major""."
echo "Avogadro minor version:$version_avo_minor""."
echo "Avogadro patch version:$version_avo_patch""."

if [[ $version_avo_major -le 0 ]] || [[ $version_avo_major -le 1  && $version_avo_minor -le 0 ]] ; then
  echo "Avogadro version is lesser or egualer than 1.0."
else
  echo "Avogadro version is greater than 1.0."
fi

if [[ $version_avo_major -le 1  && $version_avo_minor -le 0 ]] || [[ $version_avo_major -le 0 ]] ; then
  echo "Avogadro version is lesser or egualer than 1.0."
else
  echo "Avogadro version is greater than 1.0."                                                                                                                                                                           
fi


test="test"
if [ $test == "test" ] ; then
  echo "Test passed."
else
  echo "Test not passed."
fi

echo "Push 'y' button"
read a

echo "a:$a."

if [ "$a" == "y" ] ; then
  echo "You just pressed 'y'."
elif [ "$a" == "" ] ; then
  echo "You just pressed Enter only."
else
  echo "You just pressed an other key."
fi

exit 0

