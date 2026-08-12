/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-08-03
 * Description : digital camera thumbnails controller
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <QUrl>

// Local includes

#include "camiteminfo.h"
#include "digikam_export.h"

namespace Digikam
{

class CameraController;

typedef QPair<CamItemInfo, QPixmap> CachedItem;

class DIGIKAM_GUI_EXPORT CameraThumbsCtrl : public QObject
{
    Q_OBJECT

public:

    explicit CameraThumbsCtrl(CameraController* const ctrl, QWidget* const parent);
    ~CameraThumbsCtrl()                                                override;

    /**
     * Fill item with relevant information.
     * if item is not in cache, return false and information will be dispatched later through signalThumbInfoReady(),
     * else return true and information is available immediately.
     */
    bool getThumbInfo(const CamItemInfo& info, CachedItem& item) const;

    void setCacheSize(int numberOfItems);
    void clearCache();

    /**
     * Force controller to update info from device in cache.
     */
    void updateThumbInfoFromCache(const CamItemInfo& info);

    /**
     * Return camera controller instance.
     */
    CameraController* cameraController()                         const;

Q_SIGNALS:

    void signalThumbInfoReady(const CamItemInfo&);

private Q_SLOTS:

    void slotThumbInfo(const QString&, const QString&, const CamItemInfo&, const QImage&);
    void slotThumbInfoFailed(const QString&, const QString&, const CamItemInfo&);

private:

    /// Cache management methods.
    void removeItemFromCache(const QUrl& url);
    void putItemToCache(const QUrl& url, const CamItemInfo&, const QPixmap& thumb);
    bool hasItemFromCache(const QUrl& url)                       const;
    const CachedItem* retrieveItemFromCache(const QUrl& url)     const;

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam
