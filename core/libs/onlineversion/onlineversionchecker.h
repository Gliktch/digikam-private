/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-12-31
 * Description : Online version checker
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QDateTime>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT OnlineVersionChecker : public QObject
{
    Q_OBJECT

public:

    explicit OnlineVersionChecker(QObject* const parent, bool checkPreRelease = false);
    ~OnlineVersionChecker();

    void setCurrentVersion(const QString& version);
    void setCurrentBuildDate(const QDateTime& dt);

    void checkForNewVersion();
    void cancelCheck();

    QString preReleaseFileName() const;

    /**
     * Return true if the system and architecture are supported by the bundle workflow.
     * 'arch'      is the relevant prefix for the bundle architecture.
     * 'ext'       is the relevant bundle file extension.
     * 'qtVersion' is the relevant version of Qt used in the bundle file-name.
     * 'dir'       is the subdirectory if any to get the bundle file.
     */
    static bool bundleProperties(QString& arch, QString& ext,
                                 QString& qtVersion, QString& dir);

    /**
     * Return the last date as string when have been performed a check for new version.
     */
    static QString lastCheckDate();

Q_SIGNALS:

    void signalNewVersionAvailable(const QString& version);
    void signalNewVersionCheckError(const QString& error);

private Q_SLOTS:

    void slotDownloadFinished(QNetworkReply* reply);

private:

    void download(const QUrl& url);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
