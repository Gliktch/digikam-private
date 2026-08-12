/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-07
 * Description : Core database abstract backend.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dbenginebackend_p.h"
#include "coredbwatch.h"

namespace Digikam
{

class CoreDbWatch;

class Q_DECL_HIDDEN CoreDbBackendPrivate : public BdEngineBackendPrivate
{
public:

    explicit CoreDbBackendPrivate(CoreDbBackend* const backend)
        : BdEngineBackendPrivate            (backend),
          imageChangesetContainer           (this),
          imageTagChangesetContainer        (this),
          collectionImageChangesetContainer (this),
          albumChangesetContainer           (this),
          tagChangesetContainer             (this),
          albumRootChangesetContainer       (this),
          searchChangesetContainer          (this)
    {
    }

public:

    CoreDbWatch* watch = nullptr;

public:

    void sendToWatch(const ImageChangeset& changeset)
    {
        watch->sendImageChange(changeset);
    }

    void sendToWatch(const ImageTagChangeset& changeset)
    {
        watch->sendImageTagChange(changeset);
    }

    void sendToWatch(const CollectionImageChangeset& changeset)
    {
        watch->sendCollectionImageChange(changeset);
    }

    void sendToWatch(const AlbumChangeset& changeset)
    {
        watch->sendAlbumChange(changeset);
    }

    void sendToWatch(const TagChangeset& changeset)
    {
        watch->sendTagChange(changeset);
    }

    void sendToWatch(const AlbumRootChangeset& changeset)
    {
        watch->sendAlbumRootChange(changeset);
    }

    void sendToWatch(const SearchChangeset& changeset)
    {
        watch->sendSearchChange(changeset);
    }

    template <class T>
    class Q_DECL_HIDDEN ChangesetContainer
    {

    public:

        explicit ChangesetContainer(CoreDbBackendPrivate* const dd)
            : d(dd)
        {
        }

        void recordChangeset(const T& changeset)
        {
            if (d->isInTransaction)
            {
                changesets << changeset;
            }
            else
            {
                d->sendToWatch(changeset);
            }
        }

        void sendOut()
        {
            for (const T& changeset : std::as_const(changesets))
            {
                d->sendToWatch(changeset);
            }

            changesets.clear();
        }

    public:

        QList<T>                    changesets;
        CoreDbBackendPrivate* const d = nullptr;
    };

public:

    ChangesetContainer<ImageChangeset>           imageChangesetContainer;
    ChangesetContainer<ImageTagChangeset>        imageTagChangesetContainer;
    ChangesetContainer<CollectionImageChangeset> collectionImageChangesetContainer;
    ChangesetContainer<AlbumChangeset>           albumChangesetContainer;
    ChangesetContainer<TagChangeset>             tagChangesetContainer;
    ChangesetContainer<AlbumRootChangeset>       albumRootChangesetContainer;
    ChangesetContainer<SearchChangeset>          searchChangesetContainer;

public:

    void transactionFinished() override
    {
        BdEngineBackendPrivate::transactionFinished();

        imageChangesetContainer.sendOut();
        imageTagChangesetContainer.sendOut();
        collectionImageChangesetContainer.sendOut();
        albumChangesetContainer.sendOut();
        tagChangesetContainer.sendOut();
        albumRootChangesetContainer.sendOut();
        searchChangesetContainer.sendOut();
    }
};

} // namespace Digikam
