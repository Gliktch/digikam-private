/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-01-28
 * Description : Objective-C wrapper for open-with operations under MacOS
 *
 * SPDX-FileCopyrightText: 2021      by Robert Lindsay <robert dot lindsay at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dservicemenu.h"

// Qt include

#include <QString>
#include <QList>
#include <QUrl>
#include <QFileInfo>
#include <QIcon>
#include <QPixmap>
#include <QSize>

// MacOS includes

#include <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_export.h"

namespace Digikam
{

QList<QUrl> DServiceMenu::MacApplicationForFileExtension(const QString& suffix)
{
    // Code inspired from:
    // qtbase/src/plugins/platforms/cocoa/qcocoanativeinterface.mm : QCocoaNativeInterface::defaultBackgroundPixmapForQWizard()
    // qtbase/src/corelib/io/qfilesystemengine_unix.cpp            : isPackage()
    // qtbase/src/corelib/global/qlibraryinfo.cpp                  : getRelocatablePrefix()
    // qtbase/src/corelib/plugin/qlibrary_unix.cpp                 : load_sys()

    QList<QUrl> appUrls;

    if (suffix.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Suffix is empty";
        return appUrls;
    }

    // Make a Uniform Type Identifier from a filename extension.

    CFArrayRef  bundleIDs             = nullptr;
    CFStringRef extensionRef          = suffix.toCFString();
    CFStringRef uniformTypeIdentifier = UTTypeCreatePreferredIdentifierForTag(
                                                                              kUTTagClassFilenameExtension,
                                                                              extensionRef,
                                                                              nullptr
                                                                             );

    if (!uniformTypeIdentifier)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot get the Uniform Type Identifier for" << suffix;

        return appUrls;
    }

    // Get a list of all of the application bundle IDs that know how to handle this.

    bundleIDs = LSCopyAllRoleHandlersForContentType(uniformTypeIdentifier, kLSRolesViewer | kLSRolesEditor);

    if (bundleIDs)
    {
        // Find all the available applications with this bundle ID.
        // We can also get the display name and version if necessary.

        const CFIndex count = CFArrayGetCount(bundleIDs);

        for (CFIndex i = 0 ; i < count ; ++i)
        {
            CFStringRef val            = (CFStringRef)(CFArrayGetValueAtIndex(bundleIDs, i));
            CFArrayRef appsForBundleID = LSCopyApplicationURLsForBundleIdentifier(val, nullptr);

            if (appsForBundleID)
            {
                // TODO: call CFURLResourceIsReachable() on each item before to use it.

                QList<QUrl> urls;
                CFIndex size = CFArrayGetCount(appsForBundleID);

                for (CFIndex j = 0 ; j < size ; ++j)
                {
                    CFPropertyListRef prop = CFArrayGetValueAtIndex(appsForBundleID, j);

                    if (prop)
                    {
                        CFTypeID typeId = CFGetTypeID(prop);

                        if (typeId == CFURLGetTypeID())
                        {
                            urls << QUrl::fromCFURL(static_cast<CFURLRef>(prop));
                        }
                        else
                        {
                            qCWarning(DIGIKAM_GENERAL_LOG) << "Application Bundle Property type is not CFURL:"
                                                           << typeId
                                                           << "("
                                                           << QString::fromCFString(CFCopyTypeIDDescription(typeId))
                                                           << ")";
                        }
                    }
                    else
                    {
                        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot get url"
                                                       << j
                                                       << "for application"
                                                       << i;
                    }
                }

                appUrls << urls;
                CFRelease(appsForBundleID);
            }
        }

        CFRelease(bundleIDs);
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot get the Application urls list for" << suffix;
    }

    // appUrls is a list of ALL possible applications suitable for suffix.
    // Given a UI to choose one, you can then call

    // Release the resources.

    CFRelease(uniformTypeIdentifier);

    return appUrls;
}

bool DServiceMenu::MacOpenFilesWithApplication(const QList<QUrl>& fileUrls, const QUrl& appUrl)
{
    // Inspired from https://github.com/eep/fugu/blob/master/NSWorkspace(LaunchServices).m

    bool success               = true;
    LSLaunchURLSpec lspec      = { nullptr, nullptr, nullptr, 0, nullptr };
    CFMutableArrayRef arrayref = CFArrayCreateMutable(nullptr, 0, nullptr);

    for (const QUrl& fileUrl : std::as_const(fileUrls))
    {
        CFURLRef furl = fileUrl.toCFURL();
        CFArrayAppendValue(arrayref, furl);
    }

    lspec.appURL          = appUrl.toCFURL();
    lspec.itemURLs        = arrayref;
/*
    lspec.passThruParams  = params;
    lspec.launchFlags     = flags;
*/
    lspec.asyncRefCon     = nullptr;

    OSStatus status       = LSOpenFromURLSpec(&lspec, nullptr);

    if (status != noErr)
    {
        success = false;
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot start Application Bundle url"
                                       << appUrl << "for files" << fileUrls;
    }

    if (arrayref)
    {
        CFRelease(arrayref);
    }

    return success;
}

QList<QUrl> DServiceMenu::MacApplicationsForFiles(const QList<QUrl>& files)
{
    if (files.isEmpty())
    {
        return QList<QUrl>();
    }

    QString suffix            = QFileInfo(files.first().toLocalFile()).suffix();
    QList<QUrl> commonAppUrls = DServiceMenu::MacApplicationForFileExtension(suffix);

    for (const QUrl& file : std::as_const(files))
    {
        suffix            = QFileInfo(file.toLocalFile()).suffix();
        QList<QUrl> aurls = DServiceMenu::MacApplicationForFileExtension(suffix);

        for (const QUrl& url : std::as_const(aurls))
        {
            if (!commonAppUrls.contains(url))
            {
                commonAppUrls.removeAll(url);
            }
        }
    }

    return commonAppUrls;
}

QString DServiceMenu::MacApplicationBundleName(const QUrl& appUrl)
{
    return (appUrl.toLocalFile().section(QLatin1Char('/'), -2, -2));
}

QIcon DServiceMenu::MacApplicationBundleIcon(const QUrl& appUrl, int size)
{
    // Inspired from http://theocacao.com/document.page/183     // krazy:exclude=insecurenet

    // Get Image from Workspace about Application Bundle.

    NSWorkspace* const ws          = [NSWorkspace sharedWorkspace];
    NSString* const path           = appUrl.path().toNSString();
    NSImage* const macIcon         = [ws iconForFile: path];

    // Convert NSImage to QImage to QPixmap to QIcon, using PNG container in memory

    CGImageRef cgRef               = [macIcon CGImageForProposedRect:NULL context:nil hints:nil];
    NSBitmapImageRep* const bitmap = [[NSBitmapImageRep alloc] initWithCGImage:cgRef];
    [bitmap setSize:[macIcon size]];
    NSData* const pngData          = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
    QByteArray array               = QByteArray::fromNSData(pngData);
    QImage image                   = QImage::fromData(array, "PNG");
    QPixmap pix                    = QPixmap::fromImage(image.scaled(size, size));

    // Clenaup

    [bitmap autorelease];

    return (QIcon(pix));
}

} // namespace Digikam

