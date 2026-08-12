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

#include <QSet>

// Local includes

#include "OsmConverter.h"
#include "GeoWriterBackend.h"

namespace Marble
{

class GeoDataLineString;
class GeoDataRelation;
class OsmPlacemarkData;

class O5mWriter: public GeoWriterBackend
{
public:

    bool write(QIODevice* device, const GeoDataDocument& document)                                        override;

private:

    typedef QPair<QString, QString>   StringPair;
    typedef QHash<StringPair, qint32> StringTable;

private:

    void writeHeader(QDataStream& stream)                                                           const;
    void writeNodes(const OsmConverter::Nodes& nodes, QDataStream& stream)                          const;
    void writeWays(const OsmConverter::Ways& ways, QDataStream& stream)                             const;
    void writeRelations(const OsmConverter::Relations& relations, QDataStream& stream)              const;
    void writeTrailer(QDataStream& stream)                                                          const;

    void writeMultipolygonMembers(const GeoDataPolygon& polygon, qint64(&lastId)[3],
                                  const OsmPlacemarkData& osmData, StringTable& stringTable,
                                  QDataStream& stream)                                              const;
    void writeRelationMembers(const GeoDataRelation* relation, qint64(&lastId)[3],
                              const OsmPlacemarkData& osmData, StringTable& stringTable,
                              QDataStream& stream)                                                  const;
    void writeReferences(const GeoDataLineString& lineString, qint64& lastId,
                         const OsmPlacemarkData& osmData, QDataStream& stream)                      const;
    void writeVersion(const OsmPlacemarkData& osmData, QDataStream& stream)                         const;
    void writeTags(const OsmPlacemarkData& osmData, StringTable& stringTable, QDataStream& stream)  const;
    void writeStringPair(const StringPair& pair, StringTable& stringTable, QDataStream& stream)     const;
    void writeSigned(qint64 value, QDataStream& stream)                                             const;
    void writeUnsigned(quint32 value, QDataStream& stream)                                          const;
    qint32 deltaTo(double value, double previous)                                                   const;

private:

    static QSet<QString> m_blacklistedTags;

    mutable QByteArray   m_stringPairBuffer;
};

} // namespace Marble
