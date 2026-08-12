/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-09-13
 * Description : rubber item for Canvas
 *
 * SPDX-FileCopyrightText: 2013-2014 by Yiou Wang <geow812 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFlags>

// Local includes

#include "canvas.h"
#include "imagepreviewitem.h"
#include "regionframeitem.h"
#include "digikam_export.h"

class QWidget;

namespace Digikam
{

class DIGIKAM_EXPORT RubberItem : public RegionFrameItem
{
    Q_OBJECT

public:

    explicit RubberItem(ImagePreviewItem* const item);
    ~RubberItem()                                           override;

    void setCanvas(Canvas* const canvas);

protected:

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event)    override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
