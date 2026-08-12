/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-10-11
 * Description : item sort based on QCollator
 *
 * SPDX-FileCopyrightText: 2020 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemSortCollator : public QObject
{
    Q_OBJECT

public:

    ItemSortCollator();
    ~ItemSortCollator() override;

    /**
     * @brief Global instance of internal item sort collator.
     * All accessor methods are thread-safe.
     */
    static ItemSortCollator* instance();

    int itemCompare(const QString& a, const QString& b,
                    Qt::CaseSensitivity caseSensitive, bool natural)  const;

    int albumCompare(const QString& a, const QString& b,
                     Qt::CaseSensitivity caseSensitive, bool natural) const;

private:

    /// @note disabled
    explicit ItemSortCollator(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
