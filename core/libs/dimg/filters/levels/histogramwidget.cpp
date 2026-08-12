/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-21
 * Description : a widget to display an image histogram.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "histogramwidget.h"

// Qt includes

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QPropertyAnimation>
#include <QRect>
#include <QTimer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemtooltip.h"
#include "imagehistogram.h"
#include "histogrampainter.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class Q_DECL_HIDDEN HistogramWidget::Private
{

public:

    enum HistogramState
    {
        HistogramNone = 0,        ///< No current histogram values calculation.
        HistogramDataLoading,     ///< The image is being loaded
        HistogramStarted,         ///< Histogram values calculation started.
        HistogramCompleted,       ///< Histogram values calculation completed.
        HistogramFailed           ///< Histogram values calculation failed.
    };

public:

    Private() = default;

public:

    bool                             sixteenBits        = false;
    bool                             guideVisible       = false;                ///< Display color guide.
    bool                             statisticsVisible  = false;                ///< Display tooltip histogram statistics.
    bool                             inSelected         = false;
    bool                             selectMode         = false;                ///< If true, a part of the histogram can be selected !
    bool                             showProgress       = false;                ///< If true, a message will be displayed during histogram computation,
                                                                                ///< else nothing (limit flicker effect in widget especially for small
                                                                                ///< image/computation time).

    int                              renderingType      = FullImageHistogram;   ///< Using full image or image selection for histogram rendering.
    int                              range              = 255;
    HistogramState                   state              = HistogramNone;        ///< Clear drawing zone with message.

    ChannelType                      channelType        = LuminosityChannel;    ///< Channel type to draw
    HistogramScale                   scaleType          = LogScaleHistogram;    ///< Scale to use for drawing
    ImageHistogram*                  imageHistogram     = nullptr;              ///< Full image
    ImageHistogram*                  selectionHistogram = nullptr;              ///< Image selection

    // Current selection information.

    double                           xmin               = 0;
    double                           xminOrg            = 0;
    double                           xmax               = 0;

    int                              animationState     = 0;
    QPropertyAnimation*              animation          = nullptr;
    DWorkingPixmap*                  progressPix        = nullptr;

    DColor                           colorGuide;

    HistogramPainter*                histogramPainter   = nullptr;
};

HistogramWidget::HistogramWidget(int w, int h,
                                 QWidget* const parent, bool selectMode,
                                 bool showProgress, bool statisticsVisible)
    : QWidget(parent),
      d      (new Private)
{
    d->progressPix  = new DWorkingPixmap(this);
    setup(w, h, selectMode, statisticsVisible);
    d->showProgress = showProgress;
}

HistogramWidget::~HistogramWidget()
{
    d->animation->stop();

    delete d->imageHistogram;
    delete d->selectionHistogram;
    delete d;
}

void HistogramWidget::setup(int w, int h, bool selectMode, bool statisticsVisible)
{
    d->statisticsVisible = statisticsVisible;
    d->selectMode        = selectMode;
    d->histogramPainter  = new HistogramPainter(this);
    d->animation         = new QPropertyAnimation(this, "animationState", this);
    d->animation->setStartValue(0);
    d->animation->setEndValue(d->progressPix->frameCount() - 1);
    d->animation->setDuration(200 * d->progressPix->frameCount());
    d->animation->setLoopCount(-1);

    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(w, h);
}

void HistogramWidget::connectHistogram(const ImageHistogram* const histogram)
{
    connect(histogram, SIGNAL(calculationAboutToStart()),
            this, SLOT(slotCalculationAboutToStart()));

    connect(histogram, SIGNAL(calculationFinished(bool)),
            this, SLOT(slotCalculationFinished(bool)));
}

void HistogramWidget::updateData(const DImg& img, const DImg& sel, bool showProgress)
{
    d->showProgress = showProgress;
    d->sixteenBits  = !img.isNull() ? img.sixteenBit() : sel.sixteenBit();

    // We are deleting the histogram data, so we must not use it to draw any more.

    d->state = HistogramWidget::Private::HistogramNone;

    // Do not using ImageHistogram::getHistogramSegments()
    // method here because histogram hasn't yet been computed.

    d->range = d->sixteenBits ? MAX_SEGMENT_16BIT : MAX_SEGMENT_8BIT;

    Q_EMIT signalMaximumValueChanged(d->range);

    if (!img.isNull() || sel.isNull())
    {
        // do not delete main histogram if only the selection is reset

        delete d->imageHistogram;
        d->imageHistogram = nullptr;
    }

    // Calc new histogram data

    if (!img.isNull())
    {
        d->imageHistogram = new ImageHistogram(img);
        connectHistogram(d->imageHistogram);
    }

    delete d->selectionHistogram;
    d->selectionHistogram = nullptr;

    if (!sel.isNull())
    {
        d->selectionHistogram = new ImageHistogram(sel);
        connectHistogram(d->selectionHistogram);
    }
    else
    {
        if (d->renderingType == ImageSelectionHistogram)
        {
            setRenderingType(FullImageHistogram);
        }
    }

    ImageHistogram* const histo = currentHistogram();

    if (histo)
    {
        histo->calculateInThread();
    }
    else
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Current histogram is null";
    }
}

