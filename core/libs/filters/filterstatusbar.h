/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to indicate icon-view filters status
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "itemfiltersettings.h"

namespace Digikam
{

class FilterStatusBar : public QWidget
{
    Q_OBJECT

public:

    explicit FilterStatusBar(QWidget* const parent);
    ~FilterStatusBar() override;

public Q_SLOTS:

    void slotFilterMatches(bool);
    void slotFilterSettingsChanged(const ItemFilterSettings& settings);

Q_SIGNALS:

    void signalResetFilters();
    void signalPopupFiltersView();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
