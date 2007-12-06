#!/bin/sh

RM="rm -f"

$RM CMakeCache.txt CMakeOutput.log config.log warning.log cmake.check_cache
$RM install_manifest.txt
$RM doc/config-doxygen warning.log cmake_uninstall.cmake gpltemplate

$RM -r krazy

for i in `find . -type d -name "CMakeFiles" -print | sort`; do
 	$RM -r $i;
	echo "$RM -r $i"
done
for i in `find . -type d -name "CMakeTmp" -print | sort`; do
 	$RM -r $i;
	echo "$RM -r $i"
done
for i in `find . -type d -name "*.dir" -print | sort`; do
 	$RM -r $i;
	echo "$RM -r $i"
done
for i in `find . -type d -name ".svn" -print | sort`; do
 	$RM -r $i;
	echo "$RM -r $i"
done
for i in `find . -type d -name ".git" -print | sort`; do
 	$RM -r $i;
	echo "$RM -r $i"
done

for i in `find . -type f -name "*~" -print | sort`; do
 	$RM $i;
	echo "$RM $i"
done
for i in `find . -type f -name "cmake_install.cmake" -print | sort`; do
 	$RM $i;
	echo "$RM $i"
done
for i in `find . -type f -name "Makefile" -print | sort`; do
 	$RM $i;
	echo "$RM $i"
done
for i in `find . -type f -name "*.obj" -print | sort`; do
 	$RM $i;
	echo "$RM $i"
done
for i in `find . -type f -name "*.o" -print | sort`; do
 	$RM $i;
	echo "$RM $i"
done
for i in `find . -type f -name "*.disabled" -print | sort`; do
 	$RM $i;
	echo "$RM $i"
done
