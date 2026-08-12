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

# Applications to be launched directly by user (create launch scripts)
KDE_MENU_APPS="\
digikam \
showfoto \
"

# Paths to search for applications above
KDE_APP_PATHS="\
Applications/digiKam.org \
"

# Other apps - non-MacOS binaries & libraries to be included with required dylibs

if [[ $TARGET_ARCH = "x86_64" ]] ; then
    QT_PLUGIN_EXT="so"
else
    QT_PLUGIN_EXT="dylib"
fi

# Note: lib/plugins/styles/ files are *.so in both architectures. Why this mess?

OTHER_APPS="\
lib/plugins/imageformats/*.$QT_PLUGIN_EXT \
lib/plugins/styles/*.so \
lib/plugins/digikam/bqm/*.so \
lib/plugins/digikam/generic/*.so \
lib/plugins/digikam/editor/*.so \
lib/plugins/digikam/dimg/*.so \
lib/plugins/digikam/rawimport/*.so \
lib/mariadb$MARIADB_SUFFIX/bin/mysql \
lib/mariadb$MARIADB_SUFFIX/bin/mysqld \
lib/mariadb$MARIADB_SUFFIX/bin/my_print_defaults \
lib/mariadb$MARIADB_SUFFIX/bin/mysqladmin \
lib/mariadb$MARIADB_SUFFIX/bin/mysqltest \
lib/mariadb$MARIADB_SUFFIX/mysql/*.dylib \
lib/mariadb$MARIADB_SUFFIX/plugin/*.so \
bin/kbuildsycoca$DK_QTVERSION \
bin/solid-hardware$DK_QTVERSION \
bin/ffmpeg \
libexec/qt$DK_QTVERSION/plugins/imageformats/*.dylib \
libexec/qt$DK_QTVERSION/plugins/sqldrivers/*.dylib \
libexec/qt$DK_QTVERSION/plugins/platforms/*.dylib \
libexec/qt$DK_QTVERSION/plugins/iconengines/*.dylib \
libexec/qt$DK_QTVERSION/plugins/generic/*.dylib \
libexec/qt$DK_QTVERSION/plugins/styles/*.dylib \
"

if [[ $DK_QTVERSION == 5 ]] ; then

    # Qt5 specific plugins

    OTHER_APPS=$OTHER_APPS"\
    libexec/qt$DK_QTVERSION/plugins/audio/*.dylib \
    libexec/qt$DK_QTVERSION/plugins/bearer/*.dylib \
    libexec/qt$DK_QTVERSION/plugins/printsupport/*.dylib \
    libexec/qt$DK_QTVERSION/plugins/platformthemes/*.dylib \
    "

else

    # Qt6 specific plugins

    OTHER_APPS=$OTHER_APPS"\
    libexec/qt$DK_QTVERSION/plugins/multimedia/*.dylib \
    libexec/qt$DK_QTVERSION/plugins/tls/*.dylib \
    "

fi

#lib/sane/*.so \

binaries="$OTHER_APPS"

# Additional Files/Directories - to be copied recursively but not checked for dependencies
#
# NOTE: dSYM directories are copied as well and cleaned later if debug symbols must be removed in final bundle.
#
#       QtWebEngine runtime process and binary archives (.pak) are located in libexec/qt$DK_QTVERSION/lib/QtWebEngineCore.framework/
#
OTHER_DIRS="\
libexec/qt$DK_QTVERSION/translations \
lib/libdigikam*.dSYM \
lib/plugins \
lib/libgphoto2 \
lib/libgphoto2_port \
lib/mariadb$MARIADB_SUFFIX \
lib/ImageMagick* \
share/mariadb$MARIADB_SUFFIX \
share/ImageMagick* \
etc/xdg \
etc/ImageMagick* \
etc/mariadb$MARIADB_SUFFIX \
"

if [[ $DK_QTVERSION == 6 ]] ; then

    OTHER_DIRS=$OTHER_DIRS"\
    libexec/qt$DK_QTVERSION/lib/QtWebEngineCore.framework \
    "

fi

