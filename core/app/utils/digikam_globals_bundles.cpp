/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global macros, variables and flags - Bundle functions.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikam_globals_p.h"

namespace Digikam
{

bool isRunningInAppImageBundle()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    if (
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_LD_LIBRARY_PATH")) &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_QT_PLUGIN_PATH"))  &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_XDG_DATA_DIRS"))   &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_PATH"))
       )
    {
        return true;
    }

    return false;
}

bool isRunningOnNativeKDE()
{

#ifdef Q_OS_LINUX

    const QByteArray xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");

    return (!isRunningInAppImageBundle() && (xdgCurrentDesktop.toUpper() == "KDE"));

#else

    return false;

#endif

}

bool isRunningOnMacOS()
{

#ifdef Q_OS_MACOS

    return true;

#else

    return false;

#endif

}

QProcessEnvironment adjustedEnvironmentForAppImage()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // If we are running into AppImage bundle, switch env var to the right values.

    if (isRunningInAppImageBundle())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Adjusting environment variables for AppImage bundle";

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_LD_LIBRARY_PATH")).isEmpty())
        {
            env.insert(QLatin1String("LD_LIBRARY_PATH"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_LD_LIBRARY_PATH")));
        }
        else
        {
            env.remove(QLatin1String("LD_LIBRARY_PATH"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_QT_PLUGIN_PATH")).isEmpty())
        {
            env.insert(QLatin1String("QT_PLUGIN_PATH"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_QT_PLUGIN_PATH")));
        }
        else
        {
            env.remove(QLatin1String("QT_PLUGIN_PATH"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_DATA_DIRS")).isEmpty())
        {
            env.insert(QLatin1String("XDG_DATA_DIRS"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_DATA_DIRS")));
        }
        else
        {
            env.remove(QLatin1String("XDG_DATA_DIRS"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_LD_PRELOAD")).isEmpty())
        {
            env.insert(QLatin1String("LD_PRELOAD"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_LD_PRELOAD")));
        }
        else
        {
            env.remove(QLatin1String("LD_PRELOAD"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_PATH")).isEmpty())
        {
            env.insert(QLatin1String("PATH"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_PATH")));
        }
        else
        {
            env.remove(QLatin1String("PATH"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_KDE_FULL_SESSION")).isEmpty())
        {
            env.insert(QLatin1String("KDE_FULL_SESSION"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_KDE_FULL_SESSION")));
        }
        else
        {
            env.remove(QLatin1String("KDE_FULL_SESSION"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_DESKTOP_SESSION")).isEmpty())
        {
            env.insert(QLatin1String("DESKTOP_SESSION"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_DESKTOP_SESSION")));
        }
        else
        {
            env.remove(QLatin1String("DESKTOP_SESSION"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_CURRENT_DESKTOP")).isEmpty())
        {
            env.insert(QLatin1String("XDG_CURRENT_DESKTOP"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_CURRENT_DESKTOP")));
        }
        else
        {
            env.remove(QLatin1String("XDG_CURRENT_DESKTOP"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_SESSION_DESKTOP")).isEmpty())
        {
            env.insert(QLatin1String("XDG_SESSION_DESKTOP"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_SESSION_DESKTOP")));
        }
        else
        {
            env.remove(QLatin1String("XDG_SESSION_DESKTOP"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_MAGICK_CODER_MODULE_PATH")).isEmpty())
        {
            env.insert(QLatin1String("MAGICK_CODER_MODULE_PATH"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_MAGICK_CODER_MODULE_PATH")));
        }
        else
        {
            env.remove(QLatin1String("MAGICK_CODER_MODULE_PATH"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_MAGICK_CODER_FILTER_PATH")).isEmpty())
        {
            env.insert(QLatin1String("MAGICK_CODER_FILTER_PATH"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_MAGICK_CODER_FILTER_PATH")));
        }
        else
        {
            env.remove(QLatin1String("MAGICK_CODER_FILTER_PATH"));
        }
    }
    else
    {
        // cppcheck-suppress knownConditionTrueFalse
        if (isRunningOnMacOS())
        {

#ifdef DK_APPLE_PACKAGE_HOMEBREW

                env.insert(QLatin1String("DK_MARIADB_DIR"),
                    macOSBundlePrefix() + QString::fromUtf8("lib/mariadb", -1));
                env.insert(QLatin1String("PATH"),
                    macOSBundlePrefix() + QString::fromUtf8("lib/mariadb/bin:", -1) + QString::fromLocal8Bit(qgetenv("PATH")));

#endif

        }
    }

    return env;
}

QString macOSBundlePrefix()
{

#ifdef DK_APPLE_PACKAGE_HOMEBREW

    return QCoreApplication::applicationDirPath() + QString::fromUtf8("/../");

#else

    return QString::fromUtf8("/Applications/digiKam.org/digikam.app/Contents/");

#endif

}

void unloadQtTranslationFiles(QApplication& app)
{
    // HACK: We try to remove all the translators installed by ECMQmLoader.
    // The reason is that it always load translations for the system locale
    // which interferes with our effort to handle override languages. Since
    // `en_US` (or `en`) strings are defined in code, the QTranslator doesn't
    // actually handle translations for them, so even if we try to install
    // a QTranslator loaded from `en`, the strings always get translated by
    // the system language QTranslator that ECMQmLoader installed instead
    // of the English one.

    // ECMQmLoader creates all QTranslator's parented to the active QApplication instance.

    QList<QTranslator*> translators = app.findChildren<QTranslator*>(QString(), Qt::FindDirectChildrenOnly);

    for (const auto& translator : std::as_const(translators))
    {
        app.removeTranslator(translator);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Qt standard translations removed:" << translators.size();
}

void loadStdQtTranslationFiles(QApplication& app)
{
    QString transPath = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,
                                               QLatin1String("translations"),
                                               QStandardPaths::LocateDirectory);

    if (!transPath.isEmpty())
    {
        QString languagePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
                               QLatin1Char('/')                                                        +
                               QLatin1String("klanguageoverridesrc");

        qCDebug(DIGIKAM_GENERAL_LOG) << "Qt standard translations path:" << transPath;

        QLocale locale;

        if (!languagePath.isEmpty())
        {
            QSettings settings(languagePath, QSettings::IniFormat);
            settings.beginGroup(QLatin1String("Language"));
            QString language = settings.value(qApp->applicationName(), QString()).toString();
            settings.endGroup();

            if (!language.isEmpty())
            {
                QString languageName = language.split(QLatin1Char(':')).first();

                qCDebug(DIGIKAM_GENERAL_LOG) << "Language set to:" << languageName;

                locale = QLocale(languageName);
            }
        }

        const QStringList qtCatalogs =
        {
            QLatin1String("qt"),
            QLatin1String("qtbase"),
            QLatin1String("qt_help"),
            QLatin1String("qtdeclarative"),
            QLatin1String("qtquickcontrols"),
            QLatin1String("qtquickcontrols2"),
            QLatin1String("qtmultimedia"),
            QLatin1String("qtwebengine"),

#ifdef HAVE_QTXMLPATTERNS

            QLatin1String("qtxmlpatterns"),

#endif

        };

        for (const QString& catalog : std::as_const(qtCatalogs))
        {
            QTranslator* const translator = new QTranslator(&app);

            if (translator->load(locale, catalog, QLatin1String("_"), transPath))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Loaded Qt standard translations"
                                             << locale.name()
                                             << "from catalog"
                                             << catalog;

                app.installTranslator(translator);
            }
            else
            {
                delete translator;
            }
        }
    }
}

void loadEcmQtTranslationFiles(QApplication& app)
{
    // Load translations created by the ECMPoQmTools module.
    // This function is based on the code in:
    // https://invent.kde.org/frameworks/extra-cmake-modules/-/blob/master/modules/ECMQmLoader.cpp.in

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    // Qt6 version

    /**
     * Translation files have been verified with the DE i18n set located at:
     *
     * macOS:    /Applications/digikam.org/digikam.app/Contents/Resources/locale/de/LC_MESSAGES
     * AppImage: squashfs-root/usr/share/locale/de/LC_MESSAGES
     * Windows:  C:\Program Files\digiKam\data\locale\de\LC_MESSAGES
     *
     * Bundle:                                          macOS      AppImage    Windows
     *                                                  A  I        A  I        A  I
     * Legend:
     *    A = available (check with DE i18n set)
     *    I = included
     *    x = do not exists
     *    ? = to check
     */
    const QStringList ecmCatalogs =
    {
        QLatin1String("karchive6_qt"),          //      A  I        A  I        A  I
        QLatin1String("kauth6_qt"),             //      A  I        A  I        x
        QLatin1String("kbookmarks6_qt"),        //      A  I        A  I        A  I
        QLatin1String("kcodecs6_qt"),           //      A  I        A  I        A  I
        QLatin1String("kcompletion6_qt"),       //      A  I        A  I        A  I
        QLatin1String("kconfig6_qt"),           //      A  I        A  I        A  I
        QLatin1String("kcoreaddons6_qt"),       //      A  I        A  I        A  I
        QLatin1String("kdbusaddons6_qt"),       //      A  I        A  I        A  I
        QLatin1String("kglobalaccel6_qt"),      //      x           A  I        x
        QLatin1String("kitemviews6_qt"),        //      A  I        A  I        A  I
        QLatin1String("kjobwidgets6_qt"),       //      A  I        A  I        A  I
        QLatin1String("knotifications6_qt"),    //      A  I        A  I        A  I
        QLatin1String("kwidgetsaddons6_qt"),    //      A  I        A  I        A  I
        QLatin1String("kwindowsystem6_qt"),     //      A  I        A  I        A  I
        QLatin1String("solid6_qt"),             //      A  I        A  I        A  I
        QLatin1String("sonnet6_qt"),            //      A  I        A  I        A  I

#else

    // Qt5 version

    /**
     * Translation files have been verified with the DE i18n set located at:
     *
     * macOS:    /Applications/digikam.org/digikam.app/Contents/Resources/locale/de/LC_MESSAGES
     * AppImage: squashfs-root/usr/share/locale/de/LC_MESSAGES
     * Windows:  C:\Program Files\digiKam\data\locale\de\LC_MESSAGES
     *
     * Bundle:                                          macOS      AppImage    Windows
     *                                                  A  I        A  I        A  I
     * Legend:
     *    A = available (check with DE i18n set)
     *    I = included
     *    x = do not exists
     *    ? = to check
     */

    const QStringList ecmCatalogs =
    {
        QLatin1String("karchive5_qt"),          //      A  I        A  I        A  I
        QLatin1String("kauth5_qt"),             //      A  I        A  I        x
        QLatin1String("kbookmarks5_qt"),        //      A  I        A  I        A  I
        QLatin1String("kcodecs5_qt"),           //      A  I        A  I        A  I
        QLatin1String("kcompletion5_qt"),       //      A  I        A  I        A  I
        QLatin1String("kconfig5_qt"),           //      A  I        A  I        A  I
        QLatin1String("kcoreaddons5_qt"),       //      A  I        A  I        A  I
        QLatin1String("kdbusaddons5_qt"),       //      A  I        A  I        A  I
        QLatin1String("kglobalaccel5_qt"),      //      x           A  I        x
        QLatin1String("kitemviews5_qt"),        //      A  I        A  I        A  I
        QLatin1String("kjobwidgets5_qt"),       //      A  I        A  I        A  I
        QLatin1String("knotifications5_qt"),    //      A  I        A  I        A  I
        QLatin1String("kwidgetsaddons5_qt"),    //      A  I        A  I        A  I
        QLatin1String("kwindowsystem5_qt"),     //      A  I        A  I        A  I
        QLatin1String("solid5_qt"),             //      A  I        A  I        A  I
        QLatin1String("sonnet5_qt"),            //      x           A  I        A  I

#endif

    };

    /**
     * NOTE: these files handled by this function are .qm Qt formatted translation catalogs (Qt Compiled Translation Source File).
     * At the same place in the bundles, the .mo translations catalogs (Machine Objects)
     * are also present and do not need a special rules to be loaded in memory at run time
     * as it's done automatically in another place by the KDE framework.
     *
     * Typically list of .mo files present in the DE i18n catalog of the Qt6 AppImage bundle is listed below:
     *
     * breeze_kwin_deco.mo
     * digikam.mo
     * kconfigwidgets6.mo
     * kiconthemes6.mo
     * knotifyconfig6.mo
     * kservice6.mo
     * kxmlgui6.mo
     * breeze_style_config.mo
     * exiv2.mo
     * ki18n6.mo
     * kio6.mo
     * ksanecore.mo
     * ktextwidgets6.mo
     * libksane.mo
     */

    QStringList ecmLangs = KLocalizedString::languages();
    const QString langEn = QLatin1String("en");

    // Replace "en_US" with "en" because that's what we have in the locale dir.

    int indexOfEnUs      = ecmLangs.indexOf(QLatin1String("en_US"));

    if (indexOfEnUs != -1)
    {
        ecmLangs[indexOfEnUs] = langEn;
    }

    // We need to have "en" to the end of the list, because we explicitly
    // removed the "en" translators added by ECMQmLoader.
    // If "en" is already on the list, we truncate the ones after, because
    // "en" is the catch-all fallback that has the strings in code.

    int indexOfEn = ecmLangs.indexOf(langEn);

    if (indexOfEn != -1)
    {
        for (int i = (ecmLangs.size() - indexOfEn - 1) ; i > 0 ; i--)
        {
            ecmLangs.removeLast();
        }
    }
    else
    {
        ecmLangs.append(langEn);
    }

    // The last added one has the highest precedence, so we iterate the list backwards.

    QStringListIterator it(ecmLangs);
    it.toBack();

    while (it.hasPrevious())
    {
        const QString& localeDirName = it.previous();

        for (const auto& catalog : std::as_const(ecmCatalogs))
        {
            QString subPath    = QLatin1String("locale/")       +
                                 localeDirName                  +
                                 QLatin1String("/LC_MESSAGES/") +
                                 catalog                        +
                                 QLatin1String(".qm");

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            const QString root = QLibraryInfo::path(QLibraryInfo::PrefixPath);

#else

            const QString root = QLibraryInfo::location(QLibraryInfo::PrefixPath);

#endif

            // For AppImage transalotion files uses AppDataLocation.

            QString fullPath   = QStandardPaths::locate(QStandardPaths::AppDataLocation, subPath);

            if (fullPath.isEmpty())
            {
                // For distro builds probably still use GenericDataLocation, so check that too.

                fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, subPath);
            }

            if (fullPath.isEmpty())
            {
                // And, failing all, use the deps install folder

                fullPath = root + QLatin1String("/share/") + subPath;
            }

            if (!QFile::exists(fullPath))
            {
                continue;
            }

            QTranslator* const translator = new QTranslator(&app);

            if (translator->load(fullPath))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Loaded Qt ECM translations"
                                             << localeDirName
                                             << "from catalog"
                                             << catalog;

                translator->setObjectName(QString::fromUtf8("QTranslator.%1.%2").arg(localeDirName, catalog));
                app.installTranslator(translator);
            }
            else
            {
                delete translator;
            }
        }
    }
}

void installQtTranslationFiles(QApplication& app)
{

#if defined Q_OS_WIN || defined Q_OS_MACOS

    bool installTranslations = true;

#else

    bool installTranslations = isRunningInAppImageBundle();

#endif

    if (installTranslations)
    {
        unloadQtTranslationFiles(app);
        loadStdQtTranslationFiles(app);
        loadEcmQtTranslationFiles(app);
    }
}

void setMacOSEnvironment()
{
    // Safety check

    // cppcheck-suppress knownConditionTrueFalse
    if (isRunningOnMacOS())
    {

#ifdef DK_APPLE_PACKAGE_HOMEBREW

        qputenv("MAGICK_CODER_MODULE_PATH", (macOSBundlePrefix() + QString::fromUtf8("lib/ImageMagick/modules-Q16HDRI/coders", -1)).toUtf8());
        qputenv("MAGICK_CODER_FILTER_PATH", (macOSBundlePrefix() + QString::fromUtf8("lib/ImageMagick/modules-Q16HDRI/filters", -1)).toUtf8());
        qputenv("PATH", (macOSBundlePrefix() + QString::fromUtf8("bin:", -1) + QString::fromLocal8Bit(qgetenv("PATH"))).toUtf8());

#else // Macports

        // Libgphoto2 drivers
        qputenv("CAMLIBS", (macOSBundlePrefix() + QString::fromUtf8(("/lib/libgphoto2"),      -1)).toUtf8());
        qputenv("IOLIBS",  (macOSBundlePrefix() + QString::fromUtf8(("/lib/libgphoto2_port"), -1)).toUtf8());

#endif

    }
}

void setWindowsEnvironment(QApplication& app)
{
    qputenv("MAGICK_CODER_MODULE_PATH", app.applicationDirPath().toUtf8());
    qputenv("MAGICK_CODER_FILTER_PATH", app.applicationDirPath().toUtf8());

}

void setAudioBackendEnvironment()
{

#if defined(Q_OS_LINUX) && (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (!qEnvironmentVariableIsSet("QT_AUDIO_BACKEND"))
    {
        // On PipeWire systems Qt >= 6.10 defaults to its native PipeWire
        // audio backend, whose asynchronous format enumeration can fail
        // for sinks owned by other PipeWire clients (for example
        // module-filter-chain effect sinks): the affected outputs then
        // never appear in QMediaDevices::audioOutputs(), so they are
        // missing from the audio output selection of the media player
        // (QTBUG-143570). The PulseAudio backend enumerates the same
        // devices through pipewire-pulse without this defect, so prefer
        // it until the Qt issue is fixed.

        qputenv("QT_AUDIO_BACKEND", "pulseaudio");
    }

#endif

}

void delayForRemoteDebuging(int delaySecs)
{
    const QByteArray remoteDebug = qgetenv("DIGIKAM_REMOTE_DEBUG");

    if (remoteDebug.toUpper() == "ON")
    {
        qDebug() << "Waiting" << delaySecs << "seconds for remote debugging...";
        qDebug() << "Current process ID         :" << qApp->applicationPid();
        qDebug() << "Current process binary path:" << qApp->applicationFilePath();

#ifndef Q_OS_WIN

        QString appImageGDBOpt1;
        QString appImageGDBOpt2;

        if (isRunningInAppImageBundle())
        {
            appImageGDBOpt1 = QString::fromUtf8("-ex \"set sysroot %1/../..\"").arg(qApp->applicationDirPath());
            appImageGDBOpt2 = QString::fromUtf8("-ex \"set solib-search-path %1/../lib\"").arg(qApp->applicationDirPath());
        }

        qDebug() << "Command line to attach GDB :"
                 << "sudo gdb" << qApp->applicationFilePath()                   // Absolute path to the binary.
                               << "-p"<< qApp->applicationPid()                 // Process PID.
                               << "-ex \"catch throw\""                         // Handle the C++ exceptions.
                               << appImageGDBOpt1.toUtf8().constData()          // Extra option for AppImage: sandboxed area root path.
                               << appImageGDBOpt2.toUtf8().constData()          // Extra option for AppImage: internal path to shared libs.
                               << "-ex c"                                       // Continue the execution.
        ;

#endif

        QTime dwellTime = QTime::currentTime().addSecs(delaySecs);
        int diff        = 0;
        QElapsedTimer etimer;
        etimer.start();

        do
        {
            diff = QTime::currentTime().secsTo(dwellTime);

            if (etimer.elapsed() > 1000)
            {
                printf("Countdown: %i  \r", diff);
                fflush(stdout);
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                etimer.restart();
            }
        }
        while (diff > 0);
    }
}

bool detectKDEDesktopIsRunning()
{
    const QByteArray xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");

    if (!xdgCurrentDesktop.isEmpty())
    {
        return (xdgCurrentDesktop.toUpper() == "KDE");
    }

    // Classic fallbacks

    if (!qEnvironmentVariableIsEmpty("KDE_FULL_SESSION"))
    {
        return true;
    }

    return false;
}

} // namespace Digikam
