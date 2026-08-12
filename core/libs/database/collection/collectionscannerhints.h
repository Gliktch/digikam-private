/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : Hint data containers for the collection scanner
 *
 * SPDX-FileCopyrightText: 2008      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "digikam_config.h"

// Qt includes

#include <QDateTime>

#ifdef HAVE_DBUS
#   include <QDBusArgument>
#   include "dbenginedbusutils.h"
#endif

// Local includes

#include "digikam_export.h"
#include "digikam_globals.h"

namespace Digikam
{

class ItemInfo;
class CollectionLocation;
class CollectionScannerObserver;
class ItemMetadataAdjustmentHint;

class CollectionScannerHintContainer
{
public:

    /// Note: All methods of this class must be thread-safe.

    CollectionScannerHintContainer()                                 = default;
    virtual ~CollectionScannerHintContainer()                        = default;

    virtual void recordHint(const ItemMetadataAdjustmentHint& hints) = 0;

    virtual void clear()                                             = 0;

private:

    Q_DISABLE_COPY(CollectionScannerHintContainer)
};

class DIGIKAM_DATABASE_EXPORT ItemMetadataAdjustmentHint
{
public:

    /**
     * The file's has been edited writing out information from
     * the database, i.e., the db is already guaranteed to contain
     * all changed information in the file's metadata.
     * There is no need for a full rescan, optimizations are possible.
     */

    enum AdjustmentStatus
    {
        AboutToEditMetadata,       ///< The file is about to be edited. Suspends scanning. The Finished hint must follow.
        MetadataEditingFinished,   ///< The file's metadata has been edited as described above.
        MetadataEditingAborted     ///< The file's metadata has not been edited, despite sending AboutToEditMedata
    };

public:

    ItemMetadataAdjustmentHint() = default;
    explicit ItemMetadataAdjustmentHint(qlonglong id,
                                        AdjustmentStatus status,
                                        const QDateTime& modificationDateOnDisk,
                                        qlonglong fileSize);

    qlonglong id()                                                          const;
    AdjustmentStatus adjustmentStatus()                                     const;
    QDateTime modificationDate()                                            const;
    qlonglong fileSize()                                                    const;

    bool isAboutToEdit()                                                    const
    {
        return (adjustmentStatus() == AboutToEditMetadata);
    }

    bool isEditingFinished()                                                const
    {
        return (adjustmentStatus() == MetadataEditingFinished);
    }

    bool isEditingFinishedAborted()                                         const
    {
        return (adjustmentStatus() == MetadataEditingAborted);
    }

#ifdef HAVE_DBUS

    ItemMetadataAdjustmentHint& operator<<(const QDBusArgument& argument);
    const ItemMetadataAdjustmentHint& operator>>(QDBusArgument& argument)   const;

#endif

protected:

    qlonglong         m_id                  = 0;
    AdjustmentStatus  m_status              = AboutToEditMetadata;
    QDateTime         m_modificationDate;
    qlonglong         m_fileSize            = 0;
};

} // namespace Digikam
