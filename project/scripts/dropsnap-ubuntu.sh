#!/bin/bash

# Script to drop Snap under Ubuntu Linux.
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

if [[ $EUID -ne 0 ]]; then
    echo "This script should be run as root using sudo command."
    exit 1
fi

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

echo "Remove SNAP and install Firefox package"
echo "-------------------------------------------------------------------"

sudo systemctl disable snapd.service
sudo systemctl disable snapd.socket
sudo systemctl disable snapd.seeded.service

sudo snap remove firefox
sudo snap remove snap-store
sudo snap remove gtk-common-themes

sudo snap remove gnome-3-38-2004
sudo snap remove gnome-32-2204
sudo snap remove gnome-42-2204
sudo snap remove firmware-updater
sudo snap remove thunderbird
sudo snap remove core20
sudo snap remove core22
sudo snap remove bare
sudo snap remove snapd
sudo snap remove snapd-desktop-integration

sudo rm -rf /var/cache/snapd/
sudo apt autoremove --purge snapd
rm -rf ~/snap

sudo cat > /etc/apt/preferences.d/firefox-no-snap << EOF
Package: firefox*
Pin: release o=Ubuntu*
Pin-Priority: -1
EOF

sudo apt install software-properties-common
sudo add-apt-repository ppa:mozillateam/ppa

sudo apt install firefox

# Reinstall Plasma Discover without snapd dependency for the system update notifications.

sudo apt install plasma-discover snapd-
sudo apt --fix-broken install
sudo apt install plasma-discover-notifier
