#! /bin/bash

# Script to build a bundle HomeBrew installation with all digiKam dependencies in a dedicated directory
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015      by Shanti <listaccount at revenant dot org>
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Ask to run as root
#(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-homebrew.full.log) 2>&1

#################################################################################################

echo "01-build-homebrew.sh : build a bundle HomeBrew install with digiKam dependencies."
echo "---------------------------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
#ChecksRunAsRoot
ChecksXCodeCLI
ChecksCPUCores
OsxCodeName
#RegisterRemoteServers

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:/$INSTALL_PREFIX/opt/qt6/bin:$ORIG_PATH

#################################################################################################
# Homebrew settings

#export HOMEBREW_NO_INSTALL_FROM_API=1
export HOMEBREW_NO_AUTO_UPDATE=1

export HOMEBREW_CELLAR="$INSTALL_PREFIX/Cellar"
export HOMEBREW_PREFIX="$INSTALL_PREFIX"
export HOMEBREW_REPOSITORY="$INSTALL_PREFIX"
export HOMEBREW_CACHE="$INSTALL_PREFIX/cache"
export QT_AVOID_CMAKE_ARCHIVING_API=ON

sudo dseditgroup -o edit -a ${USER} -t user admin

#################################################################################################
# Check if a previous bundle already exist

CONTINUE_INSTALL=0

if [ -d "$INSTALL_PREFIX" ] ; then

    read -p "$INSTALL_PREFIX already exist. Do you want to remove it or to continue an aborted previous installation ? [(r)emove/(c)ontinue/(s)top] " answer

    if echo "$answer" | grep -iq "^r" ;then

        echo "---------- Removing existing $INSTALL_PREFIX"
        mv $INSTALL_PREFIX $INSTALL_PREFIX.old || true
        rm -rf $INSTALL_PREFIX.old || true

    elif echo "$answer" | grep -iq "^c" ;then

        echo "---------- Continue aborted previous installation in $INSTALL_PREFIX"
        CONTINUE_INSTALL=1

    else

        echo "---------- Aborting..."
        exit;

    fi

fi

if [[ $CONTINUE_INSTALL == 0 ]]; then

    #################################################################################################
    # Install HomeBrew

    # Use this section if you want to install Homebrew to a custom location

    # sudo mkdir $INSTALL_PREFIX
    # sudo chown -R ${USER} $INSTALL_PREFIX
    # git clone https://github.com/Homebrew/brew $INSTALL_PREFIX
    # eval "$($INSTALL_PREFIX/bin/brew shellenv)"

    export HOMEBREW_NO_AUTO_UPDATE=1

    # export HOMEBREW_CELLAR="$INSTALL_PREFIX/Cellar"
    # export HOMEBREW_PREFIX="$INSTALL_PREFIX"
    # export HOMEBREW_REPOSITORY="$INSTALL_PREFIX"
    # export HOMEBREW_CACHE="$INSTALL_PREFIX/cache"

    # brew update --force
    # chmod -R go-w "$(brew --prefix)/share/zsh"

    
    # Use this section to do a default Homebrew installation - not recommended
    
    NONINTERACTIVE=1 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    # (echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> /Users/${USER}/.zprofile
    # #eval "$(/opt/homebrew/bin/brew shellenv)"

fi

#################################################################################################
# Homebrew update

# removed to keep build dependencies stable

# echo -e "\n"
# echo "---------- Updating HomeBrew"
# brew -v update

# if [[ $CONTINUE_INSTALL == 0 ]]; then

# #    brew -v upgrade outdated
#     echo -e "\n"

# fi

#################################################################################################
# Dependencies build and installation

echo -e "\n"
echo "---------- Building digiKam dependencies with Homebrew"

echo -e "\n"

# Homebrew supports Qt6 only
if [[ $DK_QTVERSION = 5 ]] ; then
    # brew install qt@5
    export PATH=$ORIG_PATH
    TerminateScript
    exit
fi

${INSTALL_PREFIX}/bin/brew install \
             cmake \
             ccache \
             libpng \
             jpeg \
             libtiff \
             boost \
             eigen \
             mesa \
             gettext \
             gperf \
             libusb \
             libgphoto2 \
             jasper \
             little-cms2 \
             expat \
             libxml2 \
             libxslt \
             libical \
             bison \
             x264 \
             x265 \
             libde265 \
             libheif \
             aom \
             ffmpeg \
             wget \
             dbus \
             dbus-glib \
             imagemagick \
             jpeg-xl \
             libavif \
             fftw \
             exiv2 \
             lensfun \
             opencv \
             qt \
             qt-mariadb
#             sane-backends

echo -e "\n"

# use pip instead of Homebrew to install lxml
${INSTALL_PREFIX}/bin/python3 -m venv ${INSTALL_PREFIX}
source ${INSTALL_PREFIX}/bin/activate
python3 -m pip install --upgrade lxml

#################################################################################################

# Create the build dir for the 3rdparty deps

if [ ! -d $BUILDING_DIR ] ; then

    mkdir $BUILDING_DIR

fi

if [ ! -d $DOWNLOAD_DIR ] ; then

    mkdir $DOWNLOAD_DIR

fi

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

#################################################################################################

CopyDebugSymbols

export PATH=$ORIG_PATH

TerminateScript
