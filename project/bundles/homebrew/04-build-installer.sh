#! /bin/bash

# Script to bundle data using previously-built digiKam installation.
# and create a PKG file with Packages application (http://s.sudre.free.fr/Software/Packages/about.html)
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015      by Shanti, <listaccount at revenant dot org>
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-installer.full.log) 2>&1

#################################################################################################

echo "04-build-installer.sh : build digiKam bundle PKG."
echo "-------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksRunAsRoot
ChecksXCodeCLI
ChecksCPUCores
OsxCodeName
#RegisterRemoteServers

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:$ORIG_PATH

DKRELEASEID=`cat $ORIG_WD/data/RELEASEID.txt`

#################################################################################################
# Configurations

# Directory where this script is located (default - current directory)
BUILDDIR="$PWD"

# Directory where Packages project files are located
PROJECTDIR="$BUILDDIR/installer"

# Staging area where files to be packaged will be copied
TEMPROOT="$BUILDDIR/$RELOCATE_PREFIX"

DK_APP_CONTENTS="digikam.app/Contents"

# Applications to be launched directly by user (create launch scripts)
KDE_MENU_APPS="\
digikam \
showfoto \
"

# Paths to search for applications above
KDE_APP_PATHS="\
Applications/digiKam.org \
"
#bin \

QT_FRAMEWORKS="\
QtQuickTimeline \
Qt3DInput \
QtQuick3DAssetUtils \
QtWebEngineCore \
QtDesigner \
QtWebEngineQuick \
QtQuick3DAssetImport \
QtWebEngineWidgets \
QtQuickWidgets \
QtShaderTools \
Qt3DQuickScene2D \
QtOpcUa \
QtPdf \
Qt3DRender \
QtQuick3DRuntimeRender \
QtGui \
QtDBus \
QtQuick3DUtils \
QtPositioningQuick \
Qt3DCore \
QtLocation \
QtXml \
QtSerialPort \
QtWebView \
QtQuick \
QtScxml \
QtCore \
QtQml \
Qt3DExtras \
QtWebChannel \
QtMultimedia \
QtVirtualKeyboard \
QtOpenGL \
QtQmlCompiler \
QtOpenGLWidgets \
Qt3DQuick \
QtWidgets \
QtPositioning \
QtQuick3D \
Qt3DLogic \
QtQuickShapes \
QtNetwork \
QtSvg \
QtWebChannelQuick \
QtQmlModels \
QtSensors \
Qt3DAnimation \
QtTextToSpeech \
QtWebViewQuick \
QtSerialBus \
QtConcurrent \
QtNetworkAuth \
QtPrintSupport \
QtStateMachine \
QtSvgWidgets \
QtMultimediaWidgets \
QtSql \
QtWebEngineCore \
QtMultimediaWidgets \
QtNetworkAuth \
QtPrintSupport \
QtStateMachine \
QtSvgWidgets \
"
# Other apps - non-MacOS binaries & libraries to be included with required dylibsQT_PLUGIN_EXT

if [[ $ARCH_TARGET = "x86_64" ]] ; then
    QT_PLUGIN_EXT="so"
else
    QT_PLUGIN_EXT="dylib"
fi

# Note: lib/plugins/styles/ files are *.so in both architectures. Why this mess?
MARIADB_SOURCE_DIR="opt/mariadb"

OTHER_APPS="\
share/qt/plugins/imageformats/*.dylib \
share/qt/plugins/styles/*.dylib \
share/qt/plugins/digikam/bqm/*.so \
share/qt/plugins/digikam/generic/*.so \
share/qt/plugins/digikam/editor/*.so \
share/qt/plugins/digikam/dimg/*.so \
share/qt/plugins/digikam/rawimport/*.so \
bin/kbuildsycoca$DK_QTVERSION \
bin/solid-hardware$DK_QTVERSION \
bin/ffmpeg \
bin/hunspell \
opt/qt$DK_QTVERSION/share/qt/plugins/imageformats/*.dylib \
opt/qt$DK_QTVERSION/share/qt/plugins/sqldrivers/*.dylib \
opt/qt$DK_QTVERSION/share/qt/plugins/platforms/*.dylib \
opt/qt$DK_QTVERSION/share/qt/plugins/iconengines/*.dylib \
opt/qt$DK_QTVERSION/share/qt/plugins/generic/*.dylib \
opt/qt$DK_QTVERSION/share/qt/plugins/styles/*.dylib \
opt/qt$DK_QTVERSION/share/qt/plugins/multimedia/*.dylib \
opt/qt$DK_QTVERSION/share/qt/plugins/tls/*.dylib \
lib/libhunspell*.dylib \
opt/openssl@3/lib/*.dylib \
"

