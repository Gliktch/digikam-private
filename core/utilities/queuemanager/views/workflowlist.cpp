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

#include "workflowlist.h"

// Qt includes

#include <QApplication>
#include <QDrag>
#include <QHeaderView>
#include <QMimeData>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "workflowmanager.h"
#include "workflowdlg.h"
#include "dmessagebox.h"

namespace Digikam
{

WorkflowItem::WorkflowItem(WorkflowList* const parent, const QString& title)
    : QTreeWidgetItem(parent)
{
    setDisabled(false);
    setSelected(false);

    Workflow q = WorkflowManager::instance()->findByTitle(title);

    setIcon(0, QIcon::fromTheme(QLatin1String("step")));
    setItem(title, q.desc, q.aTools.count());
}

QString WorkflowItem::title() const
{
    return text(0);
}

int WorkflowItem::count() const
{
    return text(1).toInt();
}

void WorkflowItem::setItem(const QString& _title, const QString& _desc, int _count)
{
    if (!_title.isEmpty())
    {
        setText(0, _title);
    }

    if (!_desc.isEmpty())
    {
        setText(2, _desc);
    }

    if (_count > 0)
    {
        setText(1, QString::number(_count));
    }
}

// ---------------------------------------------------------------------------

WorkflowList::WorkflowList(QWidget* const parent)
    : QTreeWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setIconSize(QSize(22, 22));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(true);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(3);
    setHeaderHidden(false);
    setDragEnabled(true);

    QStringList titles;
    titles.append(i18nc("@title: batch workflow name",        "Title"));
    titles.append(i18nc("@title: batch workflow tools list",  "Tools"));
    titles.append(i18nc("@title: batch workflow description", "Description"));

    setHeaderLabels(titles);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(2, QHeaderView::Stretch);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));

    WorkflowManager* const mngr = WorkflowManager::instance();
    QStringList failed;
    mngr->load(failed);
    const auto qs               = mngr->queueSettingsList();

    for (const Workflow& q : qs)
    {
        slotAddQueueSettings(q.title);
    }

    if (!failed.isEmpty())
    {
        DMessageBox::showInformationList(QMessageBox::Information,
                                         qApp->activeWindow(),
                                         i18nc("@title:window", "Batch Queue Manager"),
                                         i18nc("@info", "Some Workflows cannot be loaded from your config file due to an incompatible "
                                                        "version of a tool."),
                                         failed);
    }
}

void WorkflowList::slotAddQueueSettings(const QString& title)
{
    const WorkflowItem* const item = findByTitle(title);

    if (!item)
    {
        new WorkflowItem(this, title);
    }
}

void WorkflowList::slotRemoveQueueSettings(const QString& title)
{
    WorkflowItem* const item = findByTitle(title);

    if (item)
    {
        delete item;
    }
}

WorkflowItem* WorkflowList::findByTitle(const QString& title)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        WorkflowItem* const item = dynamic_cast<WorkflowItem*>(*it);

        if (item && (item->title() == title))
        {
            return item;
        }

        ++it;
    }

    return nullptr;
}

void WorkflowList::startDrag(Qt::DropActions /*supportedActions*/)
{
    QList<QTreeWidgetItem*> list = selectedItems();

    if (!list.isEmpty())
    {
        const WorkflowItem* const item = dynamic_cast<WorkflowItem*>(list.first());

        if (!item)
        {
            return;
        }

        QPixmap icon(QIcon::fromTheme(QLatin1String("step")).pixmap(48));
        int w = icon.width();
        int h = icon.height();

        QPixmap pix(w + 4, h + 4);
        QString text(QString::number(item->count()));

        QPainter p(&pix);
        p.fillRect(0, 0, pix.width() - 1, pix.height() - 1, QColor(Qt::white));
        p.setPen(QPen(Qt::black, 1));
        p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
        p.drawPixmap(2, 2, icon);
        QRect r           = p.boundingRect(2, 2, w, h, Qt::AlignLeft | Qt::AlignTop, text);
        r.setWidth(qMax(r.width(), r.height()));
        r.setHeight(qMax(r.width(), r.height()));
        p.fillRect(r, QColor(0, 80, 0));
        p.setPen(Qt::white);
        QFont f(font());
        f.setBold(true);
        p.setFont(f);
        p.drawText(r, Qt::AlignCenter, text);
        p.end();

        QDrag* const drag = new QDrag(this);
        drag->setMimeData(mimeData(list));
        drag->setPixmap(pix);
        drag->exec();

        if (drag->target())
        {
            m_lastAssignedTitle = item->title();
        }
    }
}

