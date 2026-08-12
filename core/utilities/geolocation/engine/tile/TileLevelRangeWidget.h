/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class DIGIKAM_EXPORT TileLevelRangeWidget: public QWidget
{
    Q_OBJECT

public:

    explicit TileLevelRangeWidget(QWidget* const parent = nullptr, Qt::WindowFlags const f = Qt::WindowFlags());
    ~TileLevelRangeWidget() override;

    QSize sizeHint() const override;

    void setAllowedLevelRange(int const minimumLevel, int const maximumLevel);
    void setDefaultLevel(int const);

    int topLevel() const;
    int bottomLevel() const;

Q_SIGNALS:

    void topLevelChanged(int);
    void bottomLevelChanged(int);

private Q_SLOTS:

    void setMaximumTopLevel(int const);
    void setMinimumBottomLevel(int const);

private:

    Q_DISABLE_COPY(TileLevelRangeWidget)

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Marble
