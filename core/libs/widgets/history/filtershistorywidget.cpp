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

#include "filtershistorywidget.h"

// Qt includes

#include <QGridLayout>
#include <QTreeView>
#include <QMenu>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "itemfiltershistorymodel.h"
#include "itemfiltershistorytreeitem.h"
#include "itemfiltershistoryitemdelegate.h"

namespace Digikam
{

RemoveFilterAction::RemoveFilterAction(const QString& label, const QModelIndex& index, QObject* const parent)
    : QAction(label, parent),
      m_index(index)
{
}

void RemoveFilterAction::setIndex(const QModelIndex& index)
{
    m_index = index;
}

void RemoveFilterAction::triggerSlot()
{
    Q_EMIT actionTriggered(m_index);
}

// -------------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN FiltersHistoryWidget::Private
{
public:

    Private() = default;

    QTreeView*                      view        = nullptr;
    ItemFiltersHistoryModel*        model       = nullptr;
    QGridLayout*                    layout      = nullptr;
    ItemFiltersHistoryItemDelegate* delegate    = nullptr;
    QLabel*                         headerLabel = nullptr;
};

FiltersHistoryWidget::FiltersHistoryWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->layout      = new QGridLayout(this);
    d->view        = new QTreeView(this);
    d->delegate    = new ItemFiltersHistoryItemDelegate(this);
    d->model       = new ItemFiltersHistoryModel(nullptr, QUrl());
    d->headerLabel = new QLabel(this);

    d->headerLabel->setText(i18n("Used filters"));

    d->layout->addWidget(d->headerLabel);
    d->layout->addWidget(d->view);

    d->view->setItemDelegate(d->delegate);
    d->view->setModel(d->model);
    d->view->setRootIsDecorated(false);
    d->view->setContextMenuPolicy(Qt::CustomContextMenu);
    d->view->setHeaderHidden(true);

    connect(d->view, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showCustomContextMenu(QPoint)));
}

FiltersHistoryWidget::~FiltersHistoryWidget()
{
    delete d->model;
    delete d->delegate;
    delete d;
}

void FiltersHistoryWidget::setCurrentUrl(const QUrl& url)
{
    d->model->setUrl(url);
    d->view->setModel(d->model);
    d->view->update();
}

void FiltersHistoryWidget::showCustomContextMenu(const QPoint& position)
{
    QList<QAction*> actions;

    if (d->view->indexAt(position).isValid())
    {
        QModelIndex index = d->view->indexAt(position);

        QString s(i18n("Remove filter"));
        RemoveFilterAction* removeFilterAction = new RemoveFilterAction(s, index, nullptr);
        removeFilterAction->setDisabled(true);

        if (!index.model()->parent(index).isValid())
        {
            actions.append(removeFilterAction);

            connect(removeFilterAction, SIGNAL(triggered()),
                    removeFilterAction, SLOT(triggerSlot()));

            connect(removeFilterAction, SIGNAL(actionTriggered(QModelIndex)),
                    d->model, SLOT(removeEntry(QModelIndex)));
        }
    }

    if (actions.count() > 0)
    {
        QMenu::exec(actions, d->view->mapToGlobal(position));
    }
}

void FiltersHistoryWidget::setHistory(const DImageHistory& history)
{
    d->model->setupModelData(history.entries());
}

void FiltersHistoryWidget::setEnabledEntries(int count)
{
    d->model->setEnabledEntries(count);
}

void FiltersHistoryWidget::disableEntries(int count)
{
    d->model->disableEntries(count);
}

void FiltersHistoryWidget::enableEntries(int count)
{
    d->model->enableEntries(count);
}

void FiltersHistoryWidget::clearData()
{
    d->model->removeRows(0, d->model->rowCount(), d->model->index(0, 0));
}

} // namespace Digikam

#include "moc_filtershistorywidget.cpp"
