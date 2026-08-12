#! /bin/bash

# Script to bundle data using previously-built KDE and digiKam installation
# and create a Windows installer file with NSIS application
#
# Dependencies:
#
#   - NSIS makensis program for Windows.
#   - DumpBin from VSCommunity C++ profiling tools.
#   - Mt from VSCommunity C++ Desktop tools.
#   - ImageMagick for the run-time dlls.
#   - Msys2/Rsync CLI tools for Windows.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

export LANG=C

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-installer.full.log) 2>&1

#################################################################################################

echo "04-build-installer.sh : build digiKam Windows installer."
echo "--------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksCPUCores
RegisterRemoteServers
AppendVCPKGPaths

#################################################################################################

# Check if NSIS CLI tool is installed.

if ! which "/c/Program Files (x86)/NSIS/Bin/makensis" ; then
    echo "NSIS CLI tool is not installed"
    echo "Install Msys2::nsis with pacman tool."
    exit 1
else
    echo "Check NSIS CLI tools passed..."
fi

# Check if the DumpBin CLI tool is installed.

DUMP_BIN="`find "/c/Program Files/Microsoft Visual Studio/" -name "dumpbin.exe" -type f -executable | grep 'Hostx64/x64/dumpbin.exe'`"
echo "$DUMP_BIN"

if [ ! -f "$DUMP_BIN" ] ; then
    echo "DumpBin CLI tool is not installed"
    echo "Install VSCommunity C++ Profiling tools component."
    exit 1
else
    echo "Check DumpBin CLI tools passed..."
fi

# Check if the Mt CLI tool is installed.

MT_BIN="`find "/c/Program Files (x86)/Windows Kits/" -name "mt.exe" -type f -executable | grep '/x64/mt.exe'`"
echo "$MT_BIN"

if [ ! -f "$MT_BIN" ] ; then
    echo "Mt CLI tool is not installed"
    echo "Install VSCommunity C++ Desktop tools component."
    exit 1
else
    echo "Check Mt CLI tools passed..."
fi

# Check for the ImageMagick install directory.

MAGICK_PREFIX="`find "/c/Program Files/" -maxdepth 1 -name "ImageMagick*" -type d`"
echo "$MAGICK_PREFIX"

if [ ! -d "$MAGICK_PREFIX" ] ; then
    echo "ImageMagick is not installed"
    exit 1
else
    echo "Check ImageMagick install path passed..."
fi

#################################################################################################
# Configurations

# Directory where this script is located (default - current directory)
BUILDDIR="$PWD"

# Directory where bundle files are located
BUNDLEDIR="$BUILDDIR/temp"

ORIG_WD="`pwd`"

DK_RELEASEID=`cat $ORIG_WD/data/RELEASEID.txt`

#################################################################################################
# Copy files

echo -e "\n---------- Preparing files to host in the bundle"

# Directories creation -----------------------------------------------------------------------

cd $ORIG_WD

if [ -d "$BUNDLEDIR" ] ; then
    echo "Cleanup temporary directory $BUNDLEDIR..."
    rm -fr "$BUNDLEDIR"
    mkdir "$BUNDLEDIR"
fi

mkdir -p $BUNDLEDIR/data
mkdir -p $BUNDLEDIR/etc
mkdir -p $BUNDLEDIR/share
mkdir -p $BUNDLEDIR/translations

# Data files ---------------------------------------------------------------------------------

echo -e "\n---------- Generics data"
cp -r $VCPKG_INSTALL_PREFIX/share/lensfun                                 $BUNDLEDIR/data                       2>/dev/null



if [[ $DK_QTVERSION = 6 ]] ; then

    cp -r $VCPKG_INSTALL_PREFIX/share/digikam                             $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/share/showfoto                            $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/share/solid                               $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/share/kxmlgui5                            $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/share/knotifications6                     $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/share/applications                        $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/share/Qt6/resources                       $BUNDLEDIR/                           2>/dev/null

