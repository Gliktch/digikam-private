/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management - private containers.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "collectionmanager.h"

// Qt includes

#include <QDir>
#include <QDirIterator>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QReadWriteLock>
#include <QUrlQuery>
#include <QThread>
#include <QUuid>

// KDE includes

#include <klocalizedstring.h>

// Solid includes

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wnonportable-include-path"
#endif

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>
#include <solid/opticaldisc.h>
#include <solid/predicate.h>

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbtransaction.h"
#include "coredb.h"
#include "collectionscanner.h"
#include "collectionlocation.h"
#include "filereadwritelock.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumRootLocation : public CollectionLocation
{

public:

    AlbumRootLocation() = default;

    explicit AlbumRootLocation(const AlbumRootInfo& info)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Creating new Location " << info.specificPath << " uuid " << info.identifier;
        m_id              = info.id;
        m_type            = (Type)info.type;
        QString path      = info.specificPath;
        m_caseSensitivity = (CaseSensitivity)info.caseSensitivity;

        if (
            (path != QLatin1String("/"))    &&
            path.endsWith(QLatin1Char('/'))
           )
        {
            path.chop(1);
        }

        specificPath = path;
        identifier   = info.identifier;
        m_label      = info.label;

        m_path.clear();

        setStatus((CollectionLocation::Status)info.status);
    }

    void setStatusFromFlags()
    {
        if (hidden)
        {
            m_status = CollectionLocation::LocationHidden;
        }
        else
        {
            if (available)
            {
                m_status = CollectionLocation::LocationAvailable;
            }
            else
            {
                m_status = CollectionLocation::LocationUnavailable;
            }
        }
    }

    void setCaseSensitivity(CollectionLocation::CaseSensitivity c)
    {
        m_caseSensitivity = c;
    }

    void setStatus(CollectionLocation::Status s)
    {
        m_status = s;

        // status is exclusive, and Hidden wins
        // but really both states are independent
        // - a hidden location might or might not be available

        if      (m_status == CollectionLocation::LocationAvailable)
        {
            available = true;
            hidden    = false;
        }
        else if (m_status == CollectionLocation::LocationHidden)
        {
            available = false;
            hidden    = true;
        }
        else // Unavailable, Null, Deleted
        {
            available = false;
            hidden    = false;
        }
    }

    void setId(int id)
    {
        m_id = id;
    }

    void setAbsolutePath(const QString& path)
    {
        m_path = path;
    }

    void setType(Type type)
    {
        m_type = type;
    }

    void setLabel(const QString& label)
    {
        m_label = label;
    }

public:

    QString specificPath;
    bool    available   = false;
    bool    hidden      = false;
};

// -------------------------------------------------

class Q_DECL_HIDDEN SolidVolumeInfo
{

public:

    SolidVolumeInfo() = default;

    bool isNull() const
    {
        return path.isNull();
    }

public:

    QString udi;                        ///< Solid device UDI of the StorageAccess device
    QString path;                       ///< mount path of volume, with trailing slash
    QString uuid;                       ///< UUID as from Solid
    QString label;                      ///< volume label (think of CDs)
    bool    isRemovable     = false;    ///< may be removed
    bool    isOpticalDisc   = false;    ///< is an optical disk device as CD/DVD/BR
    bool    isMounted       = false;    ///< is mounted on File System.
};

// -------------------------------------------------

class Q_DECL_HIDDEN CollectionManager::Private
{

public:

    explicit Private(CollectionManager* const ss);

    /// hack for Solid's threading problems
    QList<SolidVolumeInfo> actuallyListVolumes();
    void                   slotTriggerUpdateVolumesList();
    QList<SolidVolumeInfo> volumesListCache;


    /// Access Solid and return a list of storage volumes
    QList<SolidVolumeInfo> listVolumes();

    /**
     *  Find from a given list (usually the result of listVolumes) the volume
     *  corresponding to the location
     */
    SolidVolumeInfo findVolumeForLocation(const AlbumRootLocation* location, const QList<SolidVolumeInfo>& volumes);

    /**
     *  Find from a given list (usually the result of listVolumes) the volume
     *  on which the file path specified by the url is located.
     */
    SolidVolumeInfo findVolumeForUrl(const QUrl& fileUrl, const QList<SolidVolumeInfo>& volumes);

    /// Create the volume identifier for the given volume info
    QString volumeIdentifier(const SolidVolumeInfo& info);

    /// Create a volume identifier based on the path only
    QString volumeIdentifier(const QString& path);

    /// Create a network share identifier based on the mountpaths
    QString networkShareIdentifier(const QStringList& paths);

    /// Return the path, if location has a path-only identifier. Else returns a null string.
    QString pathFromIdentifier(const AlbumRootLocation* location);

    /// Return the path, if location has a path-only identifier. Else returns a null string.
    QStringList networkShareMountPathsFromIdentifier(const AlbumRootLocation* location);

    /// Create an MD5 hash of the top-level entries (file names, not file content) of the given path
    static QString directoryHash(const QString& path);

    /// Check if a location for specified path exists, assuming the given list of locations was deleted
    bool checkIfExists(const QString& path, QList<CollectionLocation> assumeDeleted);

    /// Make a user presentable description, regardless of current location status
    QString technicalDescription(const AlbumRootLocation* location);

     /// Get the file UUID in the collection root if possible.
    QString getCollectionUUID(const QString& path);

    /// Check or create the file UUID in the collection root if possible.
    bool checkCollectionUUID(AlbumRootLocation* const location, const QString& path);

public:

    QReadWriteLock                lock;
    QMap<int, AlbumRootLocation*> locations;
    bool                          changingDB    = false;
    QStringList                   udisToWatch;
    bool                          watchEnabled  = false;
    CollectionManager*            s             = nullptr;
};

// -------------------------------------------------

class Q_DECL_HIDDEN ChangingDB
{

public:

    explicit ChangingDB(CollectionManager::Private* const dd)
        : d(dd)
    {
        d->changingDB = true;
    }

    ~ChangingDB()
    {
        d->changingDB = false;
    }

public:

    CollectionManager::Private* const d = nullptr;
};

} // namespace Digikam
