/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : a class to manage Raw to Png conversion using threads
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "drawdecodersettings.h"
#include "actionthreadbase.h"

using namespace Digikam;

class RAWToPNGConverterThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit RAWToPNGConverterThread(QObject* const parent);
    ~RAWToPNGConverterThread() override;

    void convertRAWtoPNG(const QList<QUrl>& list,
                         const DRawDecoderSettings& settings,
                         int priority = 0);

Q_SIGNALS:

    void signalStarting(const QUrl& url);
    void signalFinished(const QUrl& url);
    void signalFailed(const QUrl& url, const QString& err);
    void signalProgress(const QUrl& url, int percent);

private Q_SLOTS:

    void slotJobDone();
    void slotJobProgress(int);
    void slotJobStarted();
};
