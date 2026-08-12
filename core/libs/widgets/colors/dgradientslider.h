/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : a color gradient slider
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2008 by Cyrille Berger <cberger at cberger dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QColor>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DGradientSlider : public QWidget
{
    Q_OBJECT

public:

    explicit DGradientSlider(QWidget* const parent = nullptr);
    ~DGradientSlider()                  override;

    void   showMiddleCursor(bool b);
    double leftValue()      const;
    double rightValue()     const;
    double middleValue()    const;
    int    gradientOffset() const;

    void setColors(const QColor& lcolor,
                   const QColor& rcolor);

public Q_SLOTS:

    void setLeftValue(double);
    void setRightValue(double);
    void setMiddleValue(double);

Q_SIGNALS:

    void leftValueChanged(double);
    void rightValueChanged(double);
    void middleValueChanged(double);

protected:

    void paintEvent(QPaintEvent*)        override;
    void mousePressEvent(QMouseEvent*)   override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*)    override;
    void leaveEvent(QEvent*)             override;

private:

    void adjustMiddleValue(double newLeftValue,
                           double newRightValue);
    inline void drawCursorAt(QPainter& painter,
                             double pos,
                             const QColor& brushColor,
                             int width,
                             int height,
                             int gradientWidth);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
