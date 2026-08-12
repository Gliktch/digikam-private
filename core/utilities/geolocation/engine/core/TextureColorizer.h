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

/**
 * The TextureColorizer maps the Elevationvalues to Legend Colors.
 */

#pragma once

// Qt includes

#include <QString>
#include <QImage>
#include <QColor>

// Local includes

#include "MarbleGlobal.h"
#include "GeoDataDocument.h"

namespace Marble
{

class GeoPainter;
class ViewportParams;

class TextureColorizer
{
public:

    TextureColorizer(const QString& seafile, const QString& landfile);
    virtual ~TextureColorizer() = default;

public:

    void addSeaDocument(const GeoDataDocument* seaDocument);

    void addLandDocument(const GeoDataDocument* landDocument);

    void setShowRelief(bool show);

    static void drawIndividualDocument(GeoPainter* painter, const GeoDataDocument* document);

    void drawTextureMap(GeoPainter* painter);

    void colorize(QImage* origimg, const ViewportParams* viewport, MapQuality mapQuality);

    void setPixel(const QRgb* coastData, QRgb* writeData, int bump, uchar grey);

private:

    QString                         m_seafile;
    QString                         m_landfile;
    QList<const GeoDataDocument*>   m_seaDocuments;
    QList<const GeoDataDocument*>   m_landDocuments;
    QImage                          m_coastImage;
    uint                            texturepalette[16][512] = {{ 0 }};
    bool                            m_showRelief            = false;
    QRgb                            m_landColor;
    QRgb                            m_seaColor;
};

} // Namespace Marble
