/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-26
 * Description : History view.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QString>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DHistoryView : public QTreeWidget
{
    Q_OBJECT

public:

    enum EntryType
    {
        StartingEntry = 0,
        SuccessEntry,
        WarningEntry,
        ErrorEntry,
        ProgressEntry,
        CancelEntry
    };

public:

    explicit DHistoryView(QWidget* const parent);
    ~DHistoryView()                     override = default;

    void addEntry(const QString& msg,
                  EntryType type,
                  const QVariant& metadata = QVariant());

Q_SIGNALS:

    void signalEntryClicked(const QVariant& metadata);

private Q_SLOTS:

    void slotItemDoubleClicked(QTreeWidgetItem*);
    void slotContextMenu();
    void slotCopy2ClipBoard();

private:

    void mouseMoveEvent(QMouseEvent*)   override;
};

} // namespace Digikam
