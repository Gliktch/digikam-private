/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-02-20
 * Description : Synchronized container for maintenance data.
 *
 * SPDX-FileCopyrightText: 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "maintenancedata.h"

// Qt includes

#include <QRecursiveMutex>
#include <QMutexLocker>

namespace Digikam
{

class Q_DECL_HIDDEN MaintenanceData::Private
{
public:

    Private() = default;

    QList<qlonglong> imageIdList;
    QList<int>       thumbnailIdList;
    QList<QString>   imagePathList;
    QList<ItemInfo>  imageInfoList;
    QList<Identity>  identitiesList;
    QList<qlonglong> similarityImageIdList;

    QRecursiveMutex  mutex;
};

MaintenanceData::MaintenanceData()
    : d(new Private)
{
}

MaintenanceData::~MaintenanceData()
{
    delete d;
}

void MaintenanceData::setImageIds(const QList<qlonglong>& ids)
{
    d->imageIdList = ids;
}

void MaintenanceData::setThumbnailIds(const QList<int>& ids)
{
    d->thumbnailIdList = ids;
}

void MaintenanceData::setImagePaths(const QList<QString>& paths)
{
    d->imagePathList = paths;
}

void MaintenanceData::setItemInfos(const QList<ItemInfo>& infos)
{
    d->imageInfoList = infos;
}

void MaintenanceData::setSimilarityImageIds(const QList<qlonglong>& ids)
{
    d->similarityImageIdList = ids;
}

void MaintenanceData::setIdentities(const QList<Identity>& identities)
{
    d->identitiesList = identities;
}

qlonglong MaintenanceData::getImageId() const
{
    QMutexLocker locker(&d->mutex);

    qlonglong id = -1;

    if (!d->imageIdList.isEmpty())
    {
        id = d->imageIdList.takeFirst();
    }

    return id;
}

int MaintenanceData::getThumbnailId() const
{
    QMutexLocker locker(&d->mutex);

    int id = -1;

    if (!d->thumbnailIdList.isEmpty())
    {
        id = d->thumbnailIdList.takeFirst();
    }

    return id;
}

QString MaintenanceData::getImagePath() const
{
    QMutexLocker locker(&d->mutex);

    QString path;

    if (!d->imagePathList.isEmpty())
    {
        path = d->imagePathList.takeFirst();
    }

    return path;
}

ItemInfo MaintenanceData::getItemInfo() const
{
    QMutexLocker locker(&d->mutex);

    ItemInfo info;

    if (!d->imageInfoList.isEmpty())
    {
        info = d->imageInfoList.takeFirst();
    }

    return info;
}

Identity MaintenanceData::getIdentity() const
{
    QMutexLocker locker(&d->mutex);

    Identity identity;

    if (!d->identitiesList.isEmpty())
    {
        identity = d->identitiesList.takeFirst();
    }

    return identity;
}

qlonglong MaintenanceData::getSimilarityImageId() const
{
    QMutexLocker locker(&d->mutex);

    qlonglong id = -1;

    if (!d->similarityImageIdList.isEmpty())
    {
        id = d->similarityImageIdList.takeFirst();
    }

    return id;
}

} // namespace Digikam