void HistogramWidget::updateSelectionData(const DImg& sel, bool showProgress)
{
    updateData(DImg(), sel, showProgress);
}

void HistogramWidget::setHistogramGuideByColor(const DColor& color)
{
    d->guideVisible = true;
    d->colorGuide   = color;
    update();
}

void HistogramWidget::setStatisticsVisible(bool b)
{
    d->statisticsVisible = b;
    update();
}

void HistogramWidget::setRenderingType(HistogramRenderingType type)
{
    if (type != d->renderingType)
    {
        d->renderingType = type;

        ImageHistogram* const nowUsedHistogram = currentHistogram();

        if (!nowUsedHistogram)
        {
            qCWarning(DIGIKAM_DIMG_LOG) << "Current histogram is null";
            return;
        }

        // already calculated?

        if (!nowUsedHistogram->isValid())
        {
            // still computing, or need to start it?

            if (nowUsedHistogram->isCalculating())
            {
                setState(HistogramWidget::Private::HistogramStarted);
            }
            else
            {
                nowUsedHistogram->calculateInThread();
            }
        }
        else
        {
            update();
        }
    }
}

HistogramRenderingType HistogramWidget::renderingType() const
{
    return (HistogramRenderingType)d->renderingType;
}

ImageHistogram* HistogramWidget::currentHistogram() const
{
    if ((d->renderingType == ImageSelectionHistogram) && d->selectionHistogram)
    {
        return d->selectionHistogram;
    }
    else
    {
        return d->imageHistogram;
    }
}

void HistogramWidget::reset()
{
    d->histogramPainter->disableHistogramGuide();
    update();
}

void HistogramWidget::startWaitingAnimation()
{
    if (d->showProgress)
    {
        d->animation->start();
    }

    setCursor(Qt::WaitCursor);
}

void HistogramWidget::stopWaitingAnimation()
{
    d->animation->stop();
    unsetCursor();
}

void HistogramWidget::setState(int state)
{
    if (d->state == state)
    {
        return;
    }

    d->state = (HistogramWidget::Private::HistogramState)state;

    switch (state)
    {
        case HistogramWidget::Private::HistogramNone:
        {
            break;
        }

        case HistogramWidget::Private::HistogramDataLoading:
        {
            startWaitingAnimation();
            break;
        }

        case HistogramWidget::Private::HistogramStarted:
        {
            startWaitingAnimation();
            break;
        }

        case HistogramWidget::Private::HistogramCompleted:
        {
            notifyValuesChanged();
            Q_EMIT signalHistogramComputationDone(d->sixteenBits);

            stopWaitingAnimation();
            update();
            break;
        }

        case HistogramWidget::Private::HistogramFailed:
        {
            Q_EMIT signalHistogramComputationFailed();

            // Remove old histogram data from memory.

            delete d->imageHistogram;
            d->imageHistogram     = nullptr;
            delete d->selectionHistogram;
            d->selectionHistogram = nullptr;

            stopWaitingAnimation();
            update();
            break;
        }
    }
}

void HistogramWidget::slotCalculationAboutToStart()
{
    // only react to the histogram that the user is currently waiting for

    if (QObject::sender() != currentHistogram())
    {
        return;
    }

    setState(HistogramWidget::Private::HistogramStarted);
}

void HistogramWidget::slotCalculationFinished(bool success)
{
    // only react to the histogram that the user is currently waiting for

    if (QObject::sender() != currentHistogram())
    {
        return;
    }

    if (success)
    {
        setState(HistogramWidget::Private::HistogramCompleted);
    }
    else
    {
        setState(HistogramWidget::Private::HistogramFailed);
    }
}

void HistogramWidget::setDataLoading()
{
    setState(HistogramWidget::Private::HistogramDataLoading);
}

void HistogramWidget::setLoadingFailed()
{
    setState(HistogramWidget::Private::HistogramFailed);
}

