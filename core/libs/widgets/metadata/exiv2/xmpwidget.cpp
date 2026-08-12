/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-07-19
 * Description : A widget to display XMP metadata
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "xmpwidget.h"

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

static const char* StandardXmpEntryList[] =
{

    "acdsee",          ///< Schema for ACDSee.
    "audio",           ///< Exiv2 Audio Metadata Schema
    "aux",             ///< Schema for Additional Exif Properties.
    "crs",             ///< Camera Raw schema.
    "dc",              ///< Dublin Core schema.
    "digiKam",         ///< Our Xmp schema used to store private information (see MetaEngine classes for details).
    "dwc",             ///< Qualified Dublin Core schema.
    "exif",            ///< Schema for Exif-specific Properties.
    "iptc",            ///< IPTC Core schema.
    "iptcExt",         ///< IPTC Extension schema.
    "kipi",            ///< Xmp schema used to store private information from tools.
    "lr",              ///< Adobe LightRoom schema.
    "MicrosoftPhoto",  ///< Microsoft schema.
    "MP",              ///< Microsoft Photo 1.2 schema.
    "mwg-rs",          ///< Metadata Working Group schema.
    "pdf",             ///< Adobe PDF schema.
    "photoshop",       ///< Adobe Photoshop schema.
    "plus",            ///< PLUS License Data Format Schema.
    "tiff",            ///< Schema for TIFF Properties
    "video",           ///< Exiv2 Video Metadata Schema
    "xmp",             ///< Basic schema.
    "xmpBJ",           ///< Basic Job Ticket schema.
    "xmpDM",           ///< Dynamic Media schema.
    "xmpMM",           ///< Media Management schema.
    "xmpRights",       ///< Rights Management schema.
    "xmpTPg",          ///< Paged-Text schema.

    "-1"
};

} // namespace

XmpWidget::XmpWidget(QWidget* const parent, const QString& name)
    : MetadataWidget(parent, name)
{
    setup();

    for (int i = 0 ; QLatin1String(StandardXmpEntryList[i]) != QLatin1String("-1") ; ++i)
    {
        m_keysFilter << QLatin1String(StandardXmpEntryList[i]);
    }
}

QString XmpWidget::getMetadataTitle() const
{
    return i18n("XMP Schema");
}

bool XmpWidget::loadFromURL(const QUrl& url)
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

        if (!metadata->hasXmp())
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

bool XmpWidget::decodeMetadata()
{
    QScopedPointer<DMetadata> data(new DMetadata(getMetadata()->data()));

    if (!data->hasXmp())
    {
        return false;
    }

    // Update all metadata contents.

    setMetadataMap(data->getXmpTagsDataList(QStringList()));

    return true;
}

void XmpWidget::buildView()
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
            setIfdList(getMetadataMap(), QStringList());

            break;
        }
    }

    MetadataWidget::buildView();
}

QString XmpWidget::getTagTitle(const QString& key)
{
    QScopedPointer<DMetadata> metadataIface(new DMetadata);
    QString title = metadataIface->getXmpTagTitle(key.toLatin1().constData());

    if (title.isEmpty())
    {
        return key.section(QLatin1Char('.'), -1);
    }

    return title;
}

QString XmpWidget::getTagDescription(const QString& key)
{
    QScopedPointer<DMetadata> metadataIface(new DMetadata);
    QString desc = metadataIface->getXmpTagDescription(key.toLatin1().constData());

    if (desc.isEmpty())
    {
        return i18n("No description available");
    }

    return desc;
}

void XmpWidget::slotSaveMetadataToFile()
{
    QUrl url = saveMetadataToFile(i18n("XMP File to Save"),
                                  QString(QLatin1String("*.xmp|") + i18n("XMP text Files (*.xmp)")));
    storeMetadataToFile(url, getMetadata()->getXmp());
}

} // namespace Digikam

#include "moc_xmpwidget.cpp"
