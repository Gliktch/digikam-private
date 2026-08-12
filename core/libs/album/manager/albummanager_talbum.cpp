/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Tag Album helpers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummanager_p.h"

namespace Digikam
{

void AlbumManager::scanTAlbums()
{
    d->scanTAlbumsTimer->stop();

    // list TAlbums directly from the db
    // first insert all the current TAlbums into a map for quick lookup

    typedef QMap<int, TAlbum*> TagMap;
    TagMap                     tmap;

    tmap.insert(0, d->rootTAlbum);

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const t = (TAlbum*)(*it);     // cppcheck-suppress constVariablePointer
        tmap.insert(t->id(), t);
        ++it;
    }

    // Retrieve the list of tags from the database

    TagInfo::List tList = CoreDbAccess().db()->scanTags();

    // sort the list. needed because we want the tags can be read in any order,
    // but we want to make sure that we are ensure to find the parent TAlbum
    // for a new TAlbum

    {
        QHash<int, TAlbum*> tagHash;

        // insert items into a dict for quick lookup

        for (TagInfo::List::const_iterator iter = tList.constBegin() ; iter != tList.constEnd() ; ++iter)
        {
            TagInfo info        = *iter;
            TAlbum* const album = new TAlbum(info.name, info.id);
            album->m_icon       = info.icon;
            album->m_iconId     = info.iconId;
            album->m_pid        = info.pid;
            tagHash.insert(info.id, album);
        }

        tList.clear();

        // also add root tag

        TAlbum* const rootTag = new TAlbum(QLatin1String("root"), 0, true);
        tagHash.insert(0, rootTag);

        // build tree

        for (QHash<int, TAlbum*>::const_iterator iter = tagHash.constBegin() ; iter != tagHash.constEnd() ; ++iter)
        {
            TAlbum* album = *iter;

            if (album->m_id == 0)
            {
                continue;
            }

            TAlbum* const parent = tagHash.value(album->m_pid);

            if (parent)
            {
                album->setParent(parent);
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find parent tag for tag "
                           << album->m_title
                           << " with pid "
                           << album->m_pid;
            }
        }

        tagHash.clear();

        // now insert the items into the list. becomes sorted

        AlbumIterator it2(rootTag);

        while (it2.current())
        {
            const TAlbum* const album = dynamic_cast<TAlbum*>(it2.current());

            if (album)
            {
                TagInfo info;
                info.id     = album->m_id;
                info.pid    = album->m_pid;
                info.name   = album->m_title;
                info.icon   = album->m_icon;
                info.iconId = album->m_iconId;
                tList.append(info);
            }

            ++it2;
        }

        // this will also delete all child albums

        delete rootTag;
    }

    for (TagInfo::List::const_iterator it3 = tList.constBegin() ; it3 != tList.constEnd() ; ++it3)
    {
        TagInfo info = *it3;

        // check if we have already added this tag

        if (tmap.contains(info.id))
        {
            continue;
        }

        // Its a new album. Find the parent of the album

        TagMap::const_iterator iter = tmap.constFind(info.pid);

        if (iter == tmap.constEnd())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find parent tag for tag "
                                           << info.name
                                           << " with pid "
                                           << info.pid;
            continue;
        }

        TAlbum* const parent = iter.value();

        // Create the new TAlbum

        TAlbum* const album = new TAlbum(info.name, info.id, false);
        album->m_icon       = info.icon;
        album->m_iconId     = info.iconId;
        insertTAlbum(album, parent);

        // also insert it in the map we are doing lookup of parent tags

        tmap.insert(info.id, album);
    }

    if (!tList.isEmpty())
    {
        Q_EMIT signalAlbumsUpdated(Album::TAG);
    }

    getTagItemsCount();
}

void AlbumManager::getTagItemsCount()
{
    d->tagItemCountTimer->stop();

    if (!ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        personItemsCount();
        return;
    }

    tagItemsCount();
    personItemsCount();
}

