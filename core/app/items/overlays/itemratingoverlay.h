/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : rating icon view item at mouse hover
 *
 * SPDX-FileCopyrightText: 2009      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

class RatingWidget;

class ItemRatingOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewDelegate)

public:

    explicit ItemRatingOverlay(QObject* const parent);

    RatingWidget* ratingWidget() const;

Q_SIGNALS:

    void ratingEdited(const QList<QModelIndex>& indexes, int rating);

protected Q_SLOTS:

    void slotRatingChanged(int);
    void slotDataChanged(const QModelIndex&, const QModelIndex&);

protected:

    void updatePosition();
    void updateRating();

protected:

    QWidget* createWidget()                    override;
    void setActive(bool)                       override;
    void visualChange()                        override;
    void hide()                                override;
    void slotEntered(const QModelIndex& index) override;
    void widgetEnterEvent()                    override;
    void widgetLeaveEvent()                    override;

protected:

    QPersistentModelIndex m_index;
};

} // namespace Digikam
