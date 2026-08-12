/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a widget to play audio track.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "audplayerwdg.h"

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QTime>
#include <QUrl>
#include <QMessageBox>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

#   include <QAudioOutput>

#endif

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dexpanderbox.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN AudPlayerWdg::Private
{
public:

    Private() = default;

public:

    QString  afile;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QMediaPlayer* mediaObject = nullptr;

#else

    DAudioPlayer* mediaObject = nullptr;

#endif

    QPushButton* playPauseBtn = nullptr;
    QPushButton* stopBtn      = nullptr;

    QLabel* elapsedTimeLabel  = nullptr;
    QLabel* totalTimeLabel    = nullptr;

    QSlider* volumeWidget     = nullptr;
};

AudPlayerWdg::AudPlayerWdg(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->mediaObject = new QMediaPlayer(this);
    d->mediaObject->setAudioOutput(new QAudioOutput(this));

    connect(d->mediaObject, SIGNAL(playbackStateChanged(QMediaPlayer::PlaybackState)),
            this, SLOT(slotPlayerStateChanged(QMediaPlayer::PlaybackState)));

    connect(d->mediaObject, SIGNAL(errorOccurred(QMediaPlayer::Error,QString)),
            this, SLOT(slotPlayerError(QMediaPlayer::Error,QString)));

    connect(d->mediaObject, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotTimeUpdaterTimeout(qint64)));

#else

    d->mediaObject = new DAudioPlayer(this);

    connect(d->mediaObject->player(), SIGNAL(stateChanged(QAVPlayer::State)),
            this, SLOT(slotPlayerStateChanged(QAVPlayer::State)));

    connect(d->mediaObject->player(), SIGNAL(errorOccurred(QAVPlayer::Error,QString)),
            this, SLOT(slotPlayerError(QAVPlayer::Error,QString)));

    connect(d->mediaObject, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotTimeUpdaterTimeout(qint64)));

#endif

    d->playPauseBtn = new QPushButton(QIcon::fromTheme(QLatin1String("media-playback-start")), QString(), this);
    d->stopBtn      = new QPushButton(QIcon::fromTheme(QLatin1String("media-playback-stop")),  QString(), this);

    d->elapsedTimeLabel          = new QLabel(this);
    DLineWidget* const separator = new DLineWidget(Qt::Vertical, this);
    d->totalTimeLabel            = new QLabel(this);
    setZeroTime();

    QLabel* const soundLabel     = new QLabel(this);
    soundLabel->setPixmap(QIcon::fromTheme(QLatin1String("speaker")).pixmap(22, 22));

    d->volumeWidget              = new QSlider(Qt::Horizontal, this);
    d->volumeWidget->setRange(0, 100);
    d->volumeWidget->setValue(100);

    // ----------------------

    QGridLayout* const grid      = new QGridLayout(this);
    grid->setContentsMargins(QMargins(0, 0, 0, 0));
    grid->setSpacing(layoutSpacing());

    grid->addWidget(d->playPauseBtn,     0, 0, 1, 1);
    grid->addWidget(d->stopBtn,          0, 1, 1, 1);
    grid->addWidget(d->elapsedTimeLabel, 0, 2, 1, 1);
    grid->addWidget(separator,           0, 3, 1, 1);
    grid->addWidget(d->totalTimeLabel,   0, 4, 1, 1);
    grid->addWidget(soundLabel,          0, 5, 1, 1);
    grid->addWidget(d->volumeWidget,     0, 6, 1, 1);

    // ----------------------

    connect(d->playPauseBtn, SIGNAL(clicked()),
            this, SLOT(slotPlay()));

    connect(d->stopBtn, SIGNAL(clicked()),
            this, SLOT(slotStop()));

    connect(d->volumeWidget, SIGNAL(valueChanged(int)),
            this, SLOT(slotSetVolume(int)));

    slotSetVolume(d->volumeWidget->value());
}

AudPlayerWdg::~AudPlayerWdg()
{
    delete d;
}

void AudPlayerWdg::setAudioFile(const QString& afile)
{
    d->afile = afile;
    slotStop();
}

