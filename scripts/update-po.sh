#!/bin/sh

echo "Updating PO translations from POT"
for x in *.po; do
    msgmerge -U $x *.pot
done