void AlbumManager::tagItemsCount()
{
    if (d->tagListJob)
    {
        disconnect(d->tagListJob, nullptr, this, nullptr);

        d->tagListJob->cancel();
        d->tagListJob = nullptr;
    }

    TagsDBJobInfo jInfo;
    jInfo.setFoldersJob();

    d->tagListJob = DBJobsManager::instance()->startTagsJobThread(jInfo);

    connect(d->tagListJob, SIGNAL(signalFinished()),
            this, SLOT(slotTagsJobResult()));

    connect(d->tagListJob, SIGNAL(foldersData(QHash<int,int>)),
            this, SLOT(slotTagsJobData(QHash<int,int>)));
}

AlbumList AlbumManager::allTAlbums() const
{
    AlbumList list;

    if (d->rootTAlbum)
    {
        list.append(d->rootTAlbum);
    }

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

/**
 * This method is not yet used
 */
QList<TAlbum*> AlbumManager::currentTAlbums() const
{
    QList<TAlbum*> talbums;
    QList<Album*>::iterator it;

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        TAlbum* const temp = dynamic_cast<TAlbum*>(*it);

        if (temp)
        {
            talbums.append(temp);
        }
    }

    return talbums;
}

TAlbum* AlbumManager::findTAlbum(int id) const
{
    if (!d->rootTAlbum)
    {
        return nullptr;
    }

    int gid = d->rootTAlbum->globalID() + id;

    return static_cast<TAlbum*>((d->allAlbumsIdHash.value(gid)));
}

TAlbum* AlbumManager::findTAlbum(const QString& tagPath) const
{
    // handle gracefully with or without leading slash

    bool withLeadingSlash = tagPath.startsWith(QLatin1Char('/'));
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const talbum = static_cast<TAlbum*>(*it);

        if (talbum->tagPath(withLeadingSlash) == tagPath)
        {
            return talbum;
        }

        ++it;
    }

    return nullptr;
}

TAlbum* AlbumManager::createTAlbum(TAlbum* parent, const QString& name,
                                   const QString& iconkde, QString& errMsg)
{
    if (!parent)
    {
        errMsg = i18n("No parent found for tag");
        return nullptr;
    }

    // sanity checks

    if (name.isEmpty())
    {
        errMsg = i18n("Tag name cannot be empty");
        return nullptr;
    }

    if (name.contains(QLatin1Char('/')))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return nullptr;
    }

    // first check if we have another album with the same name

    if (hasDirectChildAlbumWithTitle(parent, name))
    {
        errMsg = i18n("Tag name already exists");
        return nullptr;
    }

    ChangingDB changing(d);
    int id = CoreDbAccess().db()->addTag(parent->id(), name, iconkde, 0);

    if (id == -1)
    {
        errMsg = i18n("Failed to add tag to database");
        return nullptr;
    }

    TAlbum* const album = new TAlbum(name, id, false);      // cppcheck-suppress constVariablePointer
    album->m_icon       = iconkde;

    insertTAlbum(album, parent);

    const TAlbum* const personParentTag = findTAlbum(FaceTags::personParentTag());

    if (personParentTag && personParentTag->isAncestorOf(album))
    {
        FaceTags::ensureIsPerson(album->id());
    }

    Q_EMIT signalAlbumsUpdated(Album::TAG);

    return album;
}

AlbumList AlbumManager::findOrCreateTAlbums(const QStringList& _tagPaths)
{
    // find tag ids for tag paths in list, create if they don't exist

    QList<int> tagIDs = TagsCache::instance()->getOrCreateTags(_tagPaths);

    // create TAlbum objects for the newly created tags

    scanTAlbums();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        resultList.append(findTAlbum(*it));
    }

    return resultList;
}