QStringList WorkflowList::mimeTypes() const
{
    return (QStringList() << QLatin1String("digikam/workflow"));
}

void WorkflowList::mouseDoubleClickEvent(QMouseEvent*)
{
    if (viewport()->isEnabled())
    {
        slotAssignQueueSettings();
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

QMimeData* WorkflowList::mimeData(const QList<QTreeWidgetItem*>& items) const

#else

// cppcheck-suppress passedByValue
QMimeData* WorkflowList::mimeData(const QList<QTreeWidgetItem*> items) const    // clazy:exclude=function-args-by-ref

#endif

{
    QMimeData* const mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    if (!items.isEmpty())
    {
        const WorkflowItem* const item  = dynamic_cast<WorkflowItem*>(items.first());

        if (item)
        {
            stream << item->title();
        }
    }

    mimeData->setData(QLatin1String("digikam/workflow"), encodedData);

    return mimeData;
}

void WorkflowList::slotContextMenu()
{
    QList<QTreeWidgetItem*> list = selectedItems();

    if (list.isEmpty())
    {
        return;
    }

    WorkflowItem* const item    = dynamic_cast<WorkflowItem*>(list.first());
    WorkflowManager* const mngr = WorkflowManager::instance();

    if (!item)
    {
        return;
    }

    QMenu popmenu(this);
    QAction* const assignAction = new QAction(QIcon::fromTheme(QLatin1String("list-add")),     i18nc("@action", "Assign Workflow to current queue"), this);
    QAction* const propAction   = new QAction(QIcon::fromTheme(QLatin1String("configure")),    i18nc("@action", "Edit Workflow"),                    this);
    QAction* const updAction    = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")), i18nc("@action", "Update Workflow"),                  this);
    QAction* const delAction    = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),  i18nc("@action", "Delete Workflow"),                  this);

    popmenu.addAction(assignAction);
    popmenu.addAction(propAction);
    popmenu.addAction(updAction);

    if (m_lastAssignedTitle != item->title())
    {
        updAction->setDisabled(true);
    }

    popmenu.addSeparator();
    popmenu.addAction(delAction);

    const QAction* const choice = popmenu.exec(QCursor::pos());

    if      (choice == assignAction)
    {
        slotAssignQueueSettings();
    }
    else if (choice == propAction)
    {
        Workflow wfOld = mngr->findByTitle(item->title());
        Workflow wfNew = wfOld;

        if (WorkflowDlg::editProps(wfNew))
        {
            mngr->remove(wfOld);
            mngr->insert(wfNew);
            mngr->save();

            if (wfOld.title != wfNew.title)
            {
                removeItemWidget(item, 0);
                delete item;
            }
            else
            {
                item->setItem(QString(), wfNew.desc);
            }
        }
    }
    else if (choice == updAction)
    {
        Q_EMIT signalUpdateQueueSettings(item->title());

        Workflow q = WorkflowManager::instance()->findByTitle(item->title());
        item->setItem(QString(), QString(), q.aTools.count());
    }
    else if (choice == delAction)
    {
        int result = QMessageBox::warning(qApp->activeWindow(),
                                  i18nc("@title:window", "Delete Workflow?"),
                                  i18nc("@info", "Are you sure you want to "
                                                 "delete the selected workflow "
                                                 "\"%1\"?", item->title()),
                                  QMessageBox::Yes | QMessageBox::Cancel);

        if (result == QMessageBox::Yes)
        {
            Workflow wf = mngr->findByTitle(item->title());
            mngr->remove(wf);
            mngr->save();
            removeItemWidget(item, 0);
            delete item;
        }
    }
}

void WorkflowList::slotAssignQueueSettings()
{
    QList<QTreeWidgetItem*> list = selectedItems();

    if (!list.isEmpty())
    {
        const WorkflowItem* const item = dynamic_cast<WorkflowItem*>(list.first());

        if (item)
        {
            Q_EMIT signalAssignQueueSettings(item->title());

            m_lastAssignedTitle = item->title();
        }
    }
}

} // namespace Digikam

#include "moc_workflowlist.cpp"
