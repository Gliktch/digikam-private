/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-09
 * Description : resize image threads manager.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "actionthreadbase.h"
#include "mailsettings.h"

using namespace Digikam;

namespace DigikamGenericSendByMailPlugin
{

class ImageResizeThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit ImageResizeThread(QObject* const parent);
    ~ImageResizeThread() override;

    void resize(MailSettings* const settings);
    void cancel()        override;

Q_SIGNALS:

    void startingResize(const QUrl& orgUrl);
    void finishedResize(const QUrl& orgUrl, const QUrl& emailUrl, int percent);
    void failedResize(const QUrl& orgUrl, const QString& errString, int percent);

private:

    int* m_count = nullptr;    ///< Although it is private, it's address is passed to Task
};

} // namespace DigikamGenericSendByMailPlugin
