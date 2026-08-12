/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify tag albums in views
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagmodificationhelper.h"

// C++ includes

#include <iterator>

// Qt includes

#include <QApplication>
#include <QAction>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albumpointer.h"
#include "coredb.h"
#include "coredbtransaction.h"
#include "iteminfo.h"
#include "itemtagpair.h"
#include "metadatahub.h"
#include "scancontroller.h"
#include "statusprogressbar.h"
#include "tagsactionmngr.h"
#include "tagproperties.h"
#include "tageditdlg.h"
#include "facetags.h"
#include "identityprovider.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagModificationHelper::Private
{
public:

    Private() = default;

public:

    AlbumPointer<TAlbum> parentTag      = nullptr;
    QWidget*             dialogParent   = nullptr;
};

TagModificationHelper::TagModificationHelper(QObject* const parent, QWidget* const dialogParent)
    : QObject(parent),
      d      (new Private)
{
    d->dialogParent = dialogParent;
}

TagModificationHelper::~TagModificationHelper()
{
    delete d;
}

void TagModificationHelper::bindTag(QAction* action, TAlbum* album) const
{
    action->setData(QVariant::fromValue(AlbumPointer<TAlbum>(album)));
}

TAlbum* TagModificationHelper::boundTag(QObject* sender) const
{
    QAction* action = nullptr;

    if ((action = qobject_cast<QAction*>(sender)))
    {
        return action->data().value<AlbumPointer<TAlbum> >();
    }

    return nullptr;
}

void TagModificationHelper::bindMultipleTags(QAction* action, const QList<TAlbum*>& tags)
{
    action->setData(QVariant::fromValue(tags));
}

QList<TAlbum*> TagModificationHelper::boundMultipleTags(QObject* sender)
{
    QAction* action = nullptr;

    if ((action = qobject_cast<QAction*>(sender)))
    {
        return (action->data().value<QList<TAlbum*> >());
    }

    return QList<TAlbum*>();
}

TAlbum* TagModificationHelper::slotTagNew(TAlbum* parent, const QString& title, const QString& iconName)
{
    // ensure that there is a parent

    AlbumPointer<TAlbum> p(parent);

    if (!p)
    {
        p = AlbumManager::instance()->findTAlbum(0);

        if (!p)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Could not find root tag album";
            return nullptr;
        }
    }

    QString      editTitle    = title;
    QString      editIconName = iconName;
    QKeySequence ks;

    if (title.isEmpty())
    {
        bool doCreate = TagEditDlg::tagCreate(d->dialogParent, p, editTitle, editIconName, ks);

        if (!doCreate || !p)
        {
            return nullptr;
        }
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(p, editTitle, editIconName, ks, errMap);
    TagEditDlg::showtagsListCreationError(d->dialogParent, errMap);

    if (errMap.isEmpty() && !tList.isEmpty())
    {
        TAlbum* tag = nullptr;

        for (Album* const album : std::as_const(tList))
        {
            tag = static_cast<TAlbum*>(album);

            Q_EMIT tagCreated(tag);
        }

        return tag;
    }
    else
    {
        return nullptr;
    }
}

TAlbum* TagModificationHelper::slotTagNew()
{
    return slotTagNew(boundTag(sender()));
}

void TagModificationHelper::slotTagEdit(TAlbum* t)
{
    if (!t)
    {
        return;
    }

    AlbumPointer<TAlbum> tag(t);

    QString      title, icon;
    QKeySequence ks;

    bool doEdit = TagEditDlg::tagEdit(d->dialogParent, tag, title, icon, ks);

    if (!doEdit || !tag)
    {
        return;
    }

    if (tag->title() != title)
    {
        QString errMsg;

        if (AlbumManager::instance()->renameTAlbum(tag, title, errMsg))
        {
            // TODO: make an option to edit the full name of a face tag

            if (FaceTags::isPerson(tag->id()))
            {
                TagProperties props(tag->id());
                props.setProperty(TagPropertyName::person(),         title);
                props.setProperty(TagPropertyName::faceEngineName(), title);

                QString uuid = tag->property(TagPropertyName::faceEngineUuid());
                IdentityProvider::instance()->renameIdentity(uuid, title);
            }
        }
        else
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
        }
    }

    if (tag->icon() != icon)
    {
        QString errMsg;

        if (!AlbumManager::instance()->updateTAlbumIcon(tag, icon, 0, errMsg))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
        }
    }

    if (tag->property(TagPropertyName::tagKeyboardShortcut()) != ks.toString())
    {
        TagsActionMngr::defaultManager()->updateTagShortcut(tag->id(), ks);
    }

    Q_EMIT tagEdited(tag);
}

