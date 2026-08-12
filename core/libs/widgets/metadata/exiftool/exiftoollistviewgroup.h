/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool metadata list view group.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>
#include <QString>

namespace Digikam
{

class ExifToolListViewGroup : public QObject,
                              public QTreeWidgetItem
{
    Q_OBJECT

public:

    ExifToolListViewGroup(QTreeWidget* const parent,
                          const QString& group);
    ~ExifToolListViewGroup() override = default;

private:

    Q_DISABLE_COPY(ExifToolListViewGroup)
};

} // namespace Digikam
