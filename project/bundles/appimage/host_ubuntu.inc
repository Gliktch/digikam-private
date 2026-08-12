#!/bin/bash

# Rules to prepare Ubuntu 22.04 and later Linux host.
#
# Copyright (c) 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

#################################################################################################

echo -e "---------- Update Linux Ubuntu Host\n"

# for downloading package information from all configured sources.'

sudo apt-get update
sudo apt-get upgrade

# benefit from a higher version of certain software , update the key

sudo apt-key adv --refresh-keys --keyserver keyserver.ubuntu.com
sudo add-apt-repository "deb http://security.ubuntu.com/ubuntu xenial-security main"

# To fix GPP key error with some repositories
# See: https://www.skyminds.net/linux-resoudre-les-erreurs-communes-de-cle-gpg-dans-apt/

sudo apt-get update 2>&1 | \
    sed -ne 's?^.*NO_PUBKEY ??p' | \
    xargs -r -- sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys

echo -e "---------- Install New Development Packages\n"

# Install dependencies to Checkout Source Code

sudo apt-get install -y git
sudo apt-get install -y perl

required_packages=("cmake"                   # To Compile Source Code
                   "ninja-build"             # To Compile Source Code
                   "build-essential"         # To Compile Source Code
                   "libpthread-stubs0-dev"   # Development files for pthread
                   "libtiff-dev"             # Tag image file format library
                   "libpng-dev"              # PNG library
                   "libjpeg-dev"             # JPEG library
                   "liblcms2-dev"            # Little CMS 2 color management library
                   "libexpat1-dev"           # XML parsing C library
                   "libssl-dev"              # For MXE build
                  )


for pkg in ${required_packages[@]}; do

    sudo apt-get install -y ${pkg}

    current_version=$(dpkg-query --showformat='${Version}' --show ${pkg})

    case "${pkg}" in
    "cmake")
        required_version=3.3.2
        ;;
    "build-essential")
        required_version=11.0.0
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
    "liblcms2-dev")
        required_version=2.0.0
        ;;
    "libexpat1-dev")
        required_version=2.1.0
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

# Install optional dependencies to Compile And Link Source Code

optional_packages=("ruby"                               # For i18n extraction
                   "subversion"                         # For i18n extraction
                   "hunspell"                           # For check spelling
                   "libhunspell-dev"                    # For check spelling
                   "keychain"                           # For git-ssh
                   "ssh-askpass"                        # For git-ssh
                   "ccache"                             # For compiling
                   "bison"                              # For Qt build (>= 2.5.0)
                   "flex"                               # For compiling (>= 2.5.0)
                   "mysql-server"                       # Run-time: mysql internal server init
                   "libeigen3-dev"                      # >= 3.2
                   "libgomp1"                           # For Libraw compilation
                   "libxslt-dev"                        # >= 1.1.0
                   "libxml2-dev"                        # >= 2.7.0
                   "libfftw3-dev"                       # For GMic-Qt compilation
                   "libx265-dev"                        # >= 2.2
                   "wget"
                   "tar"
                   "bzip2"
                   "gettext"
                   "libtool"
                   "fuse"
                   "automake"
                   "ccache"
                   "yasm"
                   "nasm"
                   "patch"
                   "libdrm-dev"
                   "libxcb*-dev"
                   "libx11-keyboard-perl"
                   "xscreensaver"
                   "gperf"
                   "zlib1g-dev"
                   "libfuse-dev"
                   "libc6-dev"
                   "default-libmysqlclient-dev"
                   "libcppunit-dev"
                   "libc++-dev"
                   "libudev-dev"
                   "libsqlite3-dev"
                   "libexif-dev"
                   "liblzma-dev"
                   "liblz-dev"
                   "libinotifytools0-dev"
                   "libcups2-dev"
                   "libopenal-dev"
                   "libpulse-dev"
                   "libical-dev"
                   "libcap-dev"
                   "libfontconfig1-dev"
                   "patchelf"
                   "dpkg"
                   "clang"
                   "python3"
                   "python3-lxml"
                   "libboost-all-dev"
                   "libgphoto2-dev"
                   "libsane-dev"
                   "libssl-dev"
                   "appstream"
                   "libnss3-dev"
                   "libxkbcommon-dev"
                   "libxkbcommon-x11-dev"
                   "libxkbfile-dev"
                   "libxinerama-dev"
                   "libxi-dev"
                   "libxtst-dev"
                   "libxrandr-dev"
                   "libxcursor-dev"
                   "libxcomposite-dev"
                   "libxrender-dev"
                   "libxdamage-dev"
                   "libx11-xcb-dev"
                   "libltdl-dev"
                   "libglib2.0-dev"
                   "libusb-1.0-0-dev"
                   "libclang-dev"
                   "libsm-dev"
                   "freeglut3-dev"
                   "libinput-dev"
                   "libfdk-aac-dev"
                   "libx264-dev"
                   "libx265-dev"
                   "libxvidcore-dev"
                   "libvpx-dev"
                   "libtheora-dev"
                   "libvorbis-dev"
                   "libopencore-amrnb-dev"
                   "libopencore-amrwb-dev"
                   "librtmp-dev"
                   "libopus-dev"
                   "libspeex-dev"
                   "libmp3lame-dev"
                   "libfreetype6-dev"
                   "libass-dev"
                   "libegl1-mesa-dev"
                   "libgl1-mesa-dev"
                   "libgles2-mesa-dev"
                   "libglu1-mesa-dev"
                   "libdbus-1-dev"
                   "libmount-dev"
                   "curl"
                   "libicu-dev"
                   "icu-devtools"
                   "flite1-dev"
                   "libxshmfence-dev"
                   "libcanberra-dev"
                   "libasound2-dev"
                   "libgbm-dev"                         # For QtWebEngine: https://bugreports.qt.io/browse/QTBUG-129346
                  )


