/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : A widget to display IPTC metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iptcwidget.h"

// Qt includes

#include <QMap>
#include <QFile>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"

namespace
{

static const char* StandardIptcEntryList[] =
{
    "Envelope",
    "Application2",
    "-1"
};

} // namespace

namespace Digikam
{

IptcWidget::IptcWidget(QWidget* const parent, const QString& name)
    : MetadataWidget(parent, name)
{
    setup();

    for (int i = 0 ; QLatin1String(StandardIptcEntryList[i]) != QLatin1String("-1") ; ++i)
    {
        m_keysFilter << QLatin1String(StandardIptcEntryList[i]);
    }
}

QString IptcWidget::getMetadataTitle() const
{
    return i18n("IPTC Records");
}

bool IptcWidget::loadFromURL(const QUrl& url)
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

        if (!metadata->hasIptc())
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

bool IptcWidget::decodeMetadata()
{
    QScopedPointer<DMetadata> data(new DMetadata(getMetadata()->data()));

    if (!data->hasIptc())
    {
        return false;
    }

    // Update all metadata contents.

    setMetadataMap(data->getIptcTagsDataList(m_keysFilter));

    return true;
}

void IptcWidget::buildView()
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

QString IptcWidget::getTagTitle(const QString& key)
{
    QScopedPointer<DMetadata> metadataIface(new DMetadata);
    QString title = metadataIface->getIptcTagTitle(key.toLatin1().constData());

    if (title.isEmpty())
    {
        return key.section(QLatin1Char('.'), -1);
    }

    return title;
}

QString IptcWidget::getTagDescription(const QString& key)
{
    QScopedPointer<DMetadata> metadataIface(new DMetadata);
    QString desc = metadataIface->getIptcTagDescription(key.toLatin1().constData());

    if (desc.isEmpty())
    {
        return i18n("No description available");
    }

    return desc;
}

void IptcWidget::slotSaveMetadataToFile()
{
    QUrl url = saveMetadataToFile(i18n("IPTC File to Save"),
                                  QString(QLatin1String("*.iptc|") + i18n("IPTC binary Files (*.iptc)")));
    storeMetadataToFile(url, getMetadata()->getIptc());
}

} // namespace Digikam

#include "moc_iptcwidget.cpp"
