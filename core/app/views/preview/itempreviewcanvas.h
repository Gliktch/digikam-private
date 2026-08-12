/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded item-view to show the canvas preview widget.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_config.h"
#include "dimgpreviewitem.h"
#include "itempreviewview.h"
#include "iteminfo.h"

class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneHoverEvent;

namespace Digikam
{

class FaceGroup;

class ItemPreviewCanvas : public DImgPreviewItem
{
    Q_OBJECT

public:

    explicit ItemPreviewCanvas(ItemPreviewView* const view);
    ~ItemPreviewCanvas()                                      override;

    void setItemInfo(const ItemInfo& info);
    ItemInfo imageInfo()                                const;

    void setFaceGroup(FaceGroup* const group);

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent* e)         override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e)         override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* e)          override;

private:

    /// @note disabled
    ItemPreviewCanvas(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
