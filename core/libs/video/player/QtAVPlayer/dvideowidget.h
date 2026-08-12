/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-14
 * Description : QtAVPlayer video integration class.
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMediaObject>
#include <QFrame>
#include <QVideoFrame>
#include <QGraphicsView>

// QtAVPlayer includes

#include "qavplayer.h"
#include "qavvideoframe.h"
#include "qavaudioframe.h"
#include "qavaudiooutput.h"

namespace Digikam
{

class Q_DECL_HIDDEN DVideoWidget : public QFrame
{
    Q_OBJECT

public:

    explicit DVideoWidget(QWidget* const parent);
    ~DVideoWidget();

    QAVPlayer*      player()      const;
    QAVAudioOutput* audioOutput() const;
    QVideoFrame videoFrame()      const;
    QGraphicsView* view()         const;

    int videoMediaOrientation()   const;

    void setVideoItemOrientation(int orientation);
    int videoItemOrientation()    const;

    void adjustVideoSize();

Q_SIGNALS:

    void positionChanged(qint64);

private Q_SLOTS:

    void slotAudioFrame(const QAVAudioFrame& frame);
    void slotVideoFrame(const QAVVideoFrame& frame);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
