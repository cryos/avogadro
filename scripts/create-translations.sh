#!/bin/sh

sourcedir=${1-.}
lconvert=${2}

#if [ x"${lconvert}" != "x" ]; then

    for filepath in `ls ${sourcedir}/avogadro*.po`; do
        filename=`basename ${filepath}`
        lang=`echo ${filename} | cut -f 2 -d '-' | cut -f 1 -d '.'`
        ${lconvert} -i ${sourcedir}/avogadro-${lang}.po -o ./avogadro_${lang}.ts;
    done

    for filepath in `ls ${sourcedir}/libavogadro*.po`; do
       filename=`basename ${filepath}`
       lang=`echo ${filename} | cut -f 2 -d '-' | cut -f 1 -d '.'`
       ${lconvert} -i ${sourcedir}/libavogadro-${lang}.po -o ./libavogadro_${lang}.ts;
   done

#fi
