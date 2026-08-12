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

#include "hspreviewwidget.h"

// Qt includes

#include <QStyle>
#include <QImage>
#include <QPainter>
#include <QPixmap>

// Local includes

#include "dimg.h"
#include "hslfilter.h"

namespace Digikam
{

class Q_DECL_HIDDEN HSPreviewWidget::Private
{

public:

    Private() = default;

    int     xBorder = 0;

    double  hue     = 0.0;
    double  sat     = 0.0;

    QPixmap pixmap;
};

HSPreviewWidget::HSPreviewWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->xBorder = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setAttribute(Qt::WA_DeleteOnClose);
}

HSPreviewWidget::~HSPreviewWidget()
{
    delete d;
}

void HSPreviewWidget::setHS(double hue, double sat)
{
    d->hue = hue;
    d->sat = sat;
    updatePixmap();
    update();
}

void HSPreviewWidget::resizeEvent(QResizeEvent*)
{
    updatePixmap();
}

void HSPreviewWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(d->xBorder, 0, d->pixmap);
    p.end();
}

void HSPreviewWidget::updatePixmap()
{
    int xSize = width() - 2 * d->xBorder;
    int ySize = height();

    DImg   image(xSize, ySize, false, false, nullptr, false);
    QColor col;
    uint*  p  = nullptr;

    for (int s = ySize - 1 ; s >= 0 ; --s)
    {
        p = reinterpret_cast<uint*>(image.scanLine(ySize - s - 1));

        for (int h = 0 ; h < xSize ; ++h)
        {
            if ((xSize - 1) != 0)
            {
                col.setHsv(359 * h / (xSize - 1), 255, 192);
                *p = col.rgb();
                ++p;
            }
        }
    }

    HSLContainer settings;
    settings.hue        = d->hue;
    settings.saturation = d->sat;
    settings.lightness  = 0.0;
    HSLFilter hsl(&image, nullptr, settings);
    hsl.startFilterDirectly();
    image.putImageData(hsl.getTargetImage().bits());

    d->pixmap           = image.convertToPixmap();
}

} // namespace Digikam

#include "moc_hspreviewwidget.cpp"
