/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG - Makernotes storage.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Jens Mueller <tschenser at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngwriter_p.h"

// Local includes

#include "dngwriterhost.h"

namespace Digikam
{

int DNGWriter::Private::storeMakernote(DNGWriterHost& host,
                                       AutoPtr<dng_negative>& negative,
                                       DRawInfo* const identify,
                                       DRawInfo* const identifyMake,
                                       DMetadata* const meta)
{
    if (metaLoaded)     // We only process Makernote storage if Metadata can be loaded at Exif stage.
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Updating Makernote metadata to DNG Negative";

        backportNikonMakerNote(meta);
        backportCanonMakerNote(meta);
        backportPentaxMakerNote(meta);
        backportOlympusMakerNote(meta);
        backportPanasonicMakerNote(meta);
        backportSonyMakerNote(meta);
        storeLensInformation();
        backupMakernote(host, negative, identify, identifyMake, meta);
    }

    if (cancel)
    {
        return PROCESS_CANCELED;
    }

    return PROCESS_CONTINUE;
}

void DNGWriter::Private::backportNikonMakerNote(DMetadata* const meta)
{
    QString  str;
    long int num, den;
    long     val;

    if (meta->getExifTagRational("Exif.Nikon3.Lens", num, den, 0))              exif->fLensInfo[0]              = dng_urational(num, den);
    if (meta->getExifTagRational("Exif.Nikon3.Lens", num, den, 1))              exif->fLensInfo[1]              = dng_urational(num, den);
    if (meta->getExifTagRational("Exif.Nikon3.Lens", num, den, 2))              exif->fLensInfo[2]              = dng_urational(num, den);
    if (meta->getExifTagRational("Exif.Nikon3.Lens", num, den, 3))              exif->fLensInfo[3]              = dng_urational(num, den);
    if (meta->getExifTagLong("Exif.Nikon3.ISOSpeed", val, 1))                   exif->fISOSpeedRatings[1]       = (uint32)val;

    str = meta->getExifTagString("Exif.Nikon3.SerialNO");
    if (!str.isEmpty()) str = str.replace(QLatin1String("NO="), QLatin1String(""));
    if (!str.isEmpty()) exif->fCameraSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

    str = meta->getExifTagString("Exif.Nikon3.SerialNumber");
    if (!str.isEmpty()) exif->fCameraSerialNumber.Set_ASCII(str.toLatin1().constData());

    if (meta->getExifTagLong("Exif.Nikon3.ShutterCount", val))                  exif->fImageNumber              = (uint32)val;
    if (meta->getExifTagLong("Exif.NikonLd1.LensIDNumber", val))                exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
    if (meta->getExifTagLong("Exif.NikonLd2.LensIDNumber", val))                exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
    if (meta->getExifTagLong("Exif.NikonLd3.LensIDNumber", val))                exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
    if (meta->getExifTagLong("Exif.NikonLd2.FocusDistance", val))               exif->fSubjectDistance          = dng_urational((uint32)pow(10.0, val/40.0), 100);
    if (meta->getExifTagLong("Exif.NikonLd3.FocusDistance", val))               exif->fSubjectDistance          = dng_urational((uint32)pow(10.0, val/40.0), 100);
    str = meta->getExifTagString("Exif.NikonLd1.LensIDNumber");
    if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
    str = meta->getExifTagString("Exif.NikonLd2.LensIDNumber");
    if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
    str = meta->getExifTagString("Exif.NikonLd3.LensIDNumber");
    if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
}

