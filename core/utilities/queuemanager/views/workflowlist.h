/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-18
 * Description : Customized Workflow Settings list.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class WorkflowList;

class WorkflowItem : public QTreeWidgetItem
{

public:

    WorkflowItem(WorkflowList* const parent, const QString& name);
    ~WorkflowItem()                                                  override = default;

    QString title()                                            const;
    int     count()                                            const;
    void    setItem(const QString& title = QString(),
                    const QString& desc = QString(),
                    int count = 0);

private:

    Q_DISABLE_COPY(WorkflowItem)
};

// -------------------------------------------------------------------------

class WorkflowList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit WorkflowList(QWidget* const parent);
    ~WorkflowList()                                                  override = default;

Q_SIGNALS:

    void signalAssignQueueSettings(const QString&);
    void signalUpdateQueueSettings(const QString&);

public Q_SLOTS:

    void slotRemoveQueueSettings(const QString& title);
    void slotAddQueueSettings(const QString& title);

private Q_SLOTS:

    void slotContextMenu();
    void slotAssignQueueSettings();

private:

    WorkflowItem* findByTitle(const QString& title);

    void        startDrag(Qt::DropActions supportedActions)          override;
    QStringList mimeTypes()                                    const override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QMimeData*  mimeData(const QList<QTreeWidgetItem*>& items) const override;

#else

    // cppcheck-suppress passedByValue
    QMimeData*  mimeData(const QList<QTreeWidgetItem*> items)  const override;      // clazy:exclude=function-args-by-ref

#endif

    void mouseDoubleClickEvent(QMouseEvent*)                         override;

private:

    QString m_lastAssignedTitle;
};

} // namespace Digikam
