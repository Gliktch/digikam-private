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

AlbumModel::AlbumModel(RootAlbumBehavior rootBehavior, QObject* const parent)
    : AbstractCheckableAlbumModel(Album::PHYSICAL,
                                  AlbumManager::instance()->findPAlbum(0),
                                  rootBehavior, parent)
{
    m_columnHeader = i18n("Albums");
    setupThumbnailLoading();

    connect(DIO::instance(), SIGNAL(signalTrashCounters()),
            this, SLOT(slotUpdateTrashCounters()));

    connect(AlbumManager::instance(), SIGNAL(signalPAlbumsDirty(QHash<int,int>)),
            this, SLOT(setCountHash(QHash<int,int>)));

    setCountHash(AlbumManager::instance()->getPAlbumsCount());
}

PAlbum* AlbumModel::palbumForIndex(const QModelIndex& index) const
{
    return static_cast<PAlbum*>(AbstractCheckableAlbumModel::albumForIndex(index));
}

QVariant AlbumModel::decorationRoleData(Album* album) const
{
    // asynchronous signals are handled by parent class

    QPixmap thumb = AlbumThumbnailLoader::instance()->getAlbumThumbnailDirectly(static_cast<PAlbum*>(album));
    int size      = ApplicationSettings::instance()->getTreeViewIconSize();

    double ratio  = thumb.devicePixelRatio();
    int rsize     = qRound((double)size * ratio);
    thumb         = thumb.scaled(rsize, rsize, Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);

    QPixmap pix(rsize, rsize);
    pix.fill(Qt::transparent);
    pix.setDevicePixelRatio(ratio);

    QPainter p(&pix);
    p.drawPixmap((rsize - thumb.width())  / 2,
                 (rsize - thumb.height()) / 2, thumb);
    p.end();

    prepareAddExcludeDecoration(album, pix);

    return pix;
}

Album* AlbumModel::albumForId(int id) const
{
    return AlbumManager::instance()->findPAlbum(id);
}

QVariant AlbumModel::albumData(Album* a, int role) const
{
    if (
        (role == Qt::DisplayRole) &&
        showCount()               &&
        a->isTrashAlbum()
       )
    {
        const PAlbum* const palbum = static_cast<PAlbum*>(a->parent());

        if (palbum)
        {
            int counter = DIO::getTrashCounter(palbum->folderPath());

            return QString::fromUtf8("%1 (%2)").arg(albumName(a)).arg(counter);
        }
    }

    return AbstractCheckableAlbumModel::albumData(a, role);
}

void AlbumModel::slotUpdateTrashCounters()
{
    const auto albums = AlbumManager::instance()->allPAlbums();

    for (Album* const album : albums)
    {
        if (album->isTrashAlbum())
        {
            QModelIndex index = indexForAlbum(album);

            if (index.isValid())
            {
                Q_EMIT dataChanged(index, index);
            }
        }
    }
}

} // namespace Digikam

#include "moc_albummodel.cpp"
