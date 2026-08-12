/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view bookmarks item
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace ShowFoto
{

class ShowfotoFolderViewBookmarkItem : public QTreeWidgetItem
{

public:

    explicit ShowfotoFolderViewBookmarkItem(QTreeWidgetItem* const parent);
    ~ShowfotoFolderViewBookmarkItem() override = default;

    void setPath(const QString&);
    QString path()               const;

private:

    QString m_path;

    Q_DISABLE_COPY(ShowfotoFolderViewBookmarkItem)
};

} // namespace ShowFoto
