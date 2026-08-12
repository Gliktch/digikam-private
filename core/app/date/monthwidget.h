/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-02
 * Description : a widget to perform month selection.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "iteminfo.h"

class QResizeEvent;
class QPaintEvent;
class QMouseEvent;

namespace Digikam
{
class ItemFilterModel;

class MonthWidget : public QWidget
{
    Q_OBJECT

public:

    explicit MonthWidget(QWidget* const parent);
    ~MonthWidget() override;

    void setItemModel(ItemFilterModel* const model);

    void setYearMonth(int year, int month);
    QSize sizeHint() const                      override;

    void setActive(bool val);

protected:

    void resizeEvent(QResizeEvent* e)           override;
    void paintEvent(QPaintEvent*)               override;
    void mousePressEvent(QMouseEvent* e)        override;

private:

    void init();

private Q_SLOTS:

    void triggerUpdateDays();
    void updateDays();
    void slotModelDestroyed();

private:

    void resetDayCounts();
    void connectModel();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
