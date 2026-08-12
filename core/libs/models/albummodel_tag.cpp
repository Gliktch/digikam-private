/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-22
 * Description : Qt Model for Albums
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummodel_p.h"

namespace Digikam
{

TagModel::TagModel(RootAlbumBehavior rootBehavior, QObject* const parent)
    : AbstractCheckableAlbumModel(Album::TAG,
                                  AlbumManager::instance()->findTAlbum(0),
                                  rootBehavior, parent)
{
    m_columnHeader = i18nc("@title", "Tags");
    setupThumbnailLoading();

    connect(AlbumManager::instance(), SIGNAL(signalTAlbumsDirty(QHash<int,int>)),
            this, SLOT(setCountHash(QHash<int,int>)));

    setCountHash(AlbumManager::instance()->getTAlbumsCount());
}

void TagModel::setColumnHeader(const QString& header)
{
    m_columnHeader = header;
}

TAlbum* TagModel::talbumForIndex(const QModelIndex& index) const
{
    return static_cast<TAlbum*>(AbstractCheckableAlbumModel::albumForIndex(index));
}

QVariant TagModel::albumData(Album* a, int role) const
{
    if (
        (role == Qt::DisplayRole)                    &&
        !a->isRoot()                                 &&
        m_unconfirmedFaceCount.contains(a->id())     &&
        (a->id() != FaceTags::unknownPersonTagId())
       )
    {
        QString res = AbstractCheckableAlbumModel::albumData(a, role).toString() +
                      i18ncp("@info: unconfirmed faces in album", " (%1 new)", " (%1 new)",
                             m_unconfirmedFaceCount.value(a->id()));

        return res;
    }

    return AbstractCheckableAlbumModel::albumData(a, role);
}

QVariant TagModel::decorationRoleData(Album* album) const
{
    TAlbum* const tagAlbum = static_cast<TAlbum*>(album);

    if (isFaceTagModel() || tagAlbum->hasProperty(TagPropertyName::person()))
    {
        QPixmap face = AlbumThumbnailLoader::instance()->getFaceThumbnailDirectly(tagAlbum);
        int size     = isFaceTagModel() ? ApplicationSettings::instance()->getTreeViewFaceSize()
                                        : ApplicationSettings::instance()->getTreeViewIconSize();

        double ratio = face.devicePixelRatio();
        int rsize    = qRound((double)size * ratio);
        face         = face.scaled(rsize, rsize, Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);

        QPixmap pix(rsize, rsize);
        pix.fill(Qt::transparent);
        pix.setDevicePixelRatio(ratio);

        QPainter p(&pix);
        p.drawPixmap((rsize - face.width())  / 2,
                     (rsize - face.height()) / 2, face);
        p.end();

        prepareAddExcludeDecoration(album, pix);

        return pix;
    }
    else
    {
        QPixmap pix = AlbumThumbnailLoader::instance()->getTagThumbnailDirectly(tagAlbum);
        prepareAddExcludeDecoration(album, pix);

        return pix;
    }
}

QVariant TagModel::fontRoleData(Album* a) const
{
    if (
        m_unconfirmedFaceCount.contains(a->id())  &&
        (a->id() != FaceTags::unknownPersonTagId())
       )
    {
        QFont font;
        font.setBold(true);

        return font;
    }

    return QVariant();
}

Album* TagModel::albumForId(int id) const
{
    return AlbumManager::instance()->findTAlbum(id);
}

void TagModel::activateFaceTagModel()
{
    disconnect(AlbumManager::instance(), SIGNAL(signalTAlbumsDirty(QHash<int,int>)),
               this, SLOT(setCountHash(QHash<int,int>)));

    connect(AlbumManager::instance(), &AlbumManager::signalFaceCountsDirty,
            this, [this](const QHash<int, int>& faceCount,
                         const QHash<int, int>& uFaceCount,
                         const QList<int>& toUpdatedFaces)
        {
            setCountHash(faceCount);
            m_unconfirmedFaceCount = uFaceCount;

            for (int id : std::as_const(toUpdatedFaces))
            {
                Album* const album = albumForId(id);

                if (!album)
                {
                    continue;
                }

                QModelIndex index = indexForAlbum(album);

                if (!index.isValid())
                {
                    continue;
                }

                Q_EMIT dataChanged(index, index);
            }
        }
    );

    setFaceTagModel(true);
    setCountHash(AlbumManager::instance()->getFaceCount());
}

bool TagModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const TAlbum* const talbum = talbumForIndex(index);

    if (talbum && talbum->isInternalTag())
    {
        return false;
    }

    return AbstractCheckableAlbumModel::setData(index, value, role);
}

} // namespace Digikam
