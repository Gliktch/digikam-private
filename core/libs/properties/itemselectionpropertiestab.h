/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-04-11
 * Description : A tab to display information about the current selection.
 *
 * SPDX-FileCopyrightText:      2020 by Kartik Ramesh <kartikx2000 at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QPixmap>
#include <QTreeWidgetItem>

// Local includes

#include "dlayoutbox.h"
#include "iteminfo.h"
#include "thumbnailloadthread.h"
#include "itempropertiessidebardb.h"

namespace Digikam
{

class PAlbum;
class ItemFilterModel;

class ItemSelectionPropertiesTab : public DVBox
{
    Q_OBJECT

public:

    explicit ItemSelectionPropertiesTab(ItemPropertiesSideBarDB* const parent);
    ~ItemSelectionPropertiesTab() override;

    void clear();
    void setItemFilterModel(ItemFilterModel* const model);

public Q_SLOTS:

    void slotSetSelectionCount(int count);
    void slotSetSelectionSize(const QString& str);
    void slotSetSelectionMimes(const QHash<QString, int>& mimes);
    void slotSetSelectionGroups(const ItemInfoList& groups);

    void slotSetTotalCount(int count);
    void slotSetTotalSize(const QString& str);
    void slotSetTotalMimes(const QHash<QString, int>& mimes);
    void slotSetTotalGroups(const ItemInfoList& groups);

private Q_SLOTS:

    void slotOpenSelectionPressed();
    void slotOpenAlbumPressed();

private:

    PAlbum* currentPAlbum() const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
