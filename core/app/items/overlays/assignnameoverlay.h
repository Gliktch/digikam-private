/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-14
 * Description : overlay for assigning names to faces
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "itemdelegateoverlay.h"
#include "itemviewdelegate.h"

namespace Digikam
{

class AssignNameWidget;
class FaceTagsIface;
class ItemInfo;
class TaggingAction;

class AssignNameOverlay : public PersistentWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewDelegate)

public:

    explicit AssignNameOverlay(QObject* const parent);
    ~AssignNameOverlay()                                              override;

    AssignNameWidget* assignNameWidget() const;

Q_SIGNALS:

    void confirmFaces(const QList<QModelIndex>& indexes, int tagId);
    void unknownFaces(const QList<QModelIndex>& indexes);
    void removeFaces(const QList<QModelIndex>& indexes);
    void ignoreFaces(const QList<QModelIndex>& indexes);

protected Q_SLOTS:

    void slotAssigned(const TaggingAction& action, const ItemInfo&, const QVariant& faceIdentifier);
    void slotRejected(const ItemInfo&, const QVariant& faceIdentifier);
    void slotIgnored(const ItemInfo&, const QVariant& faceIdentifier);
    void slotUnknown(const ItemInfo&, const QVariant& faceIdentifier);

protected:

    QWidget* createWidget()                                           override;
    void     setActive(bool)                                          override;
    void     visualChange()                                           override;
    void     showOnIndex(const QModelIndex& index)                    override;
    bool     checkIndex(const QModelIndex& index)               const override;
    void     viewportLeaveEvent(QObject* obj, QEvent* event)          override;
    void     widgetEnterEvent()                                       override;
    void     widgetLeaveEvent()                                       override;
    void     setFocusOnWidget()                                       override;
    bool     eventFilter(QObject* o, QEvent* e)                       override;

    void updatePosition();
    void updateFace();

private:

    /// @note disabled
    AssignNameOverlay() = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
