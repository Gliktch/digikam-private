/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummodeldragdrophandler.h"

// Qt includes

#include <QMimeData>

// Local includes

#include "abstractalbummodel.h"

namespace Digikam
{

AlbumModelDragDropHandler::AlbumModelDragDropHandler(AbstractAlbumModel* model)
    : QObject(model),
      m_model(model)
{
}

AbstractAlbumModel* AlbumModelDragDropHandler::model() const
{
    return m_model;
}

bool AlbumModelDragDropHandler::dropEvent(QAbstractItemView*, const QDropEvent*, const QModelIndex&)
{
    return false;
}

Qt::DropAction AlbumModelDragDropHandler::accepts(const QDropEvent*, const QModelIndex&)
{
    return Qt::IgnoreAction;
}

QStringList AlbumModelDragDropHandler::mimeTypes() const
{
    return QStringList();
}

QMimeData* AlbumModelDragDropHandler::createMimeData(const QList<Album*>&)
{
    return nullptr;
}

bool AlbumModelDragDropHandler::acceptsMimeData(const QMimeData* mime)
{
    QStringList modelTypes = mimeTypes();

    for (int i = 0 ; i < modelTypes.count() ; ++i)
    {
        if (mime->hasFormat(modelTypes.at(i)))
/*
         && (e->dropAction() & model->supportedDropActions()))
*/
        {
            return true;
        }
    }

    return false;
}

} // namespace Digikam

#include "moc_albummodeldragdrophandler.cpp"
