/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Managing of focus point items on a GraphicsDImgView
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QGraphicsWidget>

// Local includes

#include "regionframeitem.h"
#include "focuspoint.h"

namespace Digikam
{

class FocusPointItem : public RegionFrameItem       // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

public:

    explicit FocusPointItem(QGraphicsItem* const parent);
    ~FocusPointItem()                                                         override;

    void setPoint(const FocusPoint& point);
    FocusPoint point()                                                  const;
    void setEditable(bool allowEdit);

private:

    /// @note disabled
    FocusPointItem()                                 = delete;
    FocusPointItem(const FocusPointItem&)            = delete;
    FocusPointItem& operator=(const FocusPointItem&) = delete;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)  override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
