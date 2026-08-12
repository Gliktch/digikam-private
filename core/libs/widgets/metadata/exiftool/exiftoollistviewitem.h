/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool metadata list view item.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QString>

namespace Digikam
{

class ExifToolListViewGroup;

class ExifToolListViewItem : public QTreeWidgetItem
{
public:

    ExifToolListViewItem(ExifToolListViewGroup* const parent,
                         const QString& key,
                         const QString& value,
                         const QString& desc);
    ExifToolListViewItem(ExifToolListViewGroup* const parent,
                         const QString& key);

    ~ExifToolListViewItem() override;

    QString getKey()         const;
    QString getTitle()       const;
    QString getValue()       const;
    QString getFullValue()   const;
    QString getDescription() const;

private:

    Q_DISABLE_COPY(ExifToolListViewItem)

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
