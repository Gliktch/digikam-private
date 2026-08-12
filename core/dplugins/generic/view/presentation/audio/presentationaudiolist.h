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

#include <QTime>
#include <QWidget>
#include <QString>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QUrl>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)) && defined HAVE_QTMULTIMEDIA

// QtMultimedia includes

#   include <QMediaPlayer>

#else

// QtAVPlayer includes

#   include "daudioplayer.h"

using namespace Digikam;

#endif

namespace DigikamGenericPresentationPlugin
{

class PresentationAudioListItem : public QObject,
                                  public QListWidgetItem
{
    Q_OBJECT

public:

    explicit PresentationAudioListItem(QListWidget* const parent,
                                       const QUrl& url);
    ~PresentationAudioListItem() override;

    QUrl    url()       const;
    QString artist()    const;
    QString title()     const;
    QTime   totalTime() const;
    void    setName(const QString& text);

Q_SIGNALS:

    void signalTotalTimeReady(const QUrl&, const QTime&);

private Q_SLOTS:

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)) && defined HAVE_QTMULTIMEDIA

    void slotMediaStateChanged(QMediaPlayer::MediaStatus status);
    void slotPlayerError(QMediaPlayer::Error);

#else

    void slotMediaStateChanged(QAVPlayer::MediaStatus status);
    void slotPlayerError(QAVPlayer::Error, const QString&);
    void slotDurationChanged(qint64 duration);

#endif

private:

    void showErrorDialog(const QString& err);

private:

    class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------------------

class PresentationAudioList : public QListWidget
{
    Q_OBJECT

public:

    explicit PresentationAudioList(QWidget* const parent = nullptr);

public:

    QList<QUrl> fileUrls();

Q_SIGNALS:

    void signalAddedDropItems(const QList<QUrl>& filesUrl);

protected:

    void dragEnterEvent(QDragEnterEvent*)   override;
    void dragMoveEvent(QDragMoveEvent*)     override;
    void dropEvent(QDropEvent*)             override;
};

} // namespace DigikamGenericPresentationPlugin
