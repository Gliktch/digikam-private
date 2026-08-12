/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-09-07
 * Description : Qt Model for ImportUI - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importdragdrop.h"

// Qt includes

#include <QDropEvent>
#include <QIcon>
#include <QTimer>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "importiconview.h"
#include "importui.h"
#include "ddragobjects.h"
#include "importcategorizedview.h"
#include "camiteminfo.h"
#include "digikamapp.h"
#include "itemiconview.h"

namespace Digikam
{

ImportDragDropHandler::ImportDragDropHandler(ImportItemModel* const model)
    : AbstractItemDragDropHandler(model)
{
}

ImportItemModel* ImportDragDropHandler::model() const
{
    return static_cast<ImportItemModel*>(m_model);
}

bool ImportDragDropHandler::dropEvent(QAbstractItemView* view,
                                      const QDropEvent* e,
                                      const QModelIndex& droppedOn)
{
    m_view = view;

    if (!m_view)
    {
        return false;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_position  = m_view->mapToGlobal(e->position().toPoint());
    m_modifiers = e->modifiers();

#else

    m_position  = m_view->mapToGlobal(e->pos());
    m_modifiers = e->keyboardModifiers();

#endif

    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    if (DItemDrag::canDecode(e->mimeData()))
    {
        QTimer::singleShot(0, this, &ImportDragDropHandler::slotUploadCamItems);

        return true;
    }
/*
    TODO: Implement tag dropping in import tool.
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
    }
*/
    return false;
}

Qt::DropAction ImportDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (DItemDrag::canDecode(e->mimeData()) || e->mimeData()->hasUrls())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        if      (e->modifiers() & Qt::ControlModifier)

#else

        if      (e->keyboardModifiers() & Qt::ControlModifier)

#endif

        {
            return Qt::CopyAction;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        else if (e->modifiers() & Qt::ShiftModifier)

#else

        else if (e->keyboardModifiers() & Qt::ShiftModifier)

#endif
        {
            return Qt::MoveAction;
        }

        return Qt::MoveAction;
    }

    if (
        DTagListDrag::canDecode(e->mimeData())        ||
        DCameraItemListDrag::canDecode(e->mimeData()) ||
        DCameraDragObject::canDecode(e->mimeData())
       )
    {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QStringList ImportDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;
    mimeTypes << DItemDrag::mimeTypes()
              << DTagListDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << DCameraDragObject::mimeTypes()
              << QLatin1String("text/uri-list");

    return mimeTypes;
}

QMimeData* ImportDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<CamItemInfo> infos = model()->camItemInfos(indexes);

    QStringList lst;

    for (const CamItemInfo& info : std::as_const(infos))
    {
        lst.append(info.folder + info.name);
    }

    if (lst.isEmpty())
    {
        return nullptr;
    }

    return (new DCameraItemListDrag(lst));
}

void ImportDragDropHandler::slotUploadCamItems()
{
    QList<QUrl> lst               = DigikamApp::instance()->view()->selectedUrls();

    QMenu popMenu(m_view);
    popMenu.addSection(QIcon::fromTheme(QLatin1String("digikam")), i18n("Exporting"));
    const QAction* const upAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("media-flash-sd-mmc")),
                                                i18n("Upload to Camera"));
    popMenu.addSeparator();
    popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

    const QAction* const choice   = popMenu.exec(m_position);

    if (choice)
    {
        if (choice == upAction)
        {
            ImportUI::instance()->slotUploadItems(lst);
        }
    }
}

} // namespace Digikam

#include "moc_importdragdrop.cpp"