void TagModificationHelper::slotTagEdit()
{
    slotTagEdit(boundTag(sender()));
}

void TagModificationHelper::slotTagDelete(TAlbum* t)
{
    if (!t || t->isRoot())
    {
        return;
    }

    AlbumPointer<TAlbum> tag(t);

    // find number of subtags

    int children = 0;
    AlbumIterator iter(tag);

    while (iter.current())
    {
        ++children;
        ++iter;
    }

    // ask for deletion of children

    if (children)
    {
        int result = QMessageBox::warning(d->dialogParent, qApp->applicationName(),
                                          i18np("Tag '%2' has one subtag. "
                                                "Deleting this will also delete "
                                                "the subtag.\n"
                                                "Do you want to continue?",
                                                "Tag '%2' has %1 subtags. "
                                                "Deleting this will also delete "
                                                "the subtags.\n"
                                                "Do you want to continue?",
                                                children,
                                                tag->title()),
                                          QMessageBox::Yes | QMessageBox::Cancel);

        if ((result != QMessageBox::Yes) || !tag)
        {
            return;
        }
    }

    QString message;
    QList<qlonglong> assignedItems = CoreDbAccess().db()->getItemIDsInTag(tag->id());

    if (!assignedItems.isEmpty())
    {
        message = i18np("Tag '%2' is assigned to one item. "
                        "Do you want to continue?",
                        "Tag '%2' is assigned to %1 items. "
                        "Do you want to continue?",
                        assignedItems.count(), tag->title());
    }
    else
    {
        message = i18n("Delete '%1' tag?", tag->title());
    }

    int result = QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Delete Tag"),
                                      message,
                                      QMessageBox::Yes | QMessageBox::Cancel);

    if (result == QMessageBox::Yes && tag)
    {
        Q_EMIT aboutToDeleteTag(tag);

        QList<qlonglong> imageIds;
        QString errMsg;

        if (!AlbumManager::instance()->deleteTAlbum(tag, errMsg, &imageIds))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
        }
        else
        {
            AlbumManager::instance()->askUserForWriteChangedTAlbumToFiles(imageIds);
        }
    }
}

void TagModificationHelper::slotTagDelete()
{
    slotTagDelete(boundTag(sender()));
}

