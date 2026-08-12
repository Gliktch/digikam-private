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

// Local includes

#include "digikam_export.h"

class QAction;
class QPoint;

namespace Marble
{

class MarbleWidget;
class MarbleModel;
class GeoDataCoordinates;
class GeoDataPlacemark;

/**
 * The MarbleWidgetPopupMenu handles context menus.
 */
class DIGIKAM_EXPORT MarbleWidgetPopupMenu  : public QObject
{
    Q_OBJECT

public:

    MarbleWidgetPopupMenu(MarbleWidget*, const MarbleModel*);
    ~MarbleWidgetPopupMenu() override;

    /**
      * Adds the action to the menu associated with the specified
      * mouse button
      */
    void addAction(Qt::MouseButton button, QAction* action);

    /**
     * @brief mousePosition Position of the last mouse button click
     * @return
     */
    QPoint mousePosition() const;

public Q_SLOTS:

    void  showLmbMenu(int, int);
    void  showRmbMenu(int, int);

private Q_SLOTS:

    void resetMenu();
    void slotInfoDialog();
    void slotCopyCoordinates();

private Q_SLOTS:

    void startReverseGeocoding();
    void showAddressInformation(const GeoDataCoordinates& coordinates, const GeoDataPlacemark& placemark);

private:

    Q_DISABLE_COPY(MarbleWidgetPopupMenu)

private:

    class Private;
    Private* const d = nullptr;
};

} // Namespace Marble
