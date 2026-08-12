/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow OSD widget
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 * SPDX-FileCopyrightText:      2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slideosd.h"

// Qt includes

#include <QApplication>
#include <QProgressBar>
#include <QProxyStyle>
#include <QLayout>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QEvent>
#include <QStyle>

// Local includes

#include "digikam_debug.h"
#include "slideshowloader.h"
#include "slidetoolbar.h"
#include "slideproperties.h"
#include "ratingwidget.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class Q_DECL_HIDDEN SlideVideoStyle : public QProxyStyle
{
    Q_OBJECT

public:

    using QProxyStyle::QProxyStyle;

    int styleHint(QStyle::StyleHint hint,
                  const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr,
                  QStyleHintReturn* returnData = nullptr) const override
    {
        if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
        {
            return (Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

class Q_DECL_HIDDEN SlideOSD::Private
{
public:

    Private() = default;

public:

    bool                paused          = false;
    bool                video           = false;
    bool                blink           = false;
    bool                ready           = false;

    int const           refresh         = 1000;     ///< Progress bar refresh in ms

    QProgressBar*       progressBar     = nullptr;
    QTimer*             progressTimer   = nullptr;

    DHBox*              labelsBox       = nullptr;
    DHBox*              progressBox     = nullptr;

    DHBox*              videoIndicator  = nullptr;

    QSlider*            videoSlider     = nullptr;
    QSlider*            videoVolume     = nullptr;
    QLabel*             videoLabel      = nullptr;

    SlideShowLoader*    parent          = nullptr;
    SlideProperties*    slideProps      = nullptr;
    SlideToolBar*       toolBar         = nullptr;
    RatingWidget*       ratingWidget    = nullptr;
    ColorLabelSelector* clWidget        = nullptr;
    PickLabelSelector*  plWidget        = nullptr;
    SlideShowSettings*  settings        = nullptr;
};

SlideOSD::SlideOSD(SlideShowSettings* const settings, SlideShowLoader* const parent)
    : QWidget(parent),
      d      (new Private)
{
    Qt::WindowFlags flags = Qt::FramelessWindowHint  |
                            Qt::WindowStaysOnTopHint |
                            Qt::X11BypassWindowManagerHint;

    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setMouseTracking(true);

    d->parent         = parent;
    d->settings       = settings;

    d->slideProps     = new SlideProperties(d->settings, this);

    // ---------------------------------------------------------------

    d->videoIndicator = new DHBox(this);
    d->videoSlider    = new QSlider(Qt::Horizontal, d->videoIndicator);
    d->videoSlider->setStyle(new SlideVideoStyle());
    d->videoSlider->setRange(0, 0);
    d->videoSlider->setAutoFillBackground(true);
    d->videoLabel     = new QLabel(d->videoIndicator);
    d->videoLabel->setText(QLatin1String("00:00:00 / 00:00:00"));
    d->videoLabel->setAutoFillBackground(true);
    QLabel* const spk = new QLabel(d->videoIndicator);
    spk->setPixmap(QIcon::fromTheme(QLatin1String("audio-volume-high")).pixmap(22, 22));
    d->videoVolume    = new QSlider(Qt::Horizontal, d->videoIndicator);
    d->videoVolume->setRange(0, 100);
    d->videoVolume->setValue(50);
    d->videoIndicator->setStretchFactor(d->videoSlider, 10);
    d->videoIndicator->setAutoFillBackground(true);
    d->videoIndicator->setSpacing(4);
    d->videoIndicator->hide();

    // ---------------------------------------------------------------

    d->labelsBox      = new DHBox(this);

    d->clWidget       = new ColorLabelSelector(d->labelsBox);
    d->clWidget->setFocusPolicy(Qt::NoFocus);
    d->clWidget->setMouseTracking(true);

    d->plWidget       = new PickLabelSelector(d->labelsBox);
    d->plWidget->setFocusPolicy(Qt::NoFocus);
    d->plWidget->setMouseTracking(true);

    d->ratingWidget   = new RatingWidget(d->labelsBox);
    d->ratingWidget->setTracking(false);
    d->ratingWidget->setFading(false);
    d->ratingWidget->setFocusPolicy(Qt::NoFocus);
    d->ratingWidget->setMouseTracking(true);

    d->labelsBox->setStretchFactor(new QWidget(d->labelsBox), 10);
    d->labelsBox->layout()->setAlignment(d->ratingWidget, Qt::AlignVCenter | Qt::AlignLeft);
    d->labelsBox->setMouseTracking(true);

    d->labelsBox->setVisible(d->settings->printLabels || d->settings->printRating);
    d->ratingWidget->setVisible(d->settings->printRating);
    d->clWidget->setVisible(d->settings->printLabels);
    d->plWidget->setVisible(d->settings->printLabels);

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            parent, SLOT(slotAssignRating(int)));

    connect(d->clWidget, SIGNAL(signalColorLabelChanged(int)),
            parent, SLOT(slotAssignColorLabel(int)));

    connect(d->plWidget, SIGNAL(signalPickLabelChanged(int)),
            parent, SLOT(slotAssignPickLabel(int)));

    connect(d->settings->iface, &DInfoInterface::signalColorLabelNamesUpdated,
            this, [this]()
       {
          d->clWidget->colorLabelWidget()->setDescriptionBoxVisible(false);
       }
    );

    // ---------------------------------------------------------------

    d->progressBox   = new DHBox(this);
    d->progressBox->setVisible(d->settings->showProgressIndicator);
    d->progressBox->setMouseTracking(true);

    d->progressBar   = new QProgressBar(d->progressBox);
    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->settings->delay);
    d->progressBar->setFocusPolicy(Qt::NoFocus);
    d->progressBar->setMouseTracking(true);
    d->progressBar->setValue(0);

    d->toolBar       = new SlideToolBar(d->settings, d->progressBox);

    // ---------------------------------------------------------------

    d->slideProps->installEventFilter(d->parent);
    d->clWidget->installEventFilter(this);
    d->clWidget->installEventFilter(d->parent);
    d->clWidget->colorLabelWidget()->installEventFilter(this);
    d->plWidget->installEventFilter(this);
    d->plWidget->installEventFilter(d->parent);
    d->plWidget->pickLabelWidget()->installEventFilter(this);
    d->ratingWidget->installEventFilter(this);
    d->ratingWidget->installEventFilter(d->parent);
    d->labelsBox->installEventFilter(d->parent);
    d->progressBox->installEventFilter(d->parent);
    d->progressBar->installEventFilter(d->parent);
    d->toolBar->installEventFilter(this);
    d->toolBar->installEventFilter(d->parent);

    // ---------------------------------------------------------------

    connect(d->toolBar, SIGNAL(signalPause()),
            d->parent, SLOT(slotPause()));

    connect(d->toolBar, SIGNAL(signalPlay()),
            d->parent, SLOT(slotPlay()));

    connect(d->toolBar, SIGNAL(signalNext()),
            d->parent, SLOT(slotLoadNextItem()));

    connect(d->toolBar, SIGNAL(signalPrev()),
            d->parent, SLOT(slotLoadPrevItem()));

    connect(d->toolBar, SIGNAL(signalClose()),
            d->parent, SLOT(close()));

    connect(d->toolBar, SIGNAL(signalRemoveImageFromList()),
            d->parent, SLOT(slotRemoveImageFromList()));

    connect(d->toolBar, SIGNAL(signalUpdateSettings()),
            this, SLOT(slotUpdateSettings()));

    connect(d->toolBar, SIGNAL(signalScreenSelected(int)),
            d->parent, SLOT(slotScreenSelected(int)));

    connect(d->videoSlider, SIGNAL(sliderMoved(int)),
            this, SIGNAL(signalVideoPosition(int)));

    connect(d->videoSlider, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalVideoPosition(int)));

    connect(d->videoVolume, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalVideoVolume(int)));

    // ---------------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->videoIndicator, 0, 0, 1, 2);
    grid->addWidget(d->slideProps,     1, 0, 1, 2);
    grid->addWidget(d->labelsBox,      2, 0, 1, 1);
    grid->addWidget(d->progressBox,    3, 0, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setColumnStretch(1, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(layoutSpacing());

    // ---------------------------------------------------------------

    d->progressTimer = new QTimer(this);
    d->progressTimer->setSingleShot(false);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimer()));

    QTimer::singleShot(500, this, SLOT(slotStart()));
}

