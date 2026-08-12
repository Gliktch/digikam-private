/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : file copy actions using threads.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "actionthreadbase.h"
#include "fccontainer.h"

using namespace Digikam;

namespace DigikamGenericFileCopyPlugin
{

class FCTask : public ActionJob
{
    Q_OBJECT

public:

    explicit FCTask(const QUrl& srcUrl,
                    const FCContainer& settings);
    ~FCTask()  override;

Q_SIGNALS:

    void signalUrlProcessed(const QUrl& from, const QUrl& to);

protected:

    void run() override;

private:

    /// @note disabled
    explicit FCTask(QObject*) = delete;

private:

    bool imageResize(const QString& orgPath, QUrl& destUrl);
    QUrl getUrlOrDelete(const QUrl& fileUrl) const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericFileCopyPlugin
