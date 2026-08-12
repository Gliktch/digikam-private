/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-08
 * Description : Service menu operation methods
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QIcon>
#include <QString>
#include <QStringList>

// KDE includes

// Pragma directives to reduce warnings from KDE header files.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmismatched-tags"
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <kservice.h>

// Restore warnings
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_export.h"

namespace Digikam
{

// ----------------------------------------------------------------------------

class DIGIKAM_EXPORT DServiceInfo
{
public:

    DServiceInfo();
    DServiceInfo(const QString& _name,
                 const QString& _exec,
                 const QString& _icon,
                 const QString& _topt,
                 bool           _term);
    DServiceInfo(const DServiceInfo& other);
    ~DServiceInfo();

    bool isEmpty() const;

    DServiceInfo& operator=(const DServiceInfo& other);

public:

    QString name;
    QString exec;
    QString icon;
    QString topt;
    bool    term = false;
};

// ----------------------------------------------------------------------------

class DIGIKAM_EXPORT DServiceMenu
{
public:

//@{

    // Linux Services methods implemented in dservicemenu_linux.cpp

    /**
     * Linux only: open file urls with the service.
     */
    static bool runFiles(const KService::Ptr& service, const QList<QUrl>& urls);

    static bool runFiles(const DServiceInfo& serviceInfo, const QList<QUrl>& urls);

    /**
     * Linux only: open file urls with the application command.
     */
    static bool runFiles(const QString& appCmd,
                         const QList<QUrl>& urls,
                         const KService::Ptr& service = KService::Ptr(),
                         const DServiceInfo& serviceInfo = DServiceInfo());

    /**
     * Linux only: return list of service available on desktop to open files.
     */
    static KService::List servicesForOpenWith(const QList<QUrl>& urls);

    static QList<DServiceInfo> servicesForOpen(const QList<QUrl>& urls);

    /**
     * Return the QIcon depending on the operating system.
     */
    static QIcon getIconFromService(const DServiceInfo& sinfo);

private:

    static QStringList splitStringAtSpaces(const QString& string);

//@}

//@{

    // MacOS Application Bundles methods implemented in dservicemenu_mac.mm

#ifdef Q_OS_MAC

public:

    /**
     * Given a filename extension 'suffix', here's how to find all of the
     * applications known to the MacOS who can open files of that type.
     * Return a list of suitable MacOS bundle urls for 'suffix'.
     * First one is the default MacOS bundle application.
     */
    static QList<QUrl> MacApplicationForFileExtension(const QString& suffix);

    /**
     * Function to open your file urls with a specific MacOS application bundle url.
     * Return true if application can be started.
     */
    static bool MacOpenFilesWithApplication(const QList<QUrl>& fileUrls, const QUrl& appUrl);

    /**
     * Return a list of common MacOS Application bundles suitable for a list of files.
     * The function check all common Applications which can handle urls to open files.
     * Only common Applications are returned on the list.
     * The list can be empty if no common Application is found.
     */
    static QList<QUrl> MacApplicationsForFiles(const QList<QUrl>& files);

    /**
     * Return the MacOS Application bundle name based on url.
     */
    static QString MacApplicationBundleName(const QUrl& appUrl);

    /**
     * Return the MacOS Application bundle icon based on url.
     */
    static QIcon MacApplicationBundleIcon(const QUrl& appUrl, int size = 32);

#endif

//@}

};

} // namespace Digikam
