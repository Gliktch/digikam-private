/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-09
 * Description : a widget to filter album contents by rating
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dlayoutbox.h"
#include "itemfiltersettings.h"
#include "ratingwidget.h"

class QAction;

namespace Digikam
{

class RatingFilterWidget : public RatingWidget
{
    Q_OBJECT

public:

    explicit RatingFilterWidget(QWidget* const parent);
    ~RatingFilterWidget() override;

    void setRatingFilterCondition(ItemFilterSettings::RatingCondition cond);
    ItemFilterSettings::RatingCondition ratingFilterCondition();

    void setExcludeUnratedItems(bool excluded);
    bool isUnratedItemsExcluded();

Q_SIGNALS:

    void signalRatingFilterChanged(int, ItemFilterSettings::RatingCondition, bool);

protected:

    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:

    void updateRatingTooltip();

private Q_SLOTS:

    void slotRatingChanged();

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------------------------

class RatingFilter : public DHBox
{
    Q_OBJECT

public:

    explicit RatingFilter(QWidget* const parent);
    ~RatingFilter() override;

    void reset();

    void setRating(int val);
    int  rating() const;

    void setRatingFilterCondition(ItemFilterSettings::RatingCondition cond);
    ItemFilterSettings::RatingCondition ratingFilterCondition();

    void setExcludeUnratedItems(bool excluded);
    bool isUnratedItemsExcluded();

Q_SIGNALS:

    void signalRatingFilterChanged(int, ItemFilterSettings::RatingCondition, bool);

private Q_SLOTS:

    void slotOptionsMenu();
    void slotOptionsTriggered(QAction*);
    void slotNoRatingToggled(bool checked);
    void slotRatingFilterChanged(int, ItemFilterSettings::RatingCondition, bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
