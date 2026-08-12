#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Clang tidy lint static analyzer on whole digiKam source code.
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

if ! which run-cland-tidy.py ; then

    echo "CLANG-TIDY tool from LLVM is not installed!"
    echo "See https://clang.llvm.org/extra/clang-tidy/ for details."
    exit -1

else

    CLANG_TIDY_BIN=run-clang-tidy.py

fi

echo "Found CLANG-TIDY tool: $CLANG_TIDY_BIN"

ORIG_WD="`pwd`"
REPORT_DIR="$PWD/report.tidy"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "Clang Tidy Static Analyzer task name: $TITLE"

# Clean up and prepare to scan.

rm -fr $REPORT_DIR

mkdir -p $REPORT_DIR

rm -fr ../../build.tidy
mkdir -p ../../build.tidy

$CLANG_TIDY_BIN -quiet -j$CPU_CORES -p  ../../build.tidy/ | tee $REPORT_DIR/clang-tidy.log

python3 ./clangtidy_visualizer.py $REPORT_DIR/clang-tidy.log

#rm -f $REPORT_DIR/clang-tidy.log
mv tidy.html $REPORT_DIR/index.html

if [[ $1 != "--nowebupdate" ]] ; then

    updateOnlineReport "tidy" $REPORT_DIR $TITLE $(parseGitBranch)

fi

cd $ORIG_DIR

rm -fr ../../build.tidy

TerminateScript