# Additional Data Directories - to be copied recursively
OTHER_DATA="\
share/opencv4 \
share/k* \
share/mime \
Library/Application/ \
share/applications \
lensfun \
"

if [[ $DK_QTVERSION == 6 ]] ; then

    # NOTE: Qt5Curve is located in Library/Application

    OTHER_DATA=$OTHER_DATA"\
    share/QtCurve
    "

fi

# Packaging tool paths
PACKAGESBUILD="/usr/local/bin/packagesbuild"
RECURSIVE_LIBRARY_LISTER="$BUILDDIR/rll.py"

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
# Collect dylib dependencies for all binaries,
# then copy them to the staging area (creating directories as required)

echo "---------- Collecting dependencies for applications, binaries, and libraries..."

cd "$INSTALL_PREFIX"

"$RECURSIVE_LIBRARY_LISTER" $binaries | sort -u | \
while read lib ; do
    lib="`echo $lib | sed "s:$INSTALL_PREFIX/::"`"

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
    cp -aH "$INSTALL_PREFIX/$path" "$TEMPROOT/$dir/"
done

echo -e "------------- Move libgphoto2 drivers\n"

find "$TEMPROOT/lib/libgphoto2"      -name "*.so" -type f -exec mv {} "$TEMPROOT/lib/libgphoto2" \;
find "$TEMPROOT/lib/libgphoto2_port" -name "*.so" -type f -exec mv {} "$TEMPROOT/lib/libgphoto2_port" \;

echo "---------- Copying data files..."

# Special case with data dirs. QStandardPaths::GenericDataLocation was patched everywhere
# in source code by QStandardPaths::AppDataLocation

for path in $OTHER_DATA ; do
    echo "   Copying $path"
    cp -a "$INSTALL_PREFIX/$path" "$TEMPROOT/digikam.app/Contents/Resources/"
done

# Copy digiKam hi-colors PNG icons-set to the bundle

mkdir -p "$TEMPROOT/digikam.app/Contents/Resources/icons/"
cp -a "$INSTALL_PREFIX/share/icons/hicolor" "$TEMPROOT/digikam.app/Contents/Resources/icons/"

# Copy the Qt configuration ini file

if [[ $DK_QTVERSION = 5 ]] ; then

    cp $ORIG_WD/data/qt.conf.qt5 "$TEMPROOT/digikam.app/Contents/Resources/qt.conf"

else

    cp $ORIG_WD/data/qt.conf.qt6 "$TEMPROOT/digikam.app/Contents/Resources/qt.conf"

fi

echo "---------- Copying i18n..."

i18nprefix=$INSTALL_PREFIX/share/
cd $i18nprefix

FILES=$(cat $ORIG_WD/logs/build-extralibs.full.log | grep "$INSTALL_PREFIX/share/locale/" | cut -d' ' -f3  | awk '{sub("'"$i18nprefix"'","")}1')

for FILE in $FILES ; do
    rsync -R "./$FILE" "$TEMPROOT/digikam.app/Contents/Resources/"
done

FILES=$(cat $ORIG_WD/logs/build-digikam.full.log | grep "$INSTALL_PREFIX/share/locale/" | cut -d' ' -f3  | awk '{sub("'"$i18nprefix"'","")}1')

for FILE in $FILES ; do
    rsync -R "./$FILE" "$TEMPROOT/digikam.app/Contents/Resources/"
done

# Move Qt translation data files at the right place in the bundle. See Bug #438701.

mv -v $TEMPROOT/libexec/qt$DK_QTVERSION/translations $TEMPROOT/digikam.app/Contents/Resources/

# To support localized system menu entries from MacOS. See bug #432650.

FILES=$(find "$TEMPROOT/digikam.app/Contents/Resources/locale" -type d -depth 1)

for FILE in $FILES ; do
    BASE=$(basename $FILE)
    echo "Create localized system menu entry for $BASE"
    mkdir "$TEMPROOT/digikam.app/Contents/Resources/$BASE.lproj"
done

# Showfoto resources dir must be merged with digiKam.

