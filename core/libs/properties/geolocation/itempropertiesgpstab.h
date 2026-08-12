/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab to display GPS info
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QUrl>

// Local includes

#include "dmetadata.h"
#include "digikam_export.h"
#include "gpsiteminfosorter.h"

namespace Digikam
{

class DIGIKAM_EXPORT ItemPropertiesGPSTab : public QWidget
{
    Q_OBJECT

public:

    enum WebGPSLocator
    {
        MapQuest = 0,
        GoogleMaps,
        BingMaps,
        OpenStreetMap,
        LocAlizeMaps
    };

public:

    explicit ItemPropertiesGPSTab(QWidget* const parent);
    ~ItemPropertiesGPSTab() override;

    void clearGPSInfo();
    void setGPSInfoList(const GPSItemInfo::List& list);
    void setCurrentUrl(const QUrl& url = QUrl());

    void setMetadata(DMetadata* const meta, const QUrl& url);

    int  getWebGPSLocator() const;
    void setWebGPSLocator(int locator);

    void setActive(const bool state);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

private Q_SLOTS:

    void slotGPSDetails();
    void slotCopyGPSDetails();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
