/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-14
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "digikam_config.h"

// Qt includes

#include <QUrl>
#include <QKeyEvent>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)) && defined HAVE_QTMULTIMEDIA

// QtMultimedia includes

#   include <QMediaPlayer>

#else

// QtAVPlayer includes

#   include "daudioplayer.h"

#endif

// Local includes

#include "ui_presentationaudiowidget.h"

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;

class PresentationAudioWidget : public QWidget,
                                public Ui::PresentationAudioWidget
{
    Q_OBJECT

public:

    explicit PresentationAudioWidget(QWidget* const,
                                     const QList<QUrl>&,
                                     PresentationContainer* const);
    ~PresentationAudioWidget() override;

    void enqueue(const QList<QUrl>&);
    bool canHide() const;
    bool isPaused() const;
    void setPaused(bool);
    void keyPressEvent(QKeyEvent*) override;

public Q_SLOTS:

    void slotPlay();
    void slotStop();

Q_SIGNALS:

    void signalPlay();
    void signalPause();

private Q_SLOTS:

    void slotPrev();
    void slotNext();
    void slotTimeUpdaterTimeout(qint64);
    void slotError();
    void slotSetVolume(int);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)) && defined HAVE_QTMULTIMEDIA

    void slotMediaStateChanged(QMediaPlayer::MediaStatus);
    void slotPlayerStateChanged(QMediaPlayer::PlaybackState);
    void slotPlayerError(QMediaPlayer::Error);

#else

    void slotMediaStateChanged(QAVPlayer::MediaStatus);
    void slotPlayerStateChanged(QAVPlayer::State);
    void slotPlayerError(QAVPlayer::Error, const QString&);

#endif

private:

    void checkSkip();
    void setZeroTime();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
