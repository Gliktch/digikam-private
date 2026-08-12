/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applicationsettings_p.h"

namespace Digikam
{

int ApplicationSettings::getUpdateType() const
{
    return d->updateType;
}

void ApplicationSettings::setUpdateType(int type)
{
    d->updateType = type;
}

bool ApplicationSettings::getUpdateWithDebug() const
{
    return d->updateWithDebug;
}

void ApplicationSettings::setUpdateWithDebug(bool dbg)
{
    d->updateWithDebug = dbg;
}

void ApplicationSettings::setCurrentTheme(const QString& theme)
{
    d->currentTheme = theme;
}

QString ApplicationSettings::getCurrentTheme() const
{
    return d->currentTheme;
}

void ApplicationSettings::setShowSplashScreen(bool val)
{
    d->showSplash = val;
}

bool ApplicationSettings::getShowSplashScreen() const
{
    return d->showSplash;
}

void ApplicationSettings::setSidebarTitleStyle(DMultiTabBar::TextStyle style)
{
    d->sidebarTitleStyle = style;
}

DMultiTabBar::TextStyle ApplicationSettings::getSidebarTitleStyle() const
{
    return d->sidebarTitleStyle;
}

void ApplicationSettings::setUseTrash(bool val)
{
    d->useTrash = val;
}

bool ApplicationSettings::getUseTrash() const
{
    return d->useTrash;
}

void ApplicationSettings::setShowTrashDeleteDialog(bool val)
{
    d->showTrashDeleteDialog = val;
}

bool ApplicationSettings::getShowTrashDeleteDialog() const
{
    return d->showTrashDeleteDialog;
}

void ApplicationSettings::setShowPermanentDeleteDialog(bool val)
{
    d->showPermanentDeleteDialog = val;
}

bool ApplicationSettings::getShowPermanentDeleteDialog() const
{
    return d->showPermanentDeleteDialog;
}

void ApplicationSettings::setApplySidebarChangesDirectly(bool val)
{
    d->sidebarApplyDirectly = val;
}

bool ApplicationSettings::getApplySidebarChangesDirectly() const
{
    return d->sidebarApplyDirectly;
}

void ApplicationSettings::setUseNativeFileDialog(bool val)
{
    d->useNativeFileDialog = val;
}

bool ApplicationSettings::getUseNativeFileDialog() const
{
    return d->useNativeFileDialog;
}

void ApplicationSettings::setDrawFramesToGrouped(bool val)
{
    d->drawFramesToGrouped = val;
}

bool ApplicationSettings::getDrawFramesToGrouped() const
{
    return d->drawFramesToGrouped;
}

void ApplicationSettings::setSelectFirstAlbumItem(bool val)
{
    d->selectFirstAlbumItem = val;
}

bool ApplicationSettings::getSelectFirstAlbumItem() const
{
    return d->selectFirstAlbumItem;
}

void ApplicationSettings::setExpandNewCurrentItem(bool val)
{
    d->expandNewCurrentItem = val;
}

bool ApplicationSettings::getExpandNewCurrentItem() const
{
    return d->expandNewCurrentItem;
}

void ApplicationSettings::setScrollItemToCenter(bool val)
{
    d->scrollItemToCenter = val;
}

bool ApplicationSettings::getScrollItemToCenter() const
{
    return d->scrollItemToCenter;
}

void ApplicationSettings::setShowOnlyPersonTagsInPeopleSidebar(bool val)
{
    d->showOnlyPersonTagsInPeopleSidebar = val;
}

bool ApplicationSettings::showOnlyPersonTagsInPeopleSidebar() const
{
    return d->showOnlyPersonTagsInPeopleSidebar;
}

void ApplicationSettings::setDetectFacesInNewImages(bool val)
{
    d->detectFacesInNewImages = val;
}

bool ApplicationSettings::getDetectFacesInNewImages() const
{
    return d->detectFacesInNewImages;
}

void ApplicationSettings::setFaceRecognitionBackgroundScan(bool val)
{
    d->faceRecognitionBackgroundScan = val;
}

bool ApplicationSettings::getFaceRecognitionBackgroundScan() const
{
    return d->faceRecognitionBackgroundScan;
}

void ApplicationSettings::setStringComparisonType(ApplicationSettings::StringComparisonType val)
{
    d->stringComparisonType = val;
}

ApplicationSettings::StringComparisonType ApplicationSettings::getStringComparisonType() const
{
    return d->stringComparisonType;
}

bool ApplicationSettings::isStringTypeNatural() const
{
    return (d->stringComparisonType == ApplicationSettings::Natural);
}

void ApplicationSettings::setVersionManagerSettings(const VersionManagerSettings& settings)
{
    d->versionSettings = settings;
}

VersionManagerSettings ApplicationSettings::getVersionManagerSettings() const
{
    return d->versionSettings;
}

void ApplicationSettings::setApplicationStyle(const QString& style)
{
    if (d->applicationStyle.compare(style, Qt::CaseInsensitive) != 0)
    {
        d->applicationStyle = style;
        qApp->setStyle(d->applicationStyle);
        qApp->style()->polish(qApp);
        qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to widget style: " << d->applicationStyle;
    }
}

QString ApplicationSettings::getApplicationStyle() const
{
    return d->applicationStyle;
}

void ApplicationSettings::setIconTheme(const QString& theme)
{
    d->applicationIcon = theme;
}

QString ApplicationSettings::getIconTheme() const
{
    return d->applicationIcon;
}

void ApplicationSettings::setDateTimeFormat(const QString& fmt)
{
    DATETIMEFORMATSTRING = fmt;
}

QString ApplicationSettings::getDateTimeFormat() const
{
    return DATETIMEFORMATSTRING;
}

void ApplicationSettings::setShowThumbbar(bool val)
{
    d->showThumbbar = val;
}

bool ApplicationSettings::getShowThumbbar() const
{
    return d->showThumbbar;
}

void ApplicationSettings::setRatingFilterCond(int val)
{
    d->ratingFilterCond = val;
}

int ApplicationSettings::getRatingFilterCond() const
{
    return d->ratingFilterCond;
}

void ApplicationSettings::setScanAtStart(bool val)
{
    d->scanAtStart = val;
}

bool ApplicationSettings::getScanAtStart() const
{
    return d->scanAtStart;
}

void ApplicationSettings::setCleanAtStart(bool val)
{
    d->cleanAtStart = val;
}

bool ApplicationSettings::getCleanAtStart() const
{
    return d->cleanAtStart;
}

void ApplicationSettings::setDatabaseDirSetAtCmd(bool val)
{
    d->databaseDirSetAtCmd = val;
}

bool ApplicationSettings::getDatabaseDirSetAtCmd() const
{
    return d->databaseDirSetAtCmd;
}

void ApplicationSettings::setMinimumSimilarityBound(int val)
{
    d->minimumSimilarityBound = val;
}

int  ApplicationSettings::getMinimumSimilarityBound() const
{
    return d->minimumSimilarityBound;
}

void ApplicationSettings::setDuplicatesSearchLastMinSimilarity(int val)
{
    d->duplicatesSearchLastMinSimilarity = val;
}

int  ApplicationSettings::getDuplicatesSearchLastMinSimilarity() const
{
    return d->duplicatesSearchLastMinSimilarity;
}

void ApplicationSettings::setDuplicatesSearchLastMaxSimilarity(int val)
{
    d->duplicatesSearchLastMaxSimilarity = val;
}

int  ApplicationSettings::getDuplicatesSearchLastMaxSimilarity() const
{
    return d->duplicatesSearchLastMaxSimilarity;
}

void ApplicationSettings::setDuplicatesAlbumTagRelation(int val)
{
    d->duplicatesSearchLastAlbumTagRelation = val;
}

int  ApplicationSettings::getDuplicatesAlbumTagRelation() const
{
    return d->duplicatesSearchLastAlbumTagRelation;
}

void ApplicationSettings::setDuplicatesReferenceImageSelectionMethod(HaarIface::RefImageSelMethod val)
{
    d->duplicatesSearchLastReferenceImageSelectionMethod = val;
}

HaarIface::RefImageSelMethod ApplicationSettings::getDuplicatesRefImageSelMethod() const
{
    return d->duplicatesSearchLastReferenceImageSelectionMethod;
}


void ApplicationSettings::setDuplicatesSearchRestrictions(int val)
{
    d->duplicatesSearchLastRestrictions = val;
}

int  ApplicationSettings::getDuplicatesSearchRestrictions() const
{
    return d->duplicatesSearchLastRestrictions;
}

void ApplicationSettings::setGroupingOperateOnAll(OperationType type,
                                                  ApplicationSettings::ApplyToEntireGroup applyAll)
{
    if (!d->groupingOperateOnAll.contains(type))
    {
        throw std::invalid_argument("ApplicationSettings::setGroupingOperateOnAll: Invalid operation type.");
    }

    d->groupingOperateOnAll[type] = applyAll;

    return;
}

ApplicationSettings::ApplyToEntireGroup ApplicationSettings::getGroupingOperateOnAll(
        OperationType type) const
{
    if (!d->groupingOperateOnAll.contains(type))
    {
        throw std::invalid_argument("ApplicationSettings::getGroupingOperateOnAll: Invalid operation type.");
    }

    if (type == UnspecifiedOps)
    {
        return ApplicationSettings::No;
    }

    return d->groupingOperateOnAll[type];
}

bool ApplicationSettings::askGroupingOperateOnAll(OperationType type)
{
    if (!d->groupingOperateOnAll.contains(type))
    {
        throw std::invalid_argument("ApplicationSettings::askGroupingOperateOnAll: Invalid operation type.");
    }

    if (type == UnspecifiedOps)
    {
        return false;
    }

    QPointer<QMessageBox> msgBox = new QMessageBox(qApp->activeWindow());
    msgBox->setWindowTitle(qApp->applicationName());
    msgBox->setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox->setText(QLatin1String("<p>") + ApplicationSettings::operationTypeTitle(type) +
                    QLatin1String("</p>") + i18n("Do you want to do this operation on all group items?"));

    QCheckBox* const chkBox = new QCheckBox(i18n("Remember choice for this operation"), msgBox);
    msgBox->setCheckBox(chkBox);

    if (msgBox->exec() == QMessageBox::No)
    {
        if (chkBox->isChecked())
        {
            setGroupingOperateOnAll(type, ApplicationSettings::No);
        }

        return false;
    }

    if (chkBox->isChecked())
    {
        setGroupingOperateOnAll(type, ApplicationSettings::Yes);
    }

    delete msgBox;

    return true;
}

QString ApplicationSettings::operationTypeTitle(OperationType type)
{
    switch (type)
    {
        case MetadataOps:
        {
            return i18n("Metadata");
        }

        case LightTableOps:
        {
            return i18n("Light Table");
        }

        case BQMOps:
        {
            return i18n("Batch Queue Manager");
        }

        case SlideshowOps:
        {
            return i18n("Slideshow");
        }

        case RenameOps:
        {
            return i18n("Renaming");
        }

        case ImportExportOps:
        {
            return i18n("Import/Export tools");
        }

        case ToolsOps:
        {
            return i18n("Tools (editor, panorama, stack blending, calendar, "
                        "external program)");
        }

        default:
        {
            throw std::invalid_argument("ApplicationSettings::operationTypeTitle: Invalid operation type.");
        }
    }
}

QString ApplicationSettings::operationTypeExplanation(OperationType type)
{
    switch (type)
    {
        case MetadataOps:
        {
            return i18n("Operations related to metadata, labels, ratings, tags, geolocation and rotation");
        }

        case LightTableOps:
        {
            return i18n("Adding items to the Light Table");
        }

        case BQMOps:
        {
            return i18n("Adding items to the Batch Queue Manager");
        }

        case SlideshowOps:
        {
            return i18n("Opening items in the Slideshow");
        }

        case RenameOps:
        {
            return i18n("Renaming items");
        }

        case ImportExportOps:
        {
            return i18n("Passing items to import/export tools");
        }

        case ToolsOps:
        {
            return i18n("Several tools including the editor, panorama, stack blending, "
                        "calendar, html gallery and opening with external programs");
        }

        default:
        {
            return QString();
        }
    }
}

void ApplicationSettings::setApplicationFont(const QFont& font)
{
    if (d->applicationFont != font)
    {
        d->applicationFont = font;
        qApp->setFont(d->applicationFont);
        qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to application font: " << d->applicationFont;
    }
}

QFont ApplicationSettings::getApplicationFont() const
{
    return d->applicationFont;
}

bool ApplicationSettings::getHelpBoxNotificationSeen()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Notification Messages"));
    bool value              = group.readEntry("HelpBoxNotifSeen", false);

    return value;
}

void ApplicationSettings::setHelpBoxNotificationSeen(bool val)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Notification Messages"));

    group.writeEntry("HelpBoxNotifSeen", val);
    config->sync();
}

} // namespace Digikam
