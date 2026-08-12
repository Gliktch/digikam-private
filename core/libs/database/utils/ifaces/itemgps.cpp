/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : a class to hold GPS information about an item.
 *
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemgps.h"

// Local includes

#include "coredb.h"
#include "tagscache.h"
#include "metadatahub.h"
#include "itemposition.h"
#include "scancontroller.h"
#include "metaenginesettings.h"
#include "itemextendedproperties.h"

namespace Digikam
{

ItemGPS::ItemGPS(const ItemInfo& info)
    : GPSItemContainer(info.fileUrl()),
      m_info          (info)
{
}

bool ItemGPS::loadImageData()
{
    // In first, we try to get GPS info from database.

    ItemPosition pos = m_info.imagePosition();
    m_dateTime       = m_info.dateTime();

    if (!pos.isEmpty() && pos.hasCoordinates())
    {
        m_gpsData.setLatLon(pos.latitudeNumber(), pos.longitudeNumber());

        if (pos.hasAltitude())
        {
            m_gpsData.setAltitude(pos.altitude());
        }
    }

    // mark us as not-dirty, because the data was just loaded:

    m_dirty      = false;
    m_savedState = m_gpsData;

    emitDataChanged();

    return true;
}

QString ItemGPS::saveChanges()
{
    SaveProperties p = saveProperties();

    // Save info to database.

    ItemPosition pos = m_info.imagePosition();

    if (p.shouldWriteCoordinates)
    {
        pos.setLatitude(p.latitude);
        pos.setLongitude(p.longitude);

        if (p.shouldWriteAltitude)
        {
            pos.setAltitude(p.altitude);
        }
    }

    if      (p.shouldRemoveCoordinates)
    {
        pos.remove();
    }
    else if (p.shouldRemoveAltitude)
    {
        pos.removeAltitude();
    }

    pos.apply();

    if (!m_tagList.isEmpty() && (m_writeXmpTags || m_writeMetaLoc))
    {
        IptcCoreLocationInfo locationInfo;
        QStringList tagsPath;

        for (int i = 0 ; i < m_tagList.count() ; ++i)
        {
            QString singleTagPath;
            QList<TagData> currentTagPath = m_tagList[i];

            for (int j = 0 ; j < currentTagPath.count() ; ++j)
            {
                if (currentTagPath[j].tipName != QLatin1String("{Country code}"))
                {
                    singleTagPath.append(QLatin1Char('/') + currentTagPath[j].tagName);

                    if (j == 0)
                    {
                        singleTagPath.remove(0, 1);
                    }
                }

                setLocationInfo(currentTagPath[j], locationInfo);
            }

            if (!singleTagPath.isEmpty())
            {
                tagsPath << singleTagPath;
            }
        }

        if (m_writeXmpTags)
        {
            QList<int> tagIds = TagsCache::instance()->getOrCreateTags(tagsPath);
            CoreDbAccess().db()->addTagsToItems(QList<qlonglong>() << m_info.id(), tagIds);
        }

        if (m_writeMetaLoc)
        {
            ItemExtendedProperties ep(m_info.id());
            ep.setLocation(locationInfo);
        }
    }

    MetadataHub hub;
    hub.load(m_info);

    if (MetaEngineSettings::instance()->settings().useLazySync)
    {
        hub.writeToMetadata(m_info, MetadataHub::WRITE_TAGS     |
                                    MetadataHub::WRITE_TEMPLATE |
                                    MetadataHub::WRITE_POSITION);
    }
    else
    {
        ScanController::FileMetadataWrite writeScope(m_info);
        writeScope.changed(hub.writeToMetadata(m_info, MetadataHub::WRITE_TAGS     |
                                                       MetadataHub::WRITE_TEMPLATE |
                                                       MetadataHub::WRITE_POSITION));
    }

    m_dirty        = false;
    m_savedState   = m_gpsData;
    m_tagListDirty = false;
    m_savedTagList = m_tagList;

    emitDataChanged();

    return QString();
}

} // namespace Digikam
