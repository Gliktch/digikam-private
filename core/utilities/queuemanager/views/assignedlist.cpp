/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager items list.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assignedlist.h"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QPainter>
#include <QUrl>
#include <QMimeData>
#include <QAction>
#include <QMenu>

// KDE includes

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <kactioncollection.h>
#include <klocalizedstring.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "queuemgrwindow.h"
#include "queuesettingsview.h"
#include "toolslistview.h"
#include "batchtoolsfactory.h"

namespace Digikam
{

AssignedListViewItem::AssignedListViewItem(QTreeWidget* const parent)
    : QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags());
}

AssignedListViewItem::AssignedListViewItem(QTreeWidget* const parent, QTreeWidgetItem* const preceding)
    : QTreeWidgetItem(parent, preceding)
{
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags());
}

void AssignedListViewItem::setToolSet(const BatchToolSet& set)
{
    m_set                       = set;
    setIndex(m_set.index);

    const BatchTool* const tool = BatchToolsFactory::instance()->findTool(m_set.name, m_set.group);

    if (tool)
    {
        setIcon(1, tool->toolIcon());
        setText(1, tool->toolTitle());
    }
}

BatchToolSet AssignedListViewItem::toolSet()
{
    return m_set;
}

void AssignedListViewItem::setIndex(int index)
{
    m_set.index = index;
    setText(0, QString::fromUtf8("%1").arg(m_set.index + 1));
}

// ---------------------------------------------------------------------------

AssignedListView::AssignedListView(QWidget* const parent)
    : QTreeWidget(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setWhatsThis(i18n("This is the list of batch tools assigned."));
    setIconSize(QSize(ICONSIZE, ICONSIZE));

    setDragEnabled(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setSortingEnabled(false);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(2);
    setHeaderHidden(true);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(this, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));
}

void AssignedListView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Delete)
    {
        slotRemoveCurrentTool();
    }
    else
    {
        QTreeWidget::keyPressEvent(e);
    }
}

void AssignedListView::setBusy(bool b)
{
    viewport()->setEnabled(!b);
}

AssignedBatchTools AssignedListView::assignedList()
{
    BatchSetList            list;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item)
        {
            list.append(item->toolSet());
        }

        ++it;
    }

    AssignedBatchTools tools;
    tools.m_toolsList = list;

    return tools;
}

int AssignedListView::assignedCount()
{
    return assignedList().m_toolsList.count();
}

void AssignedListView::slotRemoveCurrentTool()
{
    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(currentItem());

    if (item)
    {
        delete item;
        refreshIndex();

        Q_EMIT signalAssignedToolsChanged(assignedList());
    }

    if (assignedCount() == 0)
    {
        Q_EMIT signalToolSelected(BatchToolSet());
    }
}

void AssignedListView::slotClearToolsList()
{
    clear();

    Q_EMIT signalAssignedToolsChanged(assignedList());
    Q_EMIT signalToolSelected(BatchToolSet());
}

void AssignedListView::slotMoveCurrentToolUp()
{
    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(currentItem());

    if (item)
    {
        AssignedListViewItem* const iabove = dynamic_cast<AssignedListViewItem*>(itemAbove(item));

        if (iabove)
        {
            moveTool(item, iabove->toolSet());
            setCurrentItem(item);
        }
    }
}

void AssignedListView::slotMoveCurrentToolDown()
{
    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(currentItem());

    if (item)
    {
        AssignedListViewItem* const ibelow = dynamic_cast<AssignedListViewItem*>(itemBelow(item));

        if (ibelow)
        {
            AssignedListViewItem* const nitem = moveTool(ibelow, item->toolSet());
            setCurrentItem(nitem);
        }
    }
}

AssignedListViewItem* AssignedListView::moveTool(AssignedListViewItem* const preceding, const BatchToolSet& set)
{
    const BatchTool* const tool = BatchToolsFactory::instance()->findTool(set.name, set.group);

    if (!tool)
    {
        return nullptr;
    }

    removeTool(set);
    AssignedListViewItem* const item = insertTool(preceding, set);
    refreshIndex();

    Q_EMIT signalAssignedToolsChanged(assignedList());

    return item;
}

