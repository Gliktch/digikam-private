/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-01-08
 * Description : Hue/Saturation preview widget
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT HSPreviewWidget : public QWidget
{
    Q_OBJECT

public:

    explicit HSPreviewWidget(QWidget* const parent = nullptr);
    ~HSPreviewWidget()              override;

    void setHS(double hue, double sat);

protected:

    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent*)   override;

private:

    void updatePixmap();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
