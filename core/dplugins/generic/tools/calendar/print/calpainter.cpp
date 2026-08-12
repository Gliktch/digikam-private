/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : painter class to draw calendar.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "calpainter.h"

// Qt includes

#include <QDate>
#include <QFileInfo>
#include <QPointer>
#include <QPaintDevice>
#include <QRect>
#include <QString>
#include <QUrl>
#include <QImage>
#include <QLocale>
#include <QScopedPointer>

// Local includes

#include "calsettings.h"
#include "calsystem.h"
#include "digikam_debug.h"
#include "metaengine.h"
#include "dimg.h"
#include "previewloadthread.h"

using namespace Digikam;

namespace DigikamGenericCalendarPlugin
{

class Q_DECL_HIDDEN CalPainter::Private
{
public:

    Private() = default;

public:

    bool       cancelled   = false;

    int        orientation = MetaEngine::ORIENTATION_UNSPECIFIED;

    QImage     image;
    QUrl       imagePath;
};

CalPainter::CalPainter(QPaintDevice* const pDevice)
    : QPainter(pDevice),
      d       (new Private)
{
}

CalPainter::~CalPainter()
{
    delete d;
}

void CalPainter::cancel()
{
    d->cancelled = true;
}

void CalPainter::setImage(const QUrl& imagePath)
{
    d->imagePath   = imagePath;
    QScopedPointer<MetaEngine> meta(new MetaEngine(d->imagePath.toLocalFile()));
    d->orientation = (int)meta->getItemOrientation();
}

void CalPainter::paint(int month)
{
    if (!device())
    {
        return;
    }

    int width                         = device()->width();
    int height                        = device()->height();
    const CalSettings* const settings = CalSettings::instance();
    CalParams& params                 = CalSettings::instance()->params;

    // --------------------------------------------------

    // FIXME: magic number 42

    int days[42];
    int startDayOffset = QLocale().weekdays().constFirst();

    for (int i = 0 ; i < 42 ; ++i)
    {
        days[i] = -1;
    }

    QDate date = CalSystem().date(params.year, month, 1);
    int s      = date.dayOfWeek();

    if ((s + 7 - startDayOffset) >= 7)
    {
        s = s - 7;
    }

    for (int i = s ; i < (s + CalSystem().daysInMonth(date)) ; ++i)
    {
        days[i + (7 - startDayOffset)] = i - s + 1;
    }

    // -----------------------------------------------

    QRect rCal(0, 0, 0, 0);
    QRect rImage(0, 0, 0, 0);
    QRect rCalHeader(0, 0, 0, 0);

    int cellSizeX;
    int cellSizeY;

    switch (params.imgPos)
    {
        case CalParams::Top:
        {
            rImage.setWidth(width);
            rImage.setHeight((int)(height * params.ratio / (params.ratio + 100)));

            int remainingHeight = height - rImage.height();
            cellSizeX           = (width - 20) / 7;
            cellSizeY           = remainingHeight / 8;

            rCal.setWidth(cellSizeX * 7);
            rCal.setHeight(cellSizeY * 7);

            rCalHeader.setWidth(rCal.width());
            rCalHeader.setHeight(cellSizeY);
            rCalHeader.moveTop(rImage.bottom());
            rCalHeader.moveLeft(width / 2 - rCalHeader.width() / 2);

            rCal.moveTopLeft(rCalHeader.bottomLeft());

            break;
        }

        case CalParams::Left:
        {
            rImage.setHeight(height);
            rImage.setWidth((int)(width * params.ratio / (params.ratio + 100)));

            int remainingWidth  = width - rImage.width();
            cellSizeX           = (remainingWidth - 20) / 8;
            cellSizeY           = height / 8;

            rCal.setWidth(cellSizeX * 7);
            rCal.setHeight(cellSizeY * 7);

            rCalHeader.setWidth(rCal.width());
            rCalHeader.setHeight(cellSizeY);
            rCalHeader.moveLeft(rImage.right() + cellSizeX);

            rCal.moveTopLeft(rCalHeader.bottomLeft());

            break;
        }

        case CalParams::Right:
        {
            rImage.setHeight(height);
            rImage.setWidth((int)(width * params.ratio / (params.ratio + 100)));

            int remainingWidth  = width - rImage.width();
            cellSizeX           = (remainingWidth - 20) / 8;
            cellSizeY           = height / 8;

            rCal.setWidth(cellSizeX * 7);
            rCal.setHeight(cellSizeY * 7);

            rCalHeader.setWidth(rCal.width());
            rCalHeader.setHeight(cellSizeY);
            rCal.moveTop(rCalHeader.bottom());

            rImage.moveLeft(width - rImage.width());

            break;
        }

        default:
        {
            return;
        }
    }

    int fontPixels = cellSizeX / 3;
    params.baseFont.setPixelSize(fontPixels);

    // ---------------------------------------------------------------

    fillRect(0, 0, width, height, Qt::white);
    setFont(params.baseFont);

    // ---------------------------------------------------------------

    save();
    QFont f(params.baseFont);
    f.setBold(true);
    f.setPixelSize(f.pixelSize() + 5);
    setFont(f);
    drawText(rCalHeader, Qt::AlignLeft  | Qt::AlignVCenter, QString::number(params.year));
    drawText(rCalHeader, Qt::AlignRight | Qt::AlignVCenter, QLocale().standaloneMonthName(month));
    restore();

    // ---------------------------------------------------------------

    int   sx, sy;
    QRect r, rsmall, rSpecial;

    r.setWidth(cellSizeX);
    r.setHeight(cellSizeY);

    int index = 0;

    save();

    setPen(Qt::red);
    sy = rCal.top();

    for (int i = 0 ; i < 7 ; ++i)
    {
        int dayname = i + startDayOffset;

        if (dayname > 7)
        {
            dayname = dayname - 7;
        }

        sx     = cellSizeX * i + rCal.left();
        r.moveTopLeft(QPoint(sx, sy));
        rsmall = r;
        rsmall.setWidth(r.width() - (r.width() / 10));
        rsmall.setHeight(r.height() - 2);
        drawText(rsmall, Qt::AlignRight | Qt::AlignBottom,
                 QLocale().standaloneDayName(dayname, QLocale::ShortFormat));
    }

    restore();

    for (int j = 0 ; j < 6 ; ++j)
    {
        sy = cellSizeY * (j + 1) + rCal.top();

        for (int i = 0 ; i < 7 ; ++i)
        {
            sx     = cellSizeX * i + rCal.left();
            r.moveTopLeft(QPoint(sx, sy));
            rsmall = r;
            rsmall.setWidth(r.width() - (r.width() / 10));
            rsmall.setHeight(r.height() - 2);

            if (days[index] != -1)
            {
                if (settings->isSpecial(month, days[index]))
                {
                    save();
                    setPen(settings->getDayColor(month, days[index]));
                    drawText(rsmall, Qt::AlignRight | Qt::AlignBottom,
                             QString::number(days[index]));

                    QString descr = settings->getDayDescr(month, days[index]);

                    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Painting special info: '" << descr
                                                         << "' for date " << days[index] << "/"
                                                         << month;

                    rSpecial = rsmall;
                    rSpecial.translate(2, 0);
                    QFont fnt(params.baseFont);
                    fnt.setPixelSize(fnt.pixelSize() / 3);
                    setFont(fnt);

                    drawText(rSpecial, Qt::AlignLeft | Qt::AlignTop, descr);

                    restore();
                }
                else
                {
                    drawText(rsmall, Qt::AlignRight | Qt::AlignBottom,
                             QString::number(days[index]));
                }
            }

            index++;
        }
    }

    // ---------------------------------------------------------------

    if (params.drawLines)
    {
        sx = rCal.left();

        for (int j = 0 ; j < 8 ; ++j)
        {
            sy = cellSizeY * j + rCal.top();
            drawLine(sx, sy, rCal.right(), sy);
        }

        sy = rCal.top();

        for (int i = 0 ; i < 8 ; ++i)
        {
            sx = cellSizeX * i + rCal.left();
            drawLine(sx, sy, sx, rCal.bottom());
        }
    }

    d->image = PreviewLoadThread::loadHighQualitySynchronously(d->imagePath.toLocalFile()).copyQImage();

    if (d->image.isNull())
    {
        fillRect(rImage, Qt::blue);
    }
    else
    {
        if ( d->orientation != MetaEngine::ORIENTATION_UNSPECIFIED )
        {
            QScopedPointer<MetaEngine> meta(new MetaEngine);
            meta->rotateExifQImage(d->image, (MetaEngine::ImageOrientation)d->orientation);
        }

        Q_EMIT signalProgress(0);

        d->image = d->image.scaled(rImage.width(),
                                   rImage.height(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);

        Q_EMIT signalTotal(d->image.height());

        int h         = d->image.height();
        int x         = rImage.bottomLeft().x() + (rImage.width() - d->image.width()) / 2;
        int y         = (rImage.height() - h) / 2;
        int blockSize = 10;
        int block     = 0;

        while ((block < h) && !d->cancelled)
        {
            if (block + blockSize > h)
            {
                blockSize = h - block;
            }

            drawImage(x, y + block, d->image, 0, block, d->image.width(), blockSize);
            block += blockSize;

            Q_EMIT signalProgress(block);
        }

        Q_EMIT signalFinished();
    }
}

} // namespace Digikam

#include "moc_calpainter.cpp"
