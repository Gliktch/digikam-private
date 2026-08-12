#!/bin/bash

# Script to install dependencies under Ubuntu Linux to compile digiKam.
# This script must be run as sudo
# *Ubuntu* compatible version >= 18.04
#
# SPDX-FileCopyrightText: 2021      by TRAN Quoc Hung <quochungtran1999 at gmail dot com>
# SPDX-FileCopyrightText: 2021      by Surya K M      <suryakm_is20 dot rvitm@rvei dot edu dot in>
# SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2024      by Joerg Lohse    <joergmlpts at gmail dot com>

#
# SPDX-License-Identifier: BSD-3-Clause
#

# Check OS name and version.

OS_NAME=$(awk '/DISTRIB_ID=/' /etc/*-release | sed 's/DISTRIB_ID=//' | sed 's/\"//' | sed 's/\"//' | tr '[:upper:]' '[:lower:]')
OS_ARCH=$(uname -m | sed 's/x86_//;s/i[3-6]86/32/')
OS_VERSION=$(awk '/DISTRIB_RELEASE=/' /etc/*-release | sed 's/DISTRIB_RELEASE=//' | sed 's/[.]0/./')

echo $OS_NAME
echo $OS_ARCH
echo $OS_VERSION

if [[ "$OS_NAME" != "ubuntu" ]] ; then
    echo "Not running Linux ubuntu..."
    exit -1
fi


echo "Check the list of dependencies in the online doc API : https://www.digikam.org/api/index.html#externaldeps"
echo "-------------------------------------------------------------------"

# Import the official Ubuntu key for Ubuntu <= 24.*

if [ "${OS_VERSION%.*}" -le 24 ]; then

    if [ ! -f /usr/share/keyrings/ubuntu-archive-keyring.gpg ]; then

        sudo curl -fsSL http://archive.ubuntu.com/ubuntu/project/ubuntu-archive-keyring.gpg | sudo gpg --dearmor -o /usr/share/keyrings/ubuntu-archive-keyring.gpg
        sudo chmod 644 /usr/share/keyrings/ubuntu-archive-keyring.gpg

    fi

fi

# To download package information from all configured sources.'

sudo apt-get update
sudo apt-get upgrade

# benefit from a higher version of certain software, update the key

if [ "${LINUX_VERSION%.*}" -le 24 ]; then

    # Import the missing GPG keys automatically

    sudo apt-get update 2>&1 | grep -oE "NO_PUBKEY [0-9A-Fa-f]+" | awk '{print $2}' | xargs -r -- sudo sh -c 'for key in "$@"; do
        sudo gpg --no-default-keyring --keyring /usr/share/keyrings/"${key}-archive-keyring.gpg" --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys "$key"
        echo "deb [signed-by=/usr/share/keyrings/${key}-archive-keyring.gpg] http://archive.ubuntu.com/ubuntu $LINUX_CODENAME main restricted universe multiverse" | sudo tee /etc/apt/sources.list.d/"${key}.list"
    done' --

    # Add the security repository with the official key

    echo "deb [signed-by=/usr/share/keyrings/ubuntu-archive-keyring.gpg] http://security.ubuntu.com/ubuntu $LINUX_CODENAME-security main" | sudo tee /etc/apt/sources.list.d/security.list

    # To fix GPP key error with some repositories
    # See: https://www.skyminds.net/linux-resoudre-les-erreurs-communes-de-cle-gpg-dans-apt/

    sudo apt-get update 2>&1 | \
        sed -ne 's?^.*NO_PUBKEY ??p' | \
        xargs -r -- sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys

fi

# Install dependencies to checkout source code

sudo apt-get install -y git git-lfs

echo "-------------------------------------------------------------------"

sudo apt-get install -y perl

echo "-------------------------------------------------------------------"

# Install required dependencies to compile and link source code

required_packages=("cmake"                   # To Compile Source Code
                   "ninja-build"             # To Compile Source Code
                   "extra-cmake-modules"     # To Compile Source Code
                   "build-essential"         # To Compile Source Code
                   "qttools5-dev-tools"      # To handle Qt5 configuration.

                   "libqt5core5a"            # Qt 5 core module
                   "libqt5gui5"              # Qt 5 Gui module
                   "libqt5widgets5"          # Qt 5 widget module
                   "libqt5network5"          # Qt 5 network module
                   "libqt5sql5"              # Qt 5 SQL module
                   "libqt5sql5-mysql"        # Qt 5 SQL module
                   "libqt5sql5-sqlite"       # Qt 5 SQL module
                   "libqt5xml5"              # Qt 5 XML module
                   "libqt5concurrent5"       # Qt 5 Concurrent module
                   "libqt5printsupport5"     # Qt 5 PrintSupport module
                   "libqt5svg5"              # Qt 5 Svg module
                   "libqt5svg5-dev"          # Qt 5 Svg module
                   "libqt5webengine5"        # Qt 5 webengine module
                   "qtwebengine5-dev"        # Qt 5 webengine module
                   "libqt5networkauth5-dev"  # Qt 5 network authentication.
                   "qtmultimedia5-dev"       # QT 5 Multimedia module

                   "libkf5config-dev"        # Configuration settings framework for Qt
                   "libkf5xmlgui-dev"        # User configurable main windows
                   "libkf5i18n-dev"          # Advanced internationalization framework
                   "libkf5windowsystem-dev"  # Development files for kwindowsystem
                   "libkf5service-dev"       # Development files for kservice
                   "libkf5solid-dev"         # Qt library to query and control hardware
                   "libkf5coreaddons-dev"    # Development files - KDE Frameworks 5 addons to QtCore

                   "libopencv-dev"           # Development files for opencv
                   "libpthread-stubs0-dev"   # Development files for pthread
                   "libtiff-dev"             # Tag image file format library
                   "libpng-dev"              # PNG library
                   "libjpeg-dev"             # JPEG library
                   "libboost-all-dev"        # Boots C++ Libraries
                   "liblcms2-dev"            # Little CMS 2 color management library
                   "libexpat1-dev"           # XML parsing C library
                   "libexiv2-dev"            # Metadata manipulation library
                  )


for pkg in ${required_packages[@]}; do

    sudo apt-get install -y ${pkg}

    current_version=$(dpkg-query --showformat='${Version}' --show ${pkg})

    case "${pkg}" in
    "cmake")
        required_version=3.3.2
        ;;
    "extra-cmake-modules")
        required_version=5.5.0
        ;;
    "build-essential")
        required_version=7.2.0
        ;;
    "libqt5core5a")
        required_version=5.9.0
        ;;
    "libkf5config-dev")
        required_version=5.5.0
        ;;
    "libopencv-dev")
        required_version=3.3.0
        ;;
    "libpthread-stubs0-dev")
        required_version=2.0.0
        ;;
    "libtiff-dev")
        required_version=4.0.0
        ;;
    "libpng-dev")
        required_version=1.6.0
        ;;
    "libjpeg-dev")
        required_version=6b
        ;;
    "libboost-all-dev")
        required_version=1.55.0
        ;;
    "liblcms2-dev")
        required_version=2.0.0
        ;;
    "libexpat1-dev")
        required_version=2.1.0
        ;;
    "libexiv2-dev")
        required_version=0.27.0
        ;;
    esac

    echo $current_version

    if $(dpkg --compare-versions "$current_version" "lt" "$required_version"); then
            echo "less than $required_version";
            echo "please upgrade newer version or another packages";
    else
            echo "greater than $required_version ............. accepted";
    fi

    echo "-------------------------------------------------------------------"

done

# Install optional dependencies to compile and link source code

optional_packages=("ruby"                               # For i18n extraction
                   "subversion"                         # For i18n extraction
                   "hunspell"                           # For check spelling
                   "valgrind"                           # For debugging
                   "keychain"                           # For git-ssh
                   "ssh-askpass"                        # For git-ssh
                   "lzip"                               # For CI/CD
                   "gzip"                               # For CI/CD
                   "unzip"                              # For CI/CD
                   "unrar"                              # For CI/CD
                   "autoconf"                           # For MXE build
                   "automake"                           # For MXE build
                   "autopoint"                          # For MXE build
                   "bash"                               # For MXE build
                   "bison"                              # For MXE build
                   "bzip2"                              # For MXE build
                   "flex"                               # For MXE build
                   "gcc-mingw-w64"                      # For MXE build
                   "g++"                                # For MXE build
                   "g++-multilib"                       # For MXE build
                   "gettext"                            # For MXE build
                   "git"                                # For MXE build
                   "gperf"                              # For MXE build
                   "icoutils"                           # For MXE build
                   "intltool"                           # For MXE build
                   "libc6-dev-i386"                     # For MXE build
                   "libgdk-pixbuf2.0-dev"               # For MXE build
                   "gtk-doc-tools"                      # For MXE build
                   "libltdl-dev"                        # For MXE build
                   "libgl-dev"                          # For MXE build
                   "libssl-dev"                         # For MXE build
                   "libtool"                            # For MXE build
                   "libtool-bin"                        # For MXE build
                   "libxml-parser-perl"                 # For MXE build
                   "lzip"                               # For MXE build
                   "make"                               # For MXE build
                   "openssl"                            # For MXE build
                   "p7zip-full"                         # For MXE build
                   "patch"                              # For MXE build
                   "perl"                               # For MXE build
                   "python3"                            # For MXE build
                   "python3-mako"                       # For MXE build
                   "python3-pkg-resources"              # For MXE build
                   "python-is-python3"                  # For MXE build
                   "ruby"                               # For MXE build
                   "scons"                              # For MXE build
                   "sed"                                # For MXE build
                   "unzip"                              # For MXE build
                   "wget"                               # For MXE build
                   "xz-utils"                           # For MXE build
                   "python3-pygments"                   # For Cppcheck static analysis
                   "python3-bs4"                        # For Clazy static analysis
                   "python3-soupsieve"                  # For Clazy static analysis
                   "cppcheck"                           # For static analysis
                   "clang"                              # For static analysis
                   "clang-tidy"                         # For static analysis
                   "clang-tools"                        # For static analysis
                   "libasan4"                           # For static analysis
                   "default-jdk"                        # For static analysis
                   "libsaxonhe-java"                    # For static analysis
                   "libxml-perl"                        # For static analysis
                   "libxml-libxml-perl"                 # For static analysis
                   "libyaml-libyaml-perl"               # For static analysis
                   "libyaml-perl"                       # For static analysis
                   "libjson-perl"                       # For static analysis
                   "llvm"                               # For static analysis
                   "libomp-dev"                         # For static analysis
                   "libclang-dev"                       # For static analysis
                   "codespell"                          # For static analysis
                   "doxygen"                            # For API doc
                   "graphviz"                           # For API doc
                   "texlive-full"                       # For API doc
                   "kate"                               # For debugging
                   "ktexteditor-katepart"               # For debugging
                   "ccache"                             # For compiling
                   "hugin"                              # For run-time processing
                   "exiftool"                           # For metadata processing
                   "bison"                              # For Qt build (>= 2.5.0)
                   "flex"                               # For compiling (>= 2.5.0)
                   "mariadb-server"                     # Run-time: mysql internal server init
                   "curl"                               # For CI/CD
                   "wget"                               # For CI/CD
                   "coreutils"                          # For CI/CD
                   "dmg2img"                            # For CI/CD (MacOS)
                   "tesseract-ocr"                      # For CI/CD (MacOS)
                   "libeigen3-dev"                      # >= 3.2
                   "liblensfun-dev"                     # >= 0.2.8
                   "libgomp1"                           # For Libraw compilation
                   "libavdevice-dev"                    # >= 3.3.x
                   "libavfilter-dev"                    # >= 3.3.x
                   "libavformat-dev"                    # >= 3.3.x
                   "libavcodec-dev"                     # >= 3.3.x
                   "libavutils-dev"                     # >= 3.3.x
                   "libpostproc-dev"                    # >= 3.3.x
                   "libgphoto2-dev"                     # >= 2.5
                   "libsane-dev"                        # >= 5.0.0
                   "libjasper-dev"                      # >= 1.900.1
                   "libheif-dev"
                   "libxslt-dev"                        # >= 1.1.0
                   "libxml2-dev"                        # >= 2.7.0
                   "libtiff-dev"                        # >= 4.0
                   "libfftw3-dev"                       # For GMic-Qt compilation
                   "libx265-dev"                        # >= 2.2
                   "libmagick++-dev"                    # >= 6.7.0
                   "libqt5x11extras5-dev"               # >= 5.9
                   "libqt5test5"                        # >= 5.9
                   "libqt5xml5"                         # >= 5.9
                   "qttools5-dev"                       # >= 5.9
                   "libqt5xmlpatterns5-dev"             # >= 5.9
                   "libqt5opengl5-dev"                  # >= 5.9
                   "libkf5sane-dev"                     # >= 5.5.0
                   "libkf5threadweaver-dev"             # >= 5.5.0
                   "libkf5kio-dev"                      # >= 5.5.0
                   "libkf5sonnet-dev"                   # >= 5.5.0
                   "libkf5notifications-dev"            # >= 5.5.0
                   "libkf5notifyconfig-dev"             # >= 5.5.0
                   "libkf5filemetadata-dev"             # >= 5.5.0
                   "libkf5calendarcore-dev"             # >= 5.5.0
                   "libkf5akonadicontact-dev"           # >= 4.89.0
                   "libkf5doctools-dev"
                   "libkf5filemetadata-dev"             # Files indexer engine for Plasma desktop
                   "libinih-dev"                        # For Exiv2
                   "libbrotli-dev"                      # For Exiv2
                    #TODO add new optional packages

                   #"clazy"                              # For static analysis : disabled as it depend of older clang/llvm
                    )

for pkg in ${optional_packages[@]}; do
    sudo apt-get install -y ${pkg}
    echo "-------------------------------------------------------------------"
done

# Add symbolic links for Krazy static analyzer

sudo ln -sf /usr/share/java              /opt/saxon
sudo ln -sf /usr/share/java/Saxon-HE.jar /usr/share/java/saxon9he.jar



