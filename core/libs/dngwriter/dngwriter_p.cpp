/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngwriter_p.h"

// C++ includes

#include <cstdio>

// Qt includes

#include <QFile>

// KDE includes

#include "digikam_debug.h"

// Local includes

#include "dngwriter.h"
#include "dngwriterhost.h"

namespace Digikam
{

DNGWriter::Private::Private(DNGWriter* const dd)
    : parent(dd)
{
    reset();
}

void DNGWriter::Private::reset()
{
    cancel                  = false;
    jpegLossLessCompression = true;
    updateFileDate          = false;
    backupOriginalRawFile   = false;
    previewMode             = DNGWriter::FULL_SIZE;
}

void DNGWriter::Private::cleanup()
{
    if (::remove(QFile::encodeName(outputFile).constData()) != 0)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot remove " << outputFile;
    }
}

dng_date_time DNGWriter::Private::dngDateTime(const QDateTime& qDT) const
{
    dng_date_time dngDT;
    dngDT.fYear   = qDT.date().year();
    dngDT.fMonth  = qDT.date().month();
    dngDT.fDay    = qDT.date().day();
    dngDT.fHour   = qDT.time().hour();
    dngDT.fMinute = qDT.time().minute();
    dngDT.fSecond = qDT.time().second();

    return dngDT;
}

bool DNGWriter::Private::fujiRotate(QByteArray& rawData, DRawInfo& identify) const
{
    QByteArray tmpData(rawData);
    int height             = identify.outputSize.height();
    int width              = identify.outputSize.width();
    unsigned short* tmp    = reinterpret_cast<unsigned short*>(tmpData.data());
    unsigned short* output = reinterpret_cast<unsigned short*>(rawData.data());

    for (int row = 0 ; row < height ; ++row)
    {
        for (int col = 0 ; col < width ; ++col)
        {
            output[col * height + row] = tmp[row * width + col];
        }
    }

    identify.orientation = DRawInfo::ORIENTATION_Mirror90CCW;
    identify.outputSize  = QSize(height, width);

    // TODO: rotate margins

    return true;
}

QString DNGWriter::Private::dngErrorCodeToString(int errorCode) const
{
    switch (errorCode)
    {
        default:
        case 100000:
        {
            return QLatin1String("Unknown error");
        }

        case 100003:
        {
            return QLatin1String("Processing stopped by user (or host application) request");
        }

        case 100004:
        {
            return QLatin1String("Necessary host functionality is not present");
        }

        case 100005:
        {
            return QLatin1String("Out of memory");
        }

        case 100006:
        {
            return QLatin1String("File format is not valid");
        }

        case 100007:
        {
            return QLatin1String("Matrix has wrong shape, is badly conditioned, or similar problem");
        }

        case 100008:
        {
            return QLatin1String("Could not open file");
        }

        case 100009:
        {
            return QLatin1String("Error reading file");
        }

        case 100010:
        {
            return QLatin1String("Error writing file");
        }

        case 100011:
        {
            return QLatin1String("Unexpected end of file");
        }

        case 100012:
        {
            return QLatin1String("File is damaged in some way");
        }

        case 100013:
        {
            return QLatin1String("Image is too big to save as DNG");
        }

        case 100014:
        {
            return QLatin1String("Image is too big to save as TIFF");
        }

        case 100015:
        {
            return QLatin1String("DNG version is unsupported");
        }
    }
}

QString DNGWriter::Private::dngBayerPatternToString(int pattern) const
{
    switch (pattern)
    {
        case LinearRaw:
        {
            return QLatin1String("Bayer Linear Raw Pattern Mosaic");
        }

        case Standard:
        {
            return QLatin1String("Bayer Standard Pattern Mosaic");
        }

        case Fuji:
        {
            return QLatin1String("Bayer Fuji Pattern Mosaic");
        }

        case Fuji6x6:
        {
            return QLatin1String("Bayer Fuji 6x6 Pattern Mosaic");
        }

        case FourColor:
        {
            return QLatin1String("Bayer Four Color Pattern Mosaic");
        }

        default:    // Unknown
        {
            return QLatin1String("Bayer Unknown Pattern Mosaic");
        }
    }
}

int DNGWriter::Private::debugExtractedRAWData(const QByteArray& rawData)
{
    QString   rawdataFilePath(inputInfo.completeBaseName() + QLatin1String(".dat"));

    QFile rawdataFile(rawdataFilePath);

    if (!rawdataFile.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Cannot open file to write RAW data. Aborted..." ;

        return PROCESS_FAILED;
    }

    QDataStream rawdataStream(&rawdataFile);
    rawdataStream.writeRawData(rawData.data(), rawData.size());
    rawdataFile.close();

    return PROCESS_CONTINUE;
}

} // namespace Digikam
