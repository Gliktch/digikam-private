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

#pragma once

// Qt includes

#include <QWidget>
#include <QEvent>
#include <QUrl>

// QtAVPlayer includes

#include "qavplayer.h"

// Local includes

#include "dinfointerface.h"
#include "digikam_export.h"
#include "dvideowidget.h"

namespace Digikam
{

class DIGIKAM_EXPORT SlideVideo : public QWidget
{
    Q_OBJECT

public:

    explicit SlideVideo(QWidget* const parent);
    ~SlideVideo() override;

    void setInfoInterface(DInfoInterface* const iface);
    void setCurrentUrl(const QUrl& url);
    void pause(bool);
    void stop();
    void forward();
    void backward();

Q_SIGNALS:

    void signalVideoLoaded(bool);
    void signalVideoFinished();

    void signalVideoPosition(qint64);
    void signalVideoDuration(qint64);
    void signalVideoVolume(int);

public Q_SLOTS:

    void slotPositionChanged(int position);
    void slotVolumeChanged(int volume);

private Q_SLOTS:

    void slotPlayerStateChanged(QAVPlayer::State newState);
    void slotMediaStatusChanged(QAVPlayer::MediaStatus newStatus);
    void slotHandlePlayerError(QAVPlayer::Error err, const QString&);
    void slotPlayingStateChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
