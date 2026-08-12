/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-01-14
 * Description : QtAVPlayer video integration class.
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dvideowidget.h"

// Qt includes

#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QVBoxLayout>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DVideoWidget::Private
{

public:

    Private() = default;

public:

    QGraphicsScene*      videoScene         = nullptr;
    QGraphicsView*       videoView          = nullptr;
    QGraphicsVideoItem*  videoItem          = nullptr;

    QAVPlayer*           player             = nullptr;
    QAVAudioOutput*      audioOutput        = nullptr;

    QVideoFrame          videoFrame;
    QMutex               mutex;

    int                  videoOrientation   = 0;
};

DVideoWidget::DVideoWidget(QWidget* const parent)
    : QFrame(parent),
      d     (new Private)
{
    setMouseTracking(true);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setLineWidth(1);

    d->videoScene  = new QGraphicsScene(parent);
    d->videoView   = new QGraphicsView(d->videoScene);
    d->videoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->videoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->videoView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    d->videoView->setFrameShape(QFrame::NoFrame);
    d->videoItem   = new QGraphicsVideoItem();
    d->videoScene->addItem(d->videoItem);
    d->videoItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
    d->videoView->setMouseTracking(true);

    QVBoxLayout* const vbox2 = new QVBoxLayout(this);
    vbox2->addWidget(d->videoView);
    vbox2->setContentsMargins(QMargins(0, 0, 0, 9));
    vbox2->setSpacing(0);

    d->player      = new QAVPlayer(this);

    d->audioOutput = new QAVAudioOutput(this);

    connect(d->player, &QAVPlayer::audioFrame,
            this, &DVideoWidget::slotAudioFrame,
            Qt::DirectConnection);

    connect(d->player, &QAVPlayer::videoFrame,
            this, &DVideoWidget::slotVideoFrame,
            Qt::DirectConnection);
}

DVideoWidget::~DVideoWidget()
{
    delete d;
}

QAVPlayer* DVideoWidget::player() const
{
    return d->player;
}

QGraphicsView* DVideoWidget::view() const
{
    return d->videoView;
}

QAVAudioOutput* DVideoWidget::audioOutput() const
{
    return d->audioOutput;
}

QVideoFrame DVideoWidget::videoFrame() const
{
    QMutexLocker lock(&d->mutex);

    return d->videoFrame;
}

void DVideoWidget::slotAudioFrame(const QAVAudioFrame& frame)
{
    d->audioOutput->play(frame);
}

void DVideoWidget::slotVideoFrame(const QAVVideoFrame& frame)
{
    if (!d->videoItem->videoSurface())
    {
        return;
    }

    {
        QMutexLocker lock(&d->mutex);

        d->videoFrame = frame.convertTo(AV_PIX_FMT_RGB32);
    }

    if (
        !d->videoItem->videoSurface()->isActive() ||
        (d->videoItem->videoSurface()->surfaceFormat().frameSize() != d->videoFrame.size())
       )
    {
        QVideoSurfaceFormat f(d->videoFrame.size(), d->videoFrame.pixelFormat(), d->videoFrame.handleType());
        d->videoItem->videoSurface()->start(f);
    }

    if (d->videoItem->videoSurface()->isActive())
    {
         d->videoItem->videoSurface()->present(d->videoFrame);
    }

    Q_EMIT positionChanged(d->player->position());
}

int DVideoWidget::videoMediaOrientation() const
{
    int orientation = 0;

    if (d->player->availableVideoStreams().count() > 0)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Video Metadata from" << d->player->source();
        qCDebug(DIGIKAM_GENERAL_LOG) << "---";
        qCDebug(DIGIKAM_GENERAL_LOG) << "Video Streams Available:" << d->player->availableVideoStreams().count();

        const auto list = d->player->availableVideoStreams();

        for (const QAVStream& vstream : list)
        {
            QMap<QString, QString> vals = vstream.metadata();

            for (QMap<QString, QString>::const_iterator it = vals.constBegin() ; it != vals.constEnd() ; ++it)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << it.key() << it.value();
            }
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "---";

        QList<QAVStream> vstream = d->player->currentVideoStreams();

        if (!vstream.isEmpty())
        {
            QMap<QString, QString> vals = vstream.first().metadata();

            if (!vals.isEmpty())
            {
                if (vals.contains(QLatin1String("rotate")))
                {
                    orientation = vals[QLatin1String("rotate")].toInt();
                }
            }
        }
    }

    return orientation;
}

void DVideoWidget::adjustVideoSize()
{
    d->videoItem->resetTransform();

    QSizeF nativeSize = d->videoItem->nativeSize();

    if (
        (nativeSize.width()  < 1.0) ||
        (nativeSize.height() < 1.0)
       )
    {
        nativeSize = QSizeF(352.0, 288.0);
    }

    double ratio = (nativeSize.width() /
                    nativeSize.height());

    if (d->videoView->width() > d->videoView->height())
    {
        QSizeF vsize(d->videoView->height() * ratio,
                     d->videoView->height());
        d->videoItem->setSize(vsize);
    }
    else
    {
        QSizeF vsize(d->videoView->width(),
                     d->videoView->width() / ratio);
        d->videoItem->setSize(vsize);
    }

    d->videoView->setSceneRect(0, 0, d->videoItem->size().width(),
                                     d->videoItem->size().height());

    QPointF center = d->videoItem->boundingRect().center();
    d->videoItem->setTransformOriginPoint(center);
    d->videoItem->setRotation(d->videoOrientation);

    d->videoView->fitInView(d->videoItem, Qt::KeepAspectRatio);
    d->videoView->centerOn(d->videoItem);
    d->videoView->raise();
}

void DVideoWidget::setVideoItemOrientation(int orientation)
{
    d->videoOrientation = orientation;
    adjustVideoSize();
}

int DVideoWidget::videoItemOrientation() const
{
    return d->videoOrientation;
}

} // namespace Digikam

#include "moc_dvideowidget.cpp"
