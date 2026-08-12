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
#include <QPointF>
#include <QGraphicsWidget>
#include <QGraphicsSceneHoverEvent>

// Local includes

#include "assignnamewidget.h"
#include "regionframeitem.h"
#include "addtagscombobox.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albummanager.h"
#include "clickdragreleaseitem.h"
#include "dimgpreviewitem.h"
#include "graphicsdimgview.h"
#include "iteminfo.h"
#include "taggingaction.h"
#include "metadatahub.h"
#include "tagregion.h"
#include "digikam_debug.h"
#include "itemvisibilitycontroller.h"


namespace Digikam
{

class Album;
class ItemInfo;
class GraphicsDImgView;
class RegionFrameItem;
class TaggingAction;

class FocusPointGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:

    explicit FocusPointGroup(GraphicsDImgView* const view);
    ~FocusPointGroup()                                                          override;

    bool isVisible()                                                     const;
    ItemInfo info()                                                      const;
    bool hasVisibleItems()                                               const;

    QList<RegionFrameItem*> items()                                      const;

    void leaveEvent(QEvent*);

public:

    bool isAllowedToAddFocusPoint()                                      const;

public Q_SLOTS:

    /**
     * Shows or hides the frames
     */
    void setVisible(bool visible);
    void setVisibleItem(RegionFrameItem* const item);

    /**
     * Sets the current ItemInfo
     */
    void setInfo(const ItemInfo& info);

    void addPoint();

    void aboutToSetInfoAfterRotate(const ItemInfo& info);

protected:

    void load();

protected Q_SLOTS:

    void itemStateChanged(int);
    void slotAlbumsUpdated(int type);

    void slotAddItemStarted(const QPointF& pos);
    void slotAddItemMoving(const QRectF& rect);
    void slotAddItemFinished(const QRectF& rect);
    void slotCancelAddItem();

private:

    /// @note disabled
    FocusPointGroup()                                   = delete;
    FocusPointGroup(const FocusPointGroup&)             = delete;
    FocusPointGroup& operator=(const FocusPointGroup&)  = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
