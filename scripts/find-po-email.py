#!/usr/bin/env python

import sys, string

emails = []
for line in sys.stdin.readlines():
    cleanedLine = string.strip(string.strip(string.lstrip(line, "msgstr")), '"')
    for address in string.split(cleanedLine, ','):
        if (len(address)):
            emails.append(address)

emails.sort()
for email in emails:
    print email
