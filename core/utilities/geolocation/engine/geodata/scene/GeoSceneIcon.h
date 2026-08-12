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
#include <QColor>

// Local includes

#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

/**
 * @short Icon properties of a GeoScene document.
 */
class DIGIKAM_EXPORT GeoSceneIcon : public GeoNode
{
public:

    GeoSceneIcon();
    ~GeoSceneIcon() override;

    QString pixmap() const;
    void setPixmap(const QString&);

    QColor color() const;
    void setColor(const QColor&);

    const char* nodeType() const override;

private:

    QString m_pixmap;
    QColor  m_color;
};

} // namespace Marble
