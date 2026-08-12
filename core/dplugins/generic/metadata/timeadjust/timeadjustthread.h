/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust thread.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QMap>
#include <QUrl>

// Local includes

#include "dinfointerface.h"
#include "actionthreadbase.h"
#include "timeadjustcontainer.h"

using namespace Digikam;

namespace DigikamGenericTimeAdjustPlugin
{

class TimeAdjustThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit TimeAdjustThread(QObject* const parent, DInfoInterface* const iface);
    ~TimeAdjustThread()                                       override;

    void setUpdatedDates(const QMap<QUrl, int>& itemsMap);
    void setPreviewDates(const QMap<QUrl, int>& itemsMap);
    void setSettings(const TimeAdjustContainer& settings);

    /** Read the Used Timestamps for the url
     */
    QDateTime readTimestamp(const QUrl& url) const;
    int       indexForUrl(const QUrl& url)   const;

Q_SIGNALS:

    void signalProcessStarted(const QUrl&);
    void signalDateTimeForUrl(const QUrl&, const QDateTime&, bool);
    void signalPreviewReady(const QUrl&, const QDateTime&, const QDateTime&);
    void signalProcessEnded(const QUrl&, const QDateTime&, const QDateTime&, int);

public Q_SLOTS:

    void slotSrcTimestampChanged();

private:

    /** Called by readTimestamp() to get host timestamps
     */
    QDateTime readApplicationTimestamp(const QUrl& url) const;

    /** Called by readTimestamp() to get file name timestamp
     */
    QDateTime readFileNameTimestamp(const QUrl& url)    const;

    /** Called by readTimestamp() to get file timestamp
     */
    QDateTime readFileTimestamp(const QUrl& url)        const;

    /** Called by readTimestamp() to get file metadata timestamp
     */
    QDateTime readMetadataTimestamp(const QUrl& url)    const;

public:

    class Private;

private:

    Private* const d = nullptr;
};

} // namespace DigikamGenericTimeAdjustPlugin
