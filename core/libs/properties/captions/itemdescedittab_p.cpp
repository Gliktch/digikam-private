/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor - private container
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

ItemDescEditTab::Private::Private(ItemDescEditTab* const tab)
    : q(tab)
{
    spacing = layoutSpacing();
}

void ItemDescEditTab::Private::setupConnections()
{
    QObject::connect(titleEdit, SIGNAL(signalChanged(QString,QString)),
                     q, SLOT(slotTitleChanged()));

    QObject::connect(titleEdit, SIGNAL(signalValueAdded(QString,QString)),
                     q, SLOT(slotTitleChanged()));

    QObject::connect(titleEdit, SIGNAL(signalValueDeleted(QString)),
                     q, SLOT(slotTitleChanged()));

    QObject::connect(captionsEdit, SIGNAL(signalModified()),
                     q, SLOT(slotCommentChanged()));

    QObject::connect(dateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)),
                     q, SLOT(slotDateTimeChanged(QDateTime)));

    QObject::connect(pickLabelSelector, SIGNAL(signalPickLabelChanged(int)),
                     q, SLOT(slotPickLabelChanged(int)));

    QObject::connect(colorLabelSelector, SIGNAL(signalColorLabelChanged(int)),
                     q, SLOT(slotColorLabelChanged(int)));

    QObject::connect(ratingWidget, SIGNAL(signalRatingChanged(int)),
                     q, SLOT(slotRatingChanged(int)));

    QObject::connect(templateSelector, SIGNAL(signalTemplateSelected()),
                     q, SLOT(slotTemplateSelected()));

    QObject::connect(applyBtn, SIGNAL(clicked()),
                     q, SLOT(slotApplyAllChanges()));

    QObject::connect(applyToAllVersionsButton, SIGNAL(clicked()),
                     q, SLOT(slotApplyChangesToAllVersions()));

    QObject::connect(revertBtn, SIGNAL(clicked()),
                     q, SLOT(slotRevertAllChanges()));

    QObject::connect(moreMenu, SIGNAL(aboutToShow()),
                     q, SLOT(slotMoreMenu()));

    QObject::connect(metadataChangeTimer, SIGNAL(timeout()),
                     q, SLOT(slotReloadForMetadataChange()));

    QObject::connect(q, SIGNAL(signalAskToApplyChanges(QList<ItemInfo>,DisjointMetadata*)),
                     q, SLOT(slotAskToApplyChanges(QList<ItemInfo>,DisjointMetadata*)),
                     Qt::QueuedConnection);

    ItemAttributesWatch* const watch = ItemAttributesWatch::instance();

    QObject::connect(watch, SIGNAL(signalImagesChanged(int)),
                     q, SLOT(slotImagesChanged(int)));

    QObject::connect(watch, SIGNAL(signalImageRatingChanged(qlonglong)),
                     q, SLOT(slotImageRatingChanged(qlonglong)));

    QObject::connect(watch, SIGNAL(signalImageDateChanged(qlonglong)),
                     q, SLOT(slotImageDateChanged(qlonglong)));

    QObject::connect(watch, SIGNAL(signalImageCaptionChanged(qlonglong)),
                     q, SLOT(slotImageCaptionChanged(qlonglong)));
}

void ItemDescEditTab::Private::setupEventFilters()
{
    titleEdit->textEdit()->installEventFilter(q);
    captionsEdit->altLangStrEdit()->textEdit()->installEventFilter(q);

    dateTimeEdit->installEventFilter(q);
    pickLabelSelector->installEventFilter(q);
    colorLabelSelector->installEventFilter(q);
    ratingWidget->installEventFilter(q);

    // TODO update, what does this filter?

    tagCheckView->installEventFilter(q);
    newTagEdit->installEventFilter(q);
}

void ItemDescEditTab::Private::reset()
{
    modified = false;
    hub->resetChanged();
    applyBtn->setEnabled(false);
    revertBtn->setEnabled(false);

    Q_EMIT q->signalRightSideBarBusy(modified);

    applyToAllVersionsButton->setEnabled(false);
}

