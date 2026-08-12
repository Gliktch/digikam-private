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

#include "GeoSceneLicense.h"

// Local includes

#include "GeoSceneTypes.h"

namespace Marble
{

GeoSceneLicense::GeoSceneLicense() :
    m_attribution(OptOut)
{
    // nothing to do
}

QString GeoSceneLicense::license() const
{
    return m_fullLicense.isEmpty() ? m_shortLicense : m_fullLicense;
}

QString GeoSceneLicense::shortLicense() const
{
    return m_shortLicense.isEmpty() ? m_fullLicense : m_shortLicense;
}

GeoSceneLicense::Attribution GeoSceneLicense::attribution() const
{
    return m_attribution;
}

void GeoSceneLicense::setLicense(const QString& license)
{
    m_fullLicense = license;
}

void GeoSceneLicense::setShortLicense(const QString& license)
{
    m_shortLicense = license;
}

void GeoSceneLicense::setAttribution(Attribution attr)
{
    m_attribution = attr;
}

const char* GeoSceneLicense::nodeType() const
{
    return GeoSceneTypes::GeoSceneLicenseType;
}

} // namespace Marble
