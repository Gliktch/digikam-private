/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display non standard Exif metadata
 *               used by camera makers
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "makernotewidget.h"

// Qt includes

#include <QMap>
#include <QFile>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"

namespace Digikam
{

namespace
{

static const char* ExifEntryListToIgnore[] =
{
    "GPSInfo",
    "Iop",
    "Thumbnail",
    "SubImage1",
    "SubImage2",
    "Image",
    "Photo",
    "-1"
};

} // namespace

MakerNoteWidget::MakerNoteWidget(QWidget* const parent, const QString& name)
    : MetadataWidget(parent, name)
{
    setup();

    for (int i = 0 ; QLatin1String(ExifEntryListToIgnore[i]) != QLatin1String("-1") ; ++i)
    {
        m_keysFilter << QLatin1String(ExifEntryListToIgnore[i]);
    }
}

QString MakerNoteWidget::getMetadataTitle() const
{
    return i18n("MakerNote Exif Tags");
}

bool MakerNoteWidget::loadFromURL(const QUrl& url)
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

bool MakerNoteWidget::decodeMetadata()
{
    QScopedPointer<DMetadata> data(new DMetadata(getMetadata()->data()));

    if (!data->hasExif())
    {
        return false;
    }

    // Update all metadata contents.

    setMetadataMap(data->getExifTagsDataList(m_keysFilter,
                                             true,
                                             false)); // Do not extract binary data which can introduce time latency in GUI.

    return true;
}

void MakerNoteWidget::buildView()
{
    switch (getMode())
    {
        case CUSTOM:
        {
            setIfdList(getMetadataMap(), getTagsFilter());

            break;
        }

        case PHOTO:
        {
            setIfdList(getMetadataMap(), QStringList() << QLatin1String("FULL"));

            break;
        }

        default: // NONE
        {
            setIfdList(getMetadataMap(), QStringList());

            break;
        }
    }

    MetadataWidget::buildView();
}

QString MakerNoteWidget::getTagTitle(const QString& key)
{
    QScopedPointer<DMetadata> metadataIface(new DMetadata);
    QString title = metadataIface->getExifTagTitle(key.toLatin1().constData());

    if (title.isEmpty())
    {
        return key.section(QLatin1Char('.'), -1);
    }

    return title;
}

QString MakerNoteWidget::getTagDescription(const QString& key)
{
    QScopedPointer<DMetadata> metadataIface(new DMetadata);
    QString desc = metadataIface->getExifTagDescription(key.toLatin1().constData());

    if (desc.isEmpty())
    {
        return i18n("No description available");
    }

    return desc;
}

void MakerNoteWidget::slotSaveMetadataToFile()
{
    QUrl url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString(QLatin1String("*.exif|") + i18n("EXIF binary Files (*.exif)")));

    storeMetadataToFile(url, getMetadata()->getExifEncoded());
}

} // namespace Digikam

#include "moc_makernotewidget.cpp"
