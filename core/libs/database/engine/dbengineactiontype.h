/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-20
 * Description : Database Engine container to wrap data types
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{
/**
 * The DbEngineActionType is used by the BdEngineBackend
 * to wrap another data object within an sql statement
 * and controls whether it should be used as field entry or as value
 * (prepared to an sql statement with positional binding).
 */
class DIGIKAM_EXPORT DbEngineActionType
{
public:

    DbEngineActionType();
    DbEngineActionType(const DbEngineActionType& actionType);
    ~DbEngineActionType();

    static DbEngineActionType value(const QVariant& value);
    static DbEngineActionType fieldEntry(const QVariant& actionValue);

    /**
     * Returns the wrapped object.
     */
    QVariant getActionValue();

    /**
     * Sets the wrapped object.
     */
    void setActionValue(const QVariant& actionValue);

    /**
     * Returns true, if the entry is an value element.
     * Returns false, if the entry should be used as field entry.
     */
    bool isValue() const;

    /**
     * Sets the DBAction mode:
     * true, if the entry is an value element.
     * false, if the entry should be used as field entry.
     */
    void setValue(bool isValue);

private:

    /// @note disabled
    DbEngineActionType& operator=(const DbEngineActionType&) = delete;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::DbEngineActionType)