void ItemDescEditTab::Private::setInfos(const ItemInfoList& infos)
{
    if (infos.isEmpty())
    {
        hub->reset();
        captionsEdit->blockSignals(true);
        captionsEdit->reset();
        q->resetCaptionEditPlaceholderText();
        captionsEdit->blockSignals(false);
        titleEdit->blockSignals(true);
        titleEdit->reset();
        q->resetTitleEditPlaceholderText();
        titleEdit->blockSignals(false);
        currInfos.clear();
        resetMetadataChangeInfo();
        q->setEnabled(false);

        return;
    }

    q->setEnabled(true);
    currInfos = infos;
    modified  = false;
    resetMetadataChangeInfo();
    hub->reset();
    applyBtn->setEnabled(false);
    revertBtn->setEnabled(false);

    if (currInfos.count() > 1000)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

    // First, we read all tags of the items into the cache with one SQL query.
    // This is faster than each item individually.

    currInfos.loadTagIds();

    for (const ItemInfo& info : std::as_const(currInfos))
    {
        hub->load(info);
    }

    q->updateComments();
    q->updatePickLabel();
    q->updateColorLabel();
    q->updateRating();
    q->updateDate();
    q->updateTemplate();
    q->updateTagsView();
    q->updateRecentTags();
    setFocusToLastSelectedWidget();

    if (currInfos.count() > 1000)
    {
        QApplication::restoreOverrideCursor();
    }
}

void ItemDescEditTab::Private::resetMetadataChangeInfo()
{
    metadataChangeTimer->stop();
    metadataChangeIds.clear();
}

void ItemDescEditTab::Private::setFocusToLastSelectedWidget()
{
    if (lastSelectedWidget)
    {
        lastSelectedWidget->setFocus();
    }

    lastSelectedWidget = nullptr;
}

bool ItemDescEditTab::Private::singleSelection() const
{
    return (currInfos.count() == 1);
}

void ItemDescEditTab::Private::setMetadataWidgetStatus(int status, QWidget* const widget)
{
    if (status == DisjointMetadataDataFields::MetadataDisjoint)
    {
        // For text widgets: Set text color to color of disabled text.

        QPalette palette = widget->palette();
        palette.setColor(QPalette::Text, palette.color(QPalette::Disabled, QPalette::Text));
        widget->setPalette(palette);

        QString msg = i18nc("@info", "Selected items have different text here.");

        if      (qobject_cast<AltLangStrEdit*>(widget))
        {
            titleEdit->setPlaceholderText(msg);
        }
        else if (qobject_cast<CaptionEdit*>(widget))
        {
            captionsEdit->setPlaceholderText(msg);
        }
    }
    else
    {
        widget->setPalette(QPalette());

        if      (qobject_cast<AltLangStrEdit*>(widget))
        {
            q->resetTitleEditPlaceholderText();
        }
        else if (qobject_cast<CaptionEdit*>(widget))
        {
            q->resetCaptionEditPlaceholderText();
        }
    }
}

void ItemDescEditTab::Private::metadataChange(qlonglong imageId)
{
    if (ignoreItemAttributesWatch || modified)
    {
        // Don't lose modifications

        return;
    }

    metadataChangeIds << imageId;
    metadataChangeTimer->start();
}

void ItemDescEditTab::Private::initProgressIndicator()
{
    if (!ProgressManager::instance()->findItembyId(QLatin1String("ItemDescEditTabProgress")))
    {
        FileActionProgress* const item = new FileActionProgress(QLatin1String("ItemDescEditTabProgress"));

        QObject::connect(q, SIGNAL(signalProgressMessageChanged(QString)),
                         item, SLOT(slotProgressStatus(QString)));

        QObject::connect(q, SIGNAL(signalProgressValueChanged(float)),
                         item, SLOT(slotProgressValue(float)));

        QObject::connect(q, SIGNAL(signalProgressFinished()),
                         item, SLOT(slotCompleted()));
    }
}

} // namespace Digikam
