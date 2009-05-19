#!/bin/sh

for lang in `ls avogadro*.po | cut -f 2 -d '-' | cut -f 1 -d '.'`; do
  lconvert -i avogadro-${lang}.po -o avogadro_${lang}.ts;
done

for lang in `ls libavogadro*.po | cut -f 2 -d '-' | cut -f 1 -d '.'`; do
  lconvert -i libavogadro-${lang}.po -o libavogadro_${lang}.ts;
done