AssignedListViewItem* AssignedListView::insertTool(AssignedListViewItem* const preceding, const BatchToolSet& set)
{
    AssignedListViewItem* item = nullptr;

    if (preceding)
    {
        item = new AssignedListViewItem(this, preceding);
    }
    else
    {
        item = new AssignedListViewItem(this);
    }

    BatchTool* const tool = BatchToolsFactory::instance()->findTool(set.name, set.group);

    // NOTE: Only now create the settings widget when needed.

    if (tool && !tool->settingsWidget())
    {
        tool->registerSettingsWidget();
    }

    item->setToolSet(set);
    refreshIndex();

    Q_EMIT signalAssignedToolsChanged(assignedList());

    return item;
}

AssignedListViewItem* AssignedListView::addTool(const BatchToolSet& set)
{
    return insertTool(nullptr, set);
}

bool AssignedListView::removeTool(const BatchToolSet& set)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item && (item->toolSet() == set))
        {
            delete item;
            refreshIndex();
            return true;
        }

        ++it;
    }

    return false;
}

AssignedListViewItem* AssignedListView::findTool(const BatchToolSet& set)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item && (item->toolSet() == set))
        {
            return item;
        }

        ++it;
    }

    return nullptr;
}

Qt::DropActions AssignedListView::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}

QStringList AssignedListView::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("digikam/assignedbatchtool");

    return types;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

QMimeData* AssignedListView::mimeData(const QList<QTreeWidgetItem*>& items) const

#else

// cppcheck-suppress passedByValue
QMimeData* AssignedListView::mimeData(const QList<QTreeWidgetItem*> items) const    // clazy:exclude=function-args-by-ref

#endif

{
    QMimeData* const mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << (qint32)items.count();

    for (QTreeWidgetItem* const itm : std::as_const(items))
    {
        AssignedListViewItem* const alwi = dynamic_cast<AssignedListViewItem*>(itm);

        if (alwi)
        {
            stream << (qint32)(alwi->toolSet().group);
            stream << alwi->toolSet().name;
            stream << (qint32)alwi->toolSet().index;
            stream << (qint32)alwi->toolSet().version;
            stream << alwi->toolSet().settings;
        }
    }

    mimeData->setData(QLatin1String("digikam/assignedbatchtool"), encodedData);

    return mimeData;
}

void AssignedListView::dragEnterEvent(QDragEnterEvent* e)
{
    QTreeWidget::dragEnterEvent(e);
    e->accept();
}

void AssignedListView::dragMoveEvent(QDragMoveEvent* e)
{
    if (
        e->mimeData()->formats().contains(QLatin1String("digikam/batchtoolslist")) ||
        e->mimeData()->formats().contains(QLatin1String("digikam/assignedbatchtool"))
       )
    {
        QTreeWidget::dragMoveEvent(e);
        e->accept();

        return;
    }

    e->ignore();
}

void AssignedListView::dropEvent(QDropEvent* e)
{
    if      (e->mimeData()->formats().contains(QLatin1String("digikam/batchtoolslist")))
    {
        QByteArray ba = e->mimeData()->data(QLatin1String("digikam/batchtoolslist"));

        if (ba.size())
        {
            QDataStream ds(ba);
            QMultiMap<int, QString> map;
            ds >> map;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            AssignedListViewItem* const preceding = dynamic_cast<AssignedListViewItem*>(itemAt(e->position().toPoint()));

#else

            AssignedListViewItem* const preceding = dynamic_cast<AssignedListViewItem*>(itemAt(e->pos()));

#endif

            assignTools(map, preceding);
        }

        e->acceptProposedAction();
    }
    else if (e->mimeData()->formats().contains(QLatin1String("digikam/assignedbatchtool")))
    {
        QByteArray ba = e->mimeData()->data(QLatin1String("digikam/assignedbatchtool"));

        if (ba.size())
        {
            qint32 count;
            QDataStream ds(ba);
            ds >> count;

            for (int i = 0 ; i < count ; ++i)
            {
                qint32            group, index, version;
                QString           name;
                BatchToolSettings settings;

                ds >> group;
                ds >> name;
                ds >> index;
                ds >> version;
                ds >> settings;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                AssignedListViewItem* const preceding = dynamic_cast<AssignedListViewItem*>(itemAt(e->position().toPoint()));

#else

                AssignedListViewItem* const preceding = dynamic_cast<AssignedListViewItem*>(itemAt(e->pos()));

#endif

                BatchToolSet set;
                set.name                              = name;
                set.group                             = (BatchTool::BatchToolGroup)group;
                set.index                             = index;
                set.version                           = version;
                set.settings                          = settings;
                AssignedListViewItem* const item      = moveTool(preceding, set);
                setCurrentItem(item);
            }
        }

        e->acceptProposedAction();
    }
    else
    {
        e->ignore();
    }
}