cp -a "$TEMPROOT/showfoto.app/Contents/Resources/" "$TEMPROOT/digikam.app/Contents/Resources/"
rm -rf "$TEMPROOT/showfoto.app/Contents/Resources"

cd "$ORIG_WD"

#################################################################################################
# Move digiKam and KF run-time plugins to the right place

cp -a $TEMPROOT/lib/plugins $TEMPROOT/libexec/qt$DK_QTVERSION/
rm -rf $TEMPROOT/lib/plugins

# See bug #476290
rm -fr $TEMPROOT/libexec/qt$DK_QTVERSION/plugins/imageformats/libqjp2.dylib

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

# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Store script traces in a log file.

exec &> >(tee $HOME/digiKam_install.log) 2>&1

echo "digiKam ${DK_VERSION} installation done at $(date)."

echo "Pre-install operations:"

# This is the legacy install paths
# TODO: remove this legacy rules in the future

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

if [ -d /Applications/digiKam.org ] ; then
    echo "Removing digiKam.org from Applications folder"
    rm -r /Applications/digiKam.org
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

# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Store script traces in a log file.

exec &> >(tee -a $HOME/digiKam_install.log) 2>&1

echo "Post-install operations:"

# See bug https://bugs.kde.org/show_bug.cgi?id=496380

sudo codesign --verbose --force --deep --sign - /Applications/digiKam.org/digikam.app/Contents/MacOS/digikam
sudo codesign --verbose --force --deep --sign - /Applications/digiKam.org/showfoto.app/Contents/MacOS/showfoto
EOF

# Post-install script need to be executable

chmod 755 "$PROJECTDIR/postinstall"

#################################################################################################
# Copy icons-set resource files.

# See https://bugs.kde.org/show_bug.cgi?id=487799#c54

if [[ $DK_QTVERSION = 6 ]] ; then

    cp $INSTALL_PREFIX/share/icons/breeze/breeze-icons.rcc $TEMPROOT/digikam.app/Contents/Resources/breeze.rcc

else

    cp $ORIG_WD/../common/breeze.rcc                        $TEMPROOT/digikam.app/Contents/Resources/breeze.rcc
    cp $ORIG_WD/../common/breeze-dark.rcc                   $TEMPROOT/digikam.app/Contents/Resources/breeze-dark.rcc

fi

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

RelocatableBinaries EXECFILES[@]

echo -e "\n--- Patch RPATH in executable files"

for APP in ${EXECFILES[@]} ; do

    ISBINARY=`file "$APP" | grep "Mach-O" || true`

    # Do not touch debug extension
    ISDSYM=`file "$APP" | grep "dSYM" || true`

    # Do not patch applet files which are pure Apple API binaries
    BASENAME=`basename "$APP"`

    if [[ $ISBINARY ]] && [[ ! $ISDSYM ]] && [[ $BASENAME != "applet" ]] ; then

        install_name_tool -add_rpath @executable_path/.. $APP
        install_name_tool -add_rpath @executable_path/../.. $APP
        install_name_tool -add_rpath @executable_path/../../.. $APP
        install_name_tool -add_rpath @executable_path/../../../.. $APP
        install_name_tool -add_rpath @executable_path/../../../../.. $APP
        install_name_tool -add_rpath @executable_path/../../../../../.. $APP
        install_name_tool -add_rpath @executable_path/../../../../../../.. $APP
        install_name_tool -add_rpath @executable_path/../../../../../../../.. $APP
        install_name_tool -add_rpath @executable_path/../../../../../../../../.. $APP
        install_name_tool -add_rpath @executable_path/../../../../../../../../../.. $APP

        echo "Patch $APP"

    fi

done

#################################################################################################
# Final stage to manage file places in bundle

echo -e "\n---------- Finalize files in bundle"

mv -v $TEMPROOT/bin                           $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/share                         $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/etc                           $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/lib                           $TEMPROOT/digikam.app/Contents/
mv -v $TEMPROOT/libexec                       $TEMPROOT/digikam.app/Contents/

