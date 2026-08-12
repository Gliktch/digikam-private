/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-24
 * Description : DNG Converter thread
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

class DNGConverterTask : public ActionJob
{
    Q_OBJECT

public:

    explicit DNGConverterTask(QObject* const parent,
                              const QUrl& url,
                              const DNGConverterAction& action);
    ~DNGConverterTask() override;

    void setBackupOriginalRawFile(bool b);
    void setCompressLossLess(bool b);
    void setUpdateFileDate(bool b);
    void setPreviewMode(int mode);

Q_SIGNALS:

    void signalStarting(const DigikamGenericDNGConverterPlugin::DNGConverterActionData& ad);
    void signalFinished(const DigikamGenericDNGConverterPlugin::DNGConverterActionData& ad);

public Q_SLOTS:

    void slotCancel();

protected:

    void run()          override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericDNGConverterPlugin

