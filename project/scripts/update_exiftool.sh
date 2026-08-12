#!/bin/bash

# Script to update ExifTool online cache to files.kde.org
#
# Arguments : $1 : exiftool version (eg. 12.81).
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

if [ "$1" == "" ] ; then

    echo "Error: missing Exiftool version to handle (eg. 12.81)..."
    exit

fi

DK_UPLOADURL="digikam@tinami.kde.org"
DK_UPLOADDIR="/srv/archives/files/digikam/exiftool/"

set -e
set -C

echo "Download ExifTool archives..."

wget https://sourceforge.net/projects/exiftool/files/Image-ExifTool-$1.tar.gz/download -O Image-ExifTool-$1.tar.gz
wget https://sourceforge.net/projects/exiftool/files/exiftool-$1_64.zip/download       -O exiftool-$1_64.zip

echo "Rename archives..."

mv Image-ExifTool-$1.tar.gz Image-ExifTool.tar.gz
unzip -o exiftool-$1_64.zip -d ./
cd ./exiftool-$1_64/
zip -r ../exiftool.zip ./
cd ..
chmod -R 777 ./exiftool-$1_64/

echo "Cleanup files from download area..."

sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm *.zip"
sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm *.tar.gz"
sftp -q $DK_UPLOADURL:$DK_UPLOADDIR <<< "rm VERSION"

echo "Create new VERSION file..."

cat << EOF > "VERSION"
$1
EOF

echo "Upload new files..."

rsync -r -v --progress -e ssh Image-ExifTool.tar.gz $DK_UPLOADURL:$DK_UPLOADDIR
rsync -r -v --progress -e ssh exiftool.zip          $DK_UPLOADURL:$DK_UPLOADDIR
rsync -r -v --progress -e ssh VERSION               $DK_UPLOADURL:$DK_UPLOADDIR

echo "Cleanup local files..."

rm -fr Image-ExifTool.tar.gz
rm -fr exiftool.zip
rm -fr VERSION
rm -fr exiftool-$1_64.zip
rm -fr exiftool-$1_64

echo "All done..."
