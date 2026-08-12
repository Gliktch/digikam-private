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

#include <QObject>
#include <QRunnable>
#include <QMap>

// Local includes

#include "TileId.h"
#include "GeoDataLatLonBox.h"

class QThreadPool;

namespace Marble
{

class GeoDataDocument;
class GeoDataTreeModel;
class GeoSceneVectorTileDataset;
class GeoDataObject;
class TileLoader;

class TileRunner : public QObject, public QRunnable
{
    Q_OBJECT

public:

    TileRunner(TileLoader* loader, const GeoSceneVectorTileDataset* texture, const TileId& id);
    void run() override;

Q_SIGNALS:

    void documentLoaded(const TileId& id, GeoDataDocument* document);

private:

    TileLoader* const                      m_loader      = nullptr;
    const GeoSceneVectorTileDataset* const m_tileDataset = nullptr;
    const TileId                           m_id;
};

class VectorTileModel : public QObject
{
    Q_OBJECT

public:

    explicit VectorTileModel(TileLoader* loader, const GeoSceneVectorTileDataset* layer, GeoDataTreeModel* treeModel, QThreadPool* threadPool);

    void setViewport(const GeoDataLatLonBox& bbox);

    QString name() const;

    const GeoSceneVectorTileDataset* layer() const;

    void removeTile(GeoDataDocument* document);

    int tileZoomLevel() const;

    int cachedDocuments() const;

    void reload();

public Q_SLOTS:

    void updateTile(const TileId& id, GeoDataDocument* document);

    void clear();

Q_SIGNALS:

    void tileCompleted(const TileId& tileId);
    void tileAdded(GeoDataDocument* document);
    void tileRemoved(GeoDataDocument* document);

private Q_SLOTS:

    void cleanupTile(GeoDataObject* feature);

private:

    void removeTilesOutOfView(const GeoDataLatLonBox& boundingBox);
    void queryTiles(int tileZoomLevel, const QRect& rect);

private:

    struct CacheDocument
    {
        /** The CacheDocument takes ownership of doc */
        CacheDocument(GeoDataDocument* doc, VectorTileModel* vectorTileModel, const GeoDataLatLonBox& boundingBox);

        /** Remove the document from the tree and delete the document */
        ~CacheDocument();

        GeoDataLatLonBox latLonBox() const
        {
            return m_boundingBox;
        }

    private:

        Q_DISABLE_COPY(CacheDocument)

        GeoDataDocument* const m_document           = nullptr;
        VectorTileModel* const m_vectorTileModel    = nullptr;
        GeoDataLatLonBox       m_boundingBox;
    };

    TileLoader* const                            m_loader                = nullptr;
    const GeoSceneVectorTileDataset* const       m_layer                 = nullptr;
    GeoDataTreeModel* const                      m_treeModel             = nullptr;
    QThreadPool* const                           m_threadPool            = nullptr;
    int                                          m_tileLoadLevel;
    int                                          m_tileZoomLevel;
    QList<TileId>                                m_pendingDocuments;
    QList<GeoDataDocument*>                      m_garbageQueue;
    QMap<TileId, QSharedPointer<CacheDocument> > m_documents;
    bool                                         m_deleteDocumentsLater;
};

} // namespace Marble
