/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-09
 * Description : resize image job.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QMutex>
#include <QUrl>

// Local includes

#include "actionthreadbase.h"
#include "mailsettings.h"

using namespace Digikam;

namespace DigikamGenericSendByMailPlugin
{

class ImageResizeJob : public ActionJob
{
    Q_OBJECT

public:

    explicit ImageResizeJob(int* count = nullptr);
    ~ImageResizeJob() override = default;

public:

    QUrl          m_orgUrl;
    QString       m_destName;
    MailSettings* m_settings  = nullptr;
    int*          m_count     = nullptr;

Q_SIGNALS:

    void startingResize(const QUrl& orgUrl);
    void finishedResize(const QUrl& orgUrl, const QUrl& emailUrl, int percent);
    void failedResize(const QUrl& orgUrl, const QString& errString, int percent);

private:

    void run() override;
    bool imageResize(MailSettings* const settings,
                     const QUrl& orgUrl,
                     const QString& destName,
                     QString& err);

private:

    QMutex m_mutex;

private:

    /// @note disabled
    explicit ImageResizeJob(QObject*) = delete;
};

} // namespace DigikamGenericSendByMailPlugin
