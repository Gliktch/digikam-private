#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run CppCheck static analyzer on whole digiKam source code.
# http://cppcheck.sourceforge.net/
# Dependencies : Python::pygments module to export report as HTML.
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

# Analyzer configuration.
. ../../.cppcheck

StartScript
checksCPUCores

if [ ! -f /opt/cppcheck/bin/cppcheck ] ; then

    echo "Cppcheck Static analyzer is not installed in /opt/cppcheck."
    echo "Please install Cppcheck from https://github.com/danmar/cppcheck"
    echo "Aborted..."
    exit -1

else

    echo "Check Cppcheck static analyzer passed..."

fi

ORIG_WD="`pwd`"
REPORT_DIR="report.cppcheck"
CPPCHECK_CACHE_DIR="$HOME/.cppcheck_cache/digiKam"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "CppCheck Static Analyzer task name: $TITLE"

if [ ! -d "$CPPCHECK_CACHE_DIR" ]; then
    mkdir -p "$CPPCHECK_CACHE_DIR"
fi

rm -fr $ORIG_WD/$REPORT_DIR

# Print the skipped directories taken from the config file.

for DROP_ITEM in $IGNORE_DIRS ; do

    if [[ $DROP_ITEM != *-i ]] ; then

        echo "Skipped dir: $DROP_ITEM"

    fi

done

echo "Cppcheck defines     : $CPPCHECK_DEFINES"
echo "Cppcheck options     : $CPPCHECK_OPTIONS"
echo "Cppcheck suppressions: $CPPCHECK_SUPPRESSIONS"
echo "Cppcheck cache dir   : $CPPCHECK_CACHE_DIR"

# List sub-dirs with headers to append as cppcheck includes paths
HDIRS=$(find ../../core -name '*.h' -printf '%h\n' | sort -u)

for INCLUDE_PATH in $HDIRS ; do
    INCLUDE_DIRS+="-I $INCLUDE_PATH/ "
done

/opt/cppcheck/bin/cppcheck \
         -j1 \
         $CPPCHECK_DEFINES \
         $CPPCHECK_OPTIONS \
         --enable=all \
         --check-level=exhaustive \
         --max-ctu-depth=4 \
         --cppcheck-build-dir=$CPPCHECK_CACHE_DIR \
         $CPPCHECK_SUPPRESSIONS \
         --xml-version=2 \
         --output-file=report.cppcheck.xml \
         $IGNORE_DIRS \
         $INCLUDE_DIRS \
         ../../core

echo "Generating Cppcheck HTML reports..."

/opt/cppcheck/bin/cppcheck-htmlreport \
                    --file=report.cppcheck.xml \
                    --report-dir=$ORIG_WD/$REPORT_DIR \
                    --source-dir=. \
                    --title=$TITLE

if [[ $1 != "--nowebupdate" ]] ; then

    cd $ORIG_WD
    updateOnlineReport "cppcheck" $REPORT_DIR $TITLE $(parseGitBranch)

fi

cd $ORIG_WD

TerminateScript
