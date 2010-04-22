#!/bin/sh

echo "Stripping obsolete PO translations"
for x in *.po; do
    msgattrib --no-obsolete -o ${x}.new ${x}
    mv ${x}.new ${x}
done

