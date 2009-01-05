#!/bin/sh
BASEDIR="../avogadro"	# root of translatable sources
PROJECT="avogadro"	# project name
PACKAGE="Avogadro"  # user-readable package name
# user-readable version
VERSION=`grep 'set(VERSION' libavogadro/CMakeLists.txt | cut -f 2 -d ' ' | cut -f 1 -d ')'`
BUGADDR="avogadro-devel@lists.sourceforge.net"	# MSGID-Bugs
WDIR=`pwd`		# working dir
I18NDIR="i18n"          # i18n dir

echo "Preparing rc files"
cd ${BASEDIR}
# we use simple sorting to make sure the lines do not jump around too much from system to system
find . -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > ${WDIR}/rcfiles.list
cat ${WDIR}/rcfiles.list | xargs ${WDIR}/scripts/extractrc.sh > ${WDIR}/rc.cpp
# additional string for KAboutData
echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> ${WDIR}/rc.cpp
echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> ${WDIR}/rc.cpp
cd ${WDIR}
echo "Done preparing rc files"
 
 
echo "Extracting messages"
cd ${BASEDIR}
# see above on sorting
find . -name '*.cpp' -o -name '*.h' -o -name '*.c' | sort > ${WDIR}/infiles.list
#echo "rc.cpp" >> ${WDIR}/infiles.list
cd ${WDIR}
xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 \
	-kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
  -ktr:1 -ktrUtf8:1 --qt \
  --package-name=${PACKAGE} --package-version=${VERSION} \
	--msgid-bugs-address="${BUGADDR}" --foreign-user --copyright-holder="The Avogadro Project" \
	--files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o ${PROJECT}.pot || { echo "error while calling xgettext. aborting."; exit 1; }
echo "Done extracting messages"

# Replace some boilerplate strings
sed -e "s/SOME DESCRIPTIVE TITLE/Translations for the Avogadro molecular editor/" <${PROJECT}.pot >${PROJECT}.new
mv ${PROJECT}.new ${PROJECT}.pot
sed -e 's/Copyright (C) YEAR/Copyright (C) 2006-2009/' <${PROJECT}.pot >${PROJECT}.new
mv ${PROJECT}.new ${PROJECT}.pot
sed -e 's/as the PACKAGE package/as the Avogadro package/' <${PROJECT}.pot >${PROJECT}.new
mv ${PROJECT}.new ${PROJECT}.pot

mv ${PROJECT}.pot ${I18NDIR}

cd ${I18NDIR} 
echo "Merging translations"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
  echo $cat
  msgmerge -o $cat.new $cat ${PROJECT}.pot
  mv $cat.new $cat
done
echo "Done merging translations"
 
 
echo "Cleaning up"
cd ${WDIR}
rm rcfiles.list
rm infiles.list
rm rc.cpp
echo "Done"
