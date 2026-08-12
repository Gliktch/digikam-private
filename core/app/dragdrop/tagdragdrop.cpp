/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Qt Model for Tag - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagdragdrop.h"
#include "facetags.h"
#include "digikamitemview_p.h"

// Qt includes

#include <QDropEvent>
#include <QMenu>
#include <QIcon>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "ddragobjects.h"
#include "iteminfo.h"
#include "albumtreeview.h"

namespace Digikam
{

TagDragDropHandler::TagDragDropHandler(TagModel* const model)
    : AlbumModelDragDropHandler(model)
{
}

TagModel* TagDragDropHandler::model() const
{
    return (static_cast<TagModel*>(m_model));
}

bool TagDragDropHandler::dropEvent(QAbstractItemView* view,
                                   const QDropEvent* e,
                                   const QModelIndex& droppedOn)
{
    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    if (!view)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error: View is null!";
        return false;
    }

    m_view      = view;
    m_source    = e->source();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_position  = m_view->mapToGlobal(e->position().toPoint());
    m_modifiers = e->modifiers();

#else

    m_position  = m_view->mapToGlobal(e->pos());
    m_modifiers = e->keyboardModifiers();

#endif

    m_destAlbum = model()->talbumForIndex(droppedOn);

    m_tagIDs.clear();
    m_imageIDs.clear();

    if (DTagListDrag::canDecode(e->mimeData()))
    {
        if (!DTagListDrag::decode(e->mimeData(), m_tagIDs))
        {
            return false;
        }

        if (m_tagIDs.isEmpty())
        {
            return false;
        }

        QTimer::singleShot(0, this, [this]()
            {
                slotMoveMergeTags();
                model()->setDropIndex(QModelIndex());
            }
        );

        return true;
    }
    else if (DItemDrag::canDecode(e->mimeData()))
    {
        QList<QUrl> urls;
        QList<int>  albumIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, albumIDs, m_imageIDs))
        {
            return false;
        }

        if (urls.isEmpty() || albumIDs.isEmpty() || m_imageIDs.isEmpty())
        {
            return false;
        }

        if (m_destAlbum->id() == FaceTags::personParentTag())
        {
            return false;
        }

        // here the user dragged a "faceitem" to a "person" album

        if (m_destAlbum->hasProperty(TagPropertyName::person()))
        {
            QTimer::singleShot(0, this, [this]()
                {
                    slotConfirmPerson();
                    model()->setDropIndex(QModelIndex());
                }
            );

            return true;
        }

        QTimer::singleShot(0, this, [this]()
            {
                slotAssignTagItem();
                model()->setDropIndex(QModelIndex());
            }
        );

        return true;
    }

    return false;
}

Qt::DropAction TagDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& dropIndex)
{
    const TAlbum* const destAlbum = model()->talbumForIndex(dropIndex);

    if      (DTagListDrag::canDecode(e->mimeData()))
    {
/*
        int droppedId = 0;
*/
        QList<int> droppedId;

        if (!DTagListDrag::decode(e->mimeData(), droppedId))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "List decode error" << droppedId.isEmpty();
            return Qt::IgnoreAction;
        }

        const TAlbum* const droppedAlbum = AlbumManager::instance()->findTAlbum(droppedId.first());

        if (!droppedAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Allow dragging on empty space to move the dragged album under the root albumForIndex
        // unless the itemDrag is already at root level

        if (!destAlbum)
        {
            const Album* const palbum = droppedAlbum->parent();

            if (!palbum)
            {
                 return Qt::IgnoreAction;
            }

            if (palbum->isRoot())
            {
                return Qt::IgnoreAction;
            }
            else
            {
                return Qt::MoveAction;
            }
        }

        // Dragging an item on itself makes no sense

        if (destAlbum == droppedAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Dragging a parent on its child makes no sense

        if (droppedAlbum && droppedAlbum->isAncestorOf(destAlbum))  // cppcheck-suppress knownConditionTrueFalse
        {
            return Qt::IgnoreAction;
        }

        return Qt::MoveAction;
    }
    else if (DItemDrag::canDecode(e->mimeData()) && destAlbum && destAlbum->parent())
    {
        // Only other possibility is image items being dropped
        // And allow this only if there is a Tag to be dropped
        // on and also the Tag is not root.

        return Qt::CopyAction;
    }

    return Qt::IgnoreAction;
}

QStringList TagDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;

    mimeTypes << DTagListDrag::mimeTypes()
              << DItemDrag::mimeTypes();

    return mimeTypes;
}

