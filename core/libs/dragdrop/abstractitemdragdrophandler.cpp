/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-26
 * Description : Qt Model for Images - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "abstractitemdragdrophandler.h"

// Qt includes

#include <QMimeData>
#include <QStringList>

namespace Digikam
{

AbstractItemDragDropHandler::AbstractItemDragDropHandler(QAbstractItemModel* const model)
    : QObject(model),
      m_model(model)
{
}

AbstractItemDragDropHandler::~AbstractItemDragDropHandler()
{
}

QAbstractItemModel* AbstractItemDragDropHandler::model() const
{
    return m_model;
}

bool AbstractItemDragDropHandler::dropEvent(QAbstractItemView*, const QDropEvent*, const QModelIndex&)
{
    return false;
}

Qt::DropAction AbstractItemDragDropHandler::accepts(const QDropEvent*, const QModelIndex&)
{
    return Qt::IgnoreAction;
}

QStringList AbstractItemDragDropHandler::mimeTypes() const
{
    return QStringList();
}

QMimeData* AbstractItemDragDropHandler::createMimeData(const QList<QModelIndex>&)
{
    return nullptr;
}

bool AbstractItemDragDropHandler::acceptsMimeData(const QMimeData* mime)
{
    QStringList modelTypes = mimeTypes();

    for (int i = 0 ; i < modelTypes.count() ; ++i)
    {
        if (mime->hasFormat(modelTypes.at(i))) //&& (e->dropAction() & model->supportedDropActions()))
        {
            return true;
        }
    }

    return false;
}

} // namespace Digikam

#include "moc_abstractitemdragdrophandler.cpp"
