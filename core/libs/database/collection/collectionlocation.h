/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location abstraction
 *
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QHash>

// Local includes

#include "digikam_export.h"
#include "coredbalbuminfo.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CollectionLocation
{

public:

    enum Status
    {
        /**
         * An invalid status. A location has this status if it is not valid,
         * and it had this status before its creation (for oldStatus information)
         */
        LocationNull,

        /**
         * The location if available. This is the most common status.
         */
        LocationAvailable,

        /**
         * The location is explicitly hidden. This gives no information if
         * the location was available were it not hidden.
         */
        LocationHidden,

        /**
         * The location is currently not available. (Harddisk unplugged, CD not in drive,
         * network fs not mounted etc.) It may become available any time.
         */
        LocationUnavailable,

        /**
         * An invalid status. A location object acquires this status if it has been deleted.
         * The object then does no longer point to an existing location.
         */
        LocationDeleted
    };

public:

    enum Type
    {
        /**
         * The location is undefined.
         * Keep values constant.
         */
        Undefined       = 0,

        /**
         * The location is located on a storage device that is built-in
         * without frequent removal: Hard-disk inside the machine.
         */
        VolumeHardWired = 1,

        /**
         * The location is located on a storage device that can be removed
         * from the local machine, and is expected to be removed.
         * USB stick, USB hard-disk, CD, DVD
         */
        VolumeRemovable = 2,

        /**
         * The location is available via a network file system.
         * The availability depends on the network connection.
         */
        Network         = 3
    };

public:

    enum CaseSensitivity
    {
        /**
         * The location has an unknown case sensitivity.
         */
        UnknownCaseSensitivity,

        /**
         * The location is case insensitive.
         */
        CaseInsensitive,

        /**
         * The location is case sensitive.
         */
        CaseSensitive
    };

public:

    CollectionLocation() = default;

    /**
     * The id uniquely identifying this collection
     */
    int                 id()                  const;

    /**
     * Return as Qt case sensitivity enum of location.
     * For unknown, it is assumed to be Qt::CaseSensitive.
     */
    Qt::CaseSensitivity asQtCaseSensitivity() const;

    /**
     * The case sensitivity of location. See above for possible values.
     */
    CaseSensitivity     caseSensitivity()     const;

    /**
     * The current status. See above for possible values.
     */
    Status              status()              const;

    /**
     * The type of location. See above for possible values.
     */
    Type                type()                const;

    /**
     * The current file system path leading to this album root.
     * Only guaranteed to be valid for location with status Available.
     */
    QString             albumRootPath()       const;

    /**
     * A user-visible, optional label.
     */
    QString             label()               const;

    bool isAvailable()                        const
    {
        return (m_status == LocationAvailable);
    }

    bool isNull()                             const
    {
        return (m_status == LocationNull);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    size_t hash()                             const

#else

    uint hash()                               const

#endif

    {
        return ::qHash(m_id);
    }

public:

    QString         identifier;

protected:

    int             m_id                = -1;
    QString         m_label;
    Status          m_status            = LocationNull;
    Type            m_type              = VolumeHardWired;
    QString         m_path;
    CaseSensitivity m_caseSensitivity   = UnknownCaseSensitivity;
};

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

inline size_t qHash(const CollectionLocation& loc)

#else

inline uint qHash(const CollectionLocation& loc)

#endif

{
    return loc.hash();
}

} // namespace Digikam