SlideOSD::~SlideOSD()
{
    d->progressTimer->stop();

    delete d;
}

void SlideOSD::slotStart()
{
    d->settings->suffleImages();
    d->parent->resolveStartItem();
    d->parent->slotLoadNextItem();
    d->progressTimer->start(d->refresh);
    pause(!d->settings->autoPlayEnabled);
}

void SlideOSD::slotUpdateSettings()
{
    d->labelsBox->setVisible(d->settings->printLabels || d->settings->printRating);
    d->progressBox->setVisible(d->settings->showProgressIndicator);
    d->ratingWidget->setVisible(d->settings->printRating);
    d->clWidget->setVisible(d->settings->printLabels);
    d->plWidget->setVisible(d->settings->printLabels);
    d->progressBar->setMaximum(d->settings->delay);
    d->parent->reshuffleImages();
}

SlideToolBar* SlideOSD::toolBar() const
{
    return d->toolBar;
}

void SlideOSD::setCurrentUrl(const QUrl& url)
{
    DItemInfo item(d->settings->iface->itemInfo(url));

    // Update info text.

    d->slideProps->setCurrentUrl(url);

    // Display Labels.

    if (d->settings->printLabels)
    {
        d->clWidget->setColorLabel((ColorLabel)item.colorLabel());
        d->plWidget->setPickLabel((PickLabel)item.pickLabel());
    }

    if (d->settings->printRating)
    {
        d->ratingWidget->setRating(item.rating());
    }

    // Make the OSD the proper size

    resize(d->parent->width() - 10, d->parent->height());
    move(10, 0);
    raise();
}

