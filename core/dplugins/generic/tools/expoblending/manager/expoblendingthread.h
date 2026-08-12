/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>

// Local includes

#include "metaengine.h"
#include "enfusesettings.h"
#include "expoblendingactions.h"

class QProcess;

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class ExpoBlendingActionData;

class ExpoBlendingThread : public QThread
{
    Q_OBJECT

public:

    explicit ExpoBlendingThread(QObject* const parent);
    ~ExpoBlendingThread()                             override;

    void setEnfuseVersion(const double version);
    void setPreProcessingSettings(bool align);
    void loadProcessed(const QUrl& url);
    void identifyFiles(const QList<QUrl>& urlList);
    void convertRawFiles(const QList<QUrl>& urlList);
    void preProcessFiles(const QList<QUrl>& urlList, const QString& alignPath);
    void enfusePreview(const QList<QUrl>& alignedUrls, const QUrl& outputUrl,
                       const EnfuseSettings& settings, const QString& enfusePath);
    void enfuseFinal(const QList<QUrl>& alignedUrls, const QUrl& outputUrl,
                     const EnfuseSettings& settings, const QString& enfusePath);

    void cancel();

    /**
     * Clean up all temporary results produced so far.
     */
    void cleanUpResultFiles();

Q_SIGNALS:

    void signalStarting(const DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData& ad);
    void signalFinished(const DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData& ad);

private:

    void    run()                                     override;

    bool    preProcessingMultithreaded(const QUrl& url);
    bool    startPreProcessing(const QList<QUrl>& inUrls, bool  align, const QString& alignPath, QString& errors);
    bool    computePreview(const QUrl& inUrl, QUrl& outUrl);
    bool    convertRaw(const QUrl& inUrl, QUrl& outUrl);

    bool    startEnfuse(const QList<QUrl>& inUrls, QUrl& outUrl,
                        const EnfuseSettings& settings,
                        const QString& enfusePath, QString& errors);

    QString getProcessError(QProcess& proc)     const;

    float   getAverageSceneLuminance(const QUrl& url);
    bool    getXmpRational(const char* xmpTagName, long& num, long& den, MetaEngine* const meta);

public:

    class Private;

private:

    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
