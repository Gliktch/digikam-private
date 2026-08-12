#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Krazy static analyzer on whole digiKam source code.
# https://github.com/Krazy-collection/krazy
#
# Production revision: master
# Production patch   : ./krazy.patch
#
# Dependencies:
#
#  - Perl:Tie::IxHash and Perl:XML::LibXML modules at run-time.
#  - Saxon 9HE java xml parser (saxon.jar) to export report as HTML [https://www.saxonica.com/download/java.xml].
#    Production files: saxon9he.jar  saxon9-test.jar  saxon9-xqj.jar
#
# If '--nowebupdate' is passed as argument, static analyzer results are not pushed online at
# https://files.kde.org/digikam/reports/ (default yes).
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

. ./common.sh

StartScript
checksCPUCores

# Check run-time dependencies

if [ ! -f /opt/saxon/saxon9he.jar ] ; then

    echo "Java Saxon 9HE XML parser is not installed in /opt/saxon."
    echo "Please install Saxon from https://www.saxonica.com/download/java.xml"
    echo "Aborted..."
    exit -1

else

    echo "Check Java Saxon 9HE XML parser passed..."

fi

if [ ! -f /opt/krazy/bin/krazy2all ] ; then

    echo "Krazy Static analyzer is not installed in /opt/krazy."
    echo "Please install Krazy from https://github.com/Krazy-collection/krazy"
    echo "Aborted..."
    exit -1

else

    echo "Check Krazy static analyzer passed..."

fi

export PATH=$PATH:/opt/krazy/bin

ORIG_WD="`pwd`"
REPORT_DIR="report.krazy"

# Get active git branches to create report description string

TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "Krazy Static Analyzer task name: $TITLE"

rm -fr $ORIG_WD/$REPORT_DIR

# Compute static analyzer output as XML

krazy2all --export xml \
          --title $TITLE \
          --no-brief \
          --strict all \
          --priority all \
          --verbose \
          --check-set kde5,foss \
          --extra null,contractions,defines,license-reuse,copyright-reuse,qmacros \
          --exclude qclasses,license \
          --exclude-types qml,python,qdoc,perl \
          --topdir ../../ \
          --config ../../.krazy \
          --outfile ./report.krazy.xml \
          || true

# Clean up XML file

sed -i "s/repo-rev value=\"unknown\"/repo-rev value=\"$(parseGitBranch)$(parseGitHash)\"/g" ./report.krazy.xml

DROP_PATH=$(echo $ORIG_WD | rev | cut -d'/' -f3- | rev | sed 's_/_\\/_g')
sed -i "s/$DROP_PATH//g" ./report.krazy.xml

mkdir -p $ORIG_WD/$REPORT_DIR

# Process XML file to generate HTML

java -jar /opt/saxon/saxon9he.jar \
     -o:$REPORT_DIR/index.html \
     -im:krazy2ebn \
     ./report.krazy.xml \
     ./krazy/krazy-main.xsl \
     module=graphics \
     component= \
     submodule=digikam

cp ./krazy/style.css $ORIG_WD/$REPORT_DIR/

if [[ $1 != "--nowebupdate" ]] ; then

    cd $ORIG_WD
    updateOnlineReport "krazy" $REPORT_DIR $TITLE $(parseGitBranch)

fi

cd $ORIG_WD

TerminateScript
