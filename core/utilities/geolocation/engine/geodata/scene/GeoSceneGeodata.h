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

#include <QPen>
#include <QBrush>

// Local includes

#include "GeoSceneAbstractDataset.h"
#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

class DIGIKAM_EXPORT GeoSceneGeodata : public GeoSceneAbstractDataset
{
public:

    explicit GeoSceneGeodata(const QString& name);
    ~GeoSceneGeodata() override;
    const char* nodeType() const override;

    bool operator==(const GeoSceneGeodata& other) const;

    QString property() const;
    void setProperty(const QString& property);

    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);

    QString colorize() const;
    void setColorize(const QString& colorize);

    qreal alpha() const;
    void setAlpha(qreal alpha);

    QPen pen() const;
    void setPen(const QPen& pen);

    QBrush brush() const;
    void setBrush(const QBrush& brush);

    int renderOrder() const;
    void setRenderOrder(int renderOrder);

    QVector<QColor> colors() const;
    void setColors(const QVector<QColor>& colors);

private:

    QString         m_property;
    QString         m_sourceFile;
    QString         m_colorize;
    qreal           m_alpha;
    QPen            m_pen;
    QBrush          m_brush;
    int             m_renderOrder;
    QVector<QColor> m_colors;
};

} // namespace Marble
