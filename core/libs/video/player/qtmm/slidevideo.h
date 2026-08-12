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

#pragma once

// Qt includes

#include <QWidget>
#include <QEvent>
#include <QUrl>
#include <QMediaPlayer>

// Local includes

#include "dinfointerface.h"
#include "digikam_export.h"

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

    void slotPlayerStateChanged(QMediaPlayer::PlaybackState newState);
    void slotMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void slotHandlePlayerError(QMediaPlayer::Error, const QString&);
    void slotNativeSizeChanged();
    void slotMetaDataChanged();

private:

    void resizeEvent(QResizeEvent*) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
