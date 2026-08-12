/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : showFoto is a stand alone version of image
 *               editor with no support of digiKam database.
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikam_config.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QImageReader>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QCommandLineOption>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kmemoryinfo.h>
#include <kaboutdata.h>

#ifdef HAVE_KICONTHEMES
#   include <kiconthemes_version.h>
#   include <KIconTheme>
#endif

// ImageMagick includes


#ifdef HAVE_IMAGE_MAGICK

// Pragma directives to reduce warnings from ImageMagick header files.

#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wignored-qualifiers"
#       pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wignored-qualifiers"
#       pragma clang diagnostic ignored "-Wkeyword-macro"
#   endif

#   include <Magick++.h>

using namespace Magick;

// Restore warnings

#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif // HAVE_IMAGE_MAGICK

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "digikam_version.h"
#include "filesdownloader.h"
#include "systemsettings.h"
#include "metaengine.h"
#include "daboutdata.h"
#include "showfoto.h"

#ifdef Q_OS_WIN
#   include <windows.h>
#   include <shellapi.h>
#   include <objbase.h>
#endif

#if defined Q_OS_WIN
#   define MAIN_EXPORT extern "C" __declspec(dllexport)
#   define MAIN_FN showfoto_main
#else
#   define MAIN_EXPORT
#   define MAIN_FN main
#endif

using namespace Digikam;

MAIN_EXPORT int MAIN_FN(int argc, char** argv)
{
    SystemSettings system(QLatin1String("showfoto"));

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    // These settings has no effect with Qt6 (always enabled)

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps,
                                   system.useHighDpiPixmaps);

    if (system.useHighDpiScaling)
    {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }
    else
    {
        QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    }

#else

    KMemoryInfo memInfo;
    quint64 maxLimit = memInfo.totalPhysical() / 1024 / 1024 / 1.40;
    maxLimit         = qMax((quint64)1024, maxLimit);
    QImageReader::setAllocationLimit(maxLimit);

#endif

    if (system.softwareOpenGL)
    {
        QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    }

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // Common OpenCL rules from digikam_globals.

    setOpenCLEnvironment(system.enableOpenCL);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (system.enableHWTConv)
    {
        qunsetenv("QT_DISABLE_HW_TEXTURES_CONVERSION");
    }
    else
    {
        qputenv("QT_DISABLE_HW_TEXTURES_CONVERSION", "1");
    }


    qputenv("QT_MEDIA_BACKEND", system.videoBackend.toLatin1());

    // Common audio backend rules from digikam_globals.

    setAudioBackendEnvironment();

#endif

#if defined HAVE_KICONTHEMES && (KICONTHEMES_VERSION >= QT_VERSION_CHECK(6, 3, 0))

    KIconTheme::initTheme();

#endif

    QApplication app(argc, argv);

#ifdef Q_OS_WIN

    QDir::setCurrent(qApp->applicationDirPath());

#endif

    delayForRemoteDebuging();

    system.applyProxySettings();

    digikamSetDebugFilterRules(system.enableLogging);

#ifdef HAVE_IMAGE_MAGICK

#if defined(Q_CC_MSVC)

    setWindowsEnvironment(app);

#elif defined(Q_OS_MACOS)

    setMacOSEnvironment();

#endif

    InitializeMagick(nullptr);

#endif

#ifdef Q_OS_MACOS

    // See bug #461734

    app.setAttribute(Qt::AA_DontShowIconsInMenus, true);

#endif

    // if we have some local breeze icon resource, prefer it

    DXmlGuiWindow::setupIconTheme();

    KLocalizedString::setApplicationDomain("digikam");

    KAboutData aboutData(QLatin1String("showfoto"),     // component name
                         i18nc("@title", "Showfoto"),   // display name
                         digiKamVersion());             // NOTE: showFoto version = digiKam version

    aboutData.setShortDescription(QString::fromUtf8("%1 - %2").arg(DAboutData::digiKamSlogan()).arg(DAboutData::digiKamFamily()));
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(DAboutData::copyright());
    aboutData.setOtherText(additionalInformation());
    aboutData.setHomepage(DAboutData::webProjectUrl().url());
    aboutData.setProductName(QByteArray("digikam/showfoto-core"));   // For bugzilla
    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"),
                            i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    DAboutData::authorsRegistration(aboutData);

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QLatin1String("files"),
                                 i18nc("command line option", "File(s) or folder(s) to open"),
                                 QLatin1String("[file(s) or folder(s)]"));
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    QString iconTheme         = group.readEntry(QLatin1String("Icon Theme"), QString());
    QString colorTheme        = group.readEntry(QLatin1String("Theme"), QString::fromLatin1("Standard"));
    Q_UNUSED(colorTheme);

    // See bug #438701

    installQtTranslationFiles(app);

    // ---

    MetaEngine::initializeExiv2();

    // Force to use application icon for non plasma desktop as Unity for ex.

#if !defined(Q_OS_MACOS)

    QApplication::setWindowIcon(QIcon::fromTheme(QLatin1String("showfoto"), app.windowIcon()));

#endif

    QList<QUrl> urlList;
    QStringList urls = parser.positionalArguments();

    for (const QString& url : std::as_const(urls))
    {
        urlList.append(QUrl::fromLocalFile(url));
    }

    parser.clearPositionalArguments();

    if (!iconTheme.isEmpty())
    {
        QIcon::setThemeName(iconTheme);
    }

#ifdef Q_OS_WIN

    // Necessary to open native open with dialog on windows

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);        // krazy:exclude=null

#endif

    ShowFoto::Showfoto* const w = new ShowFoto::Showfoto(urlList);

    // If application storage place in home directory to save customized XML settings files do not exist, create it,
    // else QFile will not able to create new files as well.

    if (!QFile::exists(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)))
    {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    // If application cache place in home directory to save cached files do not exist, create it.

    if (!QFile::exists(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)))
    {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    }

    w->show();

    if (system.enableAIAutoTools || system.enableFaceEngine || system.enableAesthetic || system.enableAutoTags)
    {
        QPointer<FilesDownloader> floader = new FilesDownloader(w);

        if (!floader->checkDownloadFiles())
        {
            floader->startDownload();
        }

        delete floader;
    }

    int ret = app.exec();

#ifdef Q_OS_WIN

    // Necessary to open native open with dialog on windows

    CoUninitialize();

#endif

#ifdef HAVE_IMAGE_MAGICK
#   if MagickLibVersion >= 0x693

    TerminateMagick();

#   endif
#endif

    return ret;
}
