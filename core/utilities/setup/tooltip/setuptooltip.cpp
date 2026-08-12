/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setuptooltip_p.h"

namespace Digikam
{

SetupToolTip::SetupToolTip(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->panel                 = new QWidget(viewport());
    setWidget(d->panel);
    setWidgetResizable(true);

    d->fontSelect            = new DFontSelect(i18n("Tool-Tips Font:"), d->panel);
    d->fontSelect->setToolTip(i18n("Select here the font used to display text in tool-tips."));

    d->tab                   = new QTabWidget(d->panel);

    // ---

    setupIcon();
    setupAlbum();
    setupImport();

    // ---

    QVBoxLayout* const vlay  = new QVBoxLayout(d->panel);
    vlay->addWidget(d->fontSelect);
    vlay->addWidget(d->tab);
    vlay->addStretch();
    vlay->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    vlay->setSpacing(d->spacing);

    // ---

    readSettings();
    adjustSize();
}

SetupToolTip::~SetupToolTip()
{
    delete d;
}

void SetupToolTip::setActiveTab(ToolTipTab tab)
{
    d->tab->setCurrentIndex(tab);
}

SetupToolTip::ToolTipTab SetupToolTip::activeTab() const
{
    return (ToolTipTab)d->tab->currentIndex();
}

void SetupToolTip::slotUseFileMetadataChanged(bool b)
{
    d->cameraUseFileMetadata = b;
    refreshCameraOptions();
}

void SetupToolTip::slotImportToolTipsChanged()
{
    refreshCameraOptions();
}

void SetupToolTip::refreshCameraOptions()
{
    bool b = d->showImportToolTipsBox->isChecked();
    d->importSettingBox->setEnabled(b);
    d->showItemPhotoMakeBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoLensBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoFocalBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoExpoBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoFlashBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoWBBox->setEnabled(b && d->cameraUseFileMetadata);
}

} // namespace Digikam

#include "moc_setuptooltip.cpp"
