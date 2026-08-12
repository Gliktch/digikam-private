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

#include <QVector>

class QIODevice;
class QJsonObject;

namespace Marble
{

class GeoDataDocument;
class GeoDataGeometry;
class GeoDataIconStyle;
class GeoDataLineStyle;
class GeoDataPolyStyle;
class GeoDataLabelStyle;

class JsonParser
{
public:

    JsonParser();
    ~JsonParser();

    /**
     * @brief parse the GeoJSON file
     * @return true if parsing of the file was successful
     */
    bool read(QIODevice*);

    /**
     * @brief retrieve the parsed document and reset the parser
     * If parsing was successful, retrieve the resulting document
     * and set the contained m_document pointer to 0.
     */
    GeoDataDocument* releaseDocument();

private:

    /**
     * @brief parse a top-level GeoJSON object (FeatureCollection or Feature)
     * @param jsonObject  the object to parse
     * @return true if parsing of the top-level object was successful
     */
    bool parseGeoJsonTopLevel(const QJsonObject&);

    /**
      * @brief parse a sub-level GeoJSON object
      * @param jsonObject    the object to parse
      * @param geometryList  a list of geometries pass back to the caller
      * @param hasPoints     a boolean passed back to the caller: true if Points exist in geometry
      * @return true if parsing of the object was successful
      */
    bool parseGeoJsonSubLevel(const QJsonObject&, QVector<GeoDataGeometry*>&, bool&);

private:

    GeoDataDocument*   m_document           = nullptr;

    GeoDataIconStyle*  m_iconStylePoints    = nullptr;
    GeoDataIconStyle*  m_iconStyleOther     = nullptr;
    GeoDataLineStyle*  m_lineStyle          = nullptr;
    GeoDataPolyStyle*  m_polyStyle          = nullptr;
    GeoDataLabelStyle* m_labelStyle         = nullptr;

private:

    JsonParser(const JsonParser&)            = delete;
    JsonParser& operator=(const JsonParser&) = delete;
};

} // namespace Marble
