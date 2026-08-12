/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show item preview widget.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAction>

// Local includes

#include "mediaplayerview.h"
#include "iteminfo.h"
#include "itempreviewview.h"

namespace Digikam
{

class ImageChangeset;
class ImageTagChangeset;

class ItemPreviewVideo : public MediaPlayerView
{
    Q_OBJECT

public:

    explicit ItemPreviewVideo(QWidget* const parent);
    ~ItemPreviewVideo() override;

    void setItemInfo(const ItemInfo& info, const ItemInfo& previous, const ItemInfo& next);
    void setHostWindowActions(const HostActionsMap& actions);

Q_SIGNALS:

    void signalDeleteItem();
    void signalPopupTagsView();

private Q_SLOTS:

    void slotDeleteItem();
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);
    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotContextMenu();
    void slotSetupChanged();

    void slotImageChange(const ImageChangeset& changeset);
    void slotImageTagChange(const ImageTagChangeset& changeset);

private:

    void setupOverlays();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
