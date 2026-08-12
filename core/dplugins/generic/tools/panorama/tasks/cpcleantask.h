/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "commandtask.h"

namespace DigikamGenericPanoramaPlugin
{
class CpCleanTask : public CommandTask
{
public:

    explicit CpCleanTask(const QString& workDirPath,
                         const QUrl& input,
                         QUrl& cpCleanPtoUrl,
                         const QString& cpCleanPath);
    ~CpCleanTask() override = default;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    QUrl&       cpCleanPtoUrl;
    const QUrl& cpFindPtoUrl;

private:

    /// @note disabled
    CpCleanTask() = delete;

    Q_DISABLE_COPY(CpCleanTask)
};

} // namespace DigikamGenericPanoramaPlugin
