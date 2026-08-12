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

// Qt includes

#include <QDateTime>

// Local includes

#include "GeoDataContainer_p.h"
#include "GeoDataTypes.h"
#include "GeoDataUpdate.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataNetworkLinkControlPrivate : public GeoDataContainerPrivate
{
public:

    GeoDataNetworkLinkControlPrivate();
    GeoDataNetworkLinkControlPrivate(const GeoDataNetworkLinkControlPrivate& other);
    ~GeoDataNetworkLinkControlPrivate() override;
    GeoDataNetworkLinkControlPrivate& operator=(const GeoDataNetworkLinkControlPrivate& other);

public:

    qreal                   m_minRefreshPeriod  = 0.0;
    qreal                   m_maxSessionLength  = 0.0;
    QString                 m_cookie;
    QString                 m_message;
    QString                 m_linkName;
    QString                 m_linkDescription;
    QString                 m_linkSnippet;
    int                     m_maxLines          = 2;
    QDateTime               m_expires;
    GeoDataUpdate           m_update;
    GeoDataAbstractView*    m_abstractView      = nullptr;
};

GeoDataNetworkLinkControlPrivate::GeoDataNetworkLinkControlPrivate()
    : m_minRefreshPeriod(0.0),
      m_maxSessionLength(0.0),
      m_maxLines        (2),
      m_expires         (),
      m_update          (),
      m_abstractView    (nullptr)
{
}

GeoDataNetworkLinkControlPrivate::GeoDataNetworkLinkControlPrivate(const GeoDataNetworkLinkControlPrivate& other)
    : GeoDataContainerPrivate   (other),
      m_minRefreshPeriod        (other.m_minRefreshPeriod),
      m_maxSessionLength        (other.m_maxSessionLength),
      m_cookie                  (other.m_cookie),
      m_message                 (other.m_message),
      m_linkName                (other.m_linkName),
      m_linkDescription         (other.m_linkDescription),
      m_linkSnippet             (other.m_linkSnippet),
      m_maxLines                (other.m_maxLines),
      m_expires                 (other.m_expires),
      m_update                  (other.m_update),
      m_abstractView            (other.m_abstractView ? other.m_abstractView->copy() : nullptr)
{
}

GeoDataNetworkLinkControlPrivate::~GeoDataNetworkLinkControlPrivate()
{
    delete m_abstractView;
}

GeoDataNetworkLinkControlPrivate& GeoDataNetworkLinkControlPrivate::operator=(const GeoDataNetworkLinkControlPrivate& other)
{
    if (this == &other)
    {
        return *this;
    }

    GeoDataContainerPrivate::operator=(other);

    m_minRefreshPeriod  = other.m_minRefreshPeriod;
    m_maxSessionLength  = other.m_maxSessionLength;
    m_cookie            = other.m_cookie;
    m_message           = other.m_message;
    m_linkName          = other.m_linkName;
    m_linkDescription   = other.m_linkDescription;
    m_linkSnippet       = other.m_linkSnippet;
    m_maxLines          = other.m_maxLines;
    m_expires           = other.m_expires;
    m_update            = other.m_update;

    delete m_abstractView;
    m_abstractView      = (other.m_abstractView ? other.m_abstractView->copy() : nullptr);

    return *this;
}

} // namespace Marble
