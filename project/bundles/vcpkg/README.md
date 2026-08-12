Scripts to install compiled digiKam dependencies with MSVC under Windows
========================================================================

* RESUME

    These scripts build a binary digiKam installer for Windows x86_64 under Windows using VCPKG, MSVC tool-chain,
    and NSIS application.

    These scripts follow instructions from Qt bundle deployment for Windows as shared libraries
    available at this url: https://doc.qt.io/qt-6/windows-deployment.html

* AUTHORS

    (c) Gilles Caulier <caulier dot gilles at gmail dot com>

* DESCRIPTION

    These files and scripts are used to make binary installer of the digiKam
    Software Collection for Windows Intel 64 bits. In addition to this README,
    there are comments in the scripts that provide some additional explanations for
    how they work.

* BUNDLE CONTENTS

    Note: the bundle is relocatable and compatible with Windows 10 and later.

```
    |                                                    Executable, shared dlls, KDE plugins dlls, qt.conf
    |--- data
    |   |--- audio                                       Marble data files
    |   |--- bitmaps                                     Marble data files
    |   |--- digikam                                     digiKam data files
    |   |--- flags                                       Marble data files
    |   |--- kconf_update                                Really need ?
    |   |--- kf6                                         Really need ?
    |   |   |--- kauth
    |   |   |--- kcharselect
    |   |--- knotifications6                             Really need ?
    |   |--- kstyle                                      Breeze theme definitions
    |   |--- kxmlgui5                                    KDE xml gui description files
    |   |   |--- digikam
    |   |   |--- showfoto
    |   |--- lensfun                                     Lensfun data files
    |   |--- licenses                                    License data files
    |   |--- locale                                      KDE GUI translations
    |   |--- maps                                        Marble data files
    |   |--- mwdbii                                      Marble data files
    |   |--- naturalearth                                Marble data files
    |   |--- placemarks                                  Marble data files
    |   |--- showfoto                                    Showfoto data files
    |   |--- solid                                       Solid service descriptions
    |   |--- stars                                       Marble data files
    |   |--- svg                                         Marble data files
    |   |-- weather                                      Marble data files
    |--- etc
    |   |-- xdg                                          Really need ?
    |       |-- ui
    |--- plugins                                         Qt plugins, digiKam plugins, Marble plugins
    |--- share                                           Really need ?
    |   |-- mime                                         FreeDesktop type-mime definitions.
    |   |-- xdg                                          Really need ?
    |       |-- menus
    |-- translations                                     Qt GUI translations
```

* REQUIREMENTS

    - VirtualBox 7.x + guest extension pack             https://www.virtualbox.org/wiki/Downloads
        + Memory : 24 Gb                                Note: QtWebEngine requires a lots of memory with parallelized build
        + CPU    : 8                                    Very important: do not assign more CPU to compile QtWebEngine, else required memory become huge.
        + Disk0  : VDI static NTFS 150 Gb               C:/ SYSTEM VCPKG cache build target on $HOME/AppData/Local/vcpkg/archives
        + Disk1  : VDI static NTFS 500 Gb               E:/ DATA   for the build, download, install storage

    - Windows 10 22H2 or later                          https://www.microsoft.com/en-us/software-download/windows10

    - Git 2.42 or later                                 https://git-scm.com/download/win
        + Git-bash console
        + Bundled OpenSSH
        + OpenSSH library
        + Checkout/Commit Unix EOL
        + MinTTY Terminal

    - NSIS 3.x                                          https://nsis-dev.github.io/

    - IcoUtils 0.32 or later                            https://sourceforge.net/projects/unix-utils/files/icoutils/
        + Uncompress the binary archive to C:/icoutils

    - WinFlexBison 2.5 or later                         https://github.com/lexxmark/winflexbison/releases
        + Uncompress the binary archive to C:/bison

    - RSync 3.2 or later for git-bash                   MSys repo: https://repo.msys2.org/msys/x86_64/
        + Tutorial                                      https://prasaz.medium.com/add-rsync-to-windows-git-bash-f42736bae1b3
        + Install Msys/RSync and deps packages          
        + target: git-bash/usr install directory        C:/Program Files/Git/usr/

    - MC 4.8 or later for git-bash                      MSys repo: https://repo.msys2.org/msys/x86_64/
        + Install Msys/mc and deps packages             libssh2 ; glib2
        + target: git-bash/usr install directory        C:/Program Files/Git/usr/

    - Visual Studio 2022 or later                       https://learn.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2022
        + Desktop development with C++
            + C++ core desktop features
            + MSVC v143 - VS2022 C++ x64/x86
            + Windows 11 SDK >= 10.0.22621.0
            + Just in time debugger                     (optional)
            + C++ Cmake tools for windows
            + C++ ATL for latest v143 build tools
            + C++ MFC for latest v143 build tools
            + C++/CLI support for v143
            + C++ Clang tools for Windows (16.0.5)
            + C++ Profiling tools.                      For dumpbin.exe tool.
            + C++ Desktop tools.                        For mt.exe tool.

    - ImageMagick 7.1 or later                          https://imagemagick.org/script/download.php#windows
        + x64 Version with Q16 and HDR support          Note: IM version compiled with MSVC
        + Install only C/C++ headers and libraries
        + Disable all others features                   Note: in 'Addition Tasks' step
        + Use default install path in C:/

