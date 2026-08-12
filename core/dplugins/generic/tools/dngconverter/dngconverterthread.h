/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : DNG Converter threads manager
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "actionthreadbase.h"
#include "dngconverteractions.h"

using namespace Digikam;

namespace DigikamGenericDNGConverterPlugin
{

class DNGConverterActionData;

class DNGConverterActionThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit DNGConverterActionThread(QObject* const parent);
    ~DNGConverterActionThread() override;

    void setBackupOriginalRawFile(bool b);
    void setCompressLossLess(bool b);
    void setUpdateFileDate(bool b);
    void setPreviewMode(int mode);

    void identifyRawFile(const QUrl& url);
    void identifyRawFiles(const QList<QUrl>& urlList);

    void processRawFile(const QUrl& url);
    void processRawFiles(const QList<QUrl>& urlList);

    void cancel()               override;

Q_SIGNALS:

    void signalStarting(const DigikamGenericDNGConverterPlugin::DNGConverterActionData& ad);
    void signalFinished(const DigikamGenericDNGConverterPlugin::DNGConverterActionData& ad);

    /**
     * Signal to emit to sub-tasks to cancel processing.
     */
    void signalCancelDNGConverterTask();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericDNGConverterPlugin
