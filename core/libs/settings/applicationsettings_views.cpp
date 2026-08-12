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
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applicationsettings_p.h"

namespace Digikam
{

void ApplicationSettings::setImageSortOrder(int order)
{
    d->imageSortOrder = order;
}

int ApplicationSettings::getImageSortOrder() const
{
    return d->imageSortOrder;
}

void ApplicationSettings::setImageSorting(int sorting)
{
    d->imageSorting = sorting;
}

int ApplicationSettings::getImageSorting() const
{
    return d->imageSorting;
}

void ApplicationSettings::setImageSeparationMode(int mode)
{
    d->imageSeparationMode = mode;
}

int ApplicationSettings::getImageSeparationMode() const
{
    return d->imageSeparationMode;
}

void ApplicationSettings::setImageSeparationSortOrder(int order)
{
    d->imageSeparationSortOrder = order;
}

int ApplicationSettings::getImageSeparationSortOrder() const
{
    return d->imageSeparationSortOrder;
}

void ApplicationSettings::setItemLeftClickAction(int action)
{
    d->itemLeftClickAction = action;
}

int ApplicationSettings::getItemLeftClickAction() const
{
    return d->itemLeftClickAction;
}

void ApplicationSettings::setDefaultIconSize(int val)
{
    d->thumbnailSize = val;
}

int ApplicationSettings::getDefaultIconSize() const
{
    return d->thumbnailSize;
}

void ApplicationSettings::setIconViewFont(const QFont& font)
{
    d->iconviewFont = font;
}

QFont ApplicationSettings::getIconViewFont() const
{
    return d->iconviewFont;
}

void ApplicationSettings::setIconShowName(bool val)
{
    d->iconShowName = val;
}

bool ApplicationSettings::getIconShowName() const
{
    return d->iconShowName;
}

void ApplicationSettings::setIconShowSize(bool val)
{
    d->iconShowSize = val;
}

bool ApplicationSettings::getIconShowSize() const
{
    return d->iconShowSize;
}

void ApplicationSettings::setIconShowTitle(bool val)
{
    d->iconShowTitle = val;
}

bool ApplicationSettings::getIconShowTitle() const
{
    return d->iconShowTitle;
}

void ApplicationSettings::setIconShowComments(bool val)
{
    d->iconShowComments = val;
}

bool ApplicationSettings::getIconShowComments() const
{
    return d->iconShowComments;
}

void ApplicationSettings::setIconShowResolution(bool val)
{
    d->iconShowResolution = val;
}

bool ApplicationSettings::getIconShowResolution() const
{
    return d->iconShowResolution;
}

void ApplicationSettings::setIconShowAspectRatio(bool val)
{
    d->iconShowAspectRatio = val;
}

bool ApplicationSettings::getIconShowAspectRatio() const
{
    return d->iconShowAspectRatio;
}

void ApplicationSettings::setIconShowTags(bool val)
{
    d->iconShowTags = val;
}

bool ApplicationSettings::getIconShowTags() const
{
    return d->iconShowTags;
}

void ApplicationSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool ApplicationSettings::getIconShowDate() const
{
    return d->iconShowDate;
}

void ApplicationSettings::setIconShowModDate(bool val)
{
    d->iconShowModDate = val;
}

bool ApplicationSettings::getIconShowModDate() const
{
    return d->iconShowModDate;
}

void ApplicationSettings::setIconShowRating(bool val)
{
    d->iconShowRating = val;
}

bool ApplicationSettings::getIconShowRating() const
{
    return d->iconShowRating;
}

void ApplicationSettings::setIconShowPickLabel(bool val)
{
    d->iconShowPickLabel = val;
}

bool ApplicationSettings::getIconShowPickLabel() const
{
    return d->iconShowPickLabel;
}

void ApplicationSettings::setIconShowColorLabel(bool val)
{
    d->iconShowColorLabel = val;
}

bool ApplicationSettings::getIconShowColorLabel() const
{
    return d->iconShowColorLabel;
}

void ApplicationSettings::setIconShowImageFormat(bool val)
{
    d->iconShowImageFormat = val;
}

bool ApplicationSettings::getIconShowImageFormat() const
{
    return d->iconShowImageFormat;
}

void ApplicationSettings::setIconShowCoordinates(bool val)
{
    d->iconShowCoordinates = val;
}

bool ApplicationSettings::getIconShowCoordinates() const
{
    return d->iconShowCoordinates;
}

void ApplicationSettings::setIconShowOverlays(bool val)
{
    d->iconShowOverlays = val;
}

bool ApplicationSettings::getIconShowOverlays() const
{
    return d->iconShowOverlays;
}

void ApplicationSettings::setIconShowFullscreen(bool val)
{
   d->iconShowFullscreen = val;
}

bool ApplicationSettings::getIconShowFullscreen() const
{
   return d->iconShowFullscreen;
}

void ApplicationSettings::setPreviewSettings(const PreviewSettings& settings)
{
    d->previewSettings = settings;
}

PreviewSettings ApplicationSettings::getPreviewSettings() const
{
    return d->previewSettings;
}

void ApplicationSettings::setMagnifierZoomFactor(qreal factor)
{
    d->magnifierZoomFactor = factor;
}

qreal ApplicationSettings::getMagnifierZoomFactor() const
{
    return d->magnifierZoomFactor;
}

void ApplicationSettings::setMagnifierSize(int size)
{
    d->magnifierSize = size;
}

int ApplicationSettings::getMagnifierSize() const
{
    return d->magnifierSize;
}

void ApplicationSettings::setPreviewShowIcons(bool val)
{
    d->previewShowIcons = val;
}

bool ApplicationSettings::getPreviewShowIcons() const
{
    return d->previewShowIcons;
}

void ApplicationSettings::setPreviewAutoPlay(bool val)
{
    d->previewAutoPlay = val;
}

bool ApplicationSettings::getPreviewAutoPlay() const
{
    return d->previewAutoPlay;
}

void ApplicationSettings::setScaleFitToWindow(bool val)
{
    d->scaleFitToWindow = val;
}

bool ApplicationSettings::getScaleFitToWindow() const
{
    return d->scaleFitToWindow;
}

void ApplicationSettings::setPreviewSmoothScaled(bool val)
{
    d->imageSmoothScaled = val;

    ImageZoomSettings::setImageSmoothScale(val);
}

bool ApplicationSettings::getPreviewSmoothScaled() const
{
    return d->imageSmoothScaled;
}

void ApplicationSettings::setPreviewOverlay(bool val)
{
    d->previewOverlay = val;
}

bool ApplicationSettings::getPreviewOverlay() const
{
    return d->previewOverlay;
}

} // namespace Digikam