ln -sv "../../digikam.app/Contents/bin"       "$TEMPROOT/showfoto.app/Contents/bin"
ln -sv "../../digikam.app/Contents/etc"       "$TEMPROOT/showfoto.app/Contents/etc"
ln -sv "../../digikam.app/Contents/lib"       "$TEMPROOT/showfoto.app/Contents/lib"
ln -sv "../../digikam.app/Contents/libexec"   "$TEMPROOT/showfoto.app/Contents/libexec"
ln -sv "../../digikam.app/Contents/share"     "$TEMPROOT/showfoto.app/Contents/share"
ln -sv "../../digikam.app/Contents/Resources" "$TEMPROOT/showfoto.app/Contents/Resources"

# Specific plugin paths need to be linked in the bundle

# For digiKam

ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/designer"                      "$TEMPROOT/digikam.app/Contents/MacOS/designer"
ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/generic"                       "$TEMPROOT/digikam.app/Contents/MacOS/generic"
ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/imageformats"                  "$TEMPROOT/digikam.app/Contents/MacOS/imageformats"
ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/platforms"                     "$TEMPROOT/digikam.app/Contents/MacOS/platforms"
ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/styles"                        "$TEMPROOT/digikam.app/Contents/MacOS/styles"
ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/iconengines"                   "$TEMPROOT/digikam.app/Contents/MacOS/iconengines"
ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/sqldrivers"                    "$TEMPROOT/digikam.app/Contents/MacOS/sqldrivers"
ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/kf$DK_QTVERSION"               "$TEMPROOT/digikam.app/Contents/MacOS/kf$DK_QTVERSION"

if [[ $DK_QTVERSION = 6 ]] ; then

    ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/multimedia"                "$TEMPROOT/digikam.app/Contents/MacOS/multimedia"
    ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/tls"                       "$TEMPROOT/digikam.app/Contents/MacOS/tls"
    ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/kiconthemes$DK_QTVERSION"  "$TEMPROOT/digikam.app/Contents/MacOS/kiconthemes$DK_QTVERSION"

else

    # Qt5 only

    ln -sv "../../../digikam.app/Contents/libexec/qt$DK_QTVERSION/plugins/kauth"                     "$TEMPROOT/digikam.app/Contents/MacOS/kauth"

fi

# For Showfoto

ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/designer"                     "$TEMPROOT/showfoto.app/Contents/MacOS/designer"
ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/generic"                      "$TEMPROOT/showfoto.app/Contents/MacOS/generic"
ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/imageformats"                 "$TEMPROOT/showfoto.app/Contents/MacOS/imageformats"
ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/platforms"                    "$TEMPROOT/showfoto.app/Contents/MacOS/platforms"
ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/styles"                       "$TEMPROOT/showfoto.app/Contents/MacOS/styles"
ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/iconengines"                  "$TEMPROOT/showfoto.app/Contents/MacOS/iconengines"
ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/sqldrivers"                   "$TEMPROOT/showfoto.app/Contents/MacOS/sqldrivers"
ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/kf$DK_QTVERSION"              "$TEMPROOT/showfoto.app/Contents/MacOS/kf$DK_QTVERSION"

if [[ $DK_QTVERSION = 6 ]] ; then

    ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/multimedia"               "$TEMPROOT/showfoto.app/Contents/MacOS/multimedia"
    ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/tls"                      "$TEMPROOT/showfoto.app/Contents/MacOS/tls"
    ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/kiconthemes$DK_QTVERSION" "$TEMPROOT/showfoto.app/Contents/MacOS/kiconthemes$DK_QTVERSION"

else

    # Qt5 only

    ln -sv "../../../showfoto.app/Contents/libexec/qt$DK_QTVERSION/plugins/kauth"                     "$TEMPROOT/showfoto.app/Contents/MacOS/kauth"

fi

echo -e "\n---------- Cleanup files in bundle"

# Last cleanup

HEADERFILES=(`find $TEMPROOT -name "*.h" -o -name "*.hpp"`)

for HPP in ${HEADERFILES[@]} ; do

    rm -fv $HPP

done

rm -rfv $TEMPROOT/digikam.app/Contents/share/mariadb$MARIADB_SUFFIX/mysql-test
rm -rfv $TEMPROOT/digikam.app/Contents/share/mariadb$MARIADB_SUFFIX/sql-bench

