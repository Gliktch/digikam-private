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

namespace Marble
{

/**
 * @short Palette of a GeoScene document.
 */
class GeoScenePalette : public GeoNode
{
public:

    GeoScenePalette(const QString& type, const QString& file);

    QString type() const;
    void    setType(const QString& type);

    QString file() const;
    void    setFile(const QString& file);

    bool operator==(const GeoScenePalette& rhs) const;

    const char* nodeType() const override;

private:

    QString m_type;
    QString m_file;
};

} // namespace Marble
