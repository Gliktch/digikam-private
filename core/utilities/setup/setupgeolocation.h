/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-12-24
 * Description : geo-location setup page.
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SetupGeolocation : public QScrollArea
{
    Q_OBJECT

public:

    enum GeolocationTab
    {
        MarbleView = 0,
        MarblePlugins,
        GoogleMaps
    };

public:

    explicit SetupGeolocation(QWidget* const parent = nullptr);
    ~SetupGeolocation() override;

    void applySettings();
    void cancel();

    void setActiveTab(GeolocationTab tab);
    GeolocationTab activeTab() const;

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
