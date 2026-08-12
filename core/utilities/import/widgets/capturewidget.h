/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-06
 * Description : a widget to display camera capture preview.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QByteArray>

// Local includes

#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class CaptureWidget : public QWidget
{
    Q_OBJECT

public:

    explicit CaptureWidget(QWidget* const parent = nullptr);
    ~CaptureWidget() override;

    void setPreview(const QImage& preview);

protected:

    void paintEvent(QPaintEvent*)   override;
    void resizeEvent(QResizeEvent*) override;

private:

    void updatePixmap();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
