#!/bin/bash

# Script to bundle data using previously-built KF5 with digiKam installation
# and create a Linux AppImage bundle file.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-appimage.full.log) 2>&1

#################################################################################################

echo "04-build-appimage.sh : build digiKam AppImage bundle."
echo "-----------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./common.sh
. ./config.sh
ChecksRunAsRoot
StartScript
ChecksCPUCores
ChecksPhyMemory
HostAdjustments
RegisterRemoteServers
CheckSystemReleaseID

#################################################################################################

# Working directory

ORIG_WD="`pwd`"

DK_RELEASEID=`cat $ORIG_WD/data/RELEASEID.txt`

if [[ "$OS_NAME" == "ubuntu" ]] ; then

    LIBSUFFIX="lib/x86_64-linux-gnu"

else

    LIBSUFFIX="lib64"

fi

#################################################################################################

echo -e "------------- Prepare directories in bundle\n"

# Make sure we build from the /, parts of this script depends on that. We also need to run as root...

cd /

# Prepare the install location

rm -rf $APP_IMG_DIR/ || true
mkdir -p $APP_IMG_DIR/usr/bin
mkdir -p $APP_IMG_DIR/usr/etc
mkdir -p $APP_IMG_DIR/usr/share
mkdir -p $APP_IMG_DIR/usr/share/icons
mkdir -p $APP_IMG_DIR/usr/share/X11
mkdir -p $APP_IMG_DIR/usr/share/metainfo
mkdir -p $APP_IMG_DIR/usr/share/applications
mkdir -p $APP_IMG_DIR/usr/share/dbus-1/interfaces
mkdir -p $APP_IMG_DIR/usr/share/dbus-1/services

# make sure lib and lib64 are the same thing

mkdir -p $APP_IMG_DIR/usr/lib
mkdir -p $APP_IMG_DIR/usr/lib/libexec
mkdir -p $APP_IMG_DIR/usr/lib/libgphoto2
mkdir -p $APP_IMG_DIR/usr/lib/libgphoto2_port
cd $APP_IMG_DIR/usr
ln -s lib lib64

#################################################################################################

echo -e "------------- Copy Files in bundle\n"

cd $APP_IMG_DIR

# FIXME: How to find out which subset of plugins is really needed? I used strace when running the binary

cp -r /usr/plugins ./usr/
rm -fr ./usr/plugins/ktexteditor
rm -fr ./usr/plugins/kf5/parts
rm -fr ./usr/plugins/konsolepart.so

# See bug #476290

rm -fr ./usr/plugins/imageformats/libqjp2.so

echo -e "------------- Copy runtime data files\n"

cp -r /usr/share/digikam                                  ./usr/share
cp -r /usr/share/showfoto                                 ./usr/share

# See https://bugs.kde.org/show_bug.cgi?id=487799#c54

if [[ $DK_QTVERSION = 6 ]] ; then

    cp /usr/share/icons/breeze/breeze-icons.rcc           ./usr/share/digikam/breeze.rcc

else

    cp $ORIG_WD/../common/breeze.rcc                      ./usr/share/digikam/breeze.rcc
    cp $ORIG_WD/../common/breeze-dark.rcc                 ./usr/share/digikam/breeze-dark.rcc

fi

cd $APP_IMG_DIR/usr/share/showfoto
ln -s ../digikam/breeze.rcc                               breeze.rcc

if [[ $DK_QTVERSION = 5 ]] ; then

    ln -s ../digikam/breeze-dark.rcc                      breeze-dark.rcc

fi

cd $APP_IMG_DIR
cp $ORIG_WD/data/qt.conf                                  ./usr/bin
cp -r /usr/lensfun                                        ./usr/share

if [[ $DK_QTVERSION == 5 ]] ; then

    cp -r /usr/share/knotifications5                      ./usr/share
    cp -r /usr/share/kservices5                           ./usr/share
    cp -r /usr/share/kservicetypes5                       ./usr/share
    cp -r /usr/share/kxmlgui5                             ./usr/share
    cp -r /usr/share/kf5                                  ./usr/share

