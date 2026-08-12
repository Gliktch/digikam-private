/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-12-23
 * Description : Core database DBus interface description
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QDBusVariant>
#include <QDBusAbstractAdaptor>

// Local includes

#include "coredbwatch.h"

// qdbuscpp2xml -S -M databasewatchadaptor.h -o org.kde.digikam.DatabaseChangesetRelay.xml

class CoreDbWatchAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.digikam.DatabaseChangesetRelay")

public:

    explicit CoreDbWatchAdaptor(Digikam::CoreDbWatch* const watch);

Q_SIGNALS:

    // These signals are the same as declared in CoreDbWatch, setAutoRelaySignals will
    // automatically connect the CoreDbWatch signals to these, which are then sent over DBus.

    // NOTE: the full qualification with "Digikam::" for the changeset types in the following
    // signals and slots are required to make moc pick them up.
    // If moc does not get the namespace in its literal, DBus connections will silently break.

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
};
