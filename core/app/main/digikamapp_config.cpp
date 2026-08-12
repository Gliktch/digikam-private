/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Configure
 *
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamapp_p.h"

namespace Digikam
{

bool DigikamApp::setup()
{
    return Setup::execDialog(this, Setup::LastPageUsed);
}

bool DigikamApp::setupICC()
{
    return Setup::execSinglePage(this, Setup::ICCPage);
}

void DigikamApp::slotSetup()
{
    setup();
}

void DigikamApp::slotSetupChanged()
{
    // raw loading options might have changed

    LoadingCacheInterface::cleanCache();

    // TODO: clear history when location changed
/*
    if (ApplicationSettings::instance()->getAlbumLibraryPath() != AlbumManager::instance()->getLibraryPath())
    {
        d->view->clearHistory();
    }
*/
    const DbEngineParameters prm = ApplicationSettings::instance()->getDbEngineParameters();

    if (!AlbumManager::instance()->databaseEqual(prm))
    {
        AlbumManager::instance()->changeDatabase(ApplicationSettings::instance()->getDbEngineParameters());
    }

    if (ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        AlbumManager::instance()->prepareItemCounts();
    }

    // Load full-screen options

    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());
    readFullScreenSettings(group);

    d->view->applySettings();

    AlbumThumbnailLoader::instance()->setThumbnailSize(ApplicationSettings::instance()->getTreeViewIconSize(),
                                                       ApplicationSettings::instance()->getTreeViewFaceSize());

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->applySettings();
    }

    if (SurveyWindow::surveyWindowCreated())
    {
        SurveyWindow::surveyWindow()->applySettings();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->applySettings();
    }

    d->config->sync();
}

void DigikamApp::slotEditKeys()
{
    editKeyboardShortcuts();
}

void DigikamApp::slotThemeChanged()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();
    QString theme                       = ThemeManager::instance()->currentThemeName();

    if (
        qApp->activeWindow()                                     &&
        (settings->getCurrentTheme() != theme)                   &&
        (
         (settings->getIconTheme() == QLatin1String(""))         ||
         (settings->getIconTheme() == QLatin1String("breeze"))   ||
         (settings->getIconTheme() == QLatin1String("breeze-dark"))
        )
       )
    {
        qApp->processEvents();

#if defined HAVE_KICONTHEMES && (KICONTHEMES_VERSION >= QT_VERSION_CHECK(6, 3, 0))

        settings->setIconTheme(QString());

#else

        QColor color = qApp->palette().color(qApp->activeWindow()->backgroundRole());
        QString iconTheme;
        QString msgText;

        if (color.lightness() > 127)
        {
            msgText   = i18n("You have chosen a bright color scheme. We switch "
                             "to a dark icon theme. The icon theme is "
                             "available after a restart of digiKam.");

            iconTheme = QLatin1String("breeze");
        }
        else
        {
            msgText   = i18n("You have chosen a dark color scheme. We switch "
                             "to a bright icon theme. The icon theme is "
                             "available after a restart of digiKam.");

            iconTheme = QLatin1String("breeze-dark");
        }

        if (settings->getIconTheme() != iconTheme)
        {
            QMessageBox::information(qApp->activeWindow(),
                                     qApp->applicationName(), msgText);

            settings->setIconTheme(iconTheme);
        }

#endif

    }

    settings->setCurrentTheme(theme);
}

} // namespace Digikam
