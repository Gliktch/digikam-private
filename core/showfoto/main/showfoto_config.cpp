/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - Configure
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfoto_p.h"

namespace ShowFoto
{

void Showfoto::slotSetup()
{
    setup(false);
}

void Showfoto::slotSetupICC()
{
    setup(true);
}

bool Showfoto::setup(bool iccSetupPage)
{
    QPointer<ShowfotoSetup> setup = new ShowfotoSetup(this, iccSetupPage ? ShowfotoSetup::ICCPage : ShowfotoSetup::LastPageUsed);

    if (setup->exec() != QDialog::Accepted)
    {
        delete setup;
        return false;
    }

    KSharedConfig::openConfig()->sync();

    applySettings();

    if (d->itemsNb == 0)
    {
        slotUpdateItemInfo();
        toggleActions(false);
    }

    delete setup;

    return true;
}

void Showfoto::readSettings()
{
    d->settings        = ShowfotoSettings::instance();

    readStandardSettings();

    QString defaultDir = d->settings->getLastOpenedDir();

    if (defaultDir.isNull())
    {
        defaultDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }

    d->lastOpenedDirectory = QUrl::fromLocalFile(defaultDir);

    d->rightSideBar->loadState();
    d->leftSideBar->loadState();
    d->folderView->loadState();
    d->stackView->loadState();

    Digikam::ThemeManager::instance()->setCurrentTheme(d->settings->getCurrentTheme());

    d->thumbBar->setToolTipEnabled(d->settings->getShowToolTip());
}

void Showfoto::saveSettings()
{
    saveStandardSettings();

    d->settings->setLastOpenedDir(d->lastOpenedDirectory.toLocalFile());
    d->settings->setCurrentTheme(Digikam::ThemeManager::instance()->currentThemeName());
    d->settings->syncConfig();

    d->rightSideBar->saveState();
    d->leftSideBar->saveState();
    d->folderView->saveState();
    d->stackView->saveState();
}

void Showfoto::applySettings()
{
    applyStandardSettings();

    d->settings->readSettings();

    d->rightSideBar->setStyle(d->settings->getRightSideBarStyle() == 0 ?
                              DMultiTabBar::ActiveIconText : DMultiTabBar::AllIconsText);
    d->leftSideBar->setStyle(d->settings->getRightSideBarStyle() == 0 ?
                             DMultiTabBar::ActiveIconText : DMultiTabBar::AllIconsText);

    QString currentStyle = qApp->style()->objectName();
    QString newStyle     = d->settings->getApplicationStyle();

    if (currentStyle.compare(newStyle, Qt::CaseInsensitive) != 0)
    {
        qApp->setStyle(newStyle);
        qApp->style()->polish(qApp);
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << "Switch to widget style: " << newStyle;
    }

    m_fileDeleteAction->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    m_fileDeleteAction->setText(i18n("Delete File"));

    d->thumbBar->setToolTipEnabled(d->settings->getShowToolTip());

    d->rightSideBar->slotLoadMetadataFilters();

    applySortSettings();
}

void Showfoto::applySortSettings()
{
    if (d->settings->getReverseSort())
    {
        d->filterModel->setSortOrder(ShowfotoItemSortSettings::DescendingOrder);
        d->folderView->setSortOrder(ShowfotoItemSortSettings::DescendingOrder);
        d->stackView->setSortOrder(ShowfotoItemSortSettings::DescendingOrder);
    }
    else
    {
        d->filterModel->setSortOrder(ShowfotoItemSortSettings::AscendingOrder);
        d->folderView->setSortOrder(ShowfotoItemSortSettings::AscendingOrder);
        d->stackView->setSortOrder(ShowfotoItemSortSettings::AscendingOrder);
    }

    switch (d->settings->getSortRole())
    {
        case ShowfotoSetupMisc::SortByName:
        {
            d->filterModel->setSortRole(ShowfotoItemSortSettings::SortByFileName);
            d->folderView->setSortRole(ShowfotoFolderViewList::FileName);
            d->stackView->setSortRole(ShowfotoStackViewList::FileName);
            break;
        }

        case ShowfotoSetupMisc::SortByFileSize:
        {
            d->filterModel->setSortRole(ShowfotoItemSortSettings::SortByFileSize);
            d->folderView->setSortRole(ShowfotoFolderViewList::FileSize);
            d->stackView->setSortRole(ShowfotoStackViewList::FileSize);
            break;
        }

        default:
        {
            d->filterModel->setSortRole(ShowfotoItemSortSettings::SortByCreationDate);
            d->folderView->setSortRole(ShowfotoFolderViewList::FileDate);
            d->stackView->setSortRole(ShowfotoStackViewList::FileDate);
            break;
        }
    }
}

void Showfoto::slotThemeChanged()
{
    QString theme = ThemeManager::instance()->currentThemeName();

    if (
        qApp->activeWindow()                                        &&
        (d->settings->getCurrentTheme() != theme)                   &&
        (
         (d->settings->getIconTheme() == QLatin1String(""))         ||
         (d->settings->getIconTheme() == QLatin1String("breeze"))   ||
         (d->settings->getIconTheme() == QLatin1String("breeze-dark"))
        )
       )
    {
        qApp->processEvents();

#if defined HAVE_KICONTHEMES && (KICONTHEMES_VERSION >= QT_VERSION_CHECK(6, 3, 0))

        d->settings->setIconTheme(QString());

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

        if (d->settings->getIconTheme() != iconTheme)
        {
            QMessageBox::information(qApp->activeWindow(),
                                     qApp->applicationName(), msgText);

            d->settings->setIconTheme(iconTheme);
        }

#endif

    }

    d->settings->setCurrentTheme(theme);
}

void Showfoto::slotSetupMetadataFilters(int tab)
{
    ShowfotoSetup::execMetadataFilters(this, tab+1);
    d->rightSideBar->slotLoadMetadataFilters();
}

void Showfoto::slotSetupExifTool()
{
    ShowfotoSetup::execExifTool(this);
}

} // namespace ShowFoto
