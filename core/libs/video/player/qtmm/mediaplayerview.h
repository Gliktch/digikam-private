/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed QtMultimedia media player.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QIODevice>
#include <QSharedPointer>
#include <QStackedWidget>
#include <QEvent>
#include <QUrl>
#include <QMediaPlayer>
#include <QAction>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "dinfointerface.h"

namespace Digikam
{

class DIGIKAM_EXPORT MediaPlayerView : public QStackedWidget
{
    Q_OBJECT

public:

    explicit MediaPlayerView(QWidget* const parent);
    ~MediaPlayerView() override;

    void setCurrentItem(const QUrl& url   = QUrl(),
                        bool  hasPrevious = false,
                        bool  hasNext     = false);

    void setCurrentItem(const QSharedPointer<QIODevice>& videoData,
                        const QUrl& sourceUrl,
                        bool hasPrevious, bool hasNext);

    void setInfoInterface(DInfoInterface* const iface);
    void escapePreview();
    void reload();

    void setVideoOrientation(int degrees);
    void setMotionPhotoMode(bool enabled);
    void setMotionPhotoSourceItem(const QUrl& sourceUrl);
    bool isMotionPhotoMode() const;

    /**
     * @brief plug an extra widget at end of the toolbar.
     */
    void setToolbarExtraWidget(QWidget* const extra);

    /**
     * @brief turn off/on toolbar visibility.
     */
    void setToolbarVisible(bool b);

    /**
     * @brief plug an On Screen Display widget over the player.
     */
    void setOsdWidget(QWidget* const osd);

    /**
     * @return the list of action available in the toolbar.
     */
    QList<QAction*> actionsList() const;

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalEscapePreview();

public Q_SLOTS:

    void slotEscapePressed();
    void slotRotateVideoLeft();
    void slotRotateVideoRight();

private Q_SLOTS:

    void slotPlayerStateChanged(QMediaPlayer::PlaybackState newState);
    void slotMediaStatusChanged(QMediaPlayer::MediaStatus newStatus);
    void slotHandlePlayerError(QMediaPlayer::Error, const QString&);
    void slotCreateAudioOutputMenu();
    void slotNativeSizeChanged();
    void slotMetaDataChanged();
    void slotThemeChanged();

    /// Slidebar slots
    void slotPositionChanged(qint64 position);
    void slotDurationChanged(qint64 duration);
    void slotPlaybackRate(QAction* action);
    void slotAudioChanged(QAction* action);
    void slotVolumeChanged(int volume);
    void slotLoopToggled(bool loop);
    void slotPosition(int position);
    void slotFrameBackward();
    void slotFrameForward();
    void slotPausePlay();
    void slotCapture();

private:

    int  previewMode();
    void setPreviewMode(int mode);

protected:

    bool eventFilter(QObject* watched, QEvent* event) override;

private:

    class Private;
    Private* const d = nullptr;
};

}  // namespace Digikam
