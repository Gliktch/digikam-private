/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Core database image properties synchronizer
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "digikam_config.h"

// Qt includes

#include <QObject>
#include <QThread>

// Local includes

#include "digikam_export.h"
#include "coredbchangesets.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CoreDbWatch : public QObject
{
    Q_OBJECT

public:

    enum DatabaseMode
    {
        DatabaseMaster,
        DatabaseSlave
    };

public:

    CoreDbWatch();
    ~CoreDbWatch() override;

    void initializeRemote(DatabaseMode mode);
    void doAnyProcessing();
    void setDatabaseIdentifier(const QString& identifier);
    void setApplicationIdentifier(const QString& identifier);

    /// library-internal signal-trigger methods

    void sendDatabaseChanged();

    void sendImageChange(const ImageChangeset& changeset);
    void sendImageTagChange(const ImageTagChangeset& changeset);
    void sendCollectionImageChange(const CollectionImageChangeset& changeset);
    void sendAlbumChange(const AlbumChangeset& changeset);
    void sendTagChange(const TagChangeset& changeset);
    void sendAlbumRootChange(const AlbumRootChangeset& changeset);
    void sendSearchChange(const SearchChangeset& changeset);

Q_SIGNALS:

    /**
     * Retrieve the CoreDbWatch object from CoreDbAccess::databaseWatch().
     */

    /**
     * This does not describe a change of the contents of a table;
     * rather, it signals that a new database has been loaded.
     * That means all cached content has to be discarded.
     */
    void databaseChanged();

    /**
     * Notifies of changes in the database.
     * Connect to the set of signals that you are interested in.
     */
    void imageChange(const ImageChangeset& changeset);
    void imageTagChange(const ImageTagChangeset& changeset);
    void collectionImageChange(const CollectionImageChangeset& changeset);
    void albumChange(const AlbumChangeset& changeset);
    void tagChange(const TagChangeset& changeset);
    void albumRootChange(const AlbumRootChangeset& changeset);
    void searchChange(const SearchChangeset& changeset);

#ifdef HAVE_DBUS

protected Q_SLOTS:

    // NOTE: The full qualification with "Digikam::" for the changeset types in the following
    // signals and slots are required to make moc pick them up.
    // If moc does not get the namespace in its literal, DBus connections will silently break.

    /// DBus slots, for internal use
    void slotImageChangeDBus(const QString& databaseIdentifier,
                             const QString& applicationIdentifier,
                             const Digikam::ImageChangeset& changeset);
    void slotImageTagChangeDBus(const QString& databaseIdentifier,
                                const QString& applicationIdentifier,
                                const Digikam::ImageTagChangeset& changeset);
    void slotCollectionImageChangeDBus(const QString& databaseIdentifier,
                                       const QString& applicationIdentifier,
                                       const Digikam::CollectionImageChangeset& changeset);
    void slotAlbumChangeDBus(const QString& databaseIdentifier,
                             const QString& applicationIdentifier,
                             const Digikam::AlbumChangeset& changeset);
    void slotTagChangeDBus(const QString& databaseIdentifier,
                           const QString& applicationIdentifier,
                           const Digikam::TagChangeset& changeset);
    void slotAlbumRootChangeDBus(const QString& databaseIdentifier,
                                 const QString& applicationIdentifier,
                                 const Digikam::AlbumRootChangeset& changeset);
    void slotSearchChangeDBus(const QString& databaseIdentifier,
                              const QString& applicationIdentifier,
                              const Digikam::SearchChangeset& changeset);

Q_SIGNALS:

    /// DBus signals, for internal use
    void signalImageChangeDBus(const QString& databaseIdentifier,
                               const QString& applicationIdentifier,
                               const Digikam::ImageChangeset& changeset);
    void signalImageTagChangeDBus(const QString& databaseIdentifier,
                                  const QString& applicationIdentifier,
                                  const Digikam::ImageTagChangeset& changeset);
    void signalCollectionImageChangeDBus(const QString& databaseIdentifier,
                                         const QString& applicationIdentifier,
                                         const Digikam::CollectionImageChangeset& changeset);
    void signalAlbumChangeDBus(const QString& databaseIdentifier,
                               const QString& applicationIdentifier,
                               const Digikam::AlbumChangeset& changeset);
    void signalTagChangeDBus(const QString& databaseIdentifier,
                             const QString& applicationIdentifier,
                             const Digikam::TagChangeset& changeset);
    void signalAlbumRootChangeDBus(const QString& databaseIdentifier,
                                   const QString& applicationIdentifier,
                                   const Digikam::AlbumRootChangeset& changeset);
    void signalSearchChangeDBus(const QString& databaseIdentifier,
                                const QString& applicationIdentifier,
                                const Digikam::SearchChangeset& changeset);
#endif

public:

    class Private;

private:

    /// @note disabledd
    explicit CoreDbWatch(QObject*) = delete;

    Private* const d = nullptr;
};

#ifdef HAVE_DBUS

// ------------------------------------------------------------------------------------

class DBusSignalListenerThread : public QThread
{
    Q_OBJECT

public:

    explicit DBusSignalListenerThread(CoreDbWatch* const qq, CoreDbWatch::Private* const dd);
    ~DBusSignalListenerThread() override;

    void run()                  override;

private:

    CoreDbWatch*          const q = nullptr;
    CoreDbWatch::Private* const d = nullptr;
};

#endif

} // namespace Digikam
