#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Script to sign bundle files with a GPG key
# GPG key password must be present as simple text file in ~/.gnupg/dkorg-gpg-pwd.txt

ALLFILES=$(find . -type f -maxdepth 1)

for BUNDLE in $ALLFILES ; do

    BASENAME=$(basename $BUNDLE)

    if [[ $BASENAME != $(basename $BASH_SOURCE) ]] && [[ $BASENAME != ".." ]] && [[ $BASENAME != "." ]] ; then

        cat ~/.gnupg/dkorg-gpg-pwd.txt | gpg --batch --yes --passphrase-fd 0 -stabv "$BUNDLE" && mv "$BUNDLE.asc" "$BUNDLE.sig"
    fi

done
