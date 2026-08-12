/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-26
 * Description : Qt Model for Images - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "digikam_export.h"

class QAbstractItemView;
class QDropEvent;

namespace Digikam
{

class DIGIKAM_EXPORT AbstractItemDragDropHandler : public QObject
{
    Q_OBJECT

public:

    explicit AbstractItemDragDropHandler(QAbstractItemModel* const model);
    virtual ~AbstractItemDragDropHandler();

    virtual QAbstractItemModel* model()     const;

    /**
     * Gives the view and the occurring drop event.
     * The index is the index where the drop was dropped on.
     * It may be invalid (dropped on decoration, viewport)
     * Returns true if the event is to be accepted.
     */
    virtual bool dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn);

    /**
     * Returns if the given mime data is accepted for drop on dropIndex.
     * Returns the proposed action, or Qt::IgnoreAction if not accepted.
     */
    virtual Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex);

    /**
     * Returns the supported mime types.
     * Called by the default implementation of model's mimeTypes().
     */
    virtual QStringList mimeTypes()         const;

    /**
     * Create a mime data object for starting a drag from the given Albums
     */
    virtual QMimeData* createMimeData(const QList<QModelIndex>&);

    /**
     * Returns if the given mime data can be handled. acceptsMimeData shall return true
     * if a drop of the given mime data will be accepted on any index or place at all.
     * If this returns false, the more specific method accepts() will not be called for this drag.
     * The default implementation uses mimeTypes() to check for supported mime types.
     * There is usually no need to reimplement this.
     */
    virtual bool acceptsMimeData(const QMimeData* data);

protected:

    QAbstractItemModel* m_model = nullptr;
};

} // namespace Digikam