void TagModificationHelper::slotMultipleTagDel(const QList<TAlbum*>& tags)
{
    QString tagWithChildrens;
    QString tagWithoutImages;
    QString tagWithImages;
    QMultiMap<int, TAlbum*> sortedTags;

    for (TAlbum* const t : std::as_const(tags))
    {
        if (!t || t->isRoot())
        {
            continue;
        }

        AlbumPointer<TAlbum> tag(t);

        // find number of subtags

        int children = 0;
        AlbumIterator iter(tag);

        while (iter.current())
        {
            ++children;
            ++iter;
        }

        if (children)
        {
            tagWithChildrens.append(tag->title() + QLatin1Char(' '));
        }

        QList<qlonglong> assignedItems = CoreDbAccess().db()->getItemIDsInTag(tag->id());

        if (!assignedItems.isEmpty())
        {
            tagWithImages.append(tag->title() + QLatin1Char(' '));
        }
        else
        {
            tagWithoutImages.append(tag->title() + QLatin1Char(' '));
        }

        /**
         * Tags must be deleted from children to parents, if we don't want
         * to step on invalid index. Use QMultiMap to order them by distance
         * to root tag
         */

        const Album* parent = t;
        int depth           = 0;

        while (!parent->isRoot())
        {
            parent = parent->parent();
            depth++;
        }

        sortedTags.insert(depth, tag);
    }

    // ask for deletion of children

    if (!tagWithChildrens.isEmpty())
    {
        int result = QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                                          i18n("Tags '%1' have one or more subtags. "
                                               "Deleting them will also delete "
                                               "the subtags.\n"
                                               "Do you want to continue?",
                                               tagWithChildrens),
                                          QMessageBox::Yes | QMessageBox::Cancel);

        if (result != QMessageBox::Yes)
        {
            return;
        }
    }

    QString message;

    if (!tagWithImages.isEmpty())
    {
        message = i18n("Tags '%1' are assigned to one or more items. "
                        "Do you want to continue?",
                        tagWithImages);
    }
    else
    {
        message = i18n("Delete '%1' tag(s)?", tagWithoutImages);
    }

    int result = QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Delete Tag"),
                                      message,
                                      QMessageBox::Yes | QMessageBox::Cancel);

    if (result == QMessageBox::Yes)
    {
        const QList<TAlbum*>& sortedTagsList = sortedTags.values();
        QList<TAlbum*>::const_reverse_iterator it;
        QList<qlonglong> imageIds;

        /**
         * QMultimap doesn't provide reverse iterator, use QList.
         */
        for (it = sortedTagsList.crbegin() ; it != sortedTagsList.crend() ; ++it)
        {
            Q_EMIT aboutToDeleteTag(*it);

            QString errMsg;

            if (!AlbumManager::instance()->deleteTAlbum(*it, errMsg, &imageIds))
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
            }
        }

        AlbumManager::instance()->askUserForWriteChangedTAlbumToFiles(imageIds);
    }
}

void TagModificationHelper::slotMultipleTagDel()
{
    QList<TAlbum*> lst = boundMultipleTags(sender());

    qCDebug(DIGIKAM_GENERAL_LOG) << lst.size();

    slotMultipleTagDel(lst);
}

void TagModificationHelper::slotFaceTagDelete(TAlbum* t)
{
    QList<TAlbum*> tag;
    tag.append(t);
    slotMultipleFaceTagDel(tag);
}

void TagModificationHelper::slotFaceTagDelete()
{
    slotFaceTagDelete(boundTag(sender()));
}

