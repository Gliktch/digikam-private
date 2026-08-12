/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "GeoDataStyle.h"
#include "GeoDataNetworkLinkControl.h"
#include "GeoDataStyleMap.h"
#include "GeoDataSchema.h"
#include "GeoDataContainer_p.h"
#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataDocumentPrivate : public GeoDataContainerPrivate
{
public:

    GeoDataDocumentPrivate()
        : m_documentRole(UnknownDocument)
    {
    }

    GeoDataDocumentPrivate(const GeoDataDocumentPrivate& other)
        : GeoDataContainerPrivate(other),
          m_styleHash            (other.m_styleHash),
          m_styleMapHash         (other.m_styleMapHash),
          m_schemaHash           (other.m_schemaHash),
          m_filename             (other.m_filename),
          m_baseUri              (other.m_baseUri),
          m_networkLinkControl   (other.m_networkLinkControl),
          m_property             (other.m_property),
          m_documentRole         (other.m_documentRole)
    {
    }

    EnumFeatureId featureId() const override
    {
        return GeoDataDocumentId;
    }

public:

    QMap<QString, GeoDataStyle::Ptr>    m_styleHash;
    QMap<QString, GeoDataStyleMap>      m_styleMapHash;
    QMap<QString, GeoDataSchema>        m_schemaHash;
    QString                             m_filename;
    QString                             m_baseUri;
    GeoDataNetworkLinkControl           m_networkLinkControl;
    QString                             m_property;
    DocumentRole                        m_documentRole;
};

} // namespace Marble
