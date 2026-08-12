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

#include <QString>

// Local includes

#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

class DIGIKAM_EXPORT GeoSceneLicense : public GeoNode
{
public:

    enum Attribution
    {
        Never,
        OptOut,
        OptIn,
        Always
    };

public:

    GeoSceneLicense();

    QString license() const;
    QString shortLicense() const;
    Attribution attribution() const;

    void setLicense(const QString& license);
    void setShortLicense(const QString& license);
    void setAttribution(Attribution attr);

    const char* nodeType() const override;

private:

    QString     m_fullLicense;
    QString     m_shortLicense;
    Attribution m_attribution;
};

} // namespace Marble
