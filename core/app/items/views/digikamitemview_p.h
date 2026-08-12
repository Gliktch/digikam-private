/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt model-view for items
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol do de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_config.h"
#include "digikamitemview.h"
#include "digikamitemdelegate.h"
#include "itemrotationoverlay.h"
#include "itemfullscreenoverlay.h"
#include "applicationsettings.h"
#include "facepipelineedit.h"

namespace Digikam
{

class DigikamItemDelegate;
class ItemFaceDelegate;

class Q_DECL_HIDDEN DigikamItemView::Private : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DigikamItemView)

public:

    explicit Private(DigikamItemView* const qq);
    ~Private() override = default;

    void updateOverlays();
    void triggerRotateAction(const char* actionName);

public:

    ItemViewUtilities*       utilities          = nullptr;

    FacePipelineEdit*        newEditPipeline    = nullptr;

    DigikamItemDelegate*     normalDelegate     = nullptr;
    ItemFaceDelegate*        faceDelegate       = nullptr;

    bool                     overlaysActive     = false;
    bool                     fullscreenActive   = false;

    ItemRotateOverlay*       rotateLeftOverlay  = nullptr;
    ItemRotateOverlay*       rotateRightOverlay = nullptr;
    ItemFullScreenOverlay*   fullscreenOverlay  = nullptr;

    int                      sidebarViewMode    = NormalView;

private:

    DigikamItemView*         q_ptr              = nullptr;

private:

    /// @note disabled
    Private() = delete;
};

} // namespace Digikam
