#!/bin/sh

echo "Generating documentation .pot"
xml2po -e -o ../i18n/avogadro-doc.pot ../doc/index.docbook
