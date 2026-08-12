/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Column cpp dialog
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// KDE includes

#include <QDialog>

// Local includes

#include "tableview_columnfactory.h"
#include "tableview_shared.h"

class QMenu;
class QContextMenuEvent;

namespace Digikam
{

class TableViewConfigurationDialog : public QDialog
{
    Q_OBJECT

public:

    explicit TableViewConfigurationDialog(TableViewShared* const sharedObject,
                                          const int columnIndex,
                                          QWidget* const parentWidget);
    ~TableViewConfigurationDialog()                          override;

    TableViewColumnConfiguration getNewConfiguration() const;

private:

    class Private;

    const QScopedPointer<Private> d;
    TableViewShared* const        s = nullptr;
};

} // namespace Digikam