echo -e "\n---------- Patch config and script files in bundle"

MARIADBDIRS=(`find $TEMPROOT -type d -name "mariadb$MARIADB_SUFFIX"`)

for DIR in ${MARIADBDIRS[@]} ; do

    MARIADBFILES=(`find $DIR -type f ! -name "*.dylib" ! -name "*.so"`)

    for FILE in ${MARIADBFILES[@]} ; do

        # to handle only text files
        ISTEXT=`file "$FILE" | grep "text" || true`

        if [[ $ISTEXT ]] ; then

            NEEDPATCH=`grep "$INSTALL_PREFIX" "$FILE" || true`

            if [[ $NEEDPATCH ]] ; then

                echo -e "--- Patching $FILE..."
                sed -i '' "s|$INSTALL_PREFIX/var|$RELOCATE_PREFIX/digikam.app/Contents/var|g" $FILE
                sed -i '' "s|$INSTALL_PREFIX/lib|$RELOCATE_PREFIX/digikam.app/Contents/lib|g" $FILE
                sed -i '' "s|$INSTALL_PREFIX/share|$RELOCATE_PREFIX/digikam.app/Contents/share|g" $FILE
                sed -i '' "s|$INSTALL_PREFIX/etc|$RELOCATE_PREFIX/digikam.app/Contents/etc|g" $FILE
                sed -i '' "s|$INSTALL_PREFIX|$RELOCATE_PREFIX/digikam.app/Contents/|g" $FILE

            fi

        fi

    done

done

#################################################################################################
# See bug #436624: move mariadb shared files at basedir (this must be done after patch operations)

rsync -a "$TEMPROOT/digikam.app/Contents/share/mariadb$MARIADB_SUFFIX" "$TEMPROOT/digikam.app/Contents/lib/mariadb$MARIADB_SUFFIX/share/"
rm -fr "$TEMPROOT/digikam.app/Contents/share/mariadb$MARIADB_SUFFIX"

# At run time, digiKam will check for mariadb folder-name without revision numbers.

ln -sv "../../../../../digikam.app/Contents/lib/mariadb$MARIADB_SUFFIX/share/mariadb$MARIADB_SUFFIX" "$TEMPROOT/digikam.app/Contents/lib/mariadb$MARIADB_SUFFIX/share/mariadb"
ln -sv "../../../digikam.app/Contents/lib/mariadb$MARIADB_SUFFIX"                                    "$TEMPROOT/digikam.app/Contents/lib/mariadb"
ln -sv "../../../digikam.app/Contents/etc/mariadb$MARIADB_SUFFIX"                                    "$TEMPROOT/digikam.app/Contents/etc/mariadb"

ln -sv "../../../../digikam.app/Contents/lib/mariadb$MARIADB_SUFFIX"                                 "$TEMPROOT/digikam.app/Contents/lib/mariadb/lib"
ln -sv "../../../../digikam.app/Contents/lib/mariadb"                                                "$TEMPROOT/digikam.app/Contents/lib/mariadb$MARIADB_SUFFIX"

#################################################################################################
# Install ExifTool binary.

cd $DOWNLOAD_DIR

#if [ ! -f "$DOWNLOAD_DIR/Image-ExifTool.tar.gz" ] ; then
    wget https://mirrors.xtom.de/kde-application/digikam/exiftool/Image-ExifTool.tar.gz -O Image-ExifTool.tar.gz
#fi

tar -xvf "$DOWNLOAD_DIR/Image-ExifTool.tar.gz" -C "$TEMPROOT/digikam.app/Contents/bin"
cd $TEMPROOT/digikam.app/Contents/bin
EXIFTOOL_DIR=$(ls -d Image-ExifTool*)
ln -sv "./$EXIFTOOL_DIR" "./Image-ExifTool"
ln -sv "./Image-ExifTool/exiftool" "exiftool"

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

TARGET_INSTALLER=digiKam-$DKRELEASEID$DK_SUBVER$QT_SUF-MacOS-$TARGET_ARCH$DEBUG_SUF.pkg
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

TerminateScript
