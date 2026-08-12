/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-03
 * Description : Widget displaying filters history used on an image
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QAction>
#include <QModelIndex>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"

namespace Digikam
{

class DImageHistory;

class RemoveFilterAction : public QAction
{
    Q_OBJECT

public:

    explicit RemoveFilterAction(const QString& label, const QModelIndex& index, QObject* const parent = nullptr);
    ~RemoveFilterAction() override = default;

    void setIndex(const QModelIndex& index);

public Q_SLOTS:

    void triggerSlot();

Q_SIGNALS:

    void actionTriggered(QModelIndex index);

private:

    QModelIndex m_index;
};

// -------------------------------------------------------------------------------------

class FiltersHistoryWidget : public QWidget
{
    Q_OBJECT

public:

    explicit FiltersHistoryWidget(QWidget* const parent);
    ~FiltersHistoryWidget() override;

    void setCurrentUrl(const QUrl& url = QUrl());

public Q_SLOTS:

    void showCustomContextMenu(const QPoint& position);
    void setHistory(const DImageHistory& history);
    void clearData();
    void setEnabledEntries(int count);
    void disableEntries(int count);
    void enableEntries(int count);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
