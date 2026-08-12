/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmisc_p.h"

namespace Digikam
{

bool SetupMisc::checkSettings()
{
    // If check for new version use weekly pre-releases, warn end-user.

    if (
        (d->updateType->currentIndex()                    == 1) &&
        (ApplicationSettings::instance()->getUpdateType() == 0)
       )
    {
        d->tab->setCurrentIndex(0);

        int result = QMessageBox::warning(this, qApp->applicationName(),
                                          i18n("Check for new version option will verify the pre-releases.\n"
                                               "\"Pre-releases\" are proposed weekly to tests quickly new features.\n"
                                               "It is not recommended to use pre-release in production as bugs can remain,\n"
                                               "unless you know what you are doing.\n"
                                               "Do you want to continue?"),
                                                QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
            return true;
        }

        return false;
    }

    return true;
}

void SetupMisc::applySettings()
{
    d->systemSettingsWidget->saveSettings();

    ApplicationSettings* const settings      = ApplicationSettings::instance();
    MetaEngineSettingsContainer metaSettings = MetaEngineSettings::instance()->settings();

    settings->setShowSplashScreen(d->showSplashCheck->isChecked());
    settings->setShowTrashDeleteDialog(d->showTrashDeleteDialogCheck->isChecked());
    settings->setShowPermanentDeleteDialog(d->showPermanentDeleteDialogCheck->isChecked());
    settings->setMinimumSimilarityBound(d->minimumSimilarityBound->value());
    settings->setApplySidebarChangesDirectly(d->sidebarApplyDirectlyCheck->isChecked());
    settings->setScanAtStart(d->scanAtStart->isChecked());
    settings->setDetectFacesInNewImages(d->detectFaces->isChecked());
    settings->setFaceRecognitionBackgroundScan(d->faceRecognitionBackgroundScan->isChecked());
    settings->setCleanAtStart(d->cleanAtStart->isChecked());
    settings->setUseNativeFileDialog(d->useNativeFileDialogCheck->isChecked());
    settings->setDrawFramesToGrouped(d->drawFramesToGroupedCheck->isChecked());
    settings->setSelectFirstAlbumItem(d->selectFirstAlbumItemCheck->isChecked());
    settings->setExpandNewCurrentItem(d->expandNewCurrentItemCheck->isChecked());
    settings->setScrollItemToCenter(d->scrollItemToCenterCheck->isChecked());
    settings->setShowOnlyPersonTagsInPeopleSidebar(d->showOnlyPersonTagsInPeopleSidebarCheck->isChecked());
    settings->setSidebarTitleStyle(d->sidebarType->currentIndex() == 0 ? DMultiTabBar::ActiveIconText : DMultiTabBar::AllIconsText);
    settings->setUpdateType(d->updateType->currentIndex());
    settings->setUpdateWithDebug(d->updateWithDebug->isChecked());
    settings->setStringComparisonType((ApplicationSettings::StringComparisonType)
                                      d->stringComparisonType->itemData(d->stringComparisonType->currentIndex()).toInt());

    metaSettings.useFastScan   = d->scanAtStart->isChecked() ? d->useFastScan->isChecked()
                                                             : false;

    metaSettings.albumDateFrom = (MetaEngineSettingsContainer::AlbumDateSource)
                                 d->albumDateSource->itemData(d->albumDateSource->currentIndex()).toInt();

    MetaEngineSettings::instance()->setSettings(metaSettings);

    for (int i = 0 ; i != UnspecifiedOps ; ++i)
    {
        settings->setGroupingOperateOnAll((OperationType)i,
                                          (ApplicationSettings::ApplyToEntireGroup)d->groupingButtons.value(i)->checkedId());
    }

#ifdef HAVE_APPSTYLE_SUPPORT

    if (settings->getApplicationStyle().compare(d->applicationStyle->currentText(), Qt::CaseInsensitive) != 0)
    {
        settings->setApplicationStyle(d->applicationStyle->currentText());
        ThemeManager::instance()->updateThemeMenu();
    }

#endif

    settings->setIconTheme(d->applicationIcon->currentData().toString());
    settings->setDateTimeFormat(d->dateTimeFormatEdit->text().trimmed());
    settings->setApplicationFont(d->applicationFont->font());
    settings->saveSettings();

#ifdef HAVE_SONNET

    d->spellCheckWidget->applySettings();

#endif

    d->localizeWidget->applySettings();
}

void SetupMisc::readSettings()
{
    d->systemSettingsWidget->readSettings();

    const  ApplicationSettings* const settings = ApplicationSettings::instance();

    d->showSplashCheck->setChecked(settings->getShowSplashScreen());
    d->showTrashDeleteDialogCheck->setChecked(settings->getShowTrashDeleteDialog());
    d->showPermanentDeleteDialogCheck->setChecked(settings->getShowPermanentDeleteDialog());
    d->minimumSimilarityBound->setValue(settings->getMinimumSimilarityBound());
    d->sidebarApplyDirectlyCheck->setChecked(settings->getApplySidebarChangesDirectly());
    d->sidebarApplyDirectlyCheck->setChecked(settings->getApplySidebarChangesDirectly());
    d->scanAtStart->setChecked(settings->getScanAtStart());
    d->detectFaces->setChecked(settings->getDetectFacesInNewImages());
    d->faceRecognitionBackgroundScan->setChecked(settings->getFaceRecognitionBackgroundScan());
    d->cleanAtStart->setChecked(settings->getCleanAtStart());
    d->useNativeFileDialogCheck->setChecked(settings->getUseNativeFileDialog());
    d->drawFramesToGroupedCheck->setChecked(settings->getDrawFramesToGrouped());
    d->selectFirstAlbumItemCheck->setChecked(settings->getSelectFirstAlbumItem());
    d->expandNewCurrentItemCheck->setChecked(settings->getExpandNewCurrentItem());
    d->scrollItemToCenterCheck->setChecked(settings->getScrollItemToCenter());
    d->showOnlyPersonTagsInPeopleSidebarCheck->setChecked(settings->showOnlyPersonTagsInPeopleSidebar());
    d->sidebarType->setCurrentIndex(settings->getSidebarTitleStyle() == DMultiTabBar::ActiveIconText ? 0 : 1);
    d->updateType->setCurrentIndex(settings->getUpdateType());
    d->updateWithDebug->setChecked(settings->getUpdateWithDebug());
    d->stringComparisonType->setCurrentIndex(settings->getStringComparisonType());

    MetaEngineSettingsContainer metaSettings = MetaEngineSettings::instance()->settings();
    d->albumDateSource->setCurrentIndex((int)metaSettings.albumDateFrom);
    d->useFastScan->setChecked(metaSettings.useFastScan);

    for (int i = 0 ; i != UnspecifiedOps ; ++i)
    {
        d->groupingButtons.value(i)->button((int)settings->getGroupingOperateOnAll((OperationType)i))->setChecked(true);
    }

#ifdef HAVE_APPSTYLE_SUPPORT

    d->applicationStyle->setCurrentIndex(d->applicationStyle->findData(settings->getApplicationStyle().toLower()));

#endif

    d->applicationIcon->setCurrentIndex(d->applicationIcon->findData(settings->getIconTheme()));
    d->dateTimeFormatEdit->setText(settings->getDateTimeFormat());
    d->applicationFont->setFont(settings->getApplicationFont());

    // NOTE: Spellcheck and Localize read settings is done in widget constructor.
}

} // namespace Digikam
