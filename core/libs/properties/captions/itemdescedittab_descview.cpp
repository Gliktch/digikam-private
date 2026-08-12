/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor - Description view.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemdescedittab_p.h"

namespace Digikam
{

void ItemDescEditTab::initDescriptionView()
{
    QScrollArea* const sv       = new QScrollArea(d->tabWidget);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidgetResizable(true);

    QWidget* const captionsArea = new QWidget(sv->viewport());
    QGridLayout* const grid1    = new QGridLayout(captionsArea);
    sv->setWidget(captionsArea);

    d->titleEdit          = new AltLangStrEdit(captionsArea, 0);
    d->titleEdit->setTitle(i18nc("@title: comment title string", "Title:"));
    resetTitleEditPlaceholderText();

    d->captionsEdit       = new CaptionEdit(captionsArea);
    resetCaptionEditPlaceholderText();

    DHBox* const dateBox  = new DHBox(captionsArea);
    new QLabel(i18nc("@label", "Date:"), dateBox);
    d->dateTimeEdit       = new DDateTimeEdit(dateBox, QLatin1String("datepicker"));

    DHBox* const pickBox  = new DHBox(captionsArea);
    new QLabel(i18nc("@label", "Pick Label:"), pickBox);
    d->pickLabelSelector  = new PickLabelSelector(pickBox);
    pickBox->layout()->setAlignment(d->pickLabelSelector, Qt::AlignVCenter | Qt::AlignRight);

    DHBox* const colorBox = new DHBox(captionsArea);
    new QLabel(i18nc("@label", "Color Label:"), colorBox);
    d->colorLabelSelector = new ColorLabelSelector(colorBox);
    colorBox->layout()->setAlignment(d->colorLabelSelector, Qt::AlignVCenter | Qt::AlignRight);

    DHBox* const rateBox  = new DHBox(captionsArea);
    new QLabel(i18nc("@label", "Rating:"), rateBox);
    d->ratingWidget       = new RatingWidget(rateBox);
    rateBox->layout()->setAlignment(d->ratingWidget, Qt::AlignVCenter | Qt::AlignRight);

    // Buttons -----------------------------------------

    DHBox* const applyButtonBox = new DHBox(this);
    applyButtonBox->setSpacing(d->spacing);

    d->applyBtn                 = new QPushButton(i18nc("@action", "Apply"), applyButtonBox);
    d->applyBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->applyBtn->setEnabled(false);
    d->applyBtn->setToolTip(i18nc("@info", "Apply all changes to images"));
    //buttonsBox->setStretchFactor(d->applyBtn, 10);

    DHBox* const buttonsBox     = new DHBox(this);
    buttonsBox->setSpacing(d->spacing);

    d->revertBtn                = new QToolButton(buttonsBox);
    d->revertBtn->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->revertBtn->setToolTip(i18nc("@info", "Revert all changes"));
    d->revertBtn->setEnabled(false);

    d->applyToAllVersionsButton = new QPushButton(i18nc("@action", "Apply to all versions"), buttonsBox);
    d->applyToAllVersionsButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->applyToAllVersionsButton->setEnabled(false);
    d->applyToAllVersionsButton->setToolTip(i18nc("@info", "Apply all changes to all versions of this image"));

    d->moreButton               = new QPushButton(i18nc("@action: more actions to apply changes", "More"), buttonsBox);
    d->moreMenu                 = new QMenu(captionsArea);
    d->moreButton->setMenu(d->moreMenu);

    // --------------------------------------------------

    grid1->addWidget(d->titleEdit,    0, 0, 1, 2);
    grid1->addWidget(d->captionsEdit, 1, 0, 1, 2);
    grid1->addWidget(dateBox,         2, 0, 1, 2);
    grid1->addWidget(pickBox,         3, 0, 1, 2);
    grid1->addWidget(colorBox,        4, 0, 1, 2);
    grid1->addWidget(rateBox,         5, 0, 1, 2);
    grid1->setRowStretch(1, 10);
    grid1->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid1->setSpacing(d->spacing);

    d->tabWidget->insertTab(DESCRIPTIONS, sv, i18nc("@title", "Description"));
}

void ItemDescEditTab::setFocusToTitlesEdit()
{
    d->tabWidget->setCurrentIndex(DESCRIPTIONS);
    d->titleEdit->textEdit()->setFocus();
}

void ItemDescEditTab::setFocusToCommentsEdit()
{
    d->tabWidget->setCurrentIndex(DESCRIPTIONS);
    d->captionsEdit->altLangStrEdit()->textEdit()->setFocus();
}

void ItemDescEditTab::slotCommentChanged()
{
    d->hub->setComments(d->captionsEdit->values());
    d->setMetadataWidgetStatus(d->hub->commentsStatus(), d->captionsEdit);
    slotModified();
}

void ItemDescEditTab::slotTitleChanged()
{
    CaptionsMap titles;

    titles.fromAltLangMap(d->titleEdit->values());
    d->hub->setTitles(titles);
    d->setMetadataWidgetStatus(d->hub->titlesStatus(), d->titleEdit);
    slotModified();
}

void ItemDescEditTab::slotDateTimeChanged(const QDateTime& dateTime)
{
    d->hub->setDateTime(dateTime);
    d->setMetadataWidgetStatus(d->hub->dateTimeStatus(), d->dateTimeEdit);
    slotModified();
}

void ItemDescEditTab::slotPickLabelChanged(int pickId)
{
    d->hub->setPickLabel(pickId);

    // no handling for MetadataDisjoint needed for pick label,
    // we set it to 0 when disjoint, see below

    slotModified();
}

void ItemDescEditTab::slotColorLabelChanged(int colorId)
{
    d->hub->setColorLabel(colorId);

    // no handling for MetadataDisjoint needed for color label,
    // we set it to 0 when disjoint, see below

    slotModified();
}

void ItemDescEditTab::slotRatingChanged(int rating)
{
    d->hub->setRating(rating);

    // no handling for MetadataDisjoint needed for rating,
    // we set it to 0 when disjoint, see below

    slotModified();
}

void ItemDescEditTab::assignColorLabel(int colorId)
{
    d->colorLabelSelector->setColorLabel((ColorLabel)colorId);
}

void ItemDescEditTab::assignPickLabel(int pickId)
{
    d->pickLabelSelector->setPickLabel((PickLabel)pickId);
}

void ItemDescEditTab::assignRating(int rating)
{
    d->ratingWidget->setRating(rating);
}

void ItemDescEditTab::replaceColorLabel(int colorId)
{
    d->hub->replaceColorLabel(colorId);
    updateColorLabel();
}

void ItemDescEditTab::replacePickLabel(int pickId)
{
    d->hub->replacePickLabel(pickId);
    updatePickLabel();
}

void ItemDescEditTab::replaceRating(int rating)
{
    d->hub->replaceRating(rating);
    updateRating();
}

void ItemDescEditTab::updateComments()
{
    d->captionsEdit->blockSignals(true);
    d->captionsEdit->setValues(d->hub->comments());
    d->setMetadataWidgetStatus(d->hub->commentsStatus(), d->captionsEdit);
    d->captionsEdit->blockSignals(false);

    d->titleEdit->blockSignals(true);
    d->titleEdit->setValues(d->hub->titles().toAltLangMap());
    d->setMetadataWidgetStatus(d->hub->titlesStatus(), d->titleEdit);
    d->titleEdit->blockSignals(false);
}

void ItemDescEditTab::updatePickLabel()
{
    if (d->hub->pickLabelStatus() == DisjointMetadataDataFields::MetadataDisjoint)
    {
        d->pickLabelSelector->setPickLabel(NoPickLabel);
    }
    else
    {
        d->pickLabelSelector->setPickLabel((PickLabel)d->hub->pickLabel());
    }
}

void ItemDescEditTab::updateColorLabel()
{
    if (d->hub->colorLabelStatus() == DisjointMetadataDataFields::MetadataDisjoint)
    {
        d->colorLabelSelector->setColorLabel(NoColorLabel);
    }
    else
    {
        d->colorLabelSelector->setColorLabel((ColorLabel)d->hub->colorLabel());
    }
}

void ItemDescEditTab::updateRating()
{
    if (d->hub->ratingStatus() == DisjointMetadataDataFields::MetadataDisjoint)
    {
        d->ratingWidget->setRating(0);
    }
    else
    {
        d->ratingWidget->setRating(d->hub->rating());
    }
}

void ItemDescEditTab::updateDate()
{
    d->dateTimeEdit->blockSignals(true);
    d->dateTimeEdit->setDateTime(d->hub->dateTime());
    d->setMetadataWidgetStatus(d->hub->dateTimeStatus(), d->dateTimeEdit);
    d->dateTimeEdit->blockSignals(false);
}

void ItemDescEditTab::slotImageRatingChanged(qlonglong imageId)
{
    d->metadataChange(imageId);
}

void ItemDescEditTab::slotImageCaptionChanged(qlonglong imageId)
{
    d->metadataChange(imageId);
}

void ItemDescEditTab::slotImageDateChanged(qlonglong imageId)
{
    d->metadataChange(imageId);
}

void ItemDescEditTab::resetTitleEditPlaceholderText()
{
    d->titleEdit->setPlaceholderText(i18nc("@info", "Enter title text here."));
}

void ItemDescEditTab::resetCaptionEditPlaceholderText()
{
    d->captionsEdit->setPlaceholderText(i18nc("@info", "Enter caption text here."));
}

} // namespace Digikam
