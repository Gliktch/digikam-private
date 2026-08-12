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

#include "GeoDataTourPrimitive.h"

namespace Marble
{

class DIGIKAM_EXPORT GeoDataSoundCue : public GeoDataTourPrimitive
{
public:

    GeoDataSoundCue();
    ~GeoDataSoundCue() override;

    bool operator==(const GeoDataSoundCue& other) const;
    bool operator!=(const GeoDataSoundCue& other) const;
    const char* nodeType() const override;

    QString href() const;
    void setHref(const QString& url);

    double delayedStart() const;
    void setDelayedStart(double pause);

private:

    QString m_href;
    double  m_delayedStart;
};

} // namespace Marble