bool AlbumManager::deleteTAlbum(TAlbum* album, QString& errMsg, QList<qlonglong>* imageIds)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot delete root tag");
        return false;
    }

    if (
        FaceTags::isSystemPersonTagId(album->id()) ||
        (FaceTags::personParentTag() == album->id())
       )
    {
        errMsg = i18n("Cannot delete required face tag");
        return false;
    }

    QList<int> toBeDeletedTagIds;
    toBeDeletedTagIds << album->id();

    const Album* subAlbum = nullptr;
    AlbumIterator it(album);

    while ((subAlbum = it.current()) != nullptr)
    {
        toBeDeletedTagIds << subAlbum->id();
        ++it;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (imageIds)
    {
        // Recursively from all tags to delete.

        const auto ids = CoreDbAccess().db()->getItemIDsInTag(album->id(), true);

        for (const qlonglong& id : ids)
        {
            if (!imageIds->contains(id))
            {
                *imageIds << id;
            }
        }
    }

    for (int tagId : std::as_const(toBeDeletedTagIds))
    {
        if (FaceTags::isPerson(tagId))
        {
            const QList<qlonglong>& imgListIds = CoreDbAccess().db()->getItemIDsInTag(tagId);

            for (const qlonglong& imageId : std::as_const(imgListIds))
            {
                const QList<FaceTagsIface>& facesList = FaceTagsEditor().databaseFaces(imageId);

                for (const FaceTagsIface& face : std::as_const(facesList))
                {
                    if (face.tagId() == tagId)
                    {
                        FaceTagsIface unknownFace(FaceTagsIface::UnknownName, imageId,
                                                  FaceTags::unknownPersonTagId(), face.region());

                        FaceTagsEditor().removeFace(face);
                        FaceTagsEditor().addManually(unknownFace);
                    }
                }
            }
        }
    }

    QApplication::restoreOverrideCursor();

    {
        CoreDbAccess access;
        ChangingDB changing(d);

        for (int tagId : std::as_const(toBeDeletedTagIds))
        {
            access.db()->deleteTag(tagId);
        }
    }

    removeTAlbum(album);

    Q_EMIT signalAlbumsUpdated(Album::TAG);

    return true;
}

bool AlbumManager::renameTAlbum(TAlbum* album,
                                const QString& name,
                                QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }

    if (name.contains(QLatin1Char('/')))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return false;
    }

    // first check if we have another sibling with the same name

    if (hasDirectChildAlbumWithTitle(album->m_parent, name))
    {
        errMsg = i18n("Another tag with the same name already exists.\n"
                      "Please choose another name.");
        return false;
    }

    ChangingDB changing(d);
    CoreDbAccess().db()->setTagName(album->id(), name);
    album->setTitle(name);

    Q_EMIT signalAlbumRenamed(album);

    askUserForWriteChangedTAlbumToFiles(album);

    return true;
}

bool AlbumManager::moveTAlbum(TAlbum* album, TAlbum* newParent, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (!newParent)
    {
        errMsg = i18n("Attempt to move TAlbum to nowhere");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot move root tag");
        return false;
    }

    if (hasDirectChildAlbumWithTitle(newParent, album->title()))
    {
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                 qApp->applicationName(),
                 i18n("Another tag with the same name already exists.\n"
                      "Do you want to merge the tags?"),
                 QMessageBox::Yes | QMessageBox::No,
                 qApp->activeWindow());

        int result = msgBox->exec();
        delete msgBox;

        if (result == QMessageBox::Yes)
        {
            QString destPath(newParent->tagPath());

            if (destPath != QLatin1String("/"))
            {
                destPath += QLatin1Char('/');
            }

            TAlbum* const destAlbum = findTAlbum(destPath +
                                                 album->title());

            return mergeTAlbum(album, destAlbum, false, errMsg);
        }
        else
        {
            return true;
        }
    }

    d->currentlyMovingAlbum = album;

    Q_EMIT signalAlbumAboutToBeMoved(album);
    Q_EMIT signalAlbumAboutToBeDeleted(album);

    if (album->parent())
    {
        album->parent()->removeChild(album);
    }

    album->setParent(nullptr);

    Q_EMIT signalAlbumDeleted(album);
    Q_EMIT signalAlbumHasBeenDeleted(album);
    Q_EMIT signalAlbumAboutToBeAdded(album, newParent, newParent->lastChild());

    ChangingDB changing(d);
    CoreDbAccess().db()->setTagParentID(album->id(), newParent->id());
    album->setParent(newParent);

    Q_EMIT signalAlbumAdded(album);
    Q_EMIT signalAlbumMoved(album);
    Q_EMIT signalAlbumsUpdated(Album::TAG);

    d->currentlyMovingAlbum             = nullptr;
    const TAlbum* const personParentTag = findTAlbum(FaceTags::personParentTag());

    if (personParentTag && personParentTag->isAncestorOf(album))
    {
        FaceTags::ensureIsPerson(album->id());
    }

    askUserForWriteChangedTAlbumToFiles(album);

    return true;
}

