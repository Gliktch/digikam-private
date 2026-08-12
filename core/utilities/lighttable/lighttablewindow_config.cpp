/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam light table - Configure
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lighttablewindow_p.h"

namespace Digikam
{

void LightTableWindow::slotSetup()
{
    Setup::execDialog(this);
}

void LightTableWindow::slotColorManagementOptionsChanged()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedPreviews);
    d->viewCMViewAction->blockSignals(false);
}

void LightTableWindow::slotThemeChanged()
{
    d->previewView->checkForSelection(d->previewView->leftItemInfo());
    d->previewView->checkForSelection(d->previewView->rightItemInfo());
}

void LightTableWindow::slotApplicationSettingsChanged()
{
    d->leftSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());

    /// @todo Which part of the settings has to be reloaded?
    //d->rightSideBar->applySettings();

    d->previewView->setPreviewSettings(ApplicationSettings::instance()->getPreviewSettings());
}

void LightTableWindow::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    d->hSplitter->restoreState(group, QLatin1String("Horizontal Splitter State"));
    d->barViewDock->setShouldBeVisible(group.readEntry(QLatin1String("Show Thumbbar"), true));
    d->navigateByPairAction->setChecked(group.readEntry(QLatin1String("Navigate By Pair"), false));
    slotToggleNavigateByPair();

    QByteArray thumbbarState;
    thumbbarState = group.readEntry(QLatin1String("ThumbbarState"), thumbbarState);
    d->dockArea->restoreState(QByteArray::fromBase64(thumbbarState));

    d->leftSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Left Sidebar")));
    d->leftSideBar->loadState();
    d->rightSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Right Sidebar")));
    d->rightSideBar->loadState();

    readFullScreenSettings(group);
}

void LightTableWindow::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    d->hSplitter->saveState(group, QLatin1String("Horizontal Splitter State"));
    group.writeEntry(QLatin1String("Show Thumbbar"),    d->barViewDock->shouldBeVisible());
    group.writeEntry(QLatin1String("ThumbbarState"),    d->dockArea->saveState().toBase64());
    group.writeEntry(QLatin1String("Clear On Close"),   d->clearOnCloseAction->isChecked());
    group.writeEntry(QLatin1String("Navigate By Pair"), d->navigateByPairAction->isChecked());

    d->leftSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Left Sidebar")));
    d->leftSideBar->saveState();
    d->rightSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Right Sidebar")));
    d->rightSideBar->saveState();

    config->sync();
}

void LightTableWindow::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    d->autoLoadOnRightPanel   = group.readEntry(QLatin1String("Auto Load Right Panel"), true);
    d->autoSyncPreview        = group.readEntry(QLatin1String("Auto Sync Preview"),     true);
    d->clearOnCloseAction->setChecked(group.readEntry(QLatin1String("Clear On Close"), false));
    slotApplicationSettingsChanged();

    // Restore full screen Mode

    readFullScreenSettings(group);

    // NOTE: Image orientation settings in thumbbar is managed by image model.

    refreshView();
}

} // namespace Digikam
