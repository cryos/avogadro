#!/bin/sh

# This can probably all be done in python, but this works

for x in i18n/*.po; do
# Finding e-mail addresses
# msggrep --no-wrap -w 20480 --msgctxt -e 'EMAIL OF TRANSLATORS' $x | grep msgstr | grep "@"
# Finding names
 msggrep --no-wrap -w 20480 --msgctxt -e 'NAME OF TRANSLATORS' $x | grep msgstr
done | python ./scripts/find-po-email.py | sort -b -f -u | grep -v "Launchpad Contributions"
