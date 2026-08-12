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

#include "toolslistview.h"

// Qt includes

#include <QDrag>
#include <QHeaderView>
#include <QMimeData>
#include <QPainter>
#include <QAction>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dpluginbqm.h"

namespace Digikam
{

ToolListViewGroup::ToolListViewGroup(QTreeWidget* const parent, BatchTool::BatchToolGroup group)
    : QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsEnabled);

    setExpanded(true);
    setDisabled(false);

    m_group = group;

    switch (m_group)
    {
        case BatchTool::ColorTool:
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18nc("@title: color tools category", "Color"));
            break;
        }

        case BatchTool::EnhanceTool:
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18nc("@title: enhance tools category", "Enhance"));
            break;
        }

        case BatchTool::TransformTool:
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18nc("@title: transform tools category", "Transform"));
            break;
        }

        case BatchTool::DecorateTool:
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18nc("@title: decorate tools category", "Decorate"));
            break;
        }

        case BatchTool::FiltersTool:
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18nc("@title: filters tools category", "Filters"));
            break;
        }

        case BatchTool::ConvertTool:
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18nc("@title: convert tools category", "Convert"));
            break;
        }

        case BatchTool::MetadataTool:
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18nc("@title: metadata tools category", "Metadata"));
            break;
        }

        default:      // User customized tools.
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("user-properties")));
            setText(0, i18nc("@title: custom tools category", "Custom Tools"));
            break;
        }
    }
}

BatchTool::BatchToolGroup ToolListViewGroup::toolGroup() const
{
    return m_group;
}

// ---------------------------------------------------------------------------

ToolListViewItem::ToolListViewItem(ToolListViewGroup* const parent, BatchTool* const tool)
    : QTreeWidgetItem(parent)
{
    setDisabled(false);
    setSelected(false);

    m_tool = tool;

    if (m_tool)
    {
        setIcon(0, m_tool->toolIcon());
        setText(0, m_tool->toolTitle());
        setText(1, m_tool->toolDescription());
    }
}

BatchTool* ToolListViewItem::tool() const
{
    return m_tool;
}

// ---------------------------------------------------------------------------

ToolsListView::ToolsListView(QWidget* const parent)
    : QTreeWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setIconSize(QSize(22, 22));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSortingEnabled(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(2);
    setHeaderHidden(true);
    setDragEnabled(true);
    header()->setSectionResizeMode(QHeaderView::Stretch);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));
}

BatchToolsList ToolsListView::toolsList()
{
    BatchToolsList list;

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        const ToolListViewItem* const item = dynamic_cast<ToolListViewItem*>(*it);

        if (item)
        {
            list.append(item->tool());
        }

        ++it;
    }

    return list;
}

void ToolsListView::addTool(BatchTool* const tool)
{
    if (!tool)
    {
        return;
    }

    ToolListViewGroup* const parent = findToolGroup(tool->toolGroup());

    if (parent)
    {
        ToolListViewItem* const item = new ToolListViewItem(parent, tool);

        connect(tool, SIGNAL(signalVisible(bool)),
                this, SLOT(slotToolVisible(bool)));

        item->setHidden(!tool->plugin()->shouldLoaded());
    }
}

void ToolsListView::slotToolVisible(bool b)
{
    BatchTool* const tool = dynamic_cast<BatchTool*>(sender());

    if (tool)
    {
        ToolListViewItem* const item = findTool(tool);

        if (item)
        {
            item->setHidden(!b);
        }
    }
}

bool ToolsListView::removeTool(BatchTool* const tool)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        ToolListViewItem* const item = dynamic_cast<ToolListViewItem*>(*it);

        if (item && (item->tool() == tool))
        {
            delete item;

            return true;
        }

        ++it;
    }

    return false;
}

ToolListViewGroup* ToolsListView::findToolGroup(BatchTool::BatchToolGroup group)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        ToolListViewGroup* const item = dynamic_cast<ToolListViewGroup*>(*it);

        if (item && (item->toolGroup() == group))
        {
            return item;
        }

        ++it;
    }

    return nullptr;
}

bool ToolsListView::hasTool(BatchTool* const tool)
{
    return (findTool(tool) ? true : false);
}

ToolListViewItem* ToolsListView::findTool(BatchTool* const tool)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        ToolListViewItem* const item = dynamic_cast<ToolListViewItem*>(*it);

        if (item && (item->tool() == tool))
        {
            return item;
        }

        ++it;
    }

    return nullptr;
}

void ToolsListView::startDrag(Qt::DropActions /*supportedActions*/)
{
    QList<QTreeWidgetItem*> items = selectedItems();

    if (items.isEmpty())
    {
        return;
    }

    QPixmap icon(QIcon::fromTheme(QLatin1String("system-run")).pixmap(48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w + 4, h + 4);
    QString text(QString::number(items.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, pix.width() - 1, pix.height() - 1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
    p.drawPixmap(2, 2, icon);
    QRect r     = p.boundingRect(2, 2, w, h, Qt::AlignLeft | Qt::AlignTop, text);
    r.setWidth(qMax(r.width(), r.height()));
    r.setHeight(qMax(r.width(), r.height()));
    p.fillRect(r, QColor(0, 80, 0));
    p.setPen(Qt::white);
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);
    p.end();

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData(items));
    drag->setPixmap(pix);
    drag->exec();
}

QStringList ToolsListView::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("digikam/batchtoolslist");

    return types;
}

void ToolsListView::mouseDoubleClickEvent(QMouseEvent*)
{
    if (viewport()->isEnabled())
    {
        slotAssignTools();
    }
}

void ToolsListView::slotAssignTools()
{
    QList<QTreeWidgetItem*> items = selectedItems();

    if (items.isEmpty())
    {
        return;
    }

    QMultiMap<int, QString> map = itemsToMap(items);

    Q_EMIT signalAssignTools(map);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

QMimeData* ToolsListView::mimeData(const QList<QTreeWidgetItem*>& items) const

#else

// cppcheck-suppress passedByValue
QMimeData* ToolsListView::mimeData(const QList<QTreeWidgetItem*> items) const    // clazy:exclude=function-args-by-ref

#endif

{
    QMimeData* const mimeData   = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QMultiMap<int, QString> map = itemsToMap(items);
    stream << map;

    mimeData->setData(QLatin1String("digikam/batchtoolslist"), encodedData);

    return mimeData;
}

QMultiMap<int, QString> ToolsListView::itemsToMap(const QList<QTreeWidgetItem*>& items) const
{
    QMultiMap<int, QString> map;

    for (QTreeWidgetItem* const itm : std::as_const(items))
    {
        const ToolListViewItem* const tlwi = dynamic_cast<ToolListViewItem*>(itm);

        if (tlwi)
        {
            map.insert((int)(tlwi->tool()->toolGroup()), tlwi->tool()->objectName());
        }
    }

    return map;
}

void ToolsListView::slotContextMenu()
{
    QMenu popmenu(this);
    QAction* const action = new QAction(QIcon::fromTheme(QLatin1String("list-add")), i18nc("@action", "Assign tools"), this);

    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotAssignTools()));

    popmenu.addAction(action);
    popmenu.exec(QCursor::pos());
}

} // namespace Digikam

#include "moc_toolslistview.cpp"