for pkg in ${optional_packages[@]}; do
    sudo apt-get install -y ${pkg}
    echo "-------------------------------------------------------------------"
done

if [[ $DK_QTVERSION == 5 ]] ; then

    optional_packages_qt5=(
                           "python2"
                           "python-is-python2"
                           "libgcrypt11-dev"
                          )

    for pkg in ${optional_packages_qt5[@]}; do
        sudo apt-get install -y ${pkg}
        echo "-------------------------------------------------------------------"
    done

else

    optional_packages_qt6=(
                           "libgcrypt20-dev"
                           "libyuv-dev"
                          )

    for pkg in ${optional_packages_qt6[@]}; do
        sudo apt-get install -y ${pkg}
        echo "-------------------------------------------------------------------"
    done

fi

# Install more recent version of NodeJS for QtWebEngine.
# https://learnubuntu.com/update-node-js/

curl -sL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install nodejs

echo -e "---------- Clean-up Old Packages\n"

# Remove system based devel package to prevent conflict with new one.
sudo apt remove -y libqt5core5a   || true
sudo apt remove -y libavfilter6   || true
sudo apt remove -y libjasper-dev  || true
sudo apt remove -y libexiv2*      || true
sudo apt remove -y libheif*       || true
sudo apt remove -y *opencv*       || true
sudo apt remove -y libavif*       || true
sudo apt remove -y libaom*        || true
sudo apt remove -y libavcodec*    || true
sudo apt remove -y libavfilter*   || true
sudo apt remove -y libavformat*   || true
sudo apt remove -y libavutil*     || true
sudo apt remove -y libavresample* || true
sudo apt remove -y libswscale*    || true
sudo apt remove -y libswresample* || true
sudo apt remove -y libde265*      || true
sudo apt remove -y libmagick*     || true

echo -e "---------- Install more recent version of compiler\n"

# https://askubuntu.com/questions/1140183/install-gcc-9-on-ubuntu-18-04
# https://www.followchain.org/install-gcc-11-on-ubuntu/
# https://linuxconfig.org/how-to-switch-between-multiple-gcc-and-g-compiler-versions-on-ubuntu-20-04-lts-focal-fossa

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt install -y gcc-11
sudo apt install -y g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11

