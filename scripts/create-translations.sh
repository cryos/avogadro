#!/bin/sh

sourcedir=${1-.}
lconvert=${2}

#if [ x"${lconvert}" != "x" ]; then

    for lang in `ls ${sourcedir}/avogadro*.po | cut -f 2 -d '-' | cut -f 1 -d '.'`; do
        ${lconvert} -i ${sourcedir}/avogadro-${lang}.po -o ./avogadro_${lang}.ts;
    done

    for lang in `ls ${sourcedir}/libavogadro*.po | cut -f 2 -d '-' | cut -f 1 -d '.'`; do
        ${lconvert} -i ${sourcedir}/libavogadro-${lang}.po -o ./libavogadro_${lang}.ts;
    done

#fi