else

    cp -r /usr/share/knotifications6                      ./usr/share

    # Not available with KF6

    cp -r /usr/share/kservices6                           ./usr/share  || true
    cp -r /usr/share/kservicetypes6                       ./usr/share  || true

    # KF6 still use kxmlgui5 subdir.

    cp -r /usr/share/kxmlgui5                             ./usr/share
    cp -r /usr/share/kf6                                  ./usr/share

    # WHY it's necessary, else application menu are broken...

    ln -s ./kxmlgui6                                      ./usr/share/kxmlgui5

fi

cp -r /usr/share/solid                                    ./usr/share
cp -r /usr/share/icu                                      ./usr/share
cp -r /usr/share/mime                                     ./usr/share

# See bug #480134: an up-to-date and clean freedesktop.org database is requires

cp $ORIG_WD/data/freedesktop.org.xml                      ./usr/share/mime/packages
rm -rf ./usr/share/mime/mime.cache

# depending of OpenCV version installed, data directory is not the same.

cp -r /usr/share/OpenCV                                   ./usr/share  || true
cp -r /usr/share/opencv4                                  ./usr/share  || true

# TODO check when kf6 prefix will be used here.

cp -r /usr/share/dbus-1/services/*kde*                    ./usr/share/dbus-1/services/

if [[ $DK_QTVERSION == 5 ]] ; then

    cp -r /usr/share/dbus-1/interfaces/kf5*               ./usr/share/dbus-1/interfaces/
    cp -r /usr/${LIBSUFFIX}/libexec/kf5                   ./usr/lib/libexec/

else

    cp -r /usr/share/dbus-1/interfaces/kf6*               ./usr/share/dbus-1/interfaces/
    cp -r /usr/${LIBSUFFIX}/libexec/kf6                   ./usr/lib/libexec/

fi

echo -e "------------- Copy AppImage stream data files\n"

cp -r /usr/share/metainfo/org.kde.digikam.appdata.xml     ./usr/share/metainfo
cp -r /usr/share/metainfo/org.kde.showfoto.appdata.xml    ./usr/share/metainfo

cp -r /usr/share/applications/org.kde.digikam.desktop     ./usr/share/applications
cp -r /usr/share/applications/org.kde.showfoto.desktop    ./usr/share/applications

echo -e "------------- Copy QWebEngine bin data files\n"

cp -r /usr/resources ./usr

echo -e "------------- Copy libgphoto2 drivers\n"

find  /usr/${LIBSUFFIX}/libgphoto2      -name "*.so" -type f -exec cp {} ./usr/lib/libgphoto2 \;      2>/dev/null
find  /usr/${LIBSUFFIX}/libgphoto2_port -name "*.so" -type f -exec cp {} ./usr/lib/libgphoto2_port \; 2>/dev/null

echo -e "------------- Copy sane backends\n"

cp -r /usr/${LIBSUFFIX}/sane                            ./usr/lib
cp -r /etc/sane.d                                       ./usr/etc

echo -e "------------- Copy I18n\n"

# Qt translations files

if [[ -e /usr/translations ]]; then

    echo -e "------------- Copy Qt translations files\n"

    cp -r /usr/translations ./usr
    ln -s ../../translations ./usr/share/digikam/translations
    ln -s ../../translations ./usr/share/showfoto/translations

    # optimizations

    rm -rf ./usr/translations/assistant*    || true
    rm -rf ./usr/translations/designer*     || true
    rm -rf ./usr/translations/linguist*     || true
    rm -rf ./usr/translations/qmlviewer*    || true
    rm -rf ./usr/translations/qtmultimedia* || true
    rm -rf ./usr/translations/qtscript*     || true
    rm -rf ./usr/translations/qtquick*      || true
    rm -rf ./usr/translations/qt_help*      || true
    rm -rf ./usr/translations/qtserialport* || true
    rm -rf ./usr/translations/qtwebsockets* || true

fi

echo -e "------------- Copy KDE translations files\n"

FILES=$(cat $ORIG_WD/logs/build-extralibs.full.log | grep /usr/share/locale | grep -e .qm -e .mo | cut -d' ' -f3)

for FILE in $FILES ; do

    echo $FILE
    cp --parents $FILE ./

done

echo -e "------------- Copy digiKam translations files\n"

FILES=$(cat $ORIG_WD/logs/build-digikam.full.log | grep /usr/share/locale | grep -e .qm -e .mo | cut -d' ' -f3)

for FILE in $FILES ; do

    echo $FILE
    cp --parents $FILE ./

done

echo -e "------------- Copy digiKam icons files\n"

FILES=$(cat $ORIG_WD/logs/build-digikam.full.log | grep /usr/share/icons/ | cut -d' ' -f3)

for FILE in $FILES ; do

    if [[ $FILE != "touch:" ]] ; then

        echo $FILE
        cp --parents $FILE ./

    fi

done

echo -e "------------- Copy Git Revisions Manifest\n"

touch ./usr/share/digikam/MANIFEST.txt

FILES=$(ls $ORIG_WD/data/*_manifest.txt)

for FILE in $FILES ; do

    echo $FILE
    cat $FILE >> ./usr/share/digikam/MANIFEST.txt

done

ln -s ../digikam/MANIFEST.txt           ./usr/share/showfoto/MANIFEST.txt || true

echo -e "------------- Copy system libraries for binary compatibility\n"

# otherwise segfaults!?

cp $(ldconfig -p | grep /${LIBSUFFIX}/libsasl2.so.2    | cut -d ">" -f 2 | xargs) ./usr/lib/
cp $(ldconfig -p | grep /${LIBSUFFIX}/libGL.so.1       | cut -d ">" -f 2 | xargs) ./usr/lib/
cp $(ldconfig -p | grep /${LIBSUFFIX}/libGLU.so.1      | cut -d ">" -f 2 | xargs) ./usr/lib/

# Fedora 23 seemed to be missing SOMETHING from the Centos 6.7. The only message was:
# This application failed to start because it could not find or load the Qt platform plugin "xcb".
# Setting export QT_DEBUG_PLUGINS=1 revealed the cause.
# QLibraryPrivate::loadPlugin failed on "/usr/lib64/qt5/plugins/platforms/libqxcb.so" :
# "Cannot load library /usr/lib64/qt5/plugins/platforms/libqxcb.so: (/lib64/libEGL.so.1: undefined symbol: drmGetNodeTypeFromFd)"
# Which means that we have to copy libEGL.so.1 in too

# Otherwise Fedora 23 cannot load the Qt platform plugin "xcb"

cp $(ldconfig -p | grep /${LIBSUFFIX}/libEGL.so.1      | cut -d ">" -f 2 | xargs) ./usr/lib/

# let's not copy xcb itself, that breaks on dri3 systems https://bugs.kde.org/show_bug.cgi?id=360552
#cp $(ldconfig -p | grep libxcb.so.1 | cut -d ">" -f 2 | xargs) ./usr/lib/

# For Fedora 20

cp $(ldconfig -p | grep /${LIBSUFFIX}/libfreetype.so.6 | cut -d ">" -f 2 | xargs) ./usr/lib/

echo -e "---------- Copy target binaries\n"

cp /usr/bin/digikam                 ./usr/bin
cp /usr/bin/showfoto                ./usr/bin

if [[ $DK_QTVERSION == 5 ]] ; then

    cp /usr/bin/kbuildsycoca5       ./usr/bin
    cp /usr/bin/solid-hardware5     ./usr/bin

else

    cp /usr/bin/kbuildsycoca6       ./usr/bin
    cp /usr/bin/solid-hardware6     ./usr/bin

fi

echo -e "------------- Copy QtWebEngine runtime process\n"

[[ -e /usr/libexec/QtWebEngineProcess ]] && cp /usr/libexec/QtWebEngineProcess ./usr/bin

echo -e "------------- Copy Solid binary\n"

# For Solid action when camera is connected to computer

cp /usr/bin/qdbus                   ./usr/share/digikam/utils
sed -i "/Exec=/c\Exec=digikam-camera downloadFromUdi %i" ./usr/share/solid/actions/digikam-opencamera.desktop

#################################################################################################

echo -e "------------- Scan dependencies recursively\n"

CopyReccursiveDependencies /usr/bin/digikam                  ./usr/lib
CopyReccursiveDependencies /usr/bin/showfoto                 ./usr/lib
CopyReccursiveDependencies /usr/plugins/platforms/libqxcb.so ./usr/lib

FILES=$(ls /usr/${LIBSUFFIX}/libdigikam*.so)

for FILE in $FILES ; do
    CopyReccursiveDependencies ${FILE} ./usr/lib
done

FILES=$(ls /usr/plugins/imageformats/*.so)

for FILE in $FILES ; do
    CopyReccursiveDependencies ${FILE} ./usr/lib
done

FILES=$(ls /usr/plugins/sqldrivers/*.so)

for FILE in $FILES ; do
    CopyReccursiveDependencies ${FILE} ./usr/lib
done

FILES=$(find /usr/plugins/digikam -name "*.so")

for FILE in $FILES ; do
    CopyReccursiveDependencies ${FILE} ./usr/lib
done

# Copy in the indirect dependencies

FILES=$(find . -type f -executable)

for FILE in $FILES ; do
    CopyReccursiveDependencies ${FILE} ./usr/lib
done

#################################################################################################

echo -e "---------- Clean-up Bundle Directory Contents\n"

# The following are assumed to be part of the base system

# This list is taken from linuxdeployqt
# [https://github.com/probonopd/linuxdeployqt/blob/master/tools/linuxdeployqt/excludelist.h]
# NOTES:
#   Libglapi is included explicitly in Krita exclude list.
#   With the transition from Mageia6 to 7 to build AppImage libnss* must be included in the bundle (see bug #440689)
#   With the transition from Mageia7 to Ubuntu 18.04 to build AppImage libnsl* must be included in the bundle (see bug #455922)

EXCLUDE_FILES="\
ld-linux.so.2 \
ld-linux-x86-64.so.2 \
libanl.so.1 \
libasound.so.2 \
libBrokenLocale.so.1 \
libcidn.so.1 \
libcom_err.so.2 \
libcrypt.so.1 \
libc.so.6 \
libdl.so.2 \
libdrm.so.2 \
libexpat.so.1 \
libfontconfig.so.1 \
libfreetype.so.6 \
libgcc_s.so.1 \
libgdk_pixbuf-2.0.so.0 \
libgio-2.0.so.0 \
libglapi.so.0 \
libglib-2.0.so.0 \
libGL.so.1 \
libgobject-2.0.so.0 \
libgpg-error.so.0 \
libharfbuzz.so.0 \
libICE.so.6 \
libjack.so.0 \
libm.so.6 \
libmvec.so.1 \
libp11-kit.so.0 \
libpangocairo-1.0.so.0 \
libpthread.so.0 \
libresolv.so.2 \
librt.so.1 \
libSM.so.6 \
libstdc++.so.6 \
libthai.so.0 \
libthread_db.so.1 \
libusb-1.0.so.0 \
libutil.so.1 \
libuuid.so.1 \
libX11.so.6 \
libxcb.so.1 \
"

for FILE in $EXCLUDE_FILES ; do

    if [[ -f usr/lib/${FILE} ]] ; then

        echo -e "   ==> ${FILE} will be removed from the bundle"
        rm -f usr/lib/${FILE}

    else

        echo -e "   ==> ${FILE} DO NOT EXISTS!!!"

    fi

done

# This list is taken from older AppImage build script from krita
# NOTE: libopenal    => see bug 390162.
#       libdbus-1    => see Krita rules.
#       libxcb-dri3  => see bug 417088.
#       libidn2.so.0 => Ubuntu 22.04

EXTRA_EXCLUDE_FILES="\
libgssapi_krb5.so.2 \
libgssapi.so.3 \
libhcrypto.so.4 \
libheimbase.so.1 \
libheimntlm.so.0 \
libhx509.so.5 \
libidn.so.11 \
libidn2.so.0 \
libk5crypto.so.3 \
libkrb5.so.26 \
libkrb5.so.3 \
libkrb5support.so.0 \
libroken.so.18 \
libsasl2.so.2 \
libwind.so.0 \
libopenal.so.1 \
libdbus-1.so.3 \
libxcb-dri3.so.0 \
"

#liblber-2.4.so.2       # needed for Debian Wheezy
#libldap_r-2.4.so.2     # needed for Debian Wheezy

#libffi.so.6            # needed for Ubuntu 11.04
#libxcb-glx.so.0        # needed for Ubuntu 11.04

#libkeyutils.so.1       # Originally removed in linuxdeployqt, but needed for Gentoo (see https://bugs.kde.org/show_bug.cgi?id=406171#c2)
#libz.so.1              # needed for Mint 18.1 (see http://digikam.1695700.n4.nabble.com/digikam-6-2-0-64-bit-appimage-error-td4708921.html)

#libpango-1.0.so.0
#libpangoft2-1.0.so.0

for FILE in $EXTRA_EXCLUDE_FILES ; do

    if [[ -f usr/lib/${FILE} ]] ; then

        echo -e "   ==> ${FILE} will be removed from the bundle"
        rm -f usr/lib/${FILE}

    else

        echo -e "   ==> ${FILE} DO NOT EXISTS!!!"

    fi

done

if [[ -f usr/lib/libssl.so.1.1 ]] ; then

    ln -s libssl.so.1.1 usr/lib/libssl.so || true

fi

# We don't bundle the developer files

rm -rf usr/include         || true
rm -rf usr/lib/cmake3      || true
rm -rf usr/lib/pkgconfig   || true
rm -rf usr/share/ECM/      || true
rm -rf usr/share/gettext   || true
rm -rf usr/share/pkgconfig || true

#################################################################################################
# See LFS instruction: http://www.linuxfromscratch.org/lfs/view/systemd/chapter05/stripping.html

echo -e "------------- Strip Symbols in Binaries Files\n"

if [[ $DK_DEBUG = 1 ]] ; then

    # Note: do not remove kService debug symbols until bug #480134 is not resolved.

    FILES=$(find . -type f  -exec file {} \; | grep ELF | grep -Ev '(digikam|showfoto|libKF5Service.so.5)' | cut -d':' -f1)
#    FILES=$(find . -type f -executable | grep -Ev '(digikam|showfoto)')

else

    FILES=$(find . -type f  -exec file {} \; | grep ELF | cut -d':' -f1)
#    FILES=$(find . -type f -executable)

fi

for FILE in $FILES ; do

    echo -en "Strip symbols in: $FILE\n"
    /usr/bin/strip --strip-all ${FILE} || true

done

#################################################################################################

echo -e "------------- Strip Configuration Files \n"

# Since we set $APP_IMG_DIR as the prefix, we need to patch it away too (FIXME)
# Probably it would be better to use /app as a prefix because it has the same length for all apps

cd usr/ ; find . -type f -exec sed -i -e 's|$APP_IMG_DIR/usr/|./././././././././|g' {} \; ; cd  ..

# On openSUSE Qt is picking up the wrong libqxcb.so
# (the one from the system when in fact it should use the bundled one) - is this a Qt bug?
# Also, digiKam has a hardcoded /usr which we patch away

cd usr/ ; find . -type f -exec sed -i -e 's|/usr|././|g' {} \; ; cd ..

# We do not bundle this, so let's not search that inside the AppImage.
# Fixes "Qt: Failed to create XKB context!" and lets us enter text

sed -i -e 's|././/share/X11/|/usr/share/X11/|g' ./usr/plugins/platforminputcontexts/libcomposeplatforminputcontextplugin.so

if [[ $DK_QTVERSION == 6 ]] ; then

    sed -i -e 's|././/share/X11/|/usr/share/X11/|g' ./usr/lib/libQt6XcbQpa.so.6

else

    sed -i -e 's|././/share/X11/|/usr/share/X11/|g' ./usr/lib/libQt5XcbQpa.so.5

fi

#################################################################################################
# Install ExifTool binary.

cd $DOWNLOAD_DIR

#if [ ! -f $DOWNLOAD_DIR/Image-ExifTool.tar.gz ] ; then
    wget --no-check-certificate https://mirrors.xtom.de/kde-application/digikam/exiftool/Image-ExifTool.tar.gz -O Image-ExifTool.tar.gz
#fi

tar -xvf $DOWNLOAD_DIR/Image-ExifTool.tar.gz -C $APP_IMG_DIR/usr/bin
mv $APP_IMG_DIR/usr/bin/Image-ExifTool* $APP_IMG_DIR/usr/bin/Image-ExifTool

cd $APP_IMG_DIR/usr/bin
ln -s ./Image-ExifTool/exiftool exiftool

#################################################################################################

cd /

APP=digiKam

if [[ $DK_DEBUG = 1 ]] ; then

    DEBUG_SUF="-debug"

fi

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

APPIMAGE=$APP"-"$DK_RELEASEID$DK_SUBVER"$QT_SUF-x86-64$DEBUG_SUF.appimage"

echo -e "------------- Create Bundle with AppImage SDK stage1\n"

# Source functions

if [[ ! -s ./functions.sh ]] ; then

    # TODO: port to the new AppImageTool (https://github.com/AppImage/appimagetool/releases/tag/continuous)

    wget --no-check-certificate https://github.com/probonopd/AppImages/raw/master/functions.sh -O ./functions.sh

fi

# Install desktopintegration in usr/bin/digikam.wrapper

cd $APP_IMG_DIR

# We will use a dedicated bash script to run inside the AppImage to be sure that XDG_* variable are set for Qt5

cp ${ORIG_WD}/data/AppRun ./

# A script to run a remote GDB instance to hack dysfunction with the AppImage bundle.
# For details: https://www.digikam.org/contribute/#appimage-bundle

cp ${ORIG_WD}/data/run_digikam_gdb.sh ./

# desktop integration files

cp /usr/share/applications/org.kde.digikam.desktop      ./
cp /usr/share/applications/org.kde.showfoto.desktop     ./

cp /usr/share/icons/hicolor/256x256/apps/digikam.png    ./digikam.png
cp /usr/share/icons/hicolor/256x256/apps/showfoto.png   ./showfoto.png
cp /usr/share/icons/hicolor/256x256/apps/digikam.png    ./.DirIcon

mkdir -p $APP_IMG_DIR/usr/share/icons/default/128x128/apps
cp -r /usr/share/icons/hicolor/128x128/apps/digikam.png ./usr/share/icons/default/128x128/apps/digikam.png
cp -r /usr/share/icons/hicolor/128x128/apps/showfoto.png ./usr/share/icons/default/128x128/apps/showfoto.png

mkdir -p $APP_IMG_DIR/usr/share/icons/default/128x128/mimetypes
cp -r /usr/share/icons/hicolor/128x128/apps/digikam.png ./usr/share/icons/default/128x128/mimetypes/application-vnd.digikam.png
cp -r /usr/share/icons/hicolor/128x128/apps/showfoto.png ./usr/share/icons/default/128x128/mimetypes/application-vnd.showfoto.png

mkdir -p $ORIG_WD/bundle

rm -f $ORIG_WD/bundle/*x86-64*$DEBUG_SUF* || true

echo -e "---------- Create Bundle with AppImage SDK stage2\n"

cd /

# Get right version of Appimage toolkit.

APPIMGBIN=AppImageTool-x86_64.AppImage

if [[ ! -s ./$APPIMGBIN ]] ; then

    # TODO: port to the new AppImageTool (https://github.com/AppImage/appimagetool/releases/tag/continuous)

    wget --no-check-certificate https://github.com/AppImage/AppImageKit/releases/download/continuous/$APPIMGBIN -O ./$APPIMGBIN

fi

chmod a+x ./$APPIMGBIN

ARCH=x86_64 ./$APPIMGBIN --comp xz $APP_IMG_DIR/ $ORIG_WD/bundle/$APPIMAGE
chmod a+rwx $ORIG_WD/bundle/$APPIMAGE

#################################################################################################
# Show resume information and future instructions to host installer file to remote server

echo -e "\n---------- Compute package checksums for digiKam $DK_RELEASEID\n"  > $ORIG_WD/bundle/$APPIMAGE.sum
echo    "File       : $APPIMAGE"                                             >> $ORIG_WD/bundle/$APPIMAGE.sum
echo -n "Size       : "                                                      >> $ORIG_WD/bundle/$APPIMAGE.sum
du -h "$ORIG_WD/bundle/$APPIMAGE"     | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum
echo -n "SHA256 sum : "                                                      >> $ORIG_WD/bundle/$APPIMAGE.sum
sha256sum "$ORIG_WD/bundle/$APPIMAGE" | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum

# Checksums to post on Phabricator at release time.

sha256sum "$ORIG_WD/bundle/$APPIMAGE" > $ORIG_WD/bundle/sha256_release.sum

if [[ $DK_SIGN = 1 ]] ; then

    cat ~/.gnupg/dkorg-gpg-pwd.txt | gpg --batch --yes --passphrase-fd 0 -sabv "$ORIG_WD/bundle/$APPIMAGE"
    mv -f $ORIG_WD/bundle/$APPIMAGE.asc $ORIG_WD/bundle/$APPIMAGE.sig

    echo    "File       : $APPIMAGE.sig"                                             >> $ORIG_WD/bundle/$APPIMAGE.sum
    echo -n "Size       : "                                                          >> $ORIG_WD/bundle/$APPIMAGE.sum
    du -h "$ORIG_WD/bundle/$APPIMAGE.sig"     | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum
    echo -n "SHA256 sum : "                                                          >> $ORIG_WD/bundle/$APPIMAGE.sum
    sha256sum "$ORIG_WD/bundle/$APPIMAGE.sig" | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum

    # Checksums to post on Phabricator at release time.

    sha256sum "$ORIG_WD/bundle/$APPIMAGE.sig" >> $ORIG_WD/bundle/sha256_release.sum

fi

cat $ORIG_WD/bundle/$APPIMAGE.sum

if [[ $DK_UPLOAD = 1 ]] ; then

    echo -e "---------- Cleanup older bundle AppImage files from files.kde.org repository \n"

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm *$QT_SUF-x86-64*$DEBUG_SUF.appimage*"

    echo -e "---------- Upload new bundle AppImage files to files.kde.org repository \n"

    rsync -r -v --progress -e ssh $ORIG_WD/bundle/$APPIMAGE $DK_UPLOADURL:$DK_UPLOADDIR

    if [[ $DK_SIGN = 1 ]] ; then

        scp $ORIG_WD/bundle/$APPIMAGE.sig $DK_UPLOADURL:$DK_UPLOADDIR

    fi

    # update remote files list

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "ls digi*" > $ORIG_WD/bundle/ls.txt
    tail -n +2 $ORIG_WD/bundle/ls.txt > $ORIG_WD/bundle/ls.tmp
    cat $ORIG_WD/bundle/ls.tmp | grep -E '(.pkg |.appimage |.exe )' | grep -Ev '(debug)' > $ORIG_WD/bundle/FILES
    rm $ORIG_WD/bundle/ls.tmp
    rm $ORIG_WD/bundle/ls.txt
    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm FILES"
    rsync -r -v --progress -e ssh $ORIG_WD/bundle/FILES $DK_UPLOADURL:$DK_UPLOADDIR

else

    echo -e "\n------------------------------------------------------------------"
    curl https://download.kde.org/README_UPLOAD
    echo -e "------------------------------------------------------------------\n"

fi

#################################################################################################

TerminateScript