QMimeData* TagDragDropHandler::createMimeData(const QList<Album*>& albums)
{

    if (albums.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot drag no tag";
        return nullptr;
    }

    QList<int> ids;

    for (const Album* const album : std::as_const(albums))
    {
        ids << album->id();
    }

    return new DTagListDrag(ids);
}

void TagDragDropHandler::slotMoveMergeTags()
{
    QMenu popMenu(m_view);
    const QAction* const gotoAction  = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")), i18n("&Move Here"));
    const QAction* const mergeAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("merge")),   i18n("M&erge Here"));
    popMenu.addSeparator();
    popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

    const QAction* const choice      = popMenu.exec(m_position);

    for (int index = 0 ; index < m_tagIDs.count() ; ++index)
    {
        TAlbum* const talbum = AlbumManager::instance()->findTAlbum(m_tagIDs.at(index));

        if (!talbum)
        {
            return;
        }

        if (m_destAlbum && (talbum == m_destAlbum))
        {
            return;
        }

        if      (choice == gotoAction)
        {
            TAlbum* newParentTag = nullptr;

            if (!m_destAlbum)
            {
                // move dragItem to the root

                newParentTag = AlbumManager::instance()->findTAlbum(0);
            }
            else
            {
                // move dragItem as child of dropItem

                newParentTag = m_destAlbum;
            }

            QString errMsg;

            if (!AlbumManager::instance()->moveTAlbum(talbum, newParentTag, errMsg))
            {
                QMessageBox::critical(m_view, qApp->applicationName(), errMsg);
            }

            if (m_view->isVisible())
            {
                m_view->setVisible(true);
            }
        }
        else if (choice == mergeAction)
        {
            if (!m_destAlbum)
            {
                return;
            }

            QString errMsg;

            if (!AlbumManager::instance()->mergeTAlbum(talbum, m_destAlbum, true, errMsg))
            {
                QMessageBox::critical(m_view, qApp->applicationName(), errMsg);
            }

            if (!m_view->isVisible())
            {
                m_view->setVisible(true);
            }
        }
    }
}

void TagDragDropHandler::slotConfirmPerson()
{
    // use dropped on album

    DigikamItemView* const dview = qobject_cast<DigikamItemView*>(m_source);

    if (!dview)
    {
        return;
    }

    QString targetName              = m_destAlbum->title();

    // get selected indexes

    QModelIndexList selectedIndexes = dview->selectionModel()->selectedIndexes();

    // get the tag ids for each of the selected faces

    QList<int> faceIds              = dview->getFaceIds(selectedIndexes);

    // create list with face names

    QStringList faceNames;

    for (const int& faceId : std::as_const(faceIds))
    {
        // check that all selected faces are the same person

        if (!faceId || faceId != faceIds.first())
        {
            return;
        }

        faceNames << FaceTags::faceNameForTag(faceId);
    }

    // here we set a new thumbnail

    if      (m_destAlbum->id() == faceIds.first())
    {
        bool set     = false;
        bool confirm = false;

        if (m_modifiers == Qt::ControlModifier)
        {
            confirm = true;
        }
        else
        {
            QMenu popMenu(m_view);
            const QAction* setAction           = nullptr;
            const QAction* const confirmAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                                   i18np("Confirm face name '%2' to Item",
                                                                         "Confirm face name '%2' to Items",
                                                                         selectedIndexes.size(),
                                                                         targetName));

            if (selectedIndexes.size() == 1)
            {
                setAction = popMenu.addAction(i18n("Set as Tag Thumbnail"));
            }

            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

            const QAction* const choice = popMenu.exec(m_position);
            confirm                     = (choice == confirmAction);
            set                         = (setAction && (choice == setAction));
        }

        if      (confirm)
        {
            int dstId = m_destAlbum->parent() ? m_destAlbum->parent()->id() : -1;
            int tagId = FaceTags::getOrCreateTagForPerson(targetName, dstId);
            dview->confirmFaces(selectedIndexes, tagId);
        }
        else if (set)
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(m_destAlbum, QString(), m_imageIDs.first(), errMsg);
        }
    }
    else
    {
        //here we move assign a new face tag to the selected faces

        if (m_destAlbum->id() == FaceTags::unconfirmedPersonTagId())
        {
            return;
        }

        bool assign = false;

        if (m_modifiers == Qt::ControlModifier)
        {
            assign = true;
        }
        else
        {
            QMenu popMenu(m_view);
            const QAction* const assignAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                                  i18np("Change face name from '%2' to '%3'",
                                                                        "Change face names from '%2' to '%3'",
                                                                        faceNames.count(),
                                                                        faceNames.first(),
                                                                        targetName));
            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

            const QAction* const choice       = popMenu.exec(m_position);
            assign                            = (choice == assignAction);
        }

        if (assign)
        {
            if      (m_destAlbum->id() == FaceTags::unknownPersonTagId())
            {
                dview->unknownFaces(selectedIndexes);
            }
            else if (m_destAlbum->id() == FaceTags::ignoredPersonTagId())
            {
                dview->ignoreFaces(selectedIndexes);
            }
            else
            {
                int dstId = m_destAlbum->parent() ? m_destAlbum->parent()->id() : -1;
                int tagId = FaceTags::getOrCreateTagForPerson(targetName, dstId);
                dview->confirmFaces(selectedIndexes, tagId);
            }
        }
    }
}

