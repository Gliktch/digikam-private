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

#include "showfotofolderviewbookmarkitem.h"

// Local include

#include "digikam_debug.h"

namespace ShowFoto
{

ShowfotoFolderViewBookmarkItem::ShowfotoFolderViewBookmarkItem(QTreeWidgetItem* const parent)
    : QTreeWidgetItem(parent)
{
    setDisabled(false);
    setSelected(false);
}

void ShowfotoFolderViewBookmarkItem::setPath(const QString& path)
{
    m_path = path;
}

QString ShowfotoFolderViewBookmarkItem::path() const
{
    return m_path;
}

} // namespace ShowFoto