if [[ $ARCH_TARGET = "x86_64" ]] ; then
    OTHER_APPS="$OTHER_APPS \
    opt/zstd/lib/*.dylib \
    "
fi
#lib/plugins/imageformats/*.dylib \


#opt/qt-mariadb/share/qt/plugins/sqldrivers/*.dylib \
#lib/sane/*.so \
# opt/mariadb/bin/mysql \
# opt/mariadb/bin/mysqld \
# opt/mariadb/bin/my_print_defaults \
# opt/mariadb/bin/mysqladmin \
# opt/mariadb/bin/mysqltest \
# opt/mariadb/lib/*.dylib \
# opt/mariadb/lib/plugin/*.so \

binaries="$OTHER_APPS"

# Additional Files/Directories - to be copied recursively but not checked for dependencies
#
# NOTE: dSYM directories are copied as well and cleaned later if debug symbols must be removed in final bundle.
#
#       QtWebEngine runtime process and binary archives (.pak) are located in libexec/qt$DK_QTVERSION/lib/QtWebEngineCore.framework/
#
OTHER_DIRS="\
share/qt/translations \
share/qt/plugins \
lib/libdigikam*.dSYM \
Cellar/libgphoto2/2.5.31/lib \
lib/ImageMagick* \
share/ImageMagick* \
etc/ImageMagick* \
"
# Cellar/qt/6.7.0_2/lib/QtWebEngineCore.framework/Versions/A/Helpers/QtWebEngineProcess.app \
# Cellar/qt/6.7.0_2/lib/QtWebEngineCore.framework/Versions/A/Resources \

# etc/my.cnf
# etc/my.cnf.default
# etc/my.cnf.d
# etc/xdg \
# etc/mariadb$MARIADB_SUFFIX \

#etc/sane.d \

# Additional Data Directories - to be copied recursively
OTHER_DATA="\
share/opencv4 \
share/k* \
share/mime \
Library/Application/ \
share/applications \
share/lensfun \
share/QtCurve \
"
#share/locale \

# Packaging tool paths
PACKAGESBUILD="/usr/local/bin/packagesbuild"

echo "digiKam version: $DKRELEASEID"

# ./installer sub-dir must be writable by root
chmod 777 ${PROJECTDIR}

#################################################################################################
# Check if Packages CLI tools are installed

if [[ (! -f "$PACKAGESBUILD") ]] ; then
    echo "Packages CLI tool is not installed"
    echo "See http://s.sudre.free.fr/Software/Packages/about.html for details."
    exit 1
else
    echo "Check Packages CLI tool passed..."
fi

