/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-01-28
 * Description : drag and drop handling for Showfoto
 *
 * SPDX-FileCopyrightText: 2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotodragdrophandler.h"

// Qt includes

#include <QDropEvent>
#include <QIcon>
#include <QMimeData>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "showfotocategorizedview.h"
#include "showfotoiteminfo.h"

namespace ShowFoto
{

ShowfotoDragDropHandler::ShowfotoDragDropHandler(ShowfotoItemModel* const model)
    : AbstractItemDragDropHandler(model)
{
}

QAction* ShowfotoDragDropHandler::addGroupAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("go-bottom")),
                           i18nc("@action:inmenu Group images with this image", "Group here"));
}

QAction* ShowfotoDragDropHandler::addCancelAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18nc("@action", "C&ancel"));
}

bool ShowfotoDragDropHandler::dropEvent(QAbstractItemView* abstractview, const QDropEvent* e, const QModelIndex& droppedOn)
{
    Q_UNUSED(abstractview);

    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    QList<QUrl> urls = e->mimeData()->urls();

    Q_EMIT signalDroppedUrls(urls, true, (!urls.isEmpty() ? urls.first() : QUrl()));

    return true;
}

Qt::DropAction ShowfotoDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (e->mimeData()->hasUrls())
    {
        return Qt::LinkAction;
    }

    return Qt::IgnoreAction;
}

QStringList ShowfotoDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;
    mimeTypes << QLatin1String("text/uri-list");

    return mimeTypes;
}

QMimeData* ShowfotoDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<ShowfotoItemInfo> infos = model()->showfotoItemInfos(indexes);
    QList<QUrl> urls;

    for (const ShowfotoItemInfo& info : std::as_const(infos))
    {
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << info.url.toLocalFile();
        urls.append(info.url);
    }

    if (urls.isEmpty())
    {
        return nullptr;
    }

    QMimeData* const mimeData = new QMimeData();
    mimeData->setUrls(urls);

    return mimeData;
}

ShowfotoItemModel* ShowfotoDragDropHandler::model() const
{
    return static_cast<ShowfotoItemModel*>(m_model);
}

} // namespace Showfoto

#include "moc_showfotodragdrophandler.cpp"