void HistogramWidget::stopHistogramComputation()
{
    if (d->imageHistogram)
    {
        d->imageHistogram->stopCalculation();
    }

    if (d->selectionHistogram)
    {
        d->selectionHistogram->stopCalculation();
    }

    stopWaitingAnimation();
}

int HistogramWidget::animationState() const
{
    return d->animationState;
}

void HistogramWidget::setAnimationState(int animationState)
{
    if (d->animationState == animationState)
    {
        return;
    }

    d->animationState = animationState;
    update();
}

void HistogramWidget::paintEvent(QPaintEvent*)
{
    // Widget is disabled, not initialized,
    // or loading, but no message shall be drawn:
    // Drawing grayed frame.

    if     (
            !isEnabled()                                                                            ||
            (d->state == HistogramWidget::Private::HistogramNone)                                   ||
            (!d->showProgress && ((d->state == HistogramWidget::Private::HistogramStarted)          ||
                                  (d->state == HistogramWidget::Private::HistogramDataLoading)))
           )
    {
        QPainter p1(this);
        p1.fillRect(0, 0, width(), height(), palette().color(QPalette::Disabled, QPalette::Window));
        p1.setPen(QPen(palette().color(QPalette::Active, QPalette::WindowText), 1, Qt::SolidLine));
        p1.drawRect(0, 0, width() - 1, height() - 1);
        QPen pen(palette().color(QPalette::Disabled, QPalette::WindowText));
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);

        p1.setPen(pen);
        p1.drawRect(0, 0, width(), height());
        p1.end();

        return;
    }
    else if (
             d->showProgress &&
             ((d->state == HistogramWidget::Private::HistogramStarted)      ||
              (d->state == HistogramWidget::Private::HistogramDataLoading))
            )
    {
        // Image data is loading or histogram is being computed, we draw a message.

        // In first, we draw an animation.

        QPixmap anim = d->progressPix->frameAt(d->animationState);

        // ... and we render busy text.

        QPainter p1(this);
        p1.fillRect(0, 0, width(), height(), palette().color(QPalette::Active, QPalette::Window));
        p1.setPen(QPen(palette().color(QPalette::Active, QPalette::WindowText), 1, Qt::SolidLine));
        p1.drawRect(0, 0, width() - 1, height() - 1);
        p1.drawPixmap(width() / 2 - anim.width() / 2, anim.height(), anim);
        p1.setPen(palette().color(QPalette::Active, QPalette::Text));

        if (d->state == HistogramWidget::Private::HistogramDataLoading)
        {
            p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                        i18nc("@info: histogram", "Loading image..."));
        }
        else
        {
            p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                        i18nc("@info: histogram", "Histogram calculation..."));
        }

        p1.end();

        return;
    }
    else if (d->state == HistogramWidget::Private::HistogramFailed)
    {
        // Histogram computation failed, we draw a message.

        QPainter p1(this);
        p1.fillRect(0, 0, width(), height(), palette().color(QPalette::Active, QPalette::Window));
        p1.setPen(QPen(palette().color(QPalette::Active, QPalette::WindowText), 1, Qt::SolidLine));
        p1.drawRect(0, 0, width() - 1, height() - 1);
        p1.setPen(palette().color(QPalette::Active, QPalette::Text));
        p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                    i18nc("@info: histogram", "Histogram\ncalculation\nfailed."));
        p1.end();

        return;
    }

    // render histogram in normal case

    ImageHistogram* histogram = nullptr;

    if ((d->renderingType == ImageSelectionHistogram) && d->selectionHistogram)
    {
        histogram = d->selectionHistogram;
    }
    else
    {
        histogram = d->imageHistogram;
    }

    if (!histogram)
    {
        return;
    }

    d->histogramPainter->setHistogram(histogram);
    d->histogramPainter->setChannelType(d->channelType);
    d->histogramPainter->setScale(d->scaleType);
    d->histogramPainter->setSelection(d->xmin, d->xmax);
    d->histogramPainter->setHighlightSelection(d->selectMode);

    if (d->guideVisible == true)
    {
        d->histogramPainter->enableHistogramGuideByColor(d->colorGuide);
    }
    else
    {
        d->histogramPainter->disableHistogramGuide();
    }

    // A QPixmap is used to enable the double buffering.

    QPixmap bufferPixmap(size());

    d->histogramPainter->initFrom(this);
    d->histogramPainter->render(bufferPixmap);

    // render the pixmap on the widget

    QPainter p2(this);
    p2.drawPixmap(0, 0, bufferPixmap);
    p2.end();

    // render statistics if needed

    if (d->statisticsVisible)
    {
        DToolTipStyleSheet cnt;
        QString            tipText;
        QString            value;
        tipText = QLatin1String("<qt><table cellspacing=0 cellpadding=0>");

        tipText += cnt.cellBeg + i18nc("@info: histogram properties", "Mean:") + cnt.cellMid;
        double mean = histogram->getMean(d->channelType, 0, histogram->getHistogramSegments() - 1);
        tipText += value.setNum(mean, 'f', 1) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18nc("@info: histogram properties", "Pixels:") + cnt.cellMid;
        double pixels = histogram->getPixels();
        tipText += value.setNum((float)pixels, 'f', 0) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18nc("@info: histogram properties", "Std dev.:") + cnt.cellMid;
        double stddev = histogram->getStdDev(d->channelType, 0, histogram->getHistogramSegments() - 1);
        tipText += value.setNum(stddev, 'f', 1) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18nc("@info: histogram properties", "Count:") + cnt.cellMid;
        double counts = histogram->getCount(d->channelType, 0, histogram->getHistogramSegments() - 1);
        tipText += value.setNum((float)counts, 'f', 0) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18nc("@info: histogram properties", "Median:") + cnt.cellMid;
        double median = histogram->getMedian(d->channelType, 0, histogram->getHistogramSegments() - 1);
        tipText += value.setNum(median, 'f', 1) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18nc("@info: histogram properties", "Percent:") + cnt.cellMid;
        double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
        tipText += value.setNum(percentile, 'f', 1) + cnt.cellEnd;

        tipText += QLatin1String("</table></qt>");

        setToolTip(tipText);
    }
}

