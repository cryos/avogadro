#!/bin/sh
#
# Language script for Cmake -- run a few sed commands to create a proper
# locversion.plist file
#
# Arguments:
# ${SRC_DIR} = this directory (which contains locversion.plist.in)
# ${MAC_LANG_DIR} = build directory for ${lang}.lproj
# ${VERSION}
# ${lang}

SRC_DIR=$1
MAC_LANG_DIR=$2
VERSION=$3
lang=$4

sed -e "s/LANG/${lang}/" <${SRC_DIR}/locversion.plist.in | \
  sed -e "s/VERSION/${VERSION}/" >${MAC_LANG_DIR}/locversion.plist