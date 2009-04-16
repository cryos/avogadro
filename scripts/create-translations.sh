#!/bin/sh

for lang in `ls *.po | cut -f 2 -d '-' | cut -f 1 -d '.'`; do
  lconvert -i avogadro-${lang}.po -o avogadro_${lang}.ts;
done