QSize SlideOSD::slideShowSize() const
{
    return d->parent->size();
}

bool SlideOSD::eventFilter(QObject* obj, QEvent* ev)
{
    if (
        (obj == d->labelsBox)                    ||
        (obj == d->ratingWidget)                 ||
        (obj == d->clWidget)                     ||
        (obj == d->plWidget)                     ||
        (obj == d->clWidget->colorLabelWidget()) ||
        (obj == d->plWidget->pickLabelWidget())
       )
    {
        if (ev->type() == QEvent::Enter)
        {
            d->paused = isPaused();
            d->parent->slotPause();

            return false;
        }

        if (ev->type() == QEvent::Leave)
        {
            if (!d->paused)
            {
                d->parent->slotPlay();
            }

            return false;
        }
    }

    // pass the event on to the parent class

    return QWidget::eventFilter(obj, ev);
}

void SlideOSD::slotProgressTimer()
{
    QString str = QString::fromUtf8("(%1/%2)")
                    .arg(d->settings->fileList.indexOf(d->parent->currentItem()) + 1)
                    .arg(d->settings->fileList.count());

    if      (isPaused())
    {
        d->blink = !d->blink;

        if (d->blink)
        {
            str = QString();
        }

        d->progressBar->setFormat(str);
    }
    else if (d->video)
    {
        d->progressBar->setFormat(str);
        return;
    }
    else
    {
        d->progressBar->setFormat(str);
        d->progressBar->setMaximum(d->settings->delay);

        if (d->progressBar->value() == d->settings->delay)
        {
            if (!d->ready)
            {
                return;
            }

            d->ready = false;
            d->parent->slotLoadNextItem();
        }

        d->progressBar->setValue(d->progressBar->value()+1);
    }
}

void SlideOSD::slotPositionChanged(qint64 position)
{
    if (!d->videoSlider->isSliderDown())
    {
        d->videoSlider->blockSignals(true);
        d->videoSlider->setValue(position);
        d->videoSlider->blockSignals(false);
    }

    d->videoLabel->setText(QString::fromLatin1("%1 / %2")
                       .arg(QTime(0, 0, 0).addMSecs(position).toString(QLatin1String("HH:mm:ss")))
                       .arg(QTime(0, 0, 0).addMSecs(d->videoSlider->maximum()).toString(QLatin1String("HH:mm:ss"))));
}

void SlideOSD::slotDurationChanged(qint64 duration)
{
    qint64 max = qMax((qint64)1, duration);
    d->videoSlider->setRange(0, max);
}

void SlideOSD::slotVolumeChanged(int volume)
{
    d->videoVolume->setValue(volume);
}

void SlideOSD::pause(bool b)
{
    d->toolBar->pause(b);

    if (!b)
    {
        d->progressBar->setValue(0);
    }
}

void SlideOSD::video(bool b)
{
    d->video = b;
}

bool SlideOSD::isPaused() const
{
    return d->toolBar->isPaused();
}

bool SlideOSD::isUnderMouse() const
{
    return (
            d->videoIndicator->underMouse() ||
            d->ratingWidget->underMouse()   ||
            d->progressBar->underMouse()    ||
            d->clWidget->underMouse()       ||
            d->plWidget->underMouse()       ||
            d->toolBar->underMouse()
           );
}

void SlideOSD::setLoadingReady(bool b)
{
    d->ready = b;
}

void SlideOSD::showVideoIndicator(bool b)
{
    d->videoIndicator->setVisible(d->video && b);
}

} // namespace DigikamGenericSlideShowPlugin

#include "slideosd.moc"

#include "moc_slideosd.cpp"
