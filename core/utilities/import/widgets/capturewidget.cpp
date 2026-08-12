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

#include "capturewidget.h"

// Qt includes

#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QRect>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN CaptureWidget::Private
{
public:

    Private() = default;

    QPixmap pixmap;
    QImage  preview;
};

CaptureWidget::CaptureWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

CaptureWidget::~CaptureWidget()
{
    delete d;
}

void CaptureWidget::setPreview(const QImage& preview)
{
    d->preview = preview;
    d->pixmap  = QPixmap(contentsRect().size());

    updatePixmap();
    repaint();
}

void CaptureWidget::updatePixmap()
{
    d->pixmap.fill(palette().window().color());
    QPainter p(&(d->pixmap));

    if (!d->preview.isNull())
    {
        QPixmap pix = QPixmap::fromImage(d->preview.scaled(contentsRect().size(),
                                                           Qt::KeepAspectRatio, Qt::SmoothTransformation));
        p.drawPixmap((contentsRect().width()  - pix.width())  / 2,
                     (contentsRect().height() - pix.height()) / 2, pix,
                     0, 0, pix.width(), pix.height());
    }
    else
    {
        p.setPen(QPen(palette().text().color()));
        p.drawText(0, 0, d->pixmap.width(), d->pixmap.height(),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   i18n("Cannot display camera preview"));
    }

    p.end();
}

void CaptureWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(contentsRect().top(), contentsRect().left(), d->pixmap);
    p.end();
}

void CaptureWidget::resizeEvent(QResizeEvent*)
{
    blockSignals(true);
    d->pixmap = QPixmap(contentsRect().size());
    updatePixmap();
    repaint();
    blockSignals(false);
}

} // namespace Digikam

#include "moc_capturewidget.cpp"
