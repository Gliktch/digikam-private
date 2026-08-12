/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : a view to host all available tools.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dhistoryview.h"

namespace Digikam
{

class BatchTool;

class ToolsView : public QTabWidget
{
    Q_OBJECT

public:

    enum ViewTabs
    {
        TOOLS = 0,
        WORKFLOW,
        HISTORY
    };

public:

    explicit ToolsView(QWidget* const parent = nullptr);
    ~ToolsView() override;

    void addTool(BatchTool* const tool);
    bool removeTool(BatchTool* const tool);

    void setBusy(bool b);

    void showTab(ViewTabs t);
    void addHistoryEntry(const QString& msg,
                         DHistoryView::EntryType type,
                         int queueId = -1,
                         qlonglong itemId = -1);

Q_SIGNALS:

    void signalAssignQueueSettings(QString);
    void signalUpdateQueueSettings(QString);
    void signalAssignTools(const QMultiMap<int, QString>&);
    void signalHistoryEntryClicked(int, qlonglong);

private Q_SLOTS:

    void slotHistoryEntryClicked(const QVariant&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