bool AlbumManager::mergeTAlbum(TAlbum* album, TAlbum* destAlbum, bool dialog, QString& errMsg)
{
    if (!album || !destAlbum)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if ((album == d->rootTAlbum) || (destAlbum == d->rootTAlbum))
    {
        errMsg = i18n("Cannot merge root tag");
        return false;
    }

    if (FaceTags::isSystemPersonTagId(album->id()))
    {
        errMsg = i18n("Cannot merge required face tag");
        return false;
    }

    if (album->firstChild())
    {
        errMsg = i18n("Only a tag without children can be merged!");
        return false;
    }

    if (dialog)
    {
        int result = d->askMergeMessageBoxResult;

        if (result == -1)
        {
            QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                     qApp->applicationName(),
                     i18n("Do you want to merge tag '%1' into tag '%2'?",
                          album->title(), destAlbum->title()),
                     QMessageBox::Yes | QMessageBox::No,
                     qApp->activeWindow());
            QCheckBox* const chkBox      = new QCheckBox(i18n("Do not ask again at this session"), msgBox);
            msgBox->setCheckBox(chkBox);

            result = msgBox->exec();

            if (chkBox->isChecked())
            {
                d->askMergeMessageBoxResult = result;
            }

            delete msgBox;
        }

        if (result == QMessageBox::No)
        {
            return true;
        }
    }

    int oldId   = album->id();
    int mergeId = destAlbum->id();

    if (oldId == mergeId)
    {
        return true;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<qlonglong> imageIds = CoreDbAccess().db()->getItemIDsInTag(oldId);

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        for (const qlonglong& imageId : std::as_const(imageIds))
        {
            QList<FaceTagsIface> facesList = FaceTagsEditor().databaseFaces(imageId);
            bool foundFace                 = false;

            for (const FaceTagsIface& face : std::as_const(facesList))
            {
                if (face.tagId() == oldId)
                {
                    foundFace = true;
                    FaceTagsEditor().removeFace(face);
                    FaceTagsEditor().add(imageId, mergeId, face.region(), false);
                }
            }

            if (!foundFace)
            {
                ItemInfo info(imageId);
                info.removeTag(oldId);
                info.setTag(mergeId);
                group.allowLift();
            }
        }
    }

    QApplication::restoreOverrideCursor();

    if (!deleteTAlbum(album, errMsg))
    {
        return false;
    }

    askUserForWriteChangedTAlbumToFiles(imageIds);

    return true;
}

bool AlbumManager::updateTAlbumIcon(TAlbum* album, const QString& iconKDE,
                                    qlonglong iconID, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such tag");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }

    {
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->setTagIcon(album->id(), iconKDE, iconID);
        album->m_icon   = iconKDE;
        album->m_iconId = iconID;
    }

    Q_EMIT signalAlbumIconChanged(album);

    return true;
}

AlbumList AlbumManager::getRecentlyAssignedTags(bool includeInternal) const
{
    QList<int> tagIDs = CoreDbAccess().db()->getRecentlyAssignedTags();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        TAlbum* const album = findTAlbum(*it);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            resultList.append(album);
        }
    }

    return resultList;
}