* BUILD

    You must set the digiKam git tags to checkout right source code in installer.
    You must set the option to host debug symbols or not in installer.

    Note: Look in config.sh file for settings details.

    There are 4 scripts to be run by the user. 2 first ones installs
    VCPKG and all dependencies, next one digiKam, and last one makes a
    binary installer for Windows.

    To start compilation use these scripts in a Git-bash terminal:

    1) ./01-build-vcpkg.sh

    To build and install the base of VCPKG tools-chain including Qt framework and all low level dependencies.

    NOTE: due to long path problems while compiling under Windows and MSVC, VCPKG working directories must adjusted as shortest as possible:
        VCPKG_DIR=E:/vcpkg/
        INSTALL_DIR=E:/dk/
        DOWNLOAD_DIR=E:/d/
        BUILDING_DIR=E:/b/

        See the config.sh for details. Directories will be created automatically.

    2) ./02-build-extralibs.sh

    To Build and install the KDE framework components.

    3) ./03-build-digikam.sh

    To build and install digiKam and 3rd party plugins.

    4) ./04-build-installer.sh

    To build the Windows installer. Once you've successfully built digiKam, files to bundle may contain over 4GB of files.
    This script grabs the ~300MB needed to run digiKam and Showfoto.
    These include applications translations.

    This script creates Windows application links for the programs that will
    be run by the user (digiKam and Showfoto). It use makensis CLI tool from NSIS
    application dedicated to build Windows installer.
    The version >= 3 is required to be compatible with Windows 10.

    Depending of configuration done in config.sh, this script is able to upload automatically
    the bundle file on files.kde.org digiKam area through ssh. A valid ssh key configured
    with remote KDE server need to be set previously with the account. You can load ssh key
    at startup following instruction give at this url:

    https://unix.stackexchange.com/questions/90853/how-can-i-run-ssh-add-automatically-without-password-prompt

    Note: the bundle can be signed with GPG. You must setup your private and public keys before
    and put your passphare to a text file (~/.gnupg/dkorg-gpg-pwd.txt)

* EXTRA SCRIPTS

    makeall.sh

    This script allows to build whole 64 bits installers from scratch.

    update.sh

    This script re-build only digiKam and installers 64 bits.

* SSH CONFIGURATION

    Git-bash include openssh for windows which needs some tune to work properly with the remote KDE servers:

    - To register the git credential with your RSA key stored in C:/Users/${USER}/.ssh,
      create a C:/Users/${USER}/.bash_profile with this code:

    ```
    eval `ssh-agent`

    added_keys=`ssh-add -l`
    echo "Registered ssh keys: $added_keys"

    if [ ! $(echo $added_keys | grep -o -e "$HOME/.ssh/id_rsa") ] ; then
        ssh-add "$HOME/.ssh/id_rsa" &
    fi
    ```
    Git password will be ask once when the git-bash terminal session will be open.

    - To prevent unexpected disconnections from the remote server,
      create a C:/Users/{USER}/.ssh/ssh_config with this settings:

    ```
    Host *
    ServerAliveInterval 20
    TCPKeepAlive no
    ```

* LICENSES

    The scripts and other contents of this package are licensed
    under the GNU General Public License version 2, or any later version.
