/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-05
 * Description : a widget to find missing binaries.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QTreeWidget>

// Local includes

#include "digikam_export.h"
#include "dbinaryiface.h"

namespace Digikam
{

/**
 * This class has nothing to do with a binary search, it is a widget to search for binaries.
 */
class DIGIKAM_EXPORT DBinarySearch : public QTreeWidget
{
    Q_OBJECT

public:

    enum ColumnType
    {
        Status = 0,
        Binary,
        Version,
        Button,
        Link
    };

public:

    explicit DBinarySearch(QWidget* const parent);
    ~DBinarySearch() override;

    void addBinary(DBinaryIface& binary);
    void addDirectory(const QString& dir);
    bool allBinariesFound();

public Q_SLOTS:

    void slotAreBinariesFound();

Q_SIGNALS:

    void signalBinariesFound(bool);
    void signalAddDirectory(const QString& dir);
    void signalAddPossibleDirectory(const QString& dir);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