#################################################################################################
# clean up old cache files
if [ -f "$ORIG_WD/found_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/found_dylibs_cache.pkl"
fi

if [ -f "$ORIG_WD/processed_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/processed_dylibs_cache.pkl"
fi
if [ -f "$ORIG_WD/signed_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/signed_dylibs_cache.pkl"
fi

#################################################################################################
# Create temporary dir to build package contents

if [ -d "$TEMPROOT" ] ; then
    echo "---------- Removing temporary packaging directory $TEMPROOT"
    rm -rf "$TEMPROOT"
fi

echo "Creating $TEMPROOT"
mkdir -p "$TEMPROOT"

#################################################################################################
# Prepare applications for MacOS

echo "---------- Preparing Applications for MacOS"

for app in $KDE_MENU_APPS ; do
    echo "  $app"

    # Look for application

    for searchpath in $KDE_APP_PATHS ; do

        # Copy the application if it is found (create directory if necessary)

        if [ -d "$INSTALL_PREFIX/$searchpath/$app.app" ] ; then

            echo "    Found $app in $INSTALL_PREFIX/$searchpath"

            # Copy application directory

            echo "    Copying $app"
            cp -pr "$INSTALL_PREFIX/$searchpath/$app.app" "$TEMPROOT"

            # Add executable to list of binaries for which we need to collect dependencies for

            binaries="$binaries $searchpath/$app.app/Contents/MacOS/$app"

            chmod 755 "$TEMPROOT/$app.app"

        fi

    done

done

#################################################################################################
# Copy Qt Frameworks to the staging area (creating directories as required)

echo "---------- Copying Qt frameworks..."

rm -rf $INSTALL_PREFIX/Cellar/qt/.DS_Store || true
mkdir $TEMPROOT/Frameworks
QT_FULL_VERSION=`ls "$INSTALL_PREFIX/Cellar/qt/"`

for framework in $QT_FRAMEWORKS ; do

    echo "Copying Qt framework: $framework"

    # Copy framework
    cp -R "$INSTALL_PREFIX/Cellar/qt/$QT_FULL_VERSION/Frameworks/$framework.framework" "$TEMPROOT/Frameworks"
    cp -R "$INSTALL_PREFIX/Cellar/qt/$QT_FULL_VERSION/lib/$framework.framework" "$TEMPROOT/lib"

done

#################################################################################################
# Collect dylib dependencies for all binaries,
# then copy them to the staging area (creating directories as required)

echo "---------- Collecting dependencies for applications, binaries, and libraries..."

cd "$INSTALL_PREFIX"

binary_paths=$(ls $binaries | sort -u)

for bin in $binary_paths ; do

    echo "Scan dependencies for $bin"

    # Copy original with parsing

    lib="${bin/$INSTALL_PREFIX\//}"

    if [ ! -e "$TEMPROOT/$lib" ] ; then
        dir="${lib%/*}"

        if [ ! -d "$TEMPROOT/$dir" ] ; then
            echo "  Creating $TEMPROOT/$dir"
            mkdir -p "$TEMPROOT/$dir"
        fi

        echo "  Copying $INSTALL_PREFIX/$lib to $TEMPROOT/$dir/"
        cp -aH "$INSTALL_PREFIX/$lib" "$TEMPROOT/$dir/"
    fi
done

#################################################################################################
# Copy non-binary files and directories, creating parent directories if needed

echo "---------- Copying binary files..."

for path in $OTHER_APPS ; do
    dir="${path%/*}"

    if [ ! -d "$TEMPROOT/$dir" ] ; then
        echo "  Creating $TEMPROOT/$dir"
        mkdir -p "$TEMPROOT/$dir"
    fi

    echo "  Copying $INSTALL_PREFIX/$path to $TEMPROOT/$dir/"
    cp -aH "$INSTALL_PREFIX/$path" "$TEMPROOT/$dir/"
done

echo "---------- Copying directory contents..."

for path in $OTHER_DIRS ; do
    dir="${path%/*}"

    if [ ! -d "$TEMPROOT/$dir" ] ; then
        echo "  Creating $TEMPROOT/$dir"
        mkdir -p "$TEMPROOT/$dir"
    fi

    echo "   Copying $INSTALL_PREFIX/$path to $TEMPROOT/$dir/"
    cp -aHL "$INSTALL_PREFIX/$path" "$TEMPROOT/$dir/"
done

# See bug #476290
#rm -fr $TEMPROOT/libexec/qt$DK_QTVERSION/plugins/imageformats/libqjp2.dylib

echo "---------- Copying data files..."

# Special case with data dirs. QStandardPaths::GenericDataLocation was patched everywhere
# in source code by QStandardPaths::AppDataLocation

for path in $OTHER_DATA ; do
    echo "   Copying $path"
    cp -aL "$INSTALL_PREFIX/$path" "$TEMPROOT/$DK_APP_CONTENTS/Resources/"
done

# Copy digiKam hi-colors PNG icons-set to the bundle

mkdir -p "$TEMPROOT/digikam.app/Contents/Resources/icons/"
cp -a "$INSTALL_PREFIX/share/icons/hicolor" "$TEMPROOT/$DK_APP_CONTENTS/Resources/icons/"

echo "---------- Copying i18n..."

i18nprefix=$INSTALL_PREFIX/share/
cd $i18nprefix

FILES=$(cat $ORIG_WD/logs/build-extralibs.full.log | grep "$INSTALL_PREFIX/share/locale/" | cut -d' ' -f3  | awk '{sub("'"$i18nprefix"'","")}1')

for FILE in $FILES ; do
    rsync -R "./$FILE" "$TEMPROOT/$DK_APP_CONTENTS/Resources/"
done

FILES=$(cat $ORIG_WD/logs/build-digikam.full.log | grep "$INSTALL_PREFIX/share/locale/" | cut -d' ' -f3  | awk '{sub("'"$i18nprefix"'","")}1')

for FILE in $FILES ; do
    rsync -R "./$FILE" "$TEMPROOT/$DK_APP_CONTENTS/Resources/"
done

# Move Qt translation data files at the right place in the bundle. See Bug #438701.

mv -v $TEMPROOT/share/qt/translations $TEMPROOT/$DK_APP_CONTENTS/Resources/

# To support localized system menu entries from MacOS. See bug #432650.

FILES=$(find "$TEMPROOT/$DK_APP_CONTENTS/Resources/locale" -type d -depth 1)

for FILE in $FILES ; do
    BASE=$(basename $FILE)
    echo "Create localized system menu entry for $BASE"
    mkdir "$TEMPROOT/$DK_APP_CONTENTS/Resources/$BASE.lproj"
done

# Showfoto resources dir must be merged with digiKam.

cp -a "$TEMPROOT/showfoto.app/Contents/Resources/" "$TEMPROOT/$DK_APP_CONTENTS/Resources/"
rm -rf "$TEMPROOT/showfoto.app/Contents/Resources"

cd "$ORIG_WD"

#################################################################################################
# Move digiKam and KF6 run-time plugins to the right place

echo "---------- Move digiKam and KF6 run-time plugins\n"

rm -fr $TEMPROOT/bin/digikam.app
rm -fr $TEMPROOT/bin/showfoto.app

# cp -a  $TEMPROOT/lib/plugins/         $TEMPROOT/share/qt/plugins
# rm -rf $TEMPROOT/lib/plugins/

#################################################################################################
# Copy MariaDB 

echo "---------- Copying MariaDB to bundle\n"

rm -rf $INSTALL_PREFIX/Cellar/mariadb/.DS_Store || true
MARIADB_VERSION=`ls "$INSTALL_PREFIX/Cellar/mariadb"`

echo "MariaDB Version is: - $MARIADB_VERSION -"

mkdir $TEMPROOT/lib/mariadb

rsync -av --exclude="include/*" --exclude="lib/plugin/ha_*" --exclude="support-files/*" --exclude="share/man/*" $INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/ $TEMPROOT/lib/mariadb


#################################################################################################
# Merge Manifest files

echo "---------- Copy Git Revisions Manifest\n"

touch $TEMPROOT/digikam.app/Contents/Resources/MANIFEST.txt

FILES=$(ls $ORIG_WD/data/*_manifest.txt)

for FILE in $FILES ; do
    echo $FILE
    cat $FILE >> $TEMPROOT/digikam.app/Contents/Resources/MANIFEST.txt
done

#################################################################################################
# Create package pre-install script

echo "---------- Create package pre-install script"

# Delete /Applications entries, delete existing installation

cat << EOF > "$PROJECTDIR/preinstall"
#!/bin/bash

if [ -d /Applications/digiKam ] ; then
    echo "Removing digiKam directory from Applications folder"
    rm -r /Applications/digiKam
fi

if [ -d /Applications/digikam.app ] ; then
    echo "Removing legacy digikam.app from Applications folder"
    rm -r /Applications/digikam.app
fi

if [ -d /Applications/showfoto.app ] ; then
    echo "Removing legacy showfoto.app from Applications folder"
    rm -r /Applications/showfoto.app
fi

if [ -d "/opt/digikam" ] ; then
    echo "Removing legacy /opt/digikam"
    rm -rf "/opt/digikam"
fi
EOF

# Pre-install script need to be executable

chmod 755 "$PROJECTDIR/preinstall"

#################################################################################################
# Create package post-install script

echo "---------- Create package post-install script"

# Creates Applications menu icons

cat << EOF > "$PROJECTDIR/postinstall"
#!/bin/bash

# NOTE: Disabled with relocate bundle
#for app in $INSTALL_PREFIX/Applications/digiKam/*.app ; do
#    ln -s "\$app" /Applications/\${app##*/}
#done
EOF

# Post-install script need to be executable

chmod 755 "$PROJECTDIR/postinstall"

#################################################################################################
# Copy icons-set resource files.

cp $INSTALL_PREFIX/share/icons/breeze/breeze-icons.rcc           $TEMPROOT/$DK_APP_CONTENTS/Resources/breeze.rcc

#################################################################################################
# Cleanup symbols in binary files to free space.

echo -e "\n---------- Strip symbols in binary files\n"

if [[ $DK_DEBUG = 1 ]] ; then

    find $TEMPROOT -name "*.so"    | grep -Ev '(digikam|showfoto)' | xargs strip -SXx
    find $TEMPROOT -name "*.dylib" | grep -Ev '(digikam|showfoto)' | xargs strip -SXx

else

    # no debug symbols at all.

    find $TEMPROOT -perm +111 -type f | xargs strip -SXx 2>/dev/null

    # Under MacOS, all debug symbols are stored to separated dSYM sub directories near binary files.

    DSYMDIRS=`find $TEMPROOT -type d -name "*.dSYM"`

    for dsym in $DSYMDIRS ; do

        rm -fr $dsym
        echo "Remove debug symbols from $dsym"

    done

fi

if [[ $DK_DEBUG = 1 ]] ; then

    DEBUG_SUF="-debug"

fi

#################################################################################################
# Move directories and fix misc files

echo -e "\n---------- Move and fix files"

# Clean up OpenSSL duplicates
rm $TEMPROOT/opt/openssl@3/lib/libcrypto.dylib
rm $TEMPROOT/opt/openssl@3/lib/libssl.dylib
# fix ssl links
ln -s "../opt/openssl@3/lib/libcrypto.3.dylib" "$TEMPROOT/lib/crypto"
ln -s "../opt/openssl@3/lib/libssl.3.dylib" "$TEMPROOT/lib/ssl"

mv -v $TEMPROOT/bin                           $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/share                         $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/etc                           $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/lib                           $TEMPROOT/digikam.app/Contents/
#mv -v $TEMPROOT/libexec                       $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/opt                           $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/Frameworks                    $TEMPROOT/digikam.app/Contents/
if [ ! -d $TEMPROOT/Cellar ] ; then
    mkdir $TEMPROOT/Cellar
fi
mv -v $TEMPROOT/Cellar                        $TEMPROOT/digikam.app/Contents/

ln -sv "../../digikam.app/Contents/bin"        "$TEMPROOT/showfoto.app/Contents/bin"
ln -sv "../../digikam.app/Contents/etc"        "$TEMPROOT/showfoto.app/Contents/etc"
ln -sv "../../digikam.app/Contents/lib"        "$TEMPROOT/showfoto.app/Contents/lib"
#ln -sv "../../digikam.app/Contents/libexec"   "$TEMPROOT/showfoto.app/Contents/libexec"
ln -sv "../../digikam.app/Contents/share"      "$TEMPROOT/showfoto.app/Contents/share"
ln -sv "../../digikam.app/Contents/Resources"  "$TEMPROOT/showfoto.app/Contents/Resources"
ln -sv "../../digikam.app/Contents/opt"        "$TEMPROOT/showfoto.app/Contents/opt"
ln -sv "../../digikam.app/Contents/Frameworks" "$TEMPROOT/showfoto.app/Contents/Frameworks"
ln -sv "../../digikam.app/Contents/Cellar"     "$TEMPROOT/showfoto.app/Contents/Cellar"
ln -sv "../../digikam.app/Contents/plugins"    "$TEMPROOT/showfoto.app/Contents/plugins"

ln -sv "../lib" $TEMPROOT/digikam.app/Contents/opt/lib

rm -r "$TEMPROOT/Applications"

#################################################################################################
# Relocatable binary files. For details, see these urls:
#
# https://stackoverflow.com/questions/9263256/can-you-please-help-me-understand-how-mach-o-libraries-work-in-mac-os-x
# https://matthew-brett.github.io/docosx/mac_runtime_link.html
# https://thecourtsofchaos.com/2013/09/16/how-to-copy-and-relink-binaries-on-osx/

echo -e "\n---------- Relocatable binary files"

# Relocatable dynamic libraries with rpath

echo -e "\n--- Relocatable dynamic library files"

DYLIBFILES=(`find $TEMPROOT -name "*.dylib"`)

RelocatableBinaries DYLIBFILES[@]

# Relocatable system objects with rpath

echo -e "\n--- Relocatable system object files"

SOFILES=(`find $TEMPROOT -name "*.so"`)

RelocatableBinaries SOFILES[@]

# Relocatable executables with rpath and patch relative search path.

echo -e "\n--- Relocatable executable files"

EXECFILES=(`find $TEMPROOT -type f -perm +ugo+x ! -name "*.dylib" ! -name "*.so"`)
#EXECFILES=(`find $TEMPROOT -type f ! -name "*.dylib" ! -name "*.so" -print0 | xargs -0 -n 10 file | grep "Mach-O" | cut -d ':' -f 1`)

RelocatableBinaries EXECFILES[@]

# echo -e "\n--- Patch RPATH in executable files"

# for APP in ${EXECFILES[@]} ; do

#     ISBINARY=`file "$APP" | grep "Mach-O" || true`

#     # Do not touch debug extension
#     ISDSYM=`file "$APP" | grep "dSYM" || true`

#     # Do not patch applet files which are pure Apple API binaries
#     BASENAME=`basename "$APP"`

#     if [[ $ISBINARY ]] && [[ ! $ISDSYM ]] && [[ $BASENAME != "applet" ]] ; then

#         install_name_tool -add_rpath @executable_path/.. $APP
#         install_name_tool -add_rpath @executable_path/../.. $APP
# #        install_name_tool -add_rpath @executable_path/../../.. $APP
# #        install_name_tool -add_rpath @executable_path/../../../.. $APP
# #        install_name_tool -add_rpath @executable_path/../../../../.. $APP
# #        install_name_tool -add_rpath @executable_path/../../../../../.. $APP
# #        install_name_tool -add_rpath @executable_path/../../../../../../.. $APP
# #        install_name_tool -add_rpath @executable_path/../../../../../../../.. $APP
# #        install_name_tool -add_rpath @executable_path/../../../../../../../../.. $APP
# #        install_name_tool -add_rpath @executable_path/../../../../../../../../../.. $APP
#         codesign --force -s - $APP

#         echo "Patch $APP"

#     fi

# done

# #################################################################################################
# # fix ImageMagick config
# sed -i '' "s|$INSTALL_PREFIX|..|g" $TEMPROOT/$DK_APP_CONTENTS/lib/ImageMagick/config-Q16HDRI/configure.xml

#################################################################################################
# fix the Qt directories and copy the qt.conf file

echo -e "\n---------- Adjusting Qt"

# Copy qt.conf
cp "$ORIG_WD/data/qt.conf" "$TEMPROOT/$DK_APP_CONTENTS/Resources/qt.conf"
#QT_FULL_VERSION=`ls "$TEMPROOT/$DK_APP_CONTENTS/Cellar/qt"`

# symlink Qt dirs
ln -s "./Resources/QtCurve" "$TEMPROOT/$DK_APP_CONTENTS/share/QtCurve"
#ln -s "../Cellar/qt/$QT_FULL_VERSION/lib/QtWebEngineCore.framework/Versions/A/Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess" "$TEMPROOT/$DK_APP_CONTENTS/MacOS/QtWebEngineProcess"
ln -s "./share/qt/plugins"  "$TEMPROOT/$DK_APP_CONTENTS/plugins"
#ln -s "../../Cellar/qt/$QT_FULL_VERSION/lib/QtWebEngineCore.framework/Versions/A/Resources/qtwebengine_locales" "$TEMPROOT/$DK_APP_CONTENTS/Resources/translations/qtwebengine_locales"

# add additional rpaths and re-sign QtWebEngineProcess
WEP=`find $TEMPROOT/$DK_APP_CONTENTS -name "QtWebEngineProcess"`
install_name_tool -add_rpath @loader_path/../../../../../../../../ $WEP
codesign --force -s - $WEP

# move the QtWebEngine resources
echo -e "\n     ----- Moving QtWebEngine Resources"

#WEBENGINE_RESOURCES=`find "$TEMPROOT/$DK_APP_CONTENTS/Cellar/qt/$QT_FULL_VERSION/lib/QtWebEngineCore.framework/Versions/A/Resources" -name "*.*" -maxdepth 1 ! -name '*.plist'`
# WEBENGINE_RESOURCES=`find "$TEMPROOT/$DK_APP_CONTENTS/Frameworks/QtWebEngineCore.framework/Versions/A/Resources" -name "*.*" -maxdepth 1 ! -name '*.plist'`

# for WER in $WEBENGINE_RESOURCES ; do

#     echo "Moving WER: $WER"
#     mv "$WER" "$TEMPROOT/$DK_APP_CONTENTS/Resources"

# done

#################################################################################################
# configure MySQL/MariaDB 

echo -e "\n     ----- Rebasing MariaDB"

# clean up old cache files
if [ -f "$ORIG_WD/found_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/found_dylibs_cache.pkl"
fi

if [ -f "$ORIG_WD/processed_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/processed_dylibs_cache.pkl"
fi
if [ -f "$ORIG_WD/signed_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/signed_dylibs_cache.pkl"
fi

# move MariaDB and patch up links to libs
cp $INSTALL_PREFIX/Library/Application/digikam/database/mysql-global.conf "$TEMPROOT/$DK_APP_CONTENTS/Resources/digikam/database/mysql-global.conf"
mkdir $TEMPROOT/$DK_APP_CONTENTS/lib/mariadb/opt
# link missing libs instead of copying them
ln -s "../../../opt/openssl@3" "$TEMPROOT/$DK_APP_CONTENTS/lib/mariadb/opt/openssl@3"
ln -s "../../../opt/pcre2" "$TEMPROOT/$DK_APP_CONTENTS/lib/mariadb/opt/pcre2"
ln -s "../../../opt/lz4" "$TEMPROOT/$DK_APP_CONTENTS/lib/mariadb/opt/lz4"
ln -s "../../../opt/snappy" "$TEMPROOT/$DK_APP_CONTENTS/lib/mariadb/opt/snappy"
ln -s "../../../opt/zstd" "$TEMPROOT/$DK_APP_CONTENTS/lib/mariadb/opt/zstd"

# rebase the MariaDB binaries to the MariaDB dir
# find the binaries
MARIADB_BINARIES=`find $TEMPROOT/$DK_APP_CONTENTS/lib/mariadb/bin -type f -perm +111`

for FILE in $MARIADB_BINARIES ; do

        copy_lib="$INSTALL_PREFIX/bin/python3 $ORIG_WD/package_lib.py --file=$FILE --bundle-root=$TEMPROOT/$DK_APP_CONTENTS/lib/mariadb --homebrew=$INSTALL_PREFIX --processed-cache=update  --found-cache=update --signed-cache=update --update-binary=1 --copy=0 --preserve_rpath=0"
        eval "$copy_lib"

done

echo -e "\n---------- Patch config and script files in MariaDB"

MARIADBFILES=`find $TEMPROOT/$DK_APP_CONTENTS/lib/mariadb -type f ! -name "*.dylib" ! -name "*.so"`

for FILE in $MARIADBFILES ; do

    # to handle only text files
    ISTEXT=`file "$FILE" | grep -e "ASCII text" || true`

    if [[ $ISTEXT ]] ; then

        NEEDPATCH=`grep "$INSTALL_PREFIX" "$FILE" || true`

        if [[ $NEEDPATCH ]] ; then

            echo -e "--- Patching MariaDB text file: $FILE"
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/bin|\$DK_MARIADB_DIR/bin|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/var|\$DK_MARIADB_DIR/var|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/lib|\$DK_MARIADB_DIR/lib|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/libexec|\$DK_MARIADB_DIR/libexec|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/share|\$DK_MARIADB_DIR/share|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/etc|\$DK_MARIADB_DIR/etc|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX|\$DK_MARIADB_DIR|g" $FILE

        fi

    fi

   # to handle only Perl files
    ISPERLTEXT=`file "$FILE" | grep -e "Perl script text" || true`

    if [[ $ISPERLTEXT ]] ; then

        NEEDPATCH=`grep "$INSTALL_PREFIX" "$FILE" || true`

        if [[ $NEEDPATCH ]] ; then

            echo -e "--- Patching mariaDB Perl file: $FILE"
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/bin|\$ENV{DK_MARIADB_DIR}/bin|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/var|\$ENV{DK_MARIADB_DIR}/var|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/lib|\$ENV{DK_MARIADB_DIR}/lib|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/libexec|\$ENV{DK_MARIADB_DIR}/libexec|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/share|\$ENV{DK_MARIADB_DIR}/share|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX/Cellar/mariadb/$MARIADB_VERSION/etc|\$ENV{DK_MARIADB_DIR}/etc|g" $FILE
            sed -i '' "s|$INSTALL_PREFIX|\$ENV{DK_MARIADB_DIR}|g" $FILE

        fi

    fi

done

#################################################################################################
# Install ExifTool binary.

echo -e "\n---------- Install ExifTool"

cd $DOWNLOAD_DIR

#if [ ! -f "$DOWNLOAD_DIR/Image-ExifTool.tar.gz" ] ; then
    wget https://files.kde.org/digikam/exiftool/Image-ExifTool.tar.gz -O Image-ExifTool.tar.gz
#fi

tar -xvf "$DOWNLOAD_DIR/Image-ExifTool.tar.gz" -C "$TEMPROOT/digikam.app/Contents/bin"
cd $TEMPROOT/digikam.app/Contents/bin
EXIFTOOL_DIR=$(ls -d Image-ExifTool*)
ln -sv "./$EXIFTOOL_DIR" "./Image-ExifTool"
ln -sv "./Image-ExifTool/exiftool" "exiftool"

#################################################################################################
# Cleanup files in bundle

echo -e "\n---------- Cleanup files in bundle"

# Last cleanup

HEADERFILES=(`find $TEMPROOT -name "*.h" -o -name "*.hpp"`)

for HPP in ${HEADERFILES[@]} ; do

    rm -fv $HPP

done

#################################################################################################
# Build PKG file

echo "---------- Create MacOS package for digiKam $DKRELEASEID"

mkdir -p $ORIG_WD/bundle
rm -f $ORIG_WD/bundle/*$DEBUG_SUF* || true

if [[ $DK_VERSION != v* ]] ; then

    # with non-official release version, use build time-stamp as sub-version string.
    DK_SUBVER="-`cat $ORIG_WD/data/BUILDDATE.txt`"

else

    # with official release version, disable upload to KDE server, as this break check for new version function.
    echo -e "Official release version detected, upload is disabled.\n"
    DK_UPLOAD=0

fi

if [[ $DK_QTVERSION == 5 ]] ; then

    QT_SUF="-Qt5"

else

    QT_SUF="-Qt6"

fi

TARGET_INSTALLER=digiKam-$DKRELEASEID$DK_SUBVER$QT_SUF-MacOS-$ARCH_TARGET$DEBUG_SUF.pkg
TARGET_PKG_FILE=$BUILDDIR/bundle/$TARGET_INSTALLER
echo -e "Target PKG file : $TARGET_PKG_FILE"

chmod 666 $PROJECTDIR/digikam.pkgproj

$PACKAGESBUILD -v "$PROJECTDIR/digikam.pkgproj" --package-version "$DKRELEASEID"

mv "$PROJECTDIR/build/digikam.pkg" "$TARGET_PKG_FILE"

#################################################################################################
# Show resume information and future instructions to host PKG file to remote server

echo -e "\n---------- Compute package checksums for digiKam $DKRELEASEID\n" >  $TARGET_PKG_FILE.sum
echo    "File       : $TARGET_PKG_FILE"                                     >> $TARGET_PKG_FILE.sum
echo -n "Size       : "                                                     >> $TARGET_PKG_FILE.sum
du -h "$TARGET_PKG_FILE"        | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum
echo -n "SHA256 sum : "                                                     >> $TARGET_PKG_FILE.sum
shasum -a256 "$TARGET_PKG_FILE" | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum

# Checksums to post on Phabricator at release time.
shasum -a256 "$TARGET_PKG_FILE" > $BUILDDIR/bundle/sha256_release.sum

if [[ $DK_SIGN = 1 ]] ; then

    cat ~/.gnupg/dkorg-gpg-pwd.txt | gpg --batch --yes --passphrase-fd 0 -sabv "$TARGET_PKG_FILE"
    mv -f $TARGET_PKG_FILE.asc $TARGET_PKG_FILE.sig

    echo    "File       : $TARGET_PKG_FILE.sig"                                     >> $TARGET_PKG_FILE.sum
    echo -n "Size       : "                                                         >> $TARGET_PKG_FILE.sum
    du -h "$TARGET_PKG_FILE.sig"        | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum
    echo -n "SHA256 sum : "                                                         >> $TARGET_PKG_FILE.sum
    shasum -a256 "$TARGET_PKG_FILE.sig" | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum

    # Checksums to post on Phabricator at release time.
    shasum -a256 "$TARGET_PKG_FILE.sig" >> $BUILDDIR/bundle/sha256_release.sum

fi

cat $TARGET_PKG_FILE.sum

if [[ $DK_UPLOAD = 1 ]] ; then

    echo -e "---------- Cleanup older bundle Package files from files.kde.org repository \n"

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm *-MacOS*$DEBUG_SUF.pkg*"

    echo -e "---------- Upload new bundle Package files to files.kde.org repository \n"

    rsync -r -v --progress -e ssh $BUILDDIR/bundle/$TARGET_INSTALLER     $DK_UPLOADURL:$DK_UPLOADDIR

    if [[ $DK_SIGN = 1 ]] ; then
        scp $BUILDDIR/bundle/$TARGET_INSTALLER.sig $DK_UPLOADURL:$DK_UPLOADDIR
    fi

    # update remote files list

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "ls digi*" > $ORIG_WD/bundle/ls.txt
    tail -n +2 $ORIG_WD/bundle/ls.txt > $ORIG_WD/bundle/ls.tmp
    cat $ORIG_WD/bundle/ls.tmp | grep -E '(.pkg |.appimage |.exe )' | grep -Ev '(debug)' > $ORIG_WD/bundle/FILES
    rm $ORIG_WD/bundle/ls.tmp
    rm $ORIG_WD/bundle/ls.txt
    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm FILES"
    rsync -r -v --progress -e ssh $BUILDDIR/bundle/FILES $DK_UPLOADURL:$DK_UPLOADDIR

else
    echo -e "\n------------------------------------------------------------------"
    curl https://download.kde.org/README_UPLOAD
    echo -e "------------------------------------------------------------------\n"
fi

#################################################################################################
# clean up old cache files
if [ -f "$ORIG_WD/found_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/found_dylibs_cache.pkl"
fi

if [ -f "$ORIG_WD/processed_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/processed_dylibs_cache.pkl"
fi
if [ -f "$ORIG_WD/signed_dylibs_cache.pkl" ] ; then
    rm "$ORIG_WD/signed_dylibs_cache.pkl"
fi

#################################################################################################

TerminateScript
