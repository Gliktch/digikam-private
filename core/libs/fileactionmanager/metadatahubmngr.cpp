/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-21
 * Description : metadatahub manager
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatahubmngr.h"

// Qt includes

#include <QMutex>
#include <QProgressDialog>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "iteminfo.h"
#include "metadatahub.h"
#include "iteminfolist.h"
#include "metadatasynchronizer.h"

namespace Digikam
{

QPointer<MetadataHubMngr> MetadataHubMngr::internalPtr = QPointer<MetadataHubMngr>();

class Q_DECL_HIDDEN MetadataHubMngr::Private
{
public:

    Private() = default;

    QList<qlonglong> pendingItemIds;
    QMutex           mutex;
};

MetadataHubMngr::MetadataHubMngr()
    : d(new Private)
{
}

MetadataHubMngr::~MetadataHubMngr()
{
    delete d;
}

MetadataHubMngr* MetadataHubMngr::instance()
{
    if (internalPtr.isNull())
    {
        internalPtr = new MetadataHubMngr();
    }

    return internalPtr;
}

bool MetadataHubMngr::isCreated()
{
    return (!internalPtr.isNull());
}

void MetadataHubMngr::addPendingIds(const QList<qlonglong>& imageIds)
{
    QMutexLocker locker(&d->mutex);

    for (const qlonglong& id : std::as_const(imageIds))
    {
        if (!d->pendingItemIds.contains(id))
        {
            d->pendingItemIds.append(id);

            Q_EMIT signalPendingMetadata(d->pendingItemIds.size());
        }
   }
}

void MetadataHubMngr::addPending(const ItemInfo& info)
{
    QMutexLocker locker(&d->mutex);

    if (!d->pendingItemIds.contains(info.id()))
    {
        d->pendingItemIds.append(info.id());

        Q_EMIT signalPendingMetadata(d->pendingItemIds.size());
    }
}

void MetadataHubMngr::slotRemovePending(const ItemInfo& info)
{
    QMutexLocker locker(&d->mutex);

    if (d->pendingItemIds.isEmpty())
    {
        return;
    }

    if (d->pendingItemIds.contains(info.id()))
    {
        d->pendingItemIds.removeAll(info.id());

        Q_EMIT signalPendingMetadata(d->pendingItemIds.size());
    }
}

void MetadataHubMngr::slotApplyPending()
{
    QMutexLocker lock(&d->mutex);

    if (d->pendingItemIds.isEmpty())
    {
        return;
    }

    ItemInfoList infos;

    for (const qlonglong& id : std::as_const(d->pendingItemIds))
    {
        ItemInfo info(id);

        if (!info.isNull() && !info.isRemoved())
        {
            infos.append(info);
        }
    }

    d->pendingItemIds.clear();

    Q_EMIT signalPendingMetadata(0);

    if (infos.isEmpty())
    {
        return;
    }

    MetadataSynchronizer* const tool = new MetadataSynchronizer(infos,
                                           MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

void MetadataHubMngr::requestShutDown()
{
    QMutexLocker lock(&d->mutex);

    if (d->pendingItemIds.isEmpty())
    {
        return;
    }

    ItemInfoList infos;

    for (const qlonglong& id : std::as_const(d->pendingItemIds))
    {
        ItemInfo info(id);

        if (!info.isNull() && !info.isRemoved())
        {
            infos.append(info);
        }
    }

    d->pendingItemIds.clear();

    Q_EMIT signalPendingMetadata(0);

    if (infos.isEmpty())
    {
        return;
    }

    QPointer<QProgressDialog> dialog = new QProgressDialog;
    dialog->setMinimum(0);
    dialog->setMaximum(0);
    dialog->setMinimumDuration(100);
    dialog->setLabelText(i18nc("@label", "Apply pending changes to metadata"));

    MetadataSynchronizer* const tool = new MetadataSynchronizer(infos,
                                           MetadataSynchronizer::WriteFromDatabaseToFile);

    connect(tool, SIGNAL(signalComplete()),
            dialog, SLOT(accept()));

    tool->start();

    dialog->exec();
}

} // namespace Digikam

#include "moc_metadatahubmngr.cpp"