void HistogramWidget::mousePressEvent(QMouseEvent* e)
{
    if ((d->selectMode == true) && (d->state == HistogramWidget::Private::HistogramCompleted))
    {
        if (!d->inSelected)
        {
            d->inSelected = true;
            update();
        }

        d->xmin    = ((double)e->pos().x()) / ((double)width());
        d->xminOrg = d->xmin;
        d->xmax    = d->xmin;
        notifyValuesChanged();
    }
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent*)
{
    if ((d->selectMode == true) && (d->state == HistogramWidget::Private::HistogramCompleted))
    {
        d->inSelected = false;

        // Only single click without mouse move? Remove selection.
        if (d->xmax == d->xmin)
        {
            d->xmin = 0.0;
            d->xmax = 0.0;
            notifyValuesChanged();
            update();
        }
    }
}

void HistogramWidget::mouseMoveEvent(QMouseEvent* e)
{
    if ((d->selectMode == true) && (d->state == HistogramWidget::Private::HistogramCompleted))
    {
        setCursor(Qt::CrossCursor);

        if (d->inSelected)
        {
            double max = ((double)e->pos().x()) / ((double)width());

            if (max < d->xminOrg)
            {
                d->xmax = d->xminOrg;
                d->xmin = max;
            }
            else
            {
                d->xmin = d->xminOrg;
                d->xmax = max;
            }

            notifyValuesChanged();
            update();
        }
    }
}

void HistogramWidget::notifyValuesChanged()
{
    Q_EMIT signalIntervalChanged((int)(d->xmin * d->range), d->xmax == 0.0 ? d->range : (int)(d->xmax * d->range));
}

void HistogramWidget::slotMinValueChanged(int min)
{
    if ((d->selectMode == true) && (d->state == HistogramWidget::Private::HistogramCompleted))
    {
        if ((min == 0) && (d->xmax == 1.0))
        {
            // everything is selected means no selection

            d->xmin = 0.0;
            d->xmax = 0.0;
        }

        if ((min >= 0) && (min < d->range))
        {
            d->xmin = ((double)min) / d->range;
        }

        update();
    }
}

void HistogramWidget::slotMaxValueChanged(int max)
{
    if ((d->selectMode == true) && (d->state == HistogramWidget::Private::HistogramCompleted))
    {
        if      ((d->xmin == 0.0) && (max == d->range))
        {
            // everything is selected means no selection

            d->xmin = 0.0;
            d->xmax = 0.0;
        }
        else if ((max > 0) && (max <= d->range))
        {
            d->xmax = ((double)max) / d->range;
        }

        update();
    }
}

void HistogramWidget::setChannelType(ChannelType channel)
{
    d->channelType = channel;
    update();
}

ChannelType HistogramWidget::channelType() const
{
    return d->channelType;
}

void HistogramWidget::setScaleType(HistogramScale scale)
{
    d->scaleType = scale;
    update();
}

HistogramScale HistogramWidget::scaleType() const
{
    return d->scaleType;
}

} // namespace Digikam

#include "moc_histogramwidget.cpp"