void TagModificationHelper::slotMultipleFaceTagDel(const QList<TAlbum*>& tags)
{
    QString tagsWithChildren;
    QString tagsWithImages;

    // We use a set here since else one tag could occur more than once
    // which could lead to undefined behaviour.

    QList<TAlbum*> allPersonTagsToDelete;
    int tagsWithChildrenCount = 0;
    QList<qlonglong> allAssignedItems;
    int tagsWithImagesCount   = 0;

    for (TAlbum* const selectedTag : std::as_const(tags))
    {
        if (
            !selectedTag                                   ||
            selectedTag->isRoot()                          ||
            FaceTags::isSystemPersonTagId(selectedTag->id())
           )
        {
            continue;
        }

        // find tags and subtags with person property

        QList<TAlbum*> personTagsToDelete = getFaceTags(selectedTag);

        // If there is more than one person tag in the list,
        // the tag to remove has at least one sub tag that is a face tag.
        // Thus, we have to warn.
        //
        // If there is only one face tag, it is either the original tag,
        // or it is the only sub tag of the original tag.
        // Behave, like the face tag itself was selected.

        if (personTagsToDelete.size() > 1)
        {
            if (tagsWithChildrenCount > 0)
            {
                tagsWithChildren.append(QLatin1String(" , "));
            }

            tagsWithChildren.append(selectedTag->title());
            ++tagsWithChildrenCount;
        }

        // Get the assigned faces for all person tags to delete

        for (TAlbum* const tAlbum : std::as_const(personTagsToDelete))         // cppcheck-suppress constVariablePointer
        {
            // If the global set does not yet contain the tag

            if (!allPersonTagsToDelete.contains(tAlbum))
            {
                QList<qlonglong> assignedItems = CoreDbAccess().db()->getImagesWithImageTagProperty(
                    tAlbum->id(), ImageTagPropertyName::tagRegion());

                QList<qlonglong> autodetected  = CoreDbAccess().db()->getImagesWithImageTagProperty(
                    tAlbum->id(), ImageTagPropertyName::autodetectedFace());

                for (const qlonglong& id1 : std::as_const(autodetected))
                {
                    if (!assignedItems.contains(id1))
                    {
                        assignedItems << id1;
                    }
                }

                if (!assignedItems.isEmpty())
                {
                    // Add the items to the global set for potential untagging

                    for (const qlonglong& id2 : std::as_const(assignedItems))
                    {
                        if (!allAssignedItems.contains(id2))
                        {
                            allAssignedItems << id2;
                        }
                    }

                    if (tagsWithImagesCount > 0)
                    {
                        tagsWithImages.append(QLatin1String(" , "));
                    }

                    tagsWithImages.append(tAlbum->title());
                    ++tagsWithImagesCount;
                }
            }
        }

        // Add the found tags to the global set.

        for (TAlbum* const album : std::as_const(personTagsToDelete))            // cppcheck-suppress constVariablePointer
        {
            if (!allPersonTagsToDelete.contains(album))
            {
                allPersonTagsToDelete << album;
            }
        }
    }

    if (allPersonTagsToDelete.isEmpty() && allAssignedItems.isEmpty())
    {
        return;
    }

    // ask for deletion of children

    if (tagsWithChildrenCount)
    {
        QString message = i18np("Face tag '%2' has at least one face tag child. "
                                "Removing it will also remove the children.\n"
                                "Do you want to continue?",
                                "Face tags '%2' have at least one face tag child. "
                                "Removing them will also remove the children.\n"
                                "Do you want to continue?",
                                tagsWithChildrenCount, tagsWithChildren);

        bool removeChildren = (QMessageBox::Yes == QMessageBox::warning(qApp->activeWindow(),
                                                       qApp->applicationName(), message,
                                                       QMessageBox::Yes | QMessageBox::Cancel));

        if (!removeChildren)
        {
            return;
        }
    }

    QString message;

    if (!allAssignedItems.isEmpty())
    {
        message = i18np("Face tag '%2' is assigned to at least one item. "
                        "Do you want to continue?",
                        "Face tags '%2' are assigned to at least one item. "
                        "Do you want to continue?",
                        tagsWithImagesCount, tagsWithImages);
    }
    else
    {
        message = i18np("Remove face tag?", "Remove face tags?", tags.size());
    }

    bool removeFaceTag = (QMessageBox::Yes == QMessageBox::warning(qApp->activeWindow(),
                                                  qApp->applicationName(), message,
                                                  QMessageBox::Yes | QMessageBox::Cancel));

    if (removeFaceTag)
    {
        // Now we ask the user if we should also remove the tags from the images.

        QString msg = i18np("Remove the tag corresponding to this face tag from the images?",
                            "Remove the %1 tags corresponding to this face tags from the images?",
                            allPersonTagsToDelete.size());

        bool removeTagFromImages = (QMessageBox::Yes == QMessageBox::warning(qApp->activeWindow(),
                                                            qApp->applicationName(), msg,
                                                            QMessageBox::Yes | QMessageBox::No));

        // remove the face region from images and unassign the tag if wished

        for (const qlonglong& imageId : std::as_const(allAssignedItems))
        {
            for (const TAlbum* const tagToRemove : std::as_const(allPersonTagsToDelete))
            {
                ItemTagPair imageTagAssociation(imageId, tagToRemove->id());

                if (imageTagAssociation.isAssigned())
                {
                    imageTagAssociation.removeProperties(ImageTagPropertyName::autodetectedFace());
                    imageTagAssociation.removeProperties(ImageTagPropertyName::tagRegion());

                    if (removeTagFromImages)
                    {
                        imageTagAssociation.unAssignTag();

                        // Load the current metadata and sync the tags

                        ItemInfo info(imageId);

                        if (!info.isNull())
                        {
                            MetadataHub hub;
                            hub.load(info);

                            ScanController::FileMetadataWrite writeScope(info);
                            writeScope.changed(hub.writeToMetadata(info, MetadataHub::WRITE_TAGS, true));
                        }
                    }
                }
            }
        }

        for (TAlbum* const tAlbum : std::as_const(allPersonTagsToDelete))
        {
            TagProperties props(tAlbum->id());

            // Delete TagPropertyName::person() and TagPropertyName::faceEngineName()
            // fetch the UUID to delete the identity from facesdb

            props.removeProperties(TagPropertyName::person());
            props.removeProperties(TagPropertyName::faceEngineName());
            QString uuid = props.value(TagPropertyName::faceEngineUuid());

            qCDebug(DIGIKAM_GENERAL_LOG) << "Remove person tag properties for tag "
                                         << tAlbum->title() << " with uuid " << uuid;

            if (!uuid.isEmpty())
            {
                // Delete the UUID

                props.removeProperties(TagPropertyName::faceEngineUuid());

                // delete the faces db identity with this uuid.

                IdentityProvider* idProvider = IdentityProvider::instance();
                Identity id = idProvider->findIdentity(QLatin1String("uuid"), uuid);
                idProvider->deleteIdentity(id);

                // FaceDbAccess().db()->deleteIdentity(uuid);
            }

            // reset tag icon

            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(tAlbum, tAlbum->standardIconName(), 0, errMsg);
        }
    }
}

