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
#include <QList>

// Local includes

#include "GeoDocument.h"

namespace Marble
{

class GeoScenePalette;

/**
 * @short Filter of a GeoScene document.
 */

class GeoSceneFilter : public GeoNode
{
public:

    explicit GeoSceneFilter(const QString& name);
    ~GeoSceneFilter() override;

    QString name() const;
    void setName(const QString& name);

    QString type() const;
    void setType(const QString& type);

    QList<const GeoScenePalette*> palette() const;
    void addPalette(const GeoScenePalette* palette);
    int removePalette(const GeoScenePalette* palette);   //returns # of items removed

    const char* nodeType() const override;

private:

    QString                       m_name;
    QString                       m_type;
    QList<const GeoScenePalette*> m_palette;
};

} // namespace Marble
