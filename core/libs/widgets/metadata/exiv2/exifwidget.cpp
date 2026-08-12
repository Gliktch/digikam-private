/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display Standard Exif metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exifwidget.h"

// Qt includes

#include <QMap>
#include <QFile>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"
#include "metadatalistview.h"

namespace
{

/**
 * Standard Exif entry list from the less important to the most important for photograph.
 */
static const char* StandardExifEntryList[] =
{
    "Iop",
    "Thumbnail",
    "SubImage1",
    "SubImage2",
    "Image",
    "Photo",
    "GPSInfo",
    "-1"
};

} // namespace

namespace Digikam
{

ExifWidget::ExifWidget(QWidget* const parent, const QString& name)
    : MetadataWidget(parent, name)
{
    setup();

    for (int i = 0 ; QLatin1String(StandardExifEntryList[i]) != QLatin1String("-1") ; ++i)
    {
        m_keysFilter << QLatin1String(StandardExifEntryList[i]);
    }
}

QString ExifWidget::getMetadataTitle() const
{
    return i18n("Standard Exif Tags");
}

bool ExifWidget::loadFromURL(const QUrl& url)
{
    setFileName(url.fileName());

    if (url.isEmpty())
    {
        setMetadata();

        return false;
    }
    else
    {
        QScopedPointer<DMetadata> metadata(new DMetadata(url.toLocalFile()));

        if (!metadata->hasExif())
        {
            setMetadata();

            return false;
        }
        else
        {
            setMetadata(*metadata);
        }
    }

    return true;
}

bool ExifWidget::decodeMetadata()
{
    QScopedPointer<DMetadata> data(new DMetadata(getMetadata()->data()));

    if (!data->hasExif())
    {
        return false;
    }

    // Update all metadata contents.

    setMetadataMap(data->getExifTagsDataList(QStringList(),
                                             false,
                                             false)); // Do not extract binary data which can introduce time latency in GUI.

    return true;
}

void ExifWidget::buildView()
{
    switch (getMode())
    {
        case CUSTOM:
        {
            setIfdList(getMetadataMap(), m_keysFilter, getTagsFilter());

            break;
        }

        case PHOTO:
        {
            setIfdList(getMetadataMap(), m_keysFilter, QStringList() << QLatin1String("FULL"));

            break;
        }

        default: // NONE
        {
            setIfdList(getMetadataMap(), m_keysFilter, QStringList());

            break;
        }
    }

    MetadataWidget::buildView();
}

QString ExifWidget::getTagTitle(const QString& key)
{
    QScopedPointer<DMetadata> metadataIface(new DMetadata);
    QString title = metadataIface->getExifTagTitle(key.toLatin1().constData());

    if (title.isEmpty())
    {
        return key.section(QLatin1Char('.'), -1);
    }

    return title;
}

QString ExifWidget::getTagDescription(const QString& key)
{
    QScopedPointer<DMetadata> metadataIface(new DMetadata);
    QString desc = metadataIface->getExifTagDescription(key.toLatin1().constData());

    if (desc.isEmpty())
    {
        return i18n("No description available");
    }

    return desc;
}

void ExifWidget::slotSaveMetadataToFile()
{
    QUrl url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString(QLatin1String("*.exif|") + i18n("EXIF binary Files (*.exif)")));

    storeMetadataToFile(url, getMetadata()->getExifEncoded());
}

} // namespace Digikam

#include "moc_exifwidget.cpp"
