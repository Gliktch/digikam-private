/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-22
 * Description : Slideshow video viewer based on QtAVPlayer
 *
 * SPDX-FileCopyrightText: 2014-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slidevideo.h"

// Qt includes

#include <QApplication>
#include <QVBoxLayout>
#include <QString>
#include <QStyle>
#include <QTimer>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "metaengine.h"

namespace Digikam
{

class Q_DECL_HIDDEN SlideVideo::Private
{
public:

    Private() = default;

public:

    DInfoInterface*      iface            = nullptr;

    DVideoWidget*        videoWidget      = nullptr;
};

SlideVideo::SlideVideo(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setMouseTracking(true);

    d->videoWidget          = new DVideoWidget(this);

    QVBoxLayout* const vlay = new QVBoxLayout(this);
    vlay->addWidget(d->videoWidget, 100);
    vlay->setContentsMargins(QMargins());

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Media Player Settings"));
    int volume                = group.readEntry("Volume", 50);

    d->videoWidget->audioOutput()->setVolume(volume);

    // --------------------------------------------------------------------------

    connect(d->videoWidget->player(), SIGNAL(stateChanged(QAVPlayer::State)),
            this, SLOT(slotPlayerStateChanged(QAVPlayer::State)));

    connect(d->videoWidget->player(), SIGNAL(seeked(qint64)),
            this, SIGNAL(signalVideoPosition(qint64)),
            Qt::QueuedConnection);

    connect(d->videoWidget->player(), SIGNAL(durationChanged(qint64)),
            this, SIGNAL(signalVideoDuration(qint64)));

    connect(d->videoWidget->player(), SIGNAL(errorOccurred(QAVPlayer::Error,QString)),
            this, SLOT(slotHandlePlayerError(QAVPlayer::Error,QString)));

    connect(d->videoWidget->player(), SIGNAL(mediaStatusChanged(QAVPlayer::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QAVPlayer::MediaStatus)));

    // --------------------------------------------------------------------------

    layout()->activate();
    resize(sizeHint());
}

SlideVideo::~SlideVideo()
{
    stop();
    delete d;
}

void SlideVideo::setInfoInterface(DInfoInterface* const iface)
{
    d->iface = iface;
}

void SlideVideo::setCurrentUrl(const QUrl& url)
{
    d->videoWidget->player()->stop();

    int orientation = 0;

    if (d->iface)
    {
        DItemInfo info(d->iface->itemInfo(url));

        orientation = info.orientation();
    }

    switch (orientation)
    {
        case MetaEngine::ORIENTATION_ROT_90:
        case MetaEngine::ORIENTATION_ROT_90_HFLIP:
        case MetaEngine::ORIENTATION_ROT_90_VFLIP:
        {
            d->videoWidget->setVideoItemOrientation(90);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_180:
        {
            d->videoWidget->setVideoItemOrientation(180);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_270:
        {
            d->videoWidget->setVideoItemOrientation(270);
            break;
        }

        default:
        {
            d->videoWidget->setVideoItemOrientation(0);
            break;
        }
    }

    Q_EMIT signalVideoVolume(d->videoWidget->audioOutput()->volume() * 100.0);

    d->videoWidget->player()->setSource(url.toLocalFile());
    d->videoWidget->player()->play();
}

void SlideVideo::slotPlayerStateChanged(QAVPlayer::State newState)
{
    if (newState == QAVPlayer::PlayingState)
    {
        // Nothing to do.
    }
}

void SlideVideo::slotMediaStatusChanged(QAVPlayer::MediaStatus newStatus)
{
    switch (newStatus)
    {
        case QAVPlayer::EndOfMedia:
        {
            Q_EMIT signalVideoFinished();

            break;
        }

        case QAVPlayer::LoadedMedia:
        {
            slotPlayingStateChanged();

            Q_EMIT signalVideoLoaded(true);

            QTimer::singleShot(250, this, SLOT(slotPlayingStateChanged()));

            break;
        }

        case QAVPlayer::InvalidMedia:
        {
            Q_EMIT signalVideoLoaded(false);

            break;
        }

        default:
        {
            break;
        }
    }
}

void SlideVideo::pause(bool b)
{
    if (!b && (d->videoWidget->player()->state() != QAVPlayer::PlayingState))
    {
        d->videoWidget->player()->play();
        return;
    }

    if (b && (d->videoWidget->player()->state() != QAVPlayer::PausedState))
    {
       d->videoWidget->player()->pause();
    }
}

void SlideVideo::stop()
{
    d->videoWidget->player()->stop();
    d->videoWidget->player()->setSource(QString());
}

void SlideVideo::forward()
{
    if (d->videoWidget->player()->isSeekable())
    {
        qint64 step   = d->videoWidget->player()->duration() * 10 / 100;
        qint64 stepTo = d->videoWidget->player()->position() + step;

        if (stepTo > d->videoWidget->player()->duration())
        {
            d->videoWidget->player()->seek(d->videoWidget->player()->duration());
        }
        else
        {
            d->videoWidget->player()->seek(stepTo);
        }
    }
}

void SlideVideo::backward()
{
    if (d->videoWidget->player()->isSeekable())
    {
        qint64 step   = d->videoWidget->player()->duration() * 10 / 100;
        qint64 stepTo = d->videoWidget->player()->position() - step;

        if (stepTo < 0)
        {
            d->videoWidget->player()->seek(0);
        }
        else
        {
            d->videoWidget->player()->seek(stepTo);
        }
    }
}

void SlideVideo::slotVolumeChanged(int volume)
{
    d->videoWidget->audioOutput()->setVolume((qreal)volume / 100.0);
}

void SlideVideo::slotPositionChanged(int position)
{
    if (d->videoWidget->player()->isSeekable())
    {
        d->videoWidget->player()->seek((qint64)position);
    }
}

void SlideVideo::slotHandlePlayerError(QAVPlayer::Error /*err*/, const QString& message)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "QtAVPlayer Error: " << message;
}

void SlideVideo::slotPlayingStateChanged()
{
    int rotate = d->videoWidget->videoMediaOrientation();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Video orientation from QtAVPlayer:"
                                 << rotate;

    // cppcheck-suppress duplicateConditionalAssign
    if (rotate != d->videoWidget->videoItemOrientation())
    {
        // cppcheck-suppress duplicateConditionalAssign
        rotate = d->videoWidget->videoItemOrientation();
    }

    d->videoWidget->setVideoItemOrientation(rotate);
}

} // namespace Digikam

#include "moc_slidevideo.cpp"
