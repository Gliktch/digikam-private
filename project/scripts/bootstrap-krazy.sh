#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copy this script on root folder where are source code
# https://github.com/Krazy-collection/krazy

sudo apt install openjdk-25-jre libsaxonhe-java libxml-perl libxml-libxml-perl libyaml-libyaml-perl libyaml-perl libjson-perl

sudo ln -sf /usr/share/java              /opt/saxon
sudo ln -sf /usr/share/java/Saxon-HE.jar /usr/share/java/saxon9he.jar

sudo QMAKE=/opt/qt6/bin/qmake ./install.sh /opt/krazy


