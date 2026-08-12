/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-08-03
 * Description : digital camera thumbnails controller
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "camerathumbsctrl.h"

// Qt includes

#include <QCache>
#include <QPair>

// Local includes

#include "digikam_debug.h"
#include "cameracontroller.h"
#include "thumbnailsize.h"
#include "iccsettings.h"
#include "iccmanager.h"
#include "iccprofile.h"

namespace Digikam
{

class Q_DECL_HIDDEN CameraThumbsCtrlStaticPriv
{
public:

    CameraThumbsCtrlStaticPriv() = default;

public:

    IccProfile profile = IccProfile::sRGB();
};

Q_GLOBAL_STATIC(CameraThumbsCtrlStaticPriv, static_d)

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN CameraThumbsCtrl::Private
{

public:

    Private() = default;

    QCache<QUrl, CachedItem>   cache;  // Camera info/thumb cache based on item url keys.

    QMap<QUrl, CameraCommand*> pendingItems;

    CameraController*          controller = nullptr;
};

// --------------------------------------------------------

CameraThumbsCtrl::CameraThumbsCtrl(CameraController* const ctrl, QWidget* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->controller     = ctrl;
    static_d->profile = IccManager::displayProfile(parent);

    connect(d->controller, SIGNAL(signalThumbInfo(QString,QString,CamItemInfo,QImage)),
            this, SLOT(slotThumbInfo(QString,QString,CamItemInfo,QImage)));

    connect(d->controller, SIGNAL(signalThumbInfoFailed(QString,QString,CamItemInfo)),
            this, SLOT(slotThumbInfoFailed(QString,QString,CamItemInfo)));

    setCacheSize(200);
}

CameraThumbsCtrl::~CameraThumbsCtrl()
{
    clearCache();
}

CameraController* CameraThumbsCtrl::cameraController() const
{
    return d->controller;
}

bool CameraThumbsCtrl::getThumbInfo(const CamItemInfo& info, CachedItem& item) const
{
    if      (hasItemFromCache(info.url()))
    {
        // We look if items are not in cache.

        item = *retrieveItemFromCache(info.url());

        return true;
    }
    else if (d->pendingItems.contains(info.url()))
    {
        d->controller->moveThumbsInfo(d->pendingItems.value(info.url()));
    }
    else
    {
        // We look if items are not in pending list.

        CameraCommand* const cmd = d->controller->getThumbsInfo(CamItemInfoList() << info,
                                                                ThumbnailSize::maxThumbsSize());
        d->pendingItems.insert(info.url(), cmd);
    }

    item = CachedItem(info, d->controller->mimeTypeThumbnail(info.name).pixmap(ThumbnailSize::maxThumbsSize()));

    return false;
}

void CameraThumbsCtrl::updateThumbInfoFromCache(const CamItemInfo& info)
{
    removeItemFromCache(info.url());
    CachedItem item;
    getThumbInfo(info, item);
}

void CameraThumbsCtrl::slotThumbInfo(const QString&, const QString& file, const CamItemInfo& info, const QImage& thumb)
{
    QImage thumbnail = thumb;

    if (thumb.isNull())
    {
        thumbnail = d->controller->mimeTypeThumbnail(file).pixmap(ThumbnailSize::maxThumbsSize()).toImage();
    }

    // Color Managed view rules.

    if (IccSettings::instance()->useManagedPreviews())
    {
        IccManager::transformForDisplay(thumbnail, static_d->profile);
    }

    putItemToCache(info.url(), info, QPixmap::fromImage(thumbnail));
    d->pendingItems.remove(info.url());

    Q_EMIT signalThumbInfoReady(info);
}

void CameraThumbsCtrl::slotThumbInfoFailed(const QString& /*folder*/, const QString& file, const CamItemInfo& info)
{
    QPixmap pix = d->controller->mimeTypeThumbnail(file).pixmap(ThumbnailSize::maxThumbsSize());
    putItemToCache(info.url(), info, pix);
    d->pendingItems.remove(info.url());

    Q_EMIT signalThumbInfoReady(info);
}

// -- Cache management methods ------------------------------------------------------------

const CachedItem* CameraThumbsCtrl::retrieveItemFromCache(const QUrl& url) const
{
    return d->cache[url];
}

bool CameraThumbsCtrl::hasItemFromCache(const QUrl& url) const
{
    return d->cache.contains(url);
}

void CameraThumbsCtrl::putItemToCache(const QUrl& url, const CamItemInfo& info, const QPixmap& thumb)
{
    int infoCost  = sizeof(info);
    int thumbCost = thumb.width() * thumb.height() * thumb.depth() / 8;
    d->cache.insert(url, new CachedItem(info, thumb), infoCost + thumbCost);
}

void CameraThumbsCtrl::removeItemFromCache(const QUrl& url)
{
    d->cache.remove(url);
}

void CameraThumbsCtrl::clearCache()
{
    d->cache.clear();
}

void CameraThumbsCtrl::setCacheSize(int numberOfItems)
{
    d->cache.setMaxCost(numberOfItems * (ThumbnailSize::maxThumbsSize() * ThumbnailSize::maxThumbsSize() *
                                         QPixmap(1, 1).depth() / 8) + (numberOfItems * sizeof(CamItemInfo)));
}

} // namespace Digikam

#include "moc_camerathumbsctrl.cpp"
