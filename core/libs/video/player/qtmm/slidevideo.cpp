/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-22
 * Description : Slideshow video viewer based on QtMultimedia
 *
 * SPDX-FileCopyrightText: 2014-2025 Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QTransform>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QAudioOutput>
#include <QMediaMetaData>

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

    QUrl                 currentUrl;

    QGraphicsScene*      videoScene       = nullptr;
    QGraphicsView*       videoView        = nullptr;
    QGraphicsVideoItem*  videoItem        = nullptr;
    QMediaPlayer*        player           = nullptr;
    QAudioOutput*        audio            = nullptr;

    int                  videoOrientation = 0;

public:

    void adjustVideoSize()
    {
        videoItem->resetTransform();

        QSizeF nativeSize    = videoItem->nativeSize();
        int mediaOrientation = videoMediaOrientation();

        if (
            (nativeSize.width()  < 1.0) ||
            (nativeSize.height() < 1.0)
           )
        {
            return;
        }

        if (
            (mediaOrientation == 90) ||
            (mediaOrientation == 270)
           )
        {
            nativeSize.transpose();
        }

        double ratio = (nativeSize.width() /
                        nativeSize.height());

        if (videoView->width() > videoView->height())
        {
            QSizeF vsize(videoView->height() * ratio,
                         videoView->height());
            videoItem->setSize(vsize);
        }
        else
        {
            QSizeF vsize(videoView->width(),
                         videoView->width() / ratio);
            videoItem->setSize(vsize);
        }

        videoView->setSceneRect(0, 0, videoItem->size().width(),
                                      videoItem->size().height());

        QPointF center = videoItem->boundingRect().center();
        videoItem->setTransformOriginPoint(center);
        videoItem->setRotation(videoOrientation);

        videoView->fitInView(videoItem, Qt::KeepAspectRatio);
        videoView->centerOn(videoItem);
        videoView->raise();
    };

    int videoMediaOrientation() const
    {
        int orientation = 0;
        QVariant val    = player->metaData().value(QMediaMetaData::Orientation);

        if (!val.isNull())
        {
            orientation = val.toInt();
        }

        return orientation;
    };

    void setVideoItemOrientation(int orientation)
    {
        videoOrientation = orientation;
        adjustVideoSize();
    };
};

SlideVideo::SlideVideo(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setMouseTracking(true);

    d->videoScene  = new QGraphicsScene(this);
    d->videoView   = new QGraphicsView(d->videoScene, this);
    d->videoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->videoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->videoView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    d->videoView->setFrameShape(QFrame::NoFrame);
    d->videoItem   = new QGraphicsVideoItem();
    d->player      = new QMediaPlayer(this);
    d->audio       = new QAudioOutput(this);
    d->player->setAudioOutput(d->audio);
    d->player->setVideoOutput(d->videoItem);
    d->videoScene->addItem(d->videoItem);

    d->videoItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
    d->videoView->setMouseTracking(true);

    QVBoxLayout* const vlay = new QVBoxLayout(this);
    vlay->addWidget(d->videoView, 100);
    vlay->setContentsMargins(QMargins());

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Media Player Settings"));
    int volume                = group.readEntry("Volume", 50);

    d->audio->setVolume(volume / 100.0F);

    // --------------------------------------------------------------------------

    connect(d->player, SIGNAL(playbackStateChanged(QMediaPlayer::PlaybackState)),
            this, SLOT(slotPlayerStateChanged(QMediaPlayer::PlaybackState)));

    connect(d->player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)));

    connect(d->player, SIGNAL(positionChanged(qint64)),
            this, SIGNAL(signalVideoPosition(qint64)));

    connect(d->player, SIGNAL(durationChanged(qint64)),
            this, SIGNAL(signalVideoDuration(qint64)));

    connect(d->player, SIGNAL(errorOccurred(QMediaPlayer::Error,QString)),
            this, SLOT(slotHandlePlayerError(QMediaPlayer::Error,QString)));

    connect(d->videoItem, SIGNAL(nativeSizeChanged(QSizeF)),
            this, SLOT(slotNativeSizeChanged()));

    connect(d->player, SIGNAL(metaDataChanged()),
            this, SLOT(slotMetaDataChanged()));

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
    d->player->stop();

    int orientation = 0;
    d->currentUrl   = url;

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
            d->videoOrientation = 90;
            break;
        }

        case MetaEngine::ORIENTATION_ROT_180:
        {
            d->videoOrientation = 180;
            break;
        }

        case MetaEngine::ORIENTATION_ROT_270:
        {
            d->videoOrientation = 270;
            break;
        }

        default:
        {
            d->videoOrientation = 0;
            break;
        }
    }

    Q_EMIT signalVideoVolume(d->audio->volume() * 100.0F);

    d->player->setSource(url);
    d->player->play();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Slide video with QtMultimedia started:" << d->player->source();
}

void SlideVideo::slotPlayerStateChanged(QMediaPlayer::PlaybackState newState)
{
    if (newState == QMediaPlayer::PlayingState)
    {
        // Nothing to do.
    }
}

void SlideVideo::slotMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status)
    {
        case QMediaPlayer::EndOfMedia:
        {
            Q_EMIT signalVideoFinished();

            break;
        }

        case QMediaPlayer::LoadedMedia:
        {
            Q_EMIT signalVideoLoaded(true);

            break;
        }

        case QMediaPlayer::InvalidMedia:
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

void SlideVideo::slotHandlePlayerError(QMediaPlayer::Error, const QString& str)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "QtMultimedia Error: " << str;
}

void SlideVideo::slotNativeSizeChanged()
{
    d->adjustVideoSize();
}

void SlideVideo::slotMetaDataChanged()
{
    int rotate = d->videoMediaOrientation();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Found video orientation with QtMultimedia:"
                                 << rotate;

    rotate     = (-rotate) + d->videoOrientation;

    if      (rotate > 270)
    {
        rotate = 0;
    }
    else if (rotate < 0)
    {
        rotate = 270;
    }

    d->setVideoItemOrientation(rotate);
}

void SlideVideo::pause(bool b)
{
    if (!b && !d->player->isPlaying())
    {
        d->player->play();
        return;
    }

    d->player->pause();
}

void SlideVideo::stop()
{
    d->player->stop();
    d->player->setSource(QUrl());
}

void SlideVideo::forward()
{
    if (d->player->isSeekable())
    {
        qint64 step   = d->player->duration() * 10 / 100;
        qint64 stepTo = d->player->position() + step;

        if (stepTo > d->player->duration())
        {
            d->player->setPosition(d->player->duration());
        }
        else
        {
            d->player->setPosition(stepTo);
        }
    }
}

void SlideVideo::backward()
{
    if (d->player->isSeekable())
    {
        qint64 step   = d->player->duration() * 10 / 100;
        qint64 stepTo = d->player->position() - step;

        if (stepTo < 0)
        {
            d->player->setPosition(0);
        }
        else
        {
            d->player->setPosition(stepTo);
        }
    }
}

void SlideVideo::slotVolumeChanged(int volume)
{
    d->audio->setVolume(volume / 100.0F);
}

void SlideVideo::slotPositionChanged(int position)
{
    if (d->player->isSeekable())
    {
        d->player->setPosition((qint64)position);
    }
}

void SlideVideo::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    d->adjustVideoSize();
}

} // namespace Digikam

#include "moc_slidevideo.cpp"
