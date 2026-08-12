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

#include <QDataStream>

// Local includes

#include "ParsingRunner.h"

namespace Marble
{

class GeoDataLineString;

class Pn2Runner : public ParsingRunner
{
    Q_OBJECT

public:

    explicit Pn2Runner(QObject* const parent = nullptr);
    ~Pn2Runner()                                                                           override;

    GeoDataDocument* parseFile(const QString& fileName, DocumentRole role, QString& error) override;

private:

    static bool errorCheckLat(qint16 lat);
    static bool errorCheckLon(qint16 lon);
    static bool importPolygon(QDataStream& stream,
                              GeoDataLineString* linestring,
                              quint32 nrAbsoluteNodes);

    GeoDataDocument* parseForVersion1(const QString& fileName, DocumentRole role);
    GeoDataDocument* parseForVersion2(const QString& fileName, DocumentRole role);

private:

    QDataStream     m_stream;
    quint8          m_fileHeaderVersion     = 0;
    quint32         m_fileHeaderPolygons    = 0;

    /// Whether the file contains color indexes
    bool            m_isMapColorField       = false;
};

} // namespace Marble