void AssignedListView::slotSelectionChanged()
{
    QList<QTreeWidgetItem*> list = selectedItems();

    if (list.isEmpty())
    {
        return;
    }

    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(list.first());

    if (item)
    {
        BatchToolSet set = item->toolSet();

        Q_EMIT signalToolSelected(set);
    }
    else
    {
        Q_EMIT signalToolSelected(BatchToolSet());
    }
}

void AssignedListView::slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools& tools)
{
    // Clear assigned tools list and tool settings view.

    clear();

    Q_EMIT signalToolSelected(BatchToolSet());

    if (!tools.m_toolsList.isEmpty())
    {
        blockSignals(true);

        for (const BatchToolSet& set : std::as_const(tools.m_toolsList))
        {
            addTool(set);
        }

        blockSignals(false);
    }
}

void AssignedListView::slotSettingsChanged(const BatchToolSet& set)
{
    AssignedListViewItem* const item = findTool(set);

    if (item)
    {
        item->setToolSet(set);

        Q_EMIT signalAssignedToolsChanged(assignedList());
    }
}

void AssignedListView::slotAssignTools(const QMultiMap<int, QString>& map)
{
    if (map.isEmpty())
    {
        return;
    }

    assignTools(map, nullptr);
}

void AssignedListView::assignTools(const QMultiMap<int, QString>& map, AssignedListViewItem* const preceding)
{
    // We pop all items in reverse order to have same order than selection from Batch Tools list.

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QMultiMapIterator<int, QString> it(map);

#else

    QMapIterator<int, QString> it(map);

#endif

    it.toBack();

    while (it.hasPrevious())
    {
        it.previous();
        BatchTool::BatchToolGroup group = (BatchTool::BatchToolGroup)(it.key());
        QString name                    = it.value();
        BatchTool* const tool           = BatchToolsFactory::instance()->findTool(name, group);

        if (tool)
        {
            // NOTE: Only now create the settings widget when needed.

            if (!tool->settingsWidget())
            {
                tool->registerSettingsWidget();
            }

            BatchToolSet set;
            set.name                         = tool->objectName();
            set.group                        = tool->toolGroup();
            set.version                      = tool->toolVersion();
            set.settings                     = tool->defaultSettings();
            AssignedListViewItem* const item = insertTool(preceding, set);
            setCurrentItem(item);
        }
    }
}

void AssignedListView::slotContextMenu()
{
    if (!viewport()->isEnabled())
    {
        return;
    }

    KActionCollection* const acol = QueueMgrWindow::queueManagerWindow()->actionCollection();
    QMenu popmenu(this);
    popmenu.addAction(acol->action(QLatin1String("queuemgr_toolup")));
    popmenu.addAction(acol->action(QLatin1String("queuemgr_tooldown")));
    popmenu.addAction(acol->action(QLatin1String("queuemgr_toolremove")));
    popmenu.addSeparator();
    popmenu.addAction(acol->action(QLatin1String("queuemgr_savequeue")));
    popmenu.addAction(acol->action(QLatin1String("queuemgr_toolsclear")));
    popmenu.exec(QCursor::pos());
}

void AssignedListView::refreshIndex()
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item)
        {
            item->setIndex(indexOfTopLevelItem(item));
        }

        ++it;
    }
}

} // namespace Digikam

#include "moc_assignedlist.cpp"