QStringList AlbumManager::tagPaths(const QList<int>& tagIDs,
                                   bool leadingSlash,
                                   bool includeInternal) const
{
    QStringList tagPaths;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        const TAlbum* album = findTAlbum(*it);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            tagPaths.append(album->tagPath(leadingSlash));
        }
    }

    return tagPaths;
}

QStringList AlbumManager::tagNames(const QList<int>& tagIDs, bool includeInternal) const
{
    QStringList tagNames;

    for (int id : std::as_const(tagIDs))
    {
        const TAlbum* const album = findTAlbum(id);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            tagNames << album->title();
        }
    }

    return tagNames;
}

QHash<int, QString> AlbumManager::tagPaths(bool leadingSlash, bool includeInternal) const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        const TAlbum* const t = (TAlbum*)(*it);

        if (includeInternal || !t->isInternalTag())
        {
            hash.insert(t->id(), t->tagPath(leadingSlash));
        }

        ++it;
    }

    return hash;
}

QHash<int, QString> AlbumManager::tagNames(bool includeInternal) const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        const TAlbum* const t = (TAlbum*)(*it);

        if (includeInternal || !t->isInternalTag())
        {
            hash.insert(t->id(), t->title());
        }

        ++it;
    }

    return hash;
}

QList<int> AlbumManager::subTags(int tagId, bool recursive) const
{
    TAlbum* const album = this->findTAlbum(tagId);

    return album->childAlbumIds(recursive);
}

int AlbumManager::findTopId(int tagId) const
{
    const TAlbum* const album = findTAlbum(tagId);

    if (!album || album->isInternalTag())
    {
        return -1;
    }

    int topId           = tagId;
    const Album* parent = album;

    while (!parent->isRoot())
    {
        topId  = parent->id();
        parent = parent->parent();
    }

    return topId;
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property)
{
    AlbumList list;

    QList<int> ids = TagsCache::instance()->tagsWithProperty(property);

    for (int id : std::as_const(ids))
    {
        TAlbum* const album = findTAlbum(id);       // cppcheck-suppress constVariablePointer

        if (album)
        {
            list << album;
        }
    }

    return list;
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property, const QString& value)
{
    AlbumList list;

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        if (static_cast<TAlbum*>(*it)->property(property) == value)
        {
            list << *it;
        }

        ++it;
    }

    return list;
}

QHash<int, int> AlbumManager::getTAlbumsCount() const
{
    return d->tAlbumsCount;
}

void AlbumManager::insertTAlbum(TAlbum* album, TAlbum* parent)
{
    if (!album)
    {
        return;
    }

    Q_EMIT signalAlbumAboutToBeAdded(album, parent, parent ? parent->lastChild() : nullptr);

    if (parent)
    {
        album->setParent(parent);
    }

    d->allAlbumsIdHash.insert(album->globalID(), album);

    Q_EMIT signalAlbumAdded(album);
}

void AlbumManager::removeTAlbum(TAlbum* album)
{
    if (!album)
    {
        return;
    }

    // remove all children of this album

    Album* child        = album->firstChild();
    TAlbum* toBeRemoved = nullptr;

    while (child)
    {
        Album* const next = child->next();
        toBeRemoved       = dynamic_cast<TAlbum*>(child);

        if (toBeRemoved)
        {
            removeTAlbum(toBeRemoved);
            toBeRemoved = nullptr;
        }

        child             = next;
    }

    Q_EMIT signalAlbumAboutToBeDeleted(album);

    d->allAlbumsIdHash.remove(album->globalID());

    if (!d->currentAlbums.isEmpty())
    {
        if (album == d->currentAlbums.first())
        {
            d->currentAlbums.clear();
            Q_EMIT signalAlbumCurrentChanged(d->currentAlbums);
        }
    }

    Q_EMIT signalAlbumDeleted(album);

    album->prepareForDeletion();

    Q_EMIT signalAlbumHasBeenDeleted(album);

    delete album;
}

