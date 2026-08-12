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

#include <QObject>

// Local includes

#include "geomodelhelper.h"
#include "gpsdatacontainer.h"
#include "bookmarksmngr.h"
#include "digikam_export.h"

namespace Digikam
{

class GPSItemModel;
class GPSUndoCommand;

class DIGIKAM_EXPORT GPSBookmarkModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    enum Constants
    {
        CoordinatesRole = Qt::UserRole + 1
    };

public:

    GPSBookmarkModelHelper(BookmarksManager* const bookmarkManager,
                           GPSItemModel* const imageModel,
                           QObject* const parent = nullptr);
    ~GPSBookmarkModelHelper()                                     override;

    void setVisible(const bool state);

    QAbstractItemModel* model()                             const override;
    QItemSelectionModel* selectionModel()                   const override;
    bool itemCoordinates(const QModelIndex& index,
                         GeoCoordinates* const coordinates) const override;
    bool itemIcon(const QModelIndex& index, QPoint* const offset,
                  QSize* const size, QPixmap* const pixmap,
                  QUrl* const url)                          const override;
    PropertyFlags modelFlags() const override;
    PropertyFlags itemFlags(const QModelIndex& index)       const override;
    void snapItemsTo(const QModelIndex& targetIndex,
                     const QList<QModelIndex>& snappedIndices)    override;

private Q_SLOTS:

    void slotUpdateBookmarksModel();

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