else

    cp -r $VCPKG_INSTALL_PREFIX/bin/data/digikam                          $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/bin/data/showfoto                         $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/bin/data/solid                            $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/bin/data/kxmlgui5                         $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/bin/data/knotifications5                  $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/bin/data/applications                     $BUNDLEDIR/data                       2>/dev/null
    cp -r $VCPKG_INSTALL_PREFIX/share/qt5/resources                       $BUNDLEDIR/                           2>/dev/null

fi

cp -r $VCPKG_INSTALL_PREFIX/bin/data/k*                                   $BUNDLEDIR/data                       2>/dev/null

echo -e "\n---------- Qt config"
cp    $BUILDDIR/data/qt.conf                                              $BUNDLEDIR/                           2>/dev/null

echo -e "\n---------- icons-set"

# See https://bugs.kde.org/show_bug.cgi?id=487799#c54

if [[ $DK_QTVERSION = 6 ]] ; then

# Icons generation with vcpkg::python3 from KF6::breeze-icons do not work yet (lxml module is missing).
# Use the pre-compiled rcc file instead for the moment.
#
#    cp $VCPKG_INSTALL_PREFIX/bin/data/icons/breeze/breeze-icons.rcc       $BUNDLEDIR/breeze.rcc                 2>/dev/null

    cp $BUILDDIR/../common/breeze.rcc                                     $BUNDLEDIR/breeze.rcc                 2>/dev/null

else

    cp $BUILDDIR/../common/breeze.rcc                                     $BUNDLEDIR/breeze.rcc                 2>/dev/null
    cp $BUILDDIR/../common/breeze-dark.rcc                                $BUNDLEDIR/breeze-dark.rcc            2>/dev/null

fi

if [[ $DK_QTVERSION = 6 ]] ; then

    cp -r $VCPKG_INSTALL_PREFIX/share/icons                               $BUNDLEDIR/data                       2>/dev/null

else

    cp -r $VCPKG_INSTALL_PREFIX/bin/data/icons                            $BUNDLEDIR/data                       2>/dev/null

fi

echo -e "\n---------- i18n"

# Qt framekork translations