void AlbumManager::slotTagsJobResult()
{
    if (!d->tagListJob)
    {
        return;
    }

    if (d->tagListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list face tags";

        // Pop-up a message about the error.

        DNotificationWrapper(QString(), d->personListJob->errorsList().first(),
                             nullptr, i18n("digiKam"));
    }

    d->tagListJob = nullptr;
}

void AlbumManager::slotTagsJobData(const QHash<int, int>& tagsStatHash)
{
    if (tagsStatHash.isEmpty())
    {
        return;
    }

    d->tAlbumsCount = tagsStatHash;

    Q_EMIT signalTAlbumsDirty(tagsStatHash);
}

void AlbumManager::slotTagChange(const TagChangeset& changeset)
{
    if (d->changingDB || !d->rootTAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case TagChangeset::Added:
        case TagChangeset::Moved:
        case TagChangeset::Deleted:
        case TagChangeset::Reparented:
        {
            if (!d->scanTAlbumsTimer->isActive())
            {
                d->scanTAlbumsTimer->start();
            }

            break;
        }

        case TagChangeset::Renamed:
        case TagChangeset::IconChanged:
        {
            /**
             * @todo what happens here?
             */
            break;
        }

        case TagChangeset::PropertiesChanged:
        {
            TAlbum* const tag = findTAlbum(changeset.tagId());

            if (tag)
            {
                Q_EMIT signalTagPropertiesChanged(tag);
            }

            break;
        }

        case TagChangeset::Unknown:
        {
            break;
        }
    }
}

void AlbumManager::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (!d->rootTAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case ImageTagChangeset::Added:
        case ImageTagChangeset::Removed:
        case ImageTagChangeset::RemovedAll:

        // Add properties changed.
        // Reason: in people sidebar, the images are not
        // connected with the ImageTag table but by
        // ImageTagProperties entries.
        // Thus, the count of entries in face tags are not
        // updated. This adoption should fix the problem.

        case ImageTagChangeset::PropertiesChanged:
        {
            const auto ids = changeset.tags();

            for (int id : ids)
            {
                if (!d->toUpdatedFaces.contains(id))
                {
                    d->toUpdatedFaces << id;
                }
            }

            if (!d->tagItemCountTimer->isActive())
            {
                d->tagItemCountTimer->start();
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

void AlbumManager::askUserForWriteChangedTAlbumToFiles(TAlbum* const album)
{
    QList<qlonglong> imageIds = CoreDbAccess().db()->getItemIDsInTag(album->id(), true);
    askUserForWriteChangedTAlbumToFiles(imageIds);
}

void AlbumManager::askUserForWriteChangedTAlbumToFiles(const QList<qlonglong>& imageIds)
{
    const MetaEngineSettings* const settings = MetaEngineSettings::instance();

    if (
        (!settings->settings().saveTags && !settings->settings().saveFaceTags) ||
        imageIds.isEmpty()
       )
    {
        return;
    }

    if (settings->settings().useLazySync)
    {
        MetadataHubMngr::instance()->addPendingIds(imageIds);

        return;
    }

    if (imageIds.count() > 100)
    {
        int result = d->longTimeMessageBoxResult;

        if (result == -1)
        {
            QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                     qApp->applicationName(),
                     i18n("This operation can take a long time in the background.\n"
                          "Do you want to write the metadata to %1 files now?",
                          imageIds.count()),
                     QMessageBox::Yes | QMessageBox::No,
                     qApp->activeWindow());
            QCheckBox* const chkBox      = new QCheckBox(i18n("Do not ask again at this session"), msgBox);
            msgBox->setCheckBox(chkBox);

            result = msgBox->exec();

            if (chkBox->isChecked())
            {
                d->longTimeMessageBoxResult = result;
            }

            delete msgBox;
        }

        if (result != QMessageBox::Yes)
        {
            return;
        }
    }

    ItemInfoList infos(imageIds);
    MetadataSynchronizer* const tool = new MetadataSynchronizer(infos,
                                                                MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->setUseMultiCoreCPU(false);
    tool->start();
}

} // namespace Digikam
