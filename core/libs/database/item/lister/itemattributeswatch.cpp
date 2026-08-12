/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-05-04
 * Description : Watch image attributes
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemattributeswatch.h"

// Local includes

#include "coredbaccess.h"
#include "coredbwatch.h"

namespace Digikam
{

ItemAttributesWatch* ItemAttributesWatch::m_instance = nullptr;

ItemAttributesWatch::ItemAttributesWatch()
{
    CoreDbWatch* const dbwatch = CoreDbAccess::databaseWatch();

    connect(dbwatch, SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChange(ImageChangeset)));

    connect(dbwatch, SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));
}

ItemAttributesWatch::~ItemAttributesWatch()
{
    m_instance = nullptr;
}

void ItemAttributesWatch::cleanUp()
{
    delete m_instance;
    m_instance = nullptr;
}

void ItemAttributesWatch::shutDown()
{
    if (m_instance)
    {
        m_instance->disconnect(nullptr, nullptr, nullptr);
    }
}

ItemAttributesWatch* ItemAttributesWatch::instance()
{
    if (!m_instance)
    {
        m_instance = new ItemAttributesWatch;
    }

    return m_instance;
}

void ItemAttributesWatch::slotImageChange(const ImageChangeset& changeset)
{
    DatabaseFields::Set set = changeset.changes();

    if (
        (set & DatabaseFields::ItemCommentsAll)  ||
        (set & DatabaseFields::CreationDate)     ||
        (set & DatabaseFields::ModificationDate) ||
        (set & DatabaseFields::Rating)
       )
    {
        const auto ids = changeset.ids();

        for (const qlonglong& imageId : ids)
        {
            if (set & DatabaseFields::ItemCommentsAll)
            {
                Q_EMIT signalImageCaptionChanged(imageId);
            }

            if (
                (set & DatabaseFields::CreationDate) ||
                (set & DatabaseFields::ModificationDate)
               )
            {
                Q_EMIT signalImageDateChanged(imageId);
            }

            if (set & DatabaseFields::Rating)
            {
                Q_EMIT signalImageRatingChanged(imageId);
            }
        }
    }
}

void ItemAttributesWatch::slotImageTagChange(const ImageTagChangeset& changeset)
{
    const auto ids = changeset.ids();

    for (const qlonglong& imageId : ids)
    {
        Q_EMIT signalImageTagsChanged(imageId);
    }
}

void ItemAttributesWatch::fileMetadataChanged(const QUrl& url)
{
    Q_EMIT signalFileMetadataChanged(url);
}

/*

void ItemAttributesWatch::slotImageFieldChanged(qlonglong imageId, int field)
{
    // Translate signals

    // TODO: compress?
    // TODO!!: we have databaseaccess lock here as well. Make connection queued in one place (above!)

    switch (field)
    {
        case DatabaseAttributesWatch::ImageComment:
            Q_EMIT signalImageCaptionChanged(imageId);
            break;
        case DatabaseAttributesWatch::ImageDate:
            Q_EMIT signalImageDateChanged(imageId);
            break;
        case DatabaseAttributesWatch::ImageRating:
            Q_EMIT signalImageRatingChanged(imageId);
            break;
        case DatabaseAttributesWatch::ImageTags:
            Q_EMIT signalImageTagsChanged(imageId);
            break;
    }
}

void ItemAttributesWatch::imageTagsChanged(qint64 imageId)
{
    Q_EMIT signalImageTagsChanged(imageId);
}

void ItemAttributesWatch::imagesChanged(int albumId)
{
    Q_EMIT signalImagesChanged(albumId);
}

void ItemAttributesWatch::imageRatingChanged(qint64 imageId)
{
    Q_EMIT signalImageRatingChanged(imageId);
}

void ItemAttributesWatch::imageDateChanged(qint64 imageId)
{
    Q_EMIT signalImageDateChanged(imageId);
}

void ItemAttributesWatch::imageCaptionChanged(qint64 imageId)
{
    Q_EMIT signalImageCaptionChanged(imageId);
}

*/

} // namespace Digikam

#include "moc_itemattributeswatch.cpp"
