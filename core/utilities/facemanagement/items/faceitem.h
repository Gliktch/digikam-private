/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Face tag region item
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QGraphicsWidget>

// Local includes

#include "facetagsiface.h"
#include "assignnamewidget.h"
#include "regionframeitem.h"

namespace Digikam
{

class HidingStateChanger;

class Q_DECL_HIDDEN FaceItem : public RegionFrameItem       // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

public:

    explicit FaceItem(QGraphicsItem* const parent);
    ~FaceItem() override                                        = default;

    void setFace(const FaceTagsIface& face);
    FaceTagsIface face()                                const;
    void setHudWidget(AssignNameWidget* const widget);
    AssignNameWidget* widget()                          const;
    void switchMode(AssignNameWidget::Mode mode);
    void setEditable(bool allowEdit);
    void updateCurrentTag();

protected:

    FaceTagsIface       m_face;
    AssignNameWidget*   m_widget    = nullptr;
    HidingStateChanger* m_changer   = nullptr;

private:

    /// @note disabled

    FaceItem()                           = delete;
    FaceItem(const FaceItem&)            = delete;
    FaceItem& operator=(const FaceItem&) = delete;
};

} // namespace Digikam
