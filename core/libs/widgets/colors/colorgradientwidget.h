/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-28
 * Description : a color gradient widget
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// KDE includes

#include <QWidget>
#include <QColor>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ColorGradientWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ColorGradientWidget(Qt::Orientation orientation, int size,
                                 QWidget* const parent = nullptr);
    ~ColorGradientWidget()          override;

    void setColors(const QColor& col1, const QColor& col2);

protected:

    void paintEvent(QPaintEvent*)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
