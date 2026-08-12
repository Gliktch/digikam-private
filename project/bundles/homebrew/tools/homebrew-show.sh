#!/bin/bash

# Hide the Homebrew directory during debugging so no libraries accidentally pick up a Homebrew file

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# halt on error
set -e

. ../config.sh
. ../common.sh
StartScript
ChecksRunAsRoot

if [[ -d $INSTALL_PREFIX.hide ]] && [[ ! -d $INSTALL_PREFIX ]] ; then
    sudo mv $INSTALL_PREFIX.hide $INSTALL_PREFIX
fi

TerminateScript