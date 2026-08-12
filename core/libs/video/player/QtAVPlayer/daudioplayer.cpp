/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-01-14
 * Description : QtAVPlayer audio integration class.
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "daudioplayer.h"

// Qt includes

#include <QMediaService>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DAudioPlayer::Private
{

public:

    Private() = default;

public:

    QAVPlayer*      player      = nullptr;
    QAVAudioOutput* audioOutput = nullptr;
};

DAudioPlayer::DAudioPlayer(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->player      = new QAVPlayer(this);

    d->audioOutput = new QAVAudioOutput(this);

    connect(d->player, &QAVPlayer::audioFrame,
            this, &DAudioPlayer::slotAudioFrame,
            Qt::DirectConnection);
}

DAudioPlayer::~DAudioPlayer()
{
    delete d;
}

QAVPlayer* DAudioPlayer::player() const
{
    return d->player;
}

QAVAudioOutput* DAudioPlayer::audioOutput() const
{
    return d->audioOutput;
}

void DAudioPlayer::slotAudioFrame(const QAVAudioFrame& frame)
{
    d->audioOutput->play(frame);
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Audio position:" << d->player->position();
*/
    qCDebug(DIGIKAM_GENERAL_LOG) << "Frame position:" << frame.pts();

    Q_EMIT positionChanged((qint64)(frame.pts() * 1000));
}

void DAudioPlayer::pause()
{
    d->player->pause();
}

void DAudioPlayer::play()
{
    d->player->play();
}

void DAudioPlayer::stop()
{
    d->player->stop();
    d->player->setSource(QString());
}

void DAudioPlayer::setSource(const QUrl& url)
{
    d->player->setSource(url.toLocalFile());
}

qint64 DAudioPlayer::position() const
{
    return d->player->position();
}

qint64 DAudioPlayer::duration() const
{
    return d->player->duration();
}

void DAudioPlayer::setVolume(qreal v)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "setVolume" << v;
    d->audioOutput->setVolume(v);
}

QAVPlayer::State DAudioPlayer::state() const
{
    return d->player->state();
}

QAVPlayer::MediaStatus DAudioPlayer::mediaStatus() const
{
    return d->player->mediaStatus();
}

} // namespace Digikam

#include "moc_daudioplayer.cpp"