if [[ $DK_QTVERSION = 6 ]] ; then

    cp -r $VCPKG_INSTALL_PREFIX/translations/Qt6/*                        $BUNDLEDIR/translations               2>/dev/null

else

    cp -r $VCPKG_INSTALL_PREFIX/share/qt5/translations/*                  $BUNDLEDIR/translations               2>/dev/null

fi

rm -fr $BUNDLEDIR/translations/assistant*.qm
rm -fr $BUNDLEDIR/translations/designer*.qm
rm -fr $BUNDLEDIR/translations/linguist*.qm
rm -fr $BUNDLEDIR/translations/qt_help*.qm
# KDE framework translations
cp -r $VCPKG_INSTALL_PREFIX/bin/data/locale                               $BUNDLEDIR/data                       2>/dev/null
# digiKam translations
cp -r $VCPKG_INSTALL_PREFIX/share/locale                                  $BUNDLEDIR/data                       2>/dev/null

# See bug 471058
echo -e "\n---------- Freedesktop"
mkdir -p $BUNDLEDIR/share/mime/packages/                                                                        2>/dev/null
cp -r  $ORIG_WD/data/freedesktop.org.xml                                  $BUNDLEDIR/share/mime/packages        2>/dev/null

echo -e "\n---------- Copy Git Revisions Manifest"

touch $BUNDLEDIR/MANIFEST.txt

FILES=$(ls $ORIG_WD/data/*_manifest.txt)

for FILE in $FILES ; do
    echo $FILE
    cat $FILE >> $BUNDLEDIR/MANIFEST.txt
done

# Plugins Shared libraries -------------------------------------------------------------------

echo -e "\n---------- Qt plugins"

if [[ $DK_QTVERSION = 6 ]] ; then

    cp -r $VCPKG_INSTALL_PREFIX/Qt6/plugins                               $BUNDLEDIR/                           2>/dev/null

else

    cp -r $VCPKG_INSTALL_PREFIX/plugins                                   $BUNDLEDIR/                           2>/dev/null

fi

echo -e "\n---------- OpenGl Software for Qt6"
# Taken from https://download.qt.io/development_releases/prebuilt/llvmpipe/windows/
# See bug #479148
cp -r $ORIG_WD/data/opengl32sw.dll                                        $BUNDLEDIR/                           2>/dev/null

echo -e "\n---------- ImageMagick"

cp -r "$MAGICK_PREFIX"/CORE_RL_*.dll                                      $BUNDLEDIR/                           2>/dev/null
cp -r "$MAGICK_PREFIX"/modules/coders/*.dll                               $BUNDLEDIR/                           2>/dev/null
cp -r "$MAGICK_PREFIX"/modules/filters/*.dll                              $BUNDLEDIR/                           2>/dev/null

# Black-list of ImageMagick coders to not host in the bundles.
# Prior digiKam/Qt image loaders first which provides the same type-mimes.

IM_CODERS_BL="\
$BUNDLEDIR/IM_MOD_RL_dng_.dll   \
$BUNDLEDIR/IM_MOD_RL_exr_.dll   \
$BUNDLEDIR/IM_MOD_RL_gif_.dll   \
$BUNDLEDIR/IM_MOD_RL_hdr_.dll   \
$BUNDLEDIR/IM_MOD_RL_heic_.dll  \
$BUNDLEDIR/IM_MOD_RL_jp2_.dll   \
$BUNDLEDIR/IM_MOD_RL_jpeg_.dll  \
$BUNDLEDIR/IM_MOD_RL_jxl_.dll   \
$BUNDLEDIR/IM_MOD_RL_ora_.dll   \
$BUNDLEDIR/IM_MOD_RL_pcx_.dll   \
$BUNDLEDIR/IM_MOD_RL_pdf_.dll   \
$BUNDLEDIR/IM_MOD_RL_png_.dll   \
$BUNDLEDIR/IM_MOD_RL_psd_.dll   \
$BUNDLEDIR/IM_MOD_RL_raw_.dll   \
$BUNDLEDIR/IM_MOD_RL_svg_.dll   \
$BUNDLEDIR/IM_MOD_RL_tga_.dll   \
$BUNDLEDIR/IM_MOD_RL_tiff_.dll  \
$BUNDLEDIR/IM_MOD_RL_video_.dll \
$BUNDLEDIR/IM_MOD_RL_wbmp_.dll  \
$BUNDLEDIR/IM_MOD_RL_webp_.dll  \
$BUNDLEDIR/IM_MOD_RL_xcf_.dll   \
"

for coder in $IM_CODERS_BL ; do

    rm -rf "$coder"

done

# See bug #476290
rm -fr $BUNDLEDIR/plugins/imageformats/qjp2.dll
rm -fr $BUNDLEDIR/plugins/imageformats/qjp2.pdb

#################################################################################################
# Add debug symbols for few binary files to optimize space.
# NOTE: NSIS only support < 2Gb of file to package in the same installer. If size is bigger, a bus error exception is generated.
# Only the digiKam shared libraries debug symbols are preserved. All digiKam plugins are not present.

# First drop all existing pdb files from the bundles if any.

echo -e "\n---------- Clean up debug symbols in the bundle"

PDB_FILES="`find "$BUNDLEDIR"/plugins -name "*.pdb" -type f`"

for pdb in $PDB_FILES ; do

    if [ "$DK_DEBUG" = 0 ] ; then

        # In non debug clean all

        echo "   (ND del) ==> $pdb"
        rm -rf "$pdb"

#    NOTE: due to size limitation of NSIS archive to 2Gb,
#    the number of extra debug symbols must be limited.
#
#    elif [[ "$pdb" = *"plugins/digikam"* ]] ; then
#
#        # In debug mode preserve digiKam Plugins
#
#        echo "   (Skip) ==> $pdb"
#
#        # Enable this condition only to hack.
#
#    elif [[ "$pdb" = *"Qt${DK_QTVERSION}Core.pdb"* ]] ; then
#
#        # In debug mode preserve Qt debug symbols
#
#        echo "   (Skip) ==> $pdb"

    else

        echo "   (DB del) ==> $pdb"
        rm -rf "$pdb"

    fi

done

# Second add only wanted pdb files in the bundles

if [ "$DK_DEBUG" = 1 ] ; then

    echo -e "\n---------- Add necessary debug symbols in the bundle"

    # digikam libs
    cp -rv "$VCPKG_INSTALL_PREFIX"/bin/digikam*.pdb                        $BUNDLEDIR/                           2>/dev/null

#    if [ -f "$VCPKG_INSTALL_PREFIX"/bin/Qt${DK_QTVERSION}Core.pdb ] ; then
#
#        cp -rv "$VCPKG_INSTALL_PREFIX"/bin/Qt${DK_QTVERSION}Core.pdb       $BUNDLEDIR/                           2>/dev/null
#
#    fi

fi

echo -e "\n---------- Apply manifest file to enable UTF-8 support for file paths\n"

"$MT_BIN" \
        -manifest $VCPKG_INSTALL_PREFIX/bin/digikam.exe.manifest \
        -outputresource:$VCPKG_INSTALL_PREFIX/bin/digikam.exe;#1

"$MT_BIN" \
        -manifest $VCPKG_INSTALL_PREFIX/bin/showfoto.exe.manifest \
        -outputresource:$VCPKG_INSTALL_PREFIX/bin/showfoto.exe;#1

echo -e "\n---------- Copy executables with recursive dependencies in bundle directory\n"

# Executables and plugins shared libraries dependencies scan ---------------------------------

EXE_FILES="\
$VCPKG_INSTALL_PREFIX/bin/digikam.exe                    \
$VCPKG_INSTALL_PREFIX/bin/showfoto.exe                   \
$VCPKG_INSTALL_PREFIX/tools/libsnoretoast/snoretoast.exe \
$VCPKG_INSTALL_PREFIX/tools/ffmpeg/ffmpeg.exe            \
"

if [[ $DK_QTVERSION = 6 ]] ; then

    EXE_FILES+="\
    $VCPKG_INSTALL_PREFIX/tools/Qt6/bin/QtWebEngineProcess.exe \
    $VCPKG_INSTALL_PREFIX/bin/kbuildsycoca6.exe    \
    "

else

    EXE_FILES+="\
    $VCPKG_INSTALL_PREFIX/tools/Qt5/QtWebEngineProcess.exe \
    $VCPKG_INSTALL_PREFIX/bin/kbuildsycoca5.exe            \
    "

fi

for app in $EXE_FILES ; do

    cp $app $BUNDLEDIR/
    CopyReccursiveDependencies "$DUMP_BIN" "$app" "$BUNDLEDIR/" "$VCPKG_INSTALL_PREFIX/bin"

done

if [[ $DK_QTVERSION = 6 ]] ; then

    DLL_FILES="\
    `find  $VCPKG_INSTALL_PREFIX/Qt6/plugins -name "*.dll" -type f | sed 's|$VCPKG_INSTALL_PREFIX/Qt6/plugins||'`  \
    "

else

    DLL_FILES="\
    `find  $VCPKG_INSTALL_PREFIX/plugins -name "*.dll" -type f | sed 's|$VCPKG_INSTALL_PREFIX/plugins||'`  \
    "

fi

for app in $DLL_FILES ; do

    CopyReccursiveDependencies "$DUMP_BIN" "$app" "$BUNDLEDIR/" "$VCPKG_INSTALL_PREFIX/bin"

done

FFMPEG_DLL_FILES="\
`find  $VCPKG_INSTALL_PREFIX/tools/ffmpeg   -name "*.dll"        -type f | sed 's|$VCPKG_INSTALL_PREFIX/tools/ffmpeg||'` \
`find  $VCPKG_INSTALL_PREFIX/tools/ffmpeg   -name "*.DLL"        -type f | sed 's|$VCPKG_INSTALL_PREFIX/tools/ffmpeg||'` \
`find  $VCPKG_INSTALL_PREFIX/tools/x264/bin -name "libx264*.dll" -type f | sed 's|$VCPKG_INSTALL_PREFIX/tools/x264/bin||'` \
"

for ffmpegdll in $FFMPEG_DLL_FILES ; do

    cp $ffmpegdll $BUNDLEDIR/
    CopyReccursiveDependencies "$DUMP_BIN" "$ffmpegdll" "$BUNDLEDIR/" "$VCPKG_INSTALL_PREFIX/bin"

done

echo -e "\n---------- Copy redistributable VSCommunity compatibility dlls"

VS_DLL_VERSION=140

VS_DLL_FILES="\
$VCPKG_INSTALL_PREFIX/bin/msvcp${VS_DLL_VERSION}.dll              \
$VCPKG_INSTALL_PREFIX/bin/msvcp${VS_DLL_VERSION}_1.dll            \
$VCPKG_INSTALL_PREFIX/bin/msvcp${VS_DLL_VERSION}_2.dll            \
$VCPKG_INSTALL_PREFIX/bin/msvcp${VS_DLL_VERSION}_atomic_wait.dll  \
$VCPKG_INSTALL_PREFIX/bin/msvcp${VS_DLL_VERSION}_codecvt_ids.dll  \
$VCPKG_INSTALL_PREFIX/bin/vcruntime${VS_DLL_VERSION}_1.dll        \
$VCPKG_INSTALL_PREFIX/bin/vcruntime${VS_DLL_VERSION}.dll          \
$VCPKG_INSTALL_PREFIX/bin/concrt${VS_DLL_VERSION}.dll             \
"

for vsdll in $VS_DLL_FILES ; do

    echo -e "   => $vsdll"
    cp -r "$vsdll" $BUNDLEDIR/ 2>/dev/null

done

# Copy also the vcomp dlls for Windows 11 support

# To prevent broken array parsing of paths with spaces.
IFS=$'\n'

VS_DLL_COMP="`find "/c/Program Files/Microsoft Visual Studio/" -name "vcomp${VS_DLL_VERSION}*.dll" -type f | grep 'x64/' | grep 'OpenMP' | grep -v 'onecore'`"

for vscompdll in $VS_DLL_COMP ; do

    echo -e "   => $vscompdll"
    cp -r "$vscompdll" $BUNDLEDIR/ 2>/dev/null

done

if [[ $DK_QTVERSION = 6 ]] ; then

    # Group icon-engine plugins at the same place
    mv -f $BUNDLEDIR/plugins/kiconthemes6/iconengines/*.dll $BUNDLEDIR/plugins/iconengines/
    rm -fr $BUNDLEDIR/plugins/kiconthemes6

fi

#################################################################################################
# Install ExifTool binary.

echo -e "\n---------- Copy ExifTool executable"

cd $DOWNLOAD_DIR
curl -LO https://mirrors.xtom.de/kde-application/digikam/exiftool/exiftool.zip
unzip -o $DOWNLOAD_DIR/exiftool.zip -d $BUNDLEDIR
mv "$BUNDLEDIR/exiftool(-k).exe" "$BUNDLEDIR/exiftool.exe"

#################################################################################################

if [ "$DK_DEBUG" = 1 ] ; then

    DEBUG_SUF="-debug"

fi

if [[ "$DK_VERSION" != "v"* ]] ; then

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

TARGET_INSTALLER=digiKam-$DK_RELEASEID$DK_SUBVER$QT_SUF-Win64$DEBUG_SUF.exe
PORTABLE_FILE=digiKam-$DK_RELEASEID$DK_SUBVER$QT_SUF-Win64$DEBUG_SUF.tar.xz
CHECKSUM_FILE=digiKam-$DK_RELEASEID$DK_SUBVER$QT_SUF-Win64$DEBUG_SUF.sum

#################################################################################################
# Build NSIS installer and Portable archive.

echo -e "\n---------- Build NSIS installer and Portable archive\n"

mkdir -p $ORIG_WD/bundle

rm -f $ORIG_WD/bundle/*Win64$DEBUG_SUF* || true

cd $ORIG_WD/installer

if [ "$DK_DEBUG" = 1 ] ; then

    DEBUG_SYMB="-DDEBUG_SYMB"

fi

"/c/Program Files (x86)/NSIS/Bin/makensis" \
        -DVERSION=$DK_RELEASEID \
        -DBUNDLEPATH=$BUNDLEDIR \
        $DEBUG_SYMB \
        -DOUTPUT=$ORIG_WD/bundle/$TARGET_INSTALLER \
        ./digikam.nsi

cd $ORIG_WD
tar cf - `basename $BUNDLEDIR` --transform s/temp/digiKam/ | xz -4e > $ORIG_WD/bundle/$PORTABLE_FILE

#################################################################################################
# Show resume information and future instructions to host target files on remote server

echo -e "\n---------- Compute installer checksums for digiKam $DK_RELEASEID\n"          >  $ORIG_WD/bundle/$TARGET_INSTALLER.sum
echo    "File       : $TARGET_INSTALLER"                                                >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum
echo -n "Size       : "                                                                 >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum
du -h "$ORIG_WD/bundle/$TARGET_INSTALLER"        | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum
echo -n "SHA256 sum : "                                                                 >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum
shasum -a256 "$ORIG_WD/bundle/$TARGET_INSTALLER" | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum

# Checksums to post on Phabricator at release time.
shasum -a256 "$ORIG_WD/bundle/$TARGET_INSTALLER" > $ORIG_WD/bundle/$CHECKSUM_FILE

echo -e "\n---------- Compute Portable archive checksums for digiKam $DK_RELEASEID\n"   >  $ORIG_WD/bundle/$PORTABLE_FILE.sum
echo    "File       : $PORTABLE_FILE"                                                   >> $ORIG_WD/bundle/$PORTABLE_FILE.sum
echo -n "Size       : "                                                                 >> $ORIG_WD/bundle/$PORTABLE_FILE.sum
du -h "$ORIG_WD/bundle/$PORTABLE_FILE"        | { read first rest ; echo $first ; }     >> $ORIG_WD/bundle/$PORTABLE_FILE.sum
echo -n "SHA256 sum : "                                                                 >> $ORIG_WD/bundle/$PORTABLE_FILE.sum
shasum -a256 "$ORIG_WD/bundle/$PORTABLE_FILE" | { read first rest ; echo $first ; }     >> $ORIG_WD/bundle/$PORTABLE_FILE.sum

# Checksums to post on Phabricator at release time.
shasum -a256 "$ORIG_WD/bundle/$PORTABLE_FILE" >> $ORIG_WD/bundle/$CHECKSUM_FILE

if [ "$DK_SIGN" = 1 ] ; then

    echo -e "\n---------- Compute Signature checksums for digiKam installer $DK_RELEASEID\n"    >  $ORIG_WD/bundle/$TARGET_INSTALLER.sum

    cat ~/.gnupg/dkorg-gpg-pwd.txt | gpg --batch --yes --passphrase-fd 0 -sabv "$ORIG_WD/bundle/$TARGET_INSTALLER"
    mv -f $ORIG_WD/bundle/$TARGET_INSTALLER.asc $ORIG_WD/bundle/$TARGET_INSTALLER.sig

    echo    "File       : $TARGET_INSTALLER.sig"                                                >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum
    echo -n "Size       : "                                                                     >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum
    du -h "$ORIG_WD/bundle/$TARGET_INSTALLER.sig"        | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum
    echo -n "SHA256 sum : "                                                                     >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum
    shasum -a256 "$ORIG_WD/bundle/$TARGET_INSTALLER.sig" | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$TARGET_INSTALLER.sum

    # Checksums to post on Phabricator at release time.
    shasum -a256 "$ORIG_WD/bundle/$TARGET_INSTALLER.sig" >> $ORIG_WD/bundle/$CHECKSUM_FILE

    echo -e "\n---------- Compute Signature checksums for digiKam Portable $DK_RELEASEID\n"     >  $ORIG_WD/bundle/$PORTABLE_FILE.sum

    cat ~/.gnupg/dkorg-gpg-pwd.txt | gpg --batch --yes --passphrase-fd 0 -sabv "$ORIG_WD/bundle/$PORTABLE_FILE"
    mv -f $ORIG_WD/bundle/$PORTABLE_FILE.asc $ORIG_WD/bundle/$PORTABLE_FILE.sig

    echo    "File       : $PORTABLE_FILE.sig"                                                >> $ORIG_WD/bundle/$PORTABLE_FILE.sum
    echo -n "Size       : "                                                                  >> $ORIG_WD/bundle/$PORTABLE_FILE.sum
    du -h "$ORIG_WD/bundle/$PORTABLE_FILE.sig"        | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$PORTABLE_FILE.sum
    echo -n "SHA256 sum : "                                                                  >> $ORIG_WD/bundle/$PORTABLE_FILE.sum
    shasum -a256 "$ORIG_WD/bundle/$PORTABLE_FILE.sig" | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$PORTABLE_FILE.sum

    # Checksums to post on Phabricator at release time.
    shasum -a256 "$ORIG_WD/bundle/$PORTABLE_FILE.sig" >> $ORIG_WD/bundle/$CHECKSUM_FILE

fi

cat $ORIG_WD/bundle/$TARGET_INSTALLER.sum
cat $ORIG_WD/bundle/$PORTABLE_FILE.sum

if [ "$DK_UPLOAD" = 1 ] ; then

    echo -e "---------- Cleanup older Windows bundle files from files.kde.org repository \n"

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm *$QT_SUF-Win64$DEBUG_SUF.exe*"
    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm *$QT_SUF-Win64$DEBUG_SUF.tar.xz*"

    echo -e "---------- Upload new Windows bundle files to files.kde.org repository \n"

    UploadWithRetry $ORIG_WD/bundle/$TARGET_INSTALLER $DK_UPLOADURL $DK_UPLOADDIR 10
    UploadWithRetry $ORIG_WD/bundle/$PORTABLE_FILE    $DK_UPLOADURL $DK_UPLOADDIR 10

    if [[ $DK_SIGN = 1 ]] ; then
        scp $ORIG_WD/bundle/$TARGET_INSTALLER.sig $DK_UPLOADURL:$DK_UPLOADDIR
        scp $ORIG_WD/bundle/$PORTABLE_FILE.sig    $DK_UPLOADURL:$DK_UPLOADDIR
    fi

    # update remote files list

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "ls digi*" > $ORIG_WD/bundle/ls.txt
    tail -n +2 $ORIG_WD/bundle/ls.txt > $ORIG_WD/bundle/ls.tmp
    cat $ORIG_WD/bundle/ls.tmp | grep -E '(.pkg |.appimage |.exe )' | grep -Ev '(debug)' > $ORIG_WD/bundle/FILES
    rm $ORIG_WD/bundle/ls.tmp
    rm $ORIG_WD/bundle/ls.txt
    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm FILES"

    UploadWithRetry $BUILDDIR/bundle/FILES $DK_UPLOADURL $DK_UPLOADDIR 10

else

    echo -e "\n------------------------------------------------------------------"
    curl https://download.kde.org/README_UPLOAD
    echo -e "------------------------------------------------------------------\n"

fi

#################################################################################################

TerminateScript
