/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-04
 * Description : a tool to adjust date time stamp of images
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjust.h"

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QFile>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dimg.h"
#include "dmetadata.h"
#include "dlayoutbox.h"
#include "dfileoperations.h"

namespace DigikamBqmTimeAdjustPlugin
{

TimeAdjust::TimeAdjust(QObject* const parent)
    : BatchTool(QLatin1String("TimeAdjust"), MetadataTool, parent)
{
}

BatchTool* TimeAdjust::clone(QObject* const parent) const
{
    return new TimeAdjust(parent);
}

void TimeAdjust::registerSettingsWidget()
{
    DVBox* const vbox = new DVBox;
    m_taWidget        = new TimeAdjustSettings(vbox);
    m_settingsWidget  = vbox;

    connect(m_taWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings TimeAdjust::defaultSettings()
{
    BatchToolSettings settings;
    TimeAdjustContainer prm;

    settings.insert(QLatin1String("Custom Date"),                   prm.customDate);
    settings.insert(QLatin1String("Custom Time"),                   prm.customTime);

    settings.insert(QLatin1String("Adjustment Type"),               prm.adjustmentType);
    settings.insert(QLatin1String("Adjustment Days"),               prm.adjustmentDays);
    settings.insert(QLatin1String("Adjustment Time"),               prm.adjustmentTime);

    settings.insert(QLatin1String("Update Only If Available Time"), prm.updIfAvailable);
    settings.insert(QLatin1String("Update File Modification Time"), prm.updFileModDate);
    settings.insert(QLatin1String("Update EXIF Modification Time"), prm.updEXIFModDate);
    settings.insert(QLatin1String("Update EXIF Original Time"),     prm.updEXIFOriDate);
    settings.insert(QLatin1String("Update EXIF Digitization Time"), prm.updEXIFDigDate);
    settings.insert(QLatin1String("Update EXIF Thumbnail Time"),    prm.updEXIFThmDate);
    settings.insert(QLatin1String("Update IPTC Time"),              prm.updIPTCDate);
    settings.insert(QLatin1String("Update XMP Video Time"),         prm.updXMPVideo);
    settings.insert(QLatin1String("Update XMP Creation Time"),      prm.updXMPDate);

    settings.insert(QLatin1String("Use Timestamp Type"),            prm.dateSource);
    settings.insert(QLatin1String("Meta Timestamp Type"),           prm.metadataSource);
    settings.insert(QLatin1String("File Timestamp Type"),           prm.fileDateSource);

    return settings;
}

void TimeAdjust::slotAssignSettings2Widget()
{
    m_changeSettings   = false;

    TimeAdjustContainer prm;

    prm.customDate     = settings().value(QLatin1String("Custom Date")).toDateTime();
    prm.customTime     = settings().value(QLatin1String("Custom Time")).toDateTime();

    prm.adjustmentType = settings().value(QLatin1String("Adjustment Type")).toInt();
    prm.adjustmentDays = settings().value(QLatin1String("Adjustment Days")).toInt();
    prm.adjustmentTime = settings().value(QLatin1String("Adjustment Time")).toDateTime();

    prm.updIfAvailable = settings().value(QLatin1String("Update Only If Available Time")).toBool();
    prm.updFileModDate = settings().value(QLatin1String("Update File Modification Time")).toBool();
    prm.updEXIFModDate = settings().value(QLatin1String("Update EXIF Modification Time")).toBool();
    prm.updEXIFOriDate = settings().value(QLatin1String("Update EXIF Original Time")).toBool();
    prm.updEXIFDigDate = settings().value(QLatin1String("Update EXIF Digitization Time")).toBool();
    prm.updEXIFThmDate = settings().value(QLatin1String("Update EXIF Thumbnail Time")).toBool();
    prm.updIPTCDate    = settings().value(QLatin1String("Update IPTC Time")).toBool();
    prm.updXMPVideo    = settings().value(QLatin1String("Update XMP Video Time")).toBool();
    prm.updXMPDate     = settings().value(QLatin1String("Update XMP Creation Time")).toBool();

    prm.dateSource     = settings().value(QLatin1String("Use Timestamp Type")).toInt();
    prm.metadataSource = settings().value(QLatin1String("Meta Timestamp Type")).toInt();
    prm.fileDateSource = settings().value(QLatin1String("File Timestamp Type")).toInt();

    m_taWidget->setSettings(prm);

    m_changeSettings   = true;
}

void TimeAdjust::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        TimeAdjustContainer prm = m_taWidget->settings();
        BatchToolSettings settings;

        settings.insert(QLatin1String("Custom Date"),                   prm.customDate);
        settings.insert(QLatin1String("Custom Time"),                   prm.customTime);

        settings.insert(QLatin1String("Adjustment Type"),               prm.adjustmentType);
        settings.insert(QLatin1String("Adjustment Days"),               prm.adjustmentDays);
        settings.insert(QLatin1String("Adjustment Time"),               prm.adjustmentTime);

        settings.insert(QLatin1String("Update Only If Available Time"), prm.updIfAvailable);
        settings.insert(QLatin1String("Update File Modification Time"), prm.updFileModDate);
        settings.insert(QLatin1String("Update EXIF Modification Time"), prm.updEXIFModDate);
        settings.insert(QLatin1String("Update EXIF Original Time"),     prm.updEXIFOriDate);
        settings.insert(QLatin1String("Update EXIF Digitization Time"), prm.updEXIFDigDate);
        settings.insert(QLatin1String("Update EXIF Thumbnail Time"),    prm.updEXIFThmDate);
        settings.insert(QLatin1String("Update IPTC Time"),              prm.updIPTCDate);
        settings.insert(QLatin1String("Update XMP Video Time"),         prm.updXMPVideo);
        settings.insert(QLatin1String("Update XMP Creation Time"),      prm.updXMPDate);

        settings.insert(QLatin1String("Use Timestamp Type"),            prm.dateSource);
        settings.insert(QLatin1String("Meta Timestamp Type"),           prm.metadataSource);
        settings.insert(QLatin1String("File Timestamp Type"),           prm.fileDateSource);

        BatchTool::slotSettingsChanged(settings);
    }
}

bool TimeAdjust::toolOperations()
{
    bool metaLoadState                 = true;
    bool metadataChanged               = false;

    TimeAdjustContainer prm;

    prm.customDate                     = settings().value(QLatin1String("Custom Date")).toDateTime();
    prm.customTime                     = settings().value(QLatin1String("Custom Time")).toDateTime();

    prm.adjustmentType                 = settings().value(QLatin1String("Adjustment Type")).toInt();
    prm.adjustmentDays                 = settings().value(QLatin1String("Adjustment Days")).toInt();
    prm.adjustmentTime                 = settings().value(QLatin1String("Adjustment Time")).toDateTime();

    prm.updIfAvailable                 = settings().value(QLatin1String("Update Only If Available Time")).toBool();
    prm.updFileModDate                 = settings().value(QLatin1String("Update File Modification Time")).toBool();
    prm.updEXIFModDate                 = settings().value(QLatin1String("Update EXIF Modification Time")).toBool();
    prm.updEXIFOriDate                 = settings().value(QLatin1String("Update EXIF Original Time")).toBool();
    prm.updEXIFDigDate                 = settings().value(QLatin1String("Update EXIF Digitization Time")).toBool();
    prm.updEXIFThmDate                 = settings().value(QLatin1String("Update EXIF Thumbnail Time")).toBool();
    prm.updIPTCDate                    = settings().value(QLatin1String("Update IPTC Time")).toBool();
    prm.updXMPVideo                    = settings().value(QLatin1String("Update XMP Video Time")).toBool();
    prm.updXMPDate                     = settings().value(QLatin1String("Update XMP Creation Time")).toBool();

    prm.dateSource                     = settings().value(QLatin1String("Use Timestamp Type")).toInt();
    prm.metadataSource                 = settings().value(QLatin1String("Meta Timestamp Type")).toInt();
    prm.fileDateSource                 = settings().value(QLatin1String("File Timestamp Type")).toInt();

    QScopedPointer<DMetadata> meta(new DMetadata);

    if (prm.metadataSource != TimeAdjustContainer::XMPCREATED)
    {
        meta->setUseXMPSidecar4Reading(false);
    }

    if (image().isNull())
    {
        metaLoadState = meta->load(inputUrl().toLocalFile());
    }
    else
    {
        meta->setData(image().getMetadata());
    }

    QString exifDateTimeFormat         = QLatin1String("yyyy:MM:dd hh:mm:ss");
    QString xmpDateTimeFormat          = QLatin1String("yyyy-MM-ddThh:mm:ss");

    const QMap<QString, bool>& tagsMap = prm.getDateTimeTagsMap();
    QMap<QString, bool>::const_iterator it;

    QDateTime orgDateTime;

    switch (prm.dateSource)
    {
        case TimeAdjustContainer::METADATADATE:
        {
            switch (prm.metadataSource)
            {
                case TimeAdjustContainer::EXIFIPTCXMP:
                {
                    orgDateTime = meta->getItemDateTime();
                    break;
                }

                case TimeAdjustContainer::EXIFCREATED:
                {
                    orgDateTime = QDateTime::fromString(meta->getExifTagString("Exif.Image.DateTime"),
                                                        Qt::ISODate);
                    break;
                }

                case TimeAdjustContainer::EXIFORIGINAL:
                {
                    orgDateTime = QDateTime::fromString(meta->getExifTagString("Exif.Photo.DateTimeOriginal"),
                                                        Qt::ISODate);
                    break;
                }

                case TimeAdjustContainer::EXIFDIGITIZED:
                {
                    orgDateTime = QDateTime::fromString(meta->getExifTagString("Exif.Photo.DateTimeDigitized"),
                                                        Qt::ISODate);
                    break;
                }

                case TimeAdjustContainer::IPTCCREATED:
                {
                    orgDateTime = QDateTime(QDate::fromString(meta->getIptcTagString("Iptc.Application2.DateCreated"),
                                                              Qt::ISODate),
                                            QTime::fromString(meta->getIptcTagString("Iptc.Application2.TimeCreated"),
                                                              Qt::ISODate));
                    break;
                }

                case TimeAdjustContainer::XMPCREATED:
                {
                    orgDateTime = QDateTime::fromString(meta->getXmpTagString("Xmp.xmp.CreateDate"),
                                                        Qt::ISODate);
                    break;
                }

                case TimeAdjustContainer::FUZZYCREATED:
                {
                    orgDateTime = prm.getDateTimeFromString(meta->getExifTagString("Exif.Image.DateTime"));
                    break;
                }

                case TimeAdjustContainer::FUZZYORIGINAL:
                {
                    orgDateTime = prm.getDateTimeFromString(meta->getExifTagString("Exif.Photo.DateTimeOriginal"));
                    break;
                }

                case TimeAdjustContainer::FUZZYDIGITIZED:
                {
                    orgDateTime = prm.getDateTimeFromString(meta->getExifTagString("Exif.Photo.DateTimeDigitized"));
                    break;
                }

                default:
                {
                    // orgDateTime stays invalid
                    break;
                }
            };

            break;
        }

        case TimeAdjustContainer::CUSTOMDATE:
        {
            orgDateTime = QDateTime(prm.customDate.date(), prm.customTime.time());
            break;
        }

        case TimeAdjustContainer::FILENAME:
        {
            orgDateTime = prm.getDateTimeFromString(inputUrl().fileName());
            break;
        }

        case TimeAdjustContainer::FILEDATE:
        {
            orgDateTime = imageInfo().modDateTime();
            break;
        }

        default: // TimeAdjustContainer::APPDATE
        {
            orgDateTime = imageInfo().dateTime();
            break;
        }
    }

    if (!metaLoadState && (prm.dateSource != TimeAdjustContainer::CUSTOMDATE))
    {
        orgDateTime = imageInfo().modDateTime();
    }

    QDateTime dt = asDateTimeLocal(prm.calculateAdjustedDate(orgDateTime));

    if (!dt.isValid())
    {
        return false;
    }

    if (metaLoadState)
    {
        for (it = tagsMap.constBegin() ; it != tagsMap.constEnd() ; ++it)
        {
            if (!it.value())
            {
                continue;
            }

            if      (it.key().startsWith(QLatin1String("Exif.")))
            {
                if (
                    !prm.updIfAvailable ||
                    !meta->getExifTagString(it.key().toLatin1().constData()).isEmpty()
                   )
                {
                    meta->setExifTagString(it.key().toLatin1().constData(),
                                           dt.toString(exifDateTimeFormat));

                    metadataChanged = true;
                }
            }
            else if (it.key().startsWith(QLatin1String("Iptc.")))
            {
                if (
                    !prm.updIfAvailable ||
                    !meta->getIptcTagString(it.key().toLatin1().constData()).isEmpty()
                   )
                {
                    if      (it.key().contains(QLatin1String("Date")))
                    {
                        meta->setIptcTagString(it.key().toLatin1().constData(),
                                               dt.date().toString(Qt::ISODate));

                        metadataChanged = true;
                    }
                    else if (it.key().contains(QLatin1String("Time")))
                    {
                        meta->setIptcTagString(it.key().toLatin1().constData(),
                                               dt.time().toString(Qt::ISODate));

                        metadataChanged = true;
                    }
                }
            }
            else if (it.key().startsWith(QLatin1String("Xmp.")) && meta->supportXmp())
            {
                if (
                    !prm.updIfAvailable ||
                    !meta->getXmpTagString(it.key().toLatin1().constData()).isEmpty()
                   )
                {
                    meta->setXmpTagString(it.key().toLatin1().constData(),
                                          dt.toString(xmpDateTimeFormat));

                    metadataChanged = true;
                }
            }
        }
    }

    bool ret = true;

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret = DFileOperations::copyFile(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (ret && metadataChanged && metaLoadState)
        {
            ret = meta->save(outputUrl().toLocalFile());
        }
    }
    else
    {
        if (metadataChanged)
        {
            image().setMetadata(meta->data());
        }

        ret = savefromDImg();
    }

    if (ret && prm.updFileModDate)
    {
        ret = DFileOperations::setModificationTime(outputUrl().toLocalFile(), dt);
    }

    return ret;
}

} // namespace DigikamBqmTimeAdjustPlugin

#include "moc_timeadjust.cpp"