void AudPlayerWdg::slotSetVolume(int v)
{
    if (d->mediaObject->audioOutput())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        d->mediaObject->audioOutput()->setVolume(v / 100.0);

#else

        d->mediaObject->setVolume((qreal)v / 100.0);

#endif

    }
}

void AudPlayerWdg::slotPlay()
{
    if (!d->mediaObject)
    {
        qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Internal Media Object is null!";

        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (
        (d->mediaObject->playbackState() != QMediaPlayer::PlayingState) ||
        (d->mediaObject->playbackState() == QMediaPlayer::PausedState)
       )

#else

    if (
        (d->mediaObject->state() != QAVPlayer::PlayingState) ||
        (d->mediaObject->state() == QAVPlayer::PausedState)
       )

#endif

    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        if (d->mediaObject->playbackState() != QMediaPlayer::PlayingState)

#else

        if (d->mediaObject->state() != QAVPlayer::PlayingState)

#endif

        {
            d->mediaObject->setSource(QUrl::fromLocalFile(d->afile));
            d->mediaObject->play();
            setZeroTime();
        }
        else
        {
            d->mediaObject->pause();
        }
    }
    else
    {
        d->mediaObject->pause();
    }
}

void AudPlayerWdg::slotStop()
{
    if (!d->mediaObject)
    {
        qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Internal Media Object is null!";

        return;
    }

    d->mediaObject->stop();
    setZeroTime();
}

void AudPlayerWdg::slotTimeUpdaterTimeout(qint64 current)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (d->mediaObject->error() != QMediaPlayer::NoError)

#else

    if (d->mediaObject->mediaStatus() == QAVPlayer::InvalidMedia)

#endif

    {
        QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                             i18n("An error has occurred while playing: the media is not valid"));
        return;
    }

    int hours      = (int)(current  / (qint64)(60 * 60 * 1000));
    int mins       = (int)((current / (qint64)(60 * 1000)) - (qint64)(hours * 60));
    int secs       = (int)((current / (qint64)1000) - (qint64)(hours * 60 + mins * 60));
    QTime elapsedTime(hours, mins, secs);

    if (d->mediaObject->duration() > 0)
    {
        qint64 total  = d->mediaObject->duration();
        hours         = (int)(total  / (qint64)(60 * 60 * 1000));
        mins          = (int)((total / (qint64)(60 * 1000)) - (qint64)(hours * 60));
        secs          = (int)((total / (qint64)1000) - (qint64)(hours * 60 + mins * 60));
        QTime totalTime(hours, mins, secs);
        d->totalTimeLabel->setText(totalTime.toString(QLatin1String("H:mm:ss")));
    }

    d->elapsedTimeLabel->setText(elapsedTime.toString(QLatin1String("H:mm:ss")));
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

void AudPlayerWdg::slotPlayerError(QMediaPlayer::Error err, const QString& message)
{
    if (err != QMediaPlayer::NoError)
    {
        QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                             i18n("An error has occurred while playing: %1", message));
    }
}

void AudPlayerWdg::slotPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state)
    {
        case QMediaPlayer::PausedState:
        case QMediaPlayer::StoppedState:
        {
            d->playPauseBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
            break;
        }

        case QMediaPlayer::PlayingState:
        {
            d->playPauseBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
            break;
        }

        default:
        {
            break;
        }
    }
}

#else

void AudPlayerWdg::slotPlayerError(QAVPlayer::Error err, const QString& message)
{
    if (err != QAVPlayer::NoError)
    {
        QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                             i18n("An error has occurred while playing: %1", message));
    }
}

void AudPlayerWdg::slotPlayerStateChanged(QAVPlayer::State state)
{
    switch (state)
    {
        case QAVPlayer::PausedState:
        case QAVPlayer::StoppedState:
        {
            d->playPauseBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
            break;
        }

        case QAVPlayer::PlayingState:
        {
            d->playPauseBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
            break;
        }

        default:
        {
            break;
        }
    }
}

#endif

void AudPlayerWdg::setZeroTime()
{
    d->elapsedTimeLabel->setText(QTime(0, 0, 0).toString(QLatin1String("H:mm:ss")));
    d->totalTimeLabel->setText(QTime(0, 0, 0).toString(QLatin1String("H:mm:ss")));
}

} // namespace Digikam

#include "moc_audplayerwdg.cpp"
