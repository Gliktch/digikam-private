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
#include <QPen>
#include <QBrush>

// Local includes

#include "GeoSceneAbstractDataset.h"

namespace Marble
{

/**
 * @short Vector dataset stored in a layer.
 */
class DIGIKAM_EXPORT GeoSceneVector : public GeoSceneAbstractDataset
{
public:

    explicit GeoSceneVector(const QString& name);
    const char* nodeType() const override;

    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);

    QString feature() const;
    void setFeature(const QString& feature);

    QPen pen() const;
    void setPen(const QPen& pen);

    QBrush brush() const;
    void setBrush(const QBrush& brush);

private:

    QString m_sourceFile;
    QString m_feature;

    QPen    m_pen;
    QBrush  m_brush;
};

} // namespace Marble
