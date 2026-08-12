/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Central object for managing bookmarks
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMenu>
#include <QUrl>

// Local includes

#include "gpsdatacontainer.h"
#include "bookmarksmngr.h"
#include "digikam_export.h"

namespace Digikam
{

class GPSItemModel;
class GPSBookmarkModelHelper;

class DIGIKAM_EXPORT GPSBookmarkOwner : public QObject
{
    Q_OBJECT

public:

    GPSBookmarkOwner(GPSItemModel* const gpsItemModel, QWidget* const parent);
    ~GPSBookmarkOwner() override;

    void changeAddBookmark(const bool state);
    void setPositionAndTitle(const GeoCoordinates& coordinates, const QString& title);

    QMenu*                  getMenu()             const;
    BookmarksManager*       bookmarkManager()     const;
    GPSBookmarkModelHelper* bookmarkModelHelper() const;
    QString                 currentTitle()        const;
    QString                 currentUrl()          const;

Q_SIGNALS:

    void positionSelected(const GPSDataContainer& position);

private Q_SLOTS:

    void slotOpenBookmark(const QUrl&);
    void slotShowBookmarksDialog();
    void slotAddBookmark();

private:

    void createBookmarksMenu();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
