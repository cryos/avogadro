#!/bin/sh

# This can probably all be done in python, but this works

for x in i18n/*.po; do
 msggrep --no-wrap -w 20480 --msgctxt -e 'EMAIL OF TRANSLATORS' $x | grep msgstr | grep "@"
done | python ./scripts/find-po-email.py | uniq
