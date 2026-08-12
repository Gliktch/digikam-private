/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-23
 * Description : Qt Model for Albums
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "abstractalbummodel_p.h"

namespace Digikam
{

AbstractSpecificAlbumModel::AbstractSpecificAlbumModel(Album::Type albumType,
                                                       Album* const rootAlbum,
                                                       RootAlbumBehavior rootBehavior,
                                                       QObject* const parent)
    : AbstractAlbumModel(albumType, rootAlbum, rootBehavior, parent)
{
}

void AbstractSpecificAlbumModel::setupThumbnailLoading()
{
    AlbumThumbnailLoader* const loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album*,QPixmap)),
            this, SLOT(slotGotThumbnailFromIcon(Album*,QPixmap)));

    connect(loader, SIGNAL(signalFailed(Album*)),
            this, SLOT(slotThumbnailLost(Album*)));

    connect(loader, SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotReloadThumbnails()));
}

QString AbstractSpecificAlbumModel::columnHeader() const
{
    return m_columnHeader;
}

void AbstractSpecificAlbumModel::setColumnHeader(const QString& header)
{
    m_columnHeader = header;

    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 0);
}

void AbstractSpecificAlbumModel::slotGotThumbnailFromIcon(Album* album, const QPixmap&)
{
    // see decorationRole() method of subclasses

    if (!filterAlbum(album))
    {
        return;
    }

    QModelIndex index = indexForAlbum(album);

    Q_EMIT dataChanged(index, index);
}

void AbstractSpecificAlbumModel::slotThumbnailLost(Album*)
{
    // ignore, use default thumbnail
}

void AbstractSpecificAlbumModel::slotReloadThumbnails()
{
    // Emit dataChanged() for all albums

    emitDataChangedForChildren(rootAlbum());
}

void AbstractSpecificAlbumModel::emitDataChangedForChildren(Album* album)
{
    if (!album)
    {
        return;
    }

    for (Album* child = album->firstChild() ; child ; child = child->next())
    {
        if (filterAlbum(child))
        {
            // recurse to children of children

            emitDataChangedForChildren(child);

            // Emit signal for child

            QModelIndex index = indexForAlbum(child);

            Q_EMIT dataChanged(index, index);
        }
    }
}

} // namespace Digikam
