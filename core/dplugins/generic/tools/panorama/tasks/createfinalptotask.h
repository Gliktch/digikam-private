/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-05
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSharedPointer>

// Local includes

#include "panotask.h"
#include "ptotype.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CreateFinalPtoTask : public PanoTask
{
public:

    explicit CreateFinalPtoTask(const QString& workDirPath,
                                QSharedPointer<const PTOType> ptoData,
                                QUrl& finalPtoUrl,
                                const QRect& crop);
    ~CreateFinalPtoTask() override = default;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    PTOType     ptoData;
    QUrl&       finalPtoUrl;
    const QRect crop;

private:

    /// @note disabled
    CreateFinalPtoTask() = delete;

    Q_DISABLE_COPY(CreateFinalPtoTask)
};

} // namespace DigikamGenericPanoramaPlugin
