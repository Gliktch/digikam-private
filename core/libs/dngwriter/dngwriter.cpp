/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Jens Mueller <tschenser at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

/**
 * References about DNG:
 * DNG SDK tutorial:    www.adobeforums.com/webx/.3bc2944e
 *                      www.adobeforums.com/webx/.3c054bde
 * DNG review:          www.barrypearson.co.uk/articles/dng/index.htm
 * DNG intro:           www.adobe.com/digitalimag/pdfs/dng_primer.pdf
 *                      www.adobe.com/products/dng/pdfs/DNG_primer_manufacturers.pdf
 * DNG Specification:   wwwimages.adobe.com/content/dam/Adobe/en/products/photoshop/pdfs/dng_spec_1.5.0.0.pdf
 * TIFF/EP Spec.:       www.map.tu.chiba-u.ac.jp/IEC/100/TA2/recdoc/N4378.pdf
 * DNG SDK reference:   www.thomasdideriksen.dk/misc/File%20Formats/dng_sdk_refman.pdf
 * DNG SDK tarball:     helpx.adobe.com/photoshop/digital-negative.html#dng_sdk_download
 * DNG users forum:     www.adobeforums.com/webx/.3bb5f0ec
 *
 * Applications using DNG SDK:
 * DNG4PS2:             dng4ps2.chat.ru/index_en.html
 * CORNERFIX:           sourceforge.net/projects/cornerfix
 * ADOBE DNG CONVERTER: helpx.adobe.com/photoshop/using/adobe-dng-converter.html
 * DNGCONVERT:          github.com/jmue/dngconvert
 * MOVIE2DNG:           elphel.svn.sourceforge.net/svnroot/elphel/tools/Movie2DNG
 * RAW2DNG :            github.com/Fimagena/raw2dng
 */

#include "dngwriter_p.h"

namespace Digikam
{

DNGWriter::DNGWriter()
    : d(new Private(this))
{
    //dng_xmp_sdk::InitializeSDK();
}

DNGWriter::~DNGWriter()
{
    //dng_xmp_sdk::TerminateSDK();

    delete d;
}

void DNGWriter::cancel()
{
    d->cancel = true;
}

void DNGWriter::reset()
{
    d->reset();
}

void DNGWriter::setCompressLossLess(bool b)
{
    d->jpegLossLessCompression = b;
}

bool DNGWriter::compressLossLess() const
{
    return d->jpegLossLessCompression;
}

void DNGWriter::setUpdateFileDate(bool b)
{
    d->updateFileDate = b;
}

bool DNGWriter::updateFileDate() const
{
    return d->updateFileDate;
}

void DNGWriter::setBackupOriginalRawFile(bool b)
{
    d->backupOriginalRawFile = b;
}

bool DNGWriter::backupOriginalRawFile() const
{
    return d->backupOriginalRawFile;
}

void DNGWriter::setPreviewMode(int mode)
{
    d->previewMode = mode;
}

int DNGWriter::previewMode() const
{
    return d->previewMode;
}

void DNGWriter::setInputFile(const QString& filePath)
{
    d->inputFile = filePath;
}

void DNGWriter::setOutputFile(const QString& filePath)
{
    d->outputFile = filePath;
}

QString DNGWriter::inputFile() const
{
    return d->inputFile;
}

QString DNGWriter::outputFile() const
{
    return d->outputFile;
}

QString DNGWriter::xmpSdkVersion()
{
    return QString::fromLatin1(XMPCORE_API_VERSION_STRING);
}

QString DNGWriter::dngSdkVersion()
{
    // NOTE: DNG SDK do not seem to have a version ID shared in header.

    return QString::fromLatin1("1.7.1");
}

} // namespace Digikam
