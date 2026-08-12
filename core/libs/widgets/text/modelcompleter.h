/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-13
 * Description : A QCompleter for AbstractAlbumModels
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// QT includes

#include <QAbstractItemModel>
#include <QStringList>
#include <QCompleter>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ModelCompleter : public QCompleter
{
    Q_OBJECT

public:

    explicit ModelCompleter(QObject* const parent = nullptr);
    ~ModelCompleter() override;

    /**
     * If the given model is != null, the model is used to populate the
     * completion for this text field.
     *
     * @param model to fill from or null for manual mode
     * @param uniqueIdRole a role for which the model will return a unique integer for each entry
     * @param displayRole the role to retrieve the text for completion, default is Qt::DisplayRole.
     */
    void setItemModel(QAbstractItemModel* const model, int uniqueIdRole, int displayRole = Qt::DisplayRole);
    QAbstractItemModel* itemModel() const;

    void addItem(const QString& item);
    void setList(const QStringList& list);
    QStringList items()             const;

Q_SIGNALS:

    void signalHighlighted(int albumId);
    void signalActivated();

private Q_SLOTS:

    void slotDelayedModelTimer();
    void slotHighlighted(const QModelIndex& index);
    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotModelReset();

private:

    void sync(QAbstractItemModel* const model);
    void sync(QAbstractItemModel* const model, const QModelIndex& index);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
