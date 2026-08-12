/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Available batch tools list.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QPixmap>
#include <QMap>
#include <QString>
#include <QList>

// Local includes

#include "batchtool.h"
#include "batchtoolsfactory.h"

namespace Digikam
{

class ToolListViewGroup : public QTreeWidgetItem
{

public:

    ToolListViewGroup(QTreeWidget* const parent,
                      BatchTool::BatchToolGroup group);
    ~ToolListViewGroup()                        override = default;

    BatchTool::BatchToolGroup toolGroup() const;

private:

    BatchTool::BatchToolGroup m_group;

 private:

    Q_DISABLE_COPY(ToolListViewGroup)
};

// -------------------------------------------------------------------------

class ToolListViewItem : public QTreeWidgetItem
{

public:

    ToolListViewItem(ToolListViewGroup* const parent,
                     BatchTool* const tool);
    ~ToolListViewItem()                         override = default;

    BatchTool* tool()                     const;

private:

    BatchTool* m_tool = nullptr;

private:

    Q_DISABLE_COPY(ToolListViewItem)
};

// -------------------------------------------------------------------------

class ToolsListView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit ToolsListView(QWidget* const parent);
    ~ToolsListView()                                                               override = default;

    BatchToolsList toolsList();

    void addTool(BatchTool* const tool);
    bool removeTool(BatchTool* const tool);

Q_SIGNALS:

    void signalAssignTools(const QMultiMap<int, QString>&);

private Q_SLOTS:

    void slotContextMenu();
    void slotAssignTools();
    void slotToolVisible(bool);

private:

    bool hasTool(BatchTool* const tool);
    ToolListViewItem* findTool(BatchTool* const tool);
    ToolListViewGroup* findToolGroup(BatchTool::BatchToolGroup group);

    void        startDrag(Qt::DropActions supportedActions)                        override;
    QStringList mimeTypes()                                                  const override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QMimeData*  mimeData(const QList<QTreeWidgetItem*>& items)               const override;

#else

    // cppcheck-suppress passedByValue
    QMimeData*  mimeData(const QList<QTreeWidgetItem*> items)                const override;      // clazy:exclude=function-args-by-ref

#endif

    void mouseDoubleClickEvent(QMouseEvent*)                                       override;
    QMultiMap<int, QString> itemsToMap(const QList<QTreeWidgetItem*>& items) const;
};

} // namespace Digikam