void TagModificationHelper::slotMultipleFaceTagDel()
{
    QList<TAlbum*> lst = boundMultipleTags(sender());

    qCDebug(DIGIKAM_GENERAL_LOG) << lst.size();

    slotMultipleFaceTagDel(lst);
}

void TagModificationHelper::slotTagToFaceTag(TAlbum* tAlbum)
{
    if (!tAlbum)
    {
        return;
    }

    if (!FaceTags::isPerson(tAlbum->id()))
    {
        FaceTags::ensureIsPerson(tAlbum->id());

        // reset tag icon

        QString errMsg;
        AlbumManager::instance()->updateTAlbumIcon(tAlbum, tAlbum->standardIconName(), 0, errMsg);
    }
}

void TagModificationHelper::slotTagToFaceTag()
{
    slotTagToFaceTag(boundTag(sender()));
}

void TagModificationHelper::slotMultipleTagsToFaceTags(const QList<TAlbum*>& tags)
{
    for (TAlbum* const selectedTag : std::as_const(tags))
    {
        slotTagToFaceTag(selectedTag);
    }
}

void TagModificationHelper::slotMultipleTagsToFaceTags()
{
    QList<TAlbum*> lst = boundMultipleTags(sender());

    qCDebug(DIGIKAM_GENERAL_LOG) << lst.size();

    slotMultipleTagsToFaceTags(lst);
}

QList<TAlbum*> TagModificationHelper::getFaceTags(TAlbum* rootTag)
{
    if (!rootTag)
    {
        return QList<TAlbum*>();
    }

    QList<TAlbum*> tags;
    tags.append(rootTag);

    return getFaceTags(tags).values();
}

QSet<TAlbum*> TagModificationHelper::getFaceTags(const QList<TAlbum*>& tags)
{
    QSet<TAlbum*> faceTags;

    for (TAlbum* const tAlbum : std::as_const(tags))
    {
        if (FaceTags::isPerson(tAlbum->id()))
        {
            faceTags.insert(tAlbum);
        }

        AlbumPointer<TAlbum> tag(tAlbum);
        AlbumIterator iter(tag);

        // Get all child tags which have the person property.

        while (iter.current())
        {
            Album* const album    = iter.current();
            TAlbum* const tAlbum2 = dynamic_cast<TAlbum*>(album);

            // Make sure that no nullp pointer dereference is done.
            // though while(iter.current()) already tests for the current
            // album being true, i.e. > 0 , i.e. non-null

            if (tAlbum2 && FaceTags::isPerson(tAlbum2->id()))
            {
                faceTags.insert(tAlbum2);
            }

            ++iter;
        }
    }

    return faceTags;
}

} // namespace Digikam

#include "moc_tagmodificationhelper.cpp"
