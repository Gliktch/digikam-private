/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-07-26
 * Description : System settings container.
 *
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "systemsettings.h"

// Qt includes

#include <QSettings>
#include <QStandardPaths>
#include <QNetworkProxy>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

SystemSettings::SystemSettings(const QString& name)
{
    if (!name.isEmpty())
    {
        m_path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
                 QLatin1Char('/') + name + QLatin1String("_systemrc");
    }

    readSettings();
}

SystemSettings::~SystemSettings()
{
}

void SystemSettings::readSettings()
{
    if (m_path.isEmpty())
    {
        return;
    }

    QSettings settings(m_path, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("System"));

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

#   ifdef Q_OS_LINUX

    useHighDpiScaling    = settings.value(QLatin1String("useHighDpiScaling"), true).toBool();
    useHighDpiPixmaps    = settings.value(QLatin1String("useHighDpiPixmaps"), true).toBool();

#   else

    useHighDpiScaling    = settings.value(QLatin1String("useHighDpiScaling"), false).toBool();
    useHighDpiPixmaps    = settings.value(QLatin1String("useHighDpiPixmaps"), false).toBool();

#   endif

#endif

    if (settings.contains(QLatin1String("disableFaceEngine")))
    {
        bool oldSetting  = settings.value(QLatin1String("disableFaceEngine"), false).toBool();
        enableFaceEngine = !oldSetting;
    }
    else
    {
        enableFaceEngine = settings.value(QLatin1String("enableFaceEngine"),  true).toBool();
    }

    enableAIAutoTools    = settings.value(QLatin1String("enableAIAutoTools"), true).toBool();
    enableAesthetic      = settings.value(QLatin1String("enableAesthetic"),   true).toBool();
    enableAutoTags       = settings.value(QLatin1String("enableAutoTags"),    true).toBool();

#ifdef Q_OS_WIN

    softwareOpenGL       = settings.value(QLatin1String("softwareOpenGL"),    true).toBool();

#else

    softwareOpenGL       = settings.value(QLatin1String("softwareOpenGL"),    false).toBool();

#endif

    enableLogging        = settings.value(QLatin1String("enableLogging"),     false).toBool();
    enableOpenCL         = settings.value(QLatin1String("enableOpenCL"),      false).toBool();
    enableDnnOpenCL      = settings.value(QLatin1String("enableDnnOpenCL"),   false).toBool();
    dnnOpenCLTested      = settings.value(QLatin1String("dnnOpenCLTested"),   false).toBool();


#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    enableHWVideo        = settings.value(QLatin1String("enableHWVideo"),     false).toBool();
    enableHWTConv        = settings.value(QLatin1String("enableHWTConv"),     false).toBool();
    videoBackend         = settings.value(QLatin1String("videoBackend"),      QLatin1String("ffmpeg")).toString();

#endif
    // Faces Engine

    QString dataPath     = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  QLatin1String("digikam/facesengine"),
                                                  QStandardPaths::LocateDirectory);
    if (dataPath.isEmpty())
    {
        dataPath         = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        dataPath        += QLatin1String("/digikam/facesengine");
    }

    facesEnginePath      = settings.value(QLatin1String("facesEnginePath"),   dataPath).toString();

    // Proxy Settings

    proxyUrl             = settings.value(QLatin1String("proxyUrl"),          QString()).toString();
    proxyPort            = settings.value(QLatin1String("proxyPort"),         8080).toInt();
    proxyUser            = settings.value(QLatin1String("proxyUser"),         QString()).toString();
    proxyPass            = settings.value(QLatin1String("proxyPass"),         QString()).toString();
    proxyType            = settings.value(QLatin1String("proxyType"),         HttpProxy).toInt();
    proxyAuth            = settings.value(QLatin1String("proxyAuth"),         false).toBool();

    settings.endGroup();
}

void SystemSettings::saveSettings()
{
    if (m_path.isEmpty())
    {
        return;
    }

    QSettings settings(m_path, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("System"));

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    settings.setValue(QLatin1String("useHighDpiScaling"), useHighDpiScaling);
    settings.setValue(QLatin1String("useHighDpiPixmaps"), useHighDpiPixmaps);

#endif

    settings.setValue(QLatin1String("enableAIAutoTools"), enableAIAutoTools);
    settings.setValue(QLatin1String("enableFaceEngine"),  enableFaceEngine);
    settings.setValue(QLatin1String("enableAesthetic"),   enableAesthetic);
    settings.setValue(QLatin1String("enableAutoTags"),    enableAutoTags);
    settings.setValue(QLatin1String("softwareOpenGL"),    softwareOpenGL);
    settings.setValue(QLatin1String("enableLogging"),     enableLogging);
    settings.setValue(QLatin1String("enableOpenCL"),      enableOpenCL);
    settings.setValue(QLatin1String("enableDnnOpenCL"),   enableDnnOpenCL);
    settings.setValue(QLatin1String("dnnOpenCLTested"),   dnnOpenCLTested);

    if (settings.contains(QLatin1String("disableFaceEngine")))
    {
        settings.remove(QLatin1String("disableFaceEngine"));
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    settings.setValue(QLatin1String("enableHWVideo"),     enableHWVideo);
    settings.setValue(QLatin1String("enableHWTConv"),     enableHWTConv);
    settings.setValue(QLatin1String("videoBackend"),      videoBackend);

#endif

    settings.setValue(QLatin1String("facesEnginePath"),   facesEnginePath);

    settings.setValue(QLatin1String("proxyUrl"),          proxyUrl);
    settings.setValue(QLatin1String("proxyPort"),         proxyPort);
    settings.setValue(QLatin1String("proxyType"),         proxyType);

    if (proxyAuth)
    {
        settings.setValue(QLatin1String("proxyAuth"),     true);
        settings.setValue(QLatin1String("proxyUser"),     proxyUser);
        settings.setValue(QLatin1String("proxyPass"),     proxyPass);
    }
    else
    {
        settings.setValue(QLatin1String("proxyAuth"),     false);
    }

    settings.endGroup();
}

void SystemSettings::applyProxySettings()
{
    QNetworkProxy proxy;

    // Make sure that no proxy is used for an empty string or the default value:

    if (proxyUrl.isEmpty() || (proxyUrl == QLatin1String("http://")))
    {
        proxy.setType(QNetworkProxy::NoProxy);
    }
    else
    {
        if      (proxyType == Socks5Proxy)
        {
            proxy.setType(QNetworkProxy::Socks5Proxy);
        }
        else if (proxyType == HttpProxy)
        {
            proxy.setType(QNetworkProxy::HttpProxy);
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unknown proxy type! Using Http Proxy instead.";
            proxy.setType(QNetworkProxy::HttpProxy);
        }
    }

    proxy.setHostName(proxyUrl);
    proxy.setPort(proxyPort);

    if (proxyAuth)
    {
        proxy.setUser(proxyUser);
        proxy.setPassword(proxyPass);
    }

    QNetworkProxy::setApplicationProxy(proxy);
}

QString SystemSettings::getFacesEnginePath() const
{
    // Check if Faces Engine model data already exists in the system.

    QString sysPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                             QLatin1String("digikam/facesengine"),
                                             QStandardPaths::LocateDirectory);

    if (!sysPath.isEmpty() && (sysPath == facesEnginePath))
    {
        return sysPath;
    }

    return facesEnginePath;
}

} // namespace Digikam
