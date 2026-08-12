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

#pragma once

// KDE includes

#include <QMenu>

// Local includes

#include "abstractitemdragdrophandler.h"
#include "showfotoitemmodel.h"
#include "ddragobjects.h"

namespace ShowFoto
{

class ShowfotoDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    explicit ShowfotoDragDropHandler(ShowfotoItemModel* const model);

    ShowfotoItemModel* model()                                                                           const override;

    bool           dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn)       override;
    Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex)                                  override;
    QStringList    mimeTypes()                                                                           const override;
    QMimeData*     createMimeData(const QList<QModelIndex>&)                                                   override;

Q_SIGNALS:

    void signalDroppedUrls(const QList<QUrl>& droppedUrls, bool dropped, const QUrl& current);

private:

    QAction* addGroupAction(QMenu* const menu);
    QAction* addCancelAction(QMenu* const menu);
};

} // namespace Showfoto
