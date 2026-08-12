/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-23
 * Description : Graph data class for item history
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSharedData>

// Local includes

#include "filteraction.h"
#include "historyimageid.h"
#include "itemhistorygraph_boost.h"

namespace Digikam
{

class ItemInfo;

/**
 * Every vertex has one associated object of this class
 *
 * All entries in a vertex refer to _identical_ images.
 * There can be multiple referred images in a history entry.
 * Each single HistoryImageId can resolve into none, one,
 * or multiple ItemInfos.
 * So there is no mapping between the two fields here.
 *
 * If an image is created from multiple source images (panorama etc.),
 * there will be one vertex per source image!
 */
class DIGIKAM_DATABASE_EXPORT HistoryVertexProperties
{
public:

    ItemInfo firstItemInfo()                    const;

    bool markedAs(HistoryImageId::Type)         const;
    bool alwaysMarkedAs(HistoryImageId::Type)   const;

    bool operator==(const QString& uuid)        const;
    bool operator==(const ItemInfo& info)       const;
    bool operator==(qlonglong id)               const;
    bool operator==(const HistoryImageId& info) const;

    HistoryVertexProperties& operator+=(const QString& uuid);
    HistoryVertexProperties& operator+=(const ItemInfo& info);
    HistoryVertexProperties& operator+=(const HistoryImageId& info);

public:

    QString               uuid;
    QList<HistoryImageId> referredImages;
    QList<ItemInfo>       infos;
};

QDebug operator<<(QDebug dbg, const HistoryVertexProperties& props);
QDebug operator<<(QDebug dbg, const HistoryImageId& id);

// ------------------------------------------------------------------------------

/**
 * Every edge has one associated object of this class.
 *
 * For two vertices v1, v2 with and edge e, v1 -> v2,
 * describes the actions necessary to create v2 from v2:
 * v1 -> actions[0] -> ... -> actions[n] = v2.
 */
class DIGIKAM_DATABASE_EXPORT HistoryEdgeProperties
{
public:

    QList<FilterAction> actions;

    HistoryEdgeProperties& operator+=(const FilterAction& action);
};

typedef Graph<HistoryVertexProperties, HistoryEdgeProperties> HistoryGraph;

// ------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemHistoryGraphData : public HistoryGraph,       // clazy:exclude=copyable-polymorphic
                                                     public QSharedData
{
public:

    ItemHistoryGraphData()
        : HistoryGraph(ChildToParent)
    {
    }

    explicit ItemHistoryGraphData(const HistoryGraph& g)
        : HistoryGraph(g)
    {
    }

    // cppcheck-suppress duplInheritedMember
    ItemHistoryGraphData& operator=(const HistoryGraph& g)
    {
        HistoryGraph::operator=(g);

        return *this;
    }

    Vertex addVertex(const HistoryImageId& id);
    Vertex addVertex(const QList<HistoryImageId>& imageIds);
    Vertex addVertex(qlonglong id);
    Vertex addVertexScanned(qlonglong id);
    Vertex addVertex(const ItemInfo& info);

    void addHistory(const DImageHistory& givenHistory, qlonglong extraCurrent = 0);

    int removeNextUnresolvedVertex(int begin);

    inline QList<ItemInfo> toInfoList(const QList<Vertex>& vertices) const
    {
        QList<ItemInfo> infos;

        for (const HistoryGraph::Vertex& v : std::as_const(vertices))
        {
            infos << properties(v).infos;
        }

        return infos;
    }

    QHash<Vertex, HistoryImageId::Types> categorize() const;

protected:

    void applyProperties(Vertex& v, const QList<ItemInfo>& infos, const QList<HistoryImageId>& ids);
};

} // namespace Digikam
