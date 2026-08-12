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

#pragma once

// Qt includes

#include <QWidget>
#include <QUrl>

// Local includes

#include "slideshowsettings.h"

class QEvent;

namespace DigikamGenericSlideShowPlugin
{

class SlideShowLoader;
class SlideToolBar;

class SlideOSD : public QWidget
{
    Q_OBJECT

public:

    explicit SlideOSD(SlideShowSettings* const settings, SlideShowLoader* const parent);
    ~SlideOSD()                                 override;

    void setCurrentUrl(const QUrl& url);

    void pause(bool b);
    void video(bool b);
    bool isPaused()                 const;
    bool isUnderMouse()             const;
    void setLoadingReady(bool b);
    void showVideoIndicator(bool b);

    SlideToolBar* toolBar()         const;
    QSize slideShowSize()           const;


Q_SIGNALS:

    void signalVideoPosition(int position);
    void signalVideoVolume(int volume);

public Q_SLOTS:

    void slotPositionChanged(qint64 position);
    void slotDurationChanged(qint64 duration);
    void slotVolumeChanged(int volume);

protected:

    bool eventFilter(QObject* obj, QEvent* ev)  override;

private Q_SLOTS:

    void slotUpdateSettings();
    void slotProgressTimer();
    void slotStart();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericSlideShowPlugin
