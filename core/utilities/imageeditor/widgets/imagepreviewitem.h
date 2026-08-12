/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-27
 * Description : image preview item for image editor.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "graphicsdimgitem.h"
#include "digikam_export.h"

class QPainter;
class QStyleOptionGraphicsItem;
class QGraphicsSceneContextMenuEvent;

namespace Digikam
{

class DIGIKAM_EXPORT ImagePreviewItem : public GraphicsDImgItem
{
    Q_OBJECT

public:

    ImagePreviewItem();
    ~ImagePreviewItem()         override = default;

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

private:

    /// @note disabled
    explicit ImagePreviewItem(QObject*) = delete;
};

} // namespace Digikam