void DNGWriter::Private::backportCanonMakerNote(DMetadata* const meta)
{
    QString  str;
    long int num, den;
    long     val;

    if (meta->getExifTagLong("Exif.Canon.SerialNumber", val))                   exif->fCameraSerialNumber.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
/*
    if (meta->getExifTagLong("Exif.CanonCs.FlashActivity", val))                exif->fFlash                    = (uint32)val;
    if (meta->getExifTagLong("Exif.CanonFi.FileNumber", val))                   exif->fImageNumber              = (uint32)val;
    if (meta->getExifTagLong("Exif.CanonCs.MaxAperture", val))                  exif->fMaxApertureValue         = dng_urational(val, 100000);
*/
    if (meta->getExifTagLong("Exif.CanonCs.ExposureProgram", val))
    {
        switch (val)
        {
            case 1:
            {
                exif->fExposureProgram = 2;
                break;
            }

            case 2:
            {
                exif->fExposureProgram = 4;
                break;
            }

            case 3:
            {
                exif->fExposureProgram = 3;
                break;
            }

            case 4:
            {
                exif->fExposureProgram = 1;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    if (meta->getExifTagLong("Exif.CanonCs.MeteringMode", val))
    {
        switch (val)
        {
            case 1:
            {
                exif->fMeteringMode = 3;
                break;
            }

            case 2:
            {
                exif->fMeteringMode = 1;
                break;
            }

            case 3:
            {
                exif->fMeteringMode = 5;
                break;
            }

            case 4:
            {
                exif->fMeteringMode = 6;
                break;
            }

            case 5:
            {
                exif->fMeteringMode = 2;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    long canonLensUnits = 1;
    if (meta->getExifTagRational("Exif.CanonCs.Lens", num, den, 2))             canonLensUnits                  = num;
    if (meta->getExifTagRational("Exif.CanonCs.Lens", num, den, 0))             exif->fLensInfo[1]              = dng_urational(num, canonLensUnits);
    if (meta->getExifTagRational("Exif.CanonCs.Lens", num, den, 1))             exif->fLensInfo[0]              = dng_urational(num, canonLensUnits);
    if (meta->getExifTagRational("Exif.Canon.FocalLength", num, den, 1))        exif->fFocalLength              = dng_urational(num, canonLensUnits);
    long canonLensType = 65535;

    if (meta->getExifTagLong("Exif.CanonCs.LensType", canonLensType))           exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(canonLensType)).toLatin1().constData());
    str = meta->getExifTagString("Exif.Canon.LensModel");

    if      (!str.isEmpty())
    {
        exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
    }
    else if (canonLensType != 65535)
    {
        str = meta->getExifTagString("Exif.CanonCs.LensType");
        if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
    }

    str = meta->getExifTagString("Exif.Canon.OwnerName");
    if (!str.isEmpty()) exif->fOwnerName.Set_ASCII(str.trimmed().toLatin1().constData());

    str = meta->getExifTagString("Exif.Canon.FirmwareVersion");
    if (!str.isEmpty()) str = str.replace(QLatin1String("Firmware"), QLatin1String(""));
    if (!str.isEmpty()) str = str.replace(QLatin1String("Version"), QLatin1String(""));
    if (!str.isEmpty()) exif->fFirmware.Set_ASCII(str.trimmed().toLatin1().constData());

    str = meta->getExifTagString("Exif.CanonSi.ISOSpeed");
    if (!str.isEmpty()) exif->fISOSpeedRatings[1] = str.toInt();
}

void DNGWriter::Private::backportPentaxMakerNote(DMetadata* const meta)
{
    QString str = meta->getExifTagString("Exif.Pentax.LensType");

    if (!str.isEmpty())
    {
        exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
        exif->fLensName.TrimLeadingBlanks();
        exif->fLensName.TrimTrailingBlanks();
    }

    long pentaxLensId1 = 0;
    long pentaxLensId2 = 0;

    if ((meta->getExifTagLong("Exif.Pentax.LensType", pentaxLensId1, 0)) &&
        (meta->getExifTagLong("Exif.Pentax.LensType", pentaxLensId2, 1)))
    {
        exif->fLensID.Set_ASCII(QString::fromUtf8("%1").arg(pentaxLensId1, pentaxLensId2).toLatin1().constData());
    }
}

void DNGWriter::Private::backportOlympusMakerNote(DMetadata* const meta)
{
    QString  str;
    long     val;

    str = meta->getExifTagString("Exif.OlympusEq.SerialNumber");
    if (!str.isEmpty()) exif->fCameraSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

    str = meta->getExifTagString("Exif.OlympusEq.LensSerialNumber");
    if (!str.isEmpty()) exif->fLensSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

    str = meta->getExifTagString("Exif.OlympusEq.LensModel");
    if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());

    if (meta->getExifTagLong("Exif.OlympusEq.MinFocalLength", val))             exif->fLensInfo[0]              = dng_urational(val, 1);
    if (meta->getExifTagLong("Exif.OlympusEq.MaxFocalLength", val))             exif->fLensInfo[1]              = dng_urational(val, 1);
}

void DNGWriter::Private::backportPanasonicMakerNote(DMetadata* const meta)
{
    QString  str;

    str = meta->getExifTagString("Exif.Panasonic.LensType");
    if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());

    str = meta->getExifTagString("Exif.Panasonic.LensSerialNumber");
    if (!str.isEmpty()) exif->fLensSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());
}

void DNGWriter::Private::backportSonyMakerNote(DMetadata* const meta)
{
    QString  str;
    long     val;

    if (meta->getExifTagLong("Exif.Sony2.LensID", val))
    {
        exif->fLensID.Set_ASCII(QString::fromUtf8("%1").arg(val).toLatin1().constData());
    }

    str = meta->getExifTagString("Exif.Photo.LensModel");

    if (str.isEmpty())
    {
        str = meta->getExifTagString("Exif.Sony2.LensID");
    }

    if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
}

void DNGWriter::Private::storeLensInformation()
{
    if ((exif->fLensName.IsEmpty())          &&
        (exif->fLensInfo[0].As_real64() > 0) &&
        (exif->fLensInfo[1].As_real64() > 0))
    {
        QString     lensName;
        QTextStream stream(&lensName);
        double      dval = (double)exif->fLensInfo[0].n / (double)exif->fLensInfo[0].d;
        stream << QString::fromUtf8("%1").arg(dval, 0, 'f', 1);

        if (exif->fLensInfo[0].As_real64() != exif->fLensInfo[1].As_real64())
        {
            dval = (double)exif->fLensInfo[1].n / (double)exif->fLensInfo[1].d;
            stream << QString::fromUtf8("-%1").arg(dval, 0, 'f', 1);
        }

        stream << " mm";

        if (exif->fLensInfo[2].As_real64() > 0)
        {
            dval = (double)exif->fLensInfo[2].n / (double)exif->fLensInfo[2].d;
            stream << QString::fromUtf8(" 1/%1").arg(dval, 0, 'f', 1);
        }

        if ((exif->fLensInfo[3].As_real64() > 0) &&
            (exif->fLensInfo[2].As_real64() != exif->fLensInfo[3].As_real64()))
        {
            dval = (double)exif->fLensInfo[3].n / (double)exif->fLensInfo[3].d;
            stream << QString::fromUtf8("-%1").arg(dval, 0, 'f', 1);
        }

        exif->fLensName.Set_ASCII(lensName.toLatin1().constData());
    }
}

void DNGWriter::Private::backupMakernote(DNGWriterHost& host,
                                         AutoPtr<dng_negative>& negative,
                                         DRawInfo* const /*identify*/,
                                         DRawInfo* const identifyMake,
                                         DMetadata* const meta)
{
    // Original Markernote storage in Standard Exif tag.

    long mknOffset       = 0;
    QString mknMake      = identifyMake->make.toUpper();
    QString mknByteOrder = meta->getExifTagString("Exif.MakerNote.ByteOrder");
    QByteArray mknData   = meta->getExifTagData("Exif.Photo.MakerNote");
    meta->getExifTagLong("Exif.MakerNote.Offset", mknOffset);

    if (!mknData.isEmpty()                      &&
        ((mknMake == QLatin1String("NIKON"))    ||
         (mknMake == QLatin1String("OLYMPUS"))  ||
         (mknMake == QLatin1String("FUJIFILM")))
       )
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Makernote storage ("
                                     << mknData.size() << " bytes)" ;

        dng_memory_allocator memalloc1(gDefaultDNGMemoryAllocator);
        dng_memory_stream stream(memalloc1);
        stream.Put(mknData.constData(), mknData.size());
        AutoPtr<dng_memory_block> block(host.Allocate(mknData.size()));
        stream.SetReadPosition(0);
        stream.Get(block->Buffer(), mknData.size());

        negative->SetMakerNoteSafety(true);
        negative->SetMakerNote(block);
    }

    // Backup Makernote in DNG private tag.
    // NOTE: Exiv2 report a warning with CR2 conversion:
    // Error: Directory Canon with 25665 entries considered invalid; not read.

    if (!mknData.isEmpty()      &&
        !mknByteOrder.isEmpty() &&
        mknOffset)
    {
        bool padding                        = ((mknData.size() & 0x01) == 0x01);
        dng_memory_stream* const streamPriv = new dng_memory_stream(host.Allocator());
        streamPriv->SetBigEndian();

        // Use Adobe vendor-format for encoding MakerNotes in DNGPrivateTag

        streamPriv->Put("Adobe", 5);
        streamPriv->Put_uint8(0x00);
        streamPriv->Put("MakN", 4);
        streamPriv->Put_uint32(mknData.size() + mknByteOrder.size() + 4 + (padding ? 1 : 0));
        streamPriv->Put(mknByteOrder.toLatin1().constData(), mknByteOrder.toLatin1().size());
        streamPriv->Put_uint32(mknOffset);
        streamPriv->Put(mknData.constData(), mknData.size());

        if (padding)
        {
            streamPriv->Put_uint8(0x00);
        }

        AutoPtr<dng_memory_stream> DNGPrivateTag(streamPriv);

        if (DNGPrivateTag.Get())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Backup Makernote as DNG Private data ("
                                         << DNGPrivateTag->Length() << " bytes)";

            AutoPtr<dng_memory_block> blockPriv(DNGPrivateTag->AsMemoryBlock(host.Allocator()));
            negative->SetPrivateData(blockPriv);
        }
    }
}

} // namespace Digikam
