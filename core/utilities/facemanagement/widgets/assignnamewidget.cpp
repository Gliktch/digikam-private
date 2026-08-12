/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-12
 * Description : Widget for assignment and confirmation of names for faces
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assignnamewidget_p.h"

namespace Digikam
{

AssignNameWidget::AssignNameWidget(QWidget* const parent)
    : QFrame(parent),
      d     (new Private(this))
{
    setObjectName(QLatin1String("assignNameWidget"));
    setVisualStyle(StyledFrame);
}

AssignNameWidget::~AssignNameWidget()
{
    delete d;
}

void AssignNameWidget::setDefaultModel()
{
    setAlbumModels(nullptr, nullptr, nullptr);
}

void AssignNameWidget::setAlbumModels(TagModel* const model,
                                      TagPropertiesFilterModel* const filteredModel,
                                      CheckableAlbumFilterModel* const filterModel)
{
    // Restrict the tag properties filter model to people if configured.

    const ApplicationSettings* const settings = ApplicationSettings::instance();

    if (settings)
    {
        if (filteredModel && settings->showOnlyPersonTagsInPeopleSidebar())
        {
            filteredModel->listOnlyTagsWithProperty(TagPropertyName::person());
        }
    }

    if      (d->comboBox)
    {
        d->comboBox->setAlbumModels(model, filteredModel, filterModel);
    }
    else if (d->lineEdit)
    {
        d->lineEdit->setAlbumModels(model, filteredModel, filterModel);
    }

    if (model || filteredModel || filterModel)
    {
        // Possibly set later on box.

        d->modelsGiven      = true;
        d->tagModel         = model;
        d->tagFilterModel   = filterModel;
        d->tagFilteredModel = filteredModel;
    }
}

void AssignNameWidget::setParentTag(TAlbum* album)
{
    d->parentTag = album;

    if      (d->comboBox)
    {
        d->comboBox->setParentTag(album);
    }
    else if (d->lineEdit)
    {
        d->lineEdit->setParentTag(album);
    }
}

AddTagsComboBox* AssignNameWidget::comboBox() const
{
    return d->comboBox;
}

AddTagsLineEdit* AssignNameWidget::lineEdit() const
{
    return d->lineEdit;
}

void AssignNameWidget::setMode(Mode mode)
{
    /**
     * Reject tooltip and icon should be updated even if the
     * same mode is passed, because Unconfirmed and Unknown.
     * Faces have the same mode but different tooltips and icons.
     */
    if (
        (d->layoutMode == Compact)                    &&
        (mode == AssignNameWidget::UnconfirmedEditMode)
       )
    {
        d->updateRejectButton();
    }

    if (mode == d->mode)
    {
        return;
    }

    d->mode = mode;
    d->updateModes();
    d->updateContents();
}

AssignNameWidget::Mode AssignNameWidget::mode() const
{
    return d->mode;
}

void AssignNameWidget::setTagEntryWidgetMode(TagEntryWidgetMode mode)
{
    if (d->widgetMode == mode)
    {
        return;
    }

    d->widgetMode = mode;
    d->updateModes();
    d->updateContents();
}

AssignNameWidget::TagEntryWidgetMode AssignNameWidget::tagEntryWidgetMode() const
{
    return d->widgetMode;
}

void AssignNameWidget::setLayoutMode(LayoutMode mode)
{
    if (d->layoutMode == mode)
    {
        return;
    }

    d->layoutMode = mode;
    d->updateModes();
    d->updateContents();
}

AssignNameWidget::LayoutMode AssignNameWidget::layoutMode() const
{
    return d->layoutMode;
}

void AssignNameWidget::setVisualStyle(VisualStyle style)
{
    if (d->visualStyle == style)
    {
        return;
    }

    d->visualStyle = style;
    d->updateModes();
}

AssignNameWidget::VisualStyle AssignNameWidget::visualStyle() const
{
    return d->visualStyle;
}

void AssignNameWidget::setUserData(const ItemInfo& info, const QVariant& faceIdentifier)
{
    d->info            = info;
    d->faceIdentifier  = faceIdentifier;

    FaceTagsIface face = FaceTagsIface::fromVariant(faceIdentifier);

    /**
     * Ignored faces are drawn over with a different
     * overlay, as Reject button should be disabled.
     */

    if      (face.type() == FaceTagsIface::ConfirmedName)
    {
        setMode(AssignNameWidget::ConfirmedMode);
    }
    else if (face.type() == FaceTagsIface::IgnoredName)
    {
        setMode(AssignNameWidget::IgnoredMode);
    }
    else
    {
        setMode(AssignNameWidget::UnconfirmedEditMode);
    }
}

ItemInfo AssignNameWidget::info() const
{
    return d->info;
}

QVariant AssignNameWidget::faceIdentifier() const
{
    return d->faceIdentifier;
}

void AssignNameWidget::setCurrentFace(const FaceTagsIface& face)
{
    TAlbum* album = nullptr;

    if (!face.isNull() && !face.isUnknownName() && !face.isIgnoredName())
    {
        album = AlbumManager::instance()->findTAlbum(face.tagId());
    }

    setCurrentTag(album);
}

void AssignNameWidget::setCurrentTag(int tagId)
{
    setCurrentTag(AlbumManager::instance()->findTAlbum(tagId));
}

void AssignNameWidget::setCurrentTag(TAlbum* album)
{
    if (d->currentTag == album)
    {
        if (d->clickLabel && (d->mode != IgnoredMode))
        {
            d->clickLabel->setText(d->currentTag ? d->currentTag->title()
                                                 : QString());
        }

        return;
    }

    d->currentTag = album;
    d->updateContents();
}

void AssignNameWidget::slotConfirm()
{
    if (d->comboBox)
    {
        Q_EMIT assigned(d->comboBox->currentTaggingAction(), d->info, d->faceIdentifier);
    }

    else if (d->lineEdit)
    {
        Q_EMIT assigned(d->lineEdit->currentTaggingAction(), d->info, d->faceIdentifier);
    }
}

void AssignNameWidget::slotReject()
{
    Q_EMIT rejected(d->info, d->faceIdentifier);
}

void AssignNameWidget::slotIgnore()
{
    Q_EMIT ignored(d->info, d->faceIdentifier);
}

void AssignNameWidget::slotActionActivated(const TaggingAction& action)
{
    Q_EMIT assigned(action, d->info, d->faceIdentifier);
}

void AssignNameWidget::slotActionSelected(const TaggingAction& action)
{
    if (d->confirmButton)
    {
        d->confirmButton->setEnabled(action.isValid());
    }

    Q_EMIT selected(action, d->info, d->faceIdentifier);
}

void AssignNameWidget::slotIgnoredClicked()
{
    Q_EMIT ignoredClicked(d->info, d->faceIdentifier);
}

void AssignNameWidget::slotLabelClicked()
{
    Q_EMIT labelClicked(d->info, d->faceIdentifier);
}

void AssignNameWidget::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            return;
        }
    }

    QWidget::keyPressEvent(e);
}

void AssignNameWidget::showEvent(QShowEvent* e)
{
    if ((d->mode == UnconfirmedEditMode) || (d->mode == ConfirmedEditMode))
    {
        if      (d->comboBox)
        {
            d->comboBox->setMinimumWidth(qMax(250, size().width() - 4));

            if (d->currentTag)
            {
                d->comboBox->blockSignals(true);
                d->comboBox->setCurrentTag(d->currentTag);
                d->comboBox->blockSignals(false);
            }

            d->comboBox->lineEdit()->selectAll();
            d->comboBox->lineEdit()->setFocus();
        }
        else if (d->lineEdit)
        {
            if (d->currentTag)
            {
                d->lineEdit->blockSignals(true);
                d->lineEdit->setCurrentTag(d->currentTag);
                d->lineEdit->blockSignals(false);
            }

            d->lineEdit->selectAll();
            d->lineEdit->setFocus();
        }
    }

    QWidget::showEvent(e);
}

} // namespace Digikam

#include "moc_assignnamewidget.cpp"