void TagDragDropHandler::slotAssignTagItem()
{
    // If a ctrl key is pressed while dropping the drag object,
    // the tag is assigned to the images without showing a
    // popup menu.

    bool hasSameTopId = false;
    bool assign       = false;
    bool set          = false;

    // Use selected tags instead of dropped on.

    QList<int> tagIdList;
    QStringList tagNames;

    AbstractAlbumTreeView* const tview = dynamic_cast<AbstractAlbumTreeView*>(m_view);

    if (!tview)
    {
        return;
    }

    if (m_imageIDs.size() == 1)
    {
        const int topId = AlbumManager::instance()->findTopId(m_destAlbum->id());
        const auto tids = ItemInfo(m_imageIDs.first()).tagIds();

        for (int tid : tids)
        {
            if (AlbumManager::instance()->findTopId(tid) == topId)
            {
                hasSameTopId = true;
                break;
            }
        }
    }

    QList<Album*> selTags = tview->selectedItems();

    for (int it = 0 ; it < selTags.count() ; ++it)
    {
        const TAlbum* const temp = dynamic_cast<TAlbum*>(selTags.at(it));

        if (temp)
        {
            tagIdList << temp->id();
            tagNames  << temp->title();
        }
    }

    // If nothing selected, use dropped on tag

    if (tagIdList.isEmpty())
    {
        tagIdList << m_destAlbum->id();
        tagNames  << m_destAlbum->title();
    }

    if (m_modifiers == Qt::ControlModifier)
    {
        assign = true;
    }
    else
    {
        QMenu popMenu(m_view);
        const QAction* setAction          = nullptr;
        const QAction* const assignAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                              i18np("Assign Tag '%2' to Items",
                                                                    "Assign Tag(s) '%2' to Items",
                                                                    tagNames.count(),
                                                                    tagNames.join(QLatin1String(", "))));

        if (hasSameTopId)
        {
            setAction = popMenu.addAction(i18n("Set as Tag Thumbnail"));
        }

        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

        const QAction* const choice = popMenu.exec(m_position);
        assign                      = (choice == assignAction);
        set                         = (setAction && (choice == setAction));
    }

    if      (assign)
    {
        Q_EMIT assignTags(m_imageIDs, tagIdList);
    }
    else if (set)
    {
        QString errMsg;
        AlbumManager::instance()->updateTAlbumIcon(m_destAlbum, QString(), m_imageIDs.first(), errMsg);
    }
}

} // namespace Digikam

#include "moc_tagdragdrop.cpp"
