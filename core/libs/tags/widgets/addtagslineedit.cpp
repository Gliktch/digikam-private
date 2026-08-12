/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creating tags
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 1997      by Sven Radej <sven dot radej at iname dot com>
 * SPDX-FileCopyrightText: 1999      by Patrick Ward <PAT_WARD at HP-USA-om5 dot om dot hp dot com>
 * SPDX-FileCopyrightText: 1999      by Preston Brown <pbrown at kde dot org>
 * SPDX-FileCopyrightText: 2000-2001 by Dawit Alemayehu <adawit at kde dot org>
 * SPDX-FileCopyrightText: 2000-2001 by Carsten Pfeiffer <pfeiffer at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "addtagslineedit.h"

// Local includes

#include "digikam_debug.h"
#include "tagscompleter.h"
#include "album.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "tagtreeview.h"
#include "taggingactionfactory.h"

namespace Digikam
{

class Q_DECL_HIDDEN AddTagsLineEdit::Private
{
public:

    Private() = default;

    TagCompleter*       completer       = nullptr;
    TagTreeView*        tagView         = nullptr;
    AlbumFilterModel*   tagFilterModel  = nullptr;

    TaggingAction       currentTaggingAction;
    int                 parentTagId     = 0;

    bool                returnPressed   = false;
};

AddTagsLineEdit::AddTagsLineEdit(QWidget* const parent)
    : QLineEdit(parent),
      d        (new Private)
{
    setClearButtonEnabled(true);

    d->completer = new TagCompleter(this);
    d->completer->setMaxVisibleItems(15);

    setCompleter(d->completer);

    connect(this, SIGNAL(returnPressed()),
            this, SLOT(slotReturnPressed()));

    connect(this, SIGNAL(editingFinished()),
            this, SLOT(slotEditingFinished()));

    connect(this, SIGNAL(textEdited(QString)),
            this, SLOT(slotTextEdited(QString)));

    connect(d->completer, QOverload<const TaggingAction&>::of(&TagCompleter::signalActivated),
            d->completer, [this](const TaggingAction& action)
        {
            completerActivated(action);
        }
    );

    connect(d->completer, QOverload<const TaggingAction&>::of(&TagCompleter::signalHighlighted),
            d->completer, [this](const TaggingAction& action)
        {
            completerHighlighted(action);
        }
    );
}

AddTagsLineEdit::~AddTagsLineEdit()
{
    delete d;
}

void AddTagsLineEdit::setSupportingTagModel(TagModel* const model)
{
    d->completer->setSupportingTagModel(model);
}

void AddTagsLineEdit::setFilterModel(AlbumFilterModel* const model)
{
    d->tagFilterModel = model;
    d->completer->setTagFilterModel(d->tagFilterModel);
}

void AddTagsLineEdit::setAlbumModels(TagModel* const model,
                                     TagPropertiesFilterModel* const filteredModel,
                                     AlbumFilterModel* const filterModel)
{
    if      (filterModel)
    {
        setFilterModel(filterModel);
    }
    else if (filteredModel)
    {
        setFilterModel(filteredModel);
    }

    setSupportingTagModel(model);
}

void AddTagsLineEdit::setTagTreeView(TagTreeView* const view)
{
    if (d->tagView)
    {
        disconnect(d->tagView, &TagTreeView::currentAlbumChanged, this,
                   &AddTagsLineEdit::setParentTag);
    }

    d->tagView = view;

    if (d->tagView)
    {
        connect(d->tagView, &TagTreeView::currentAlbumChanged, this,
                &AddTagsLineEdit::setParentTag);

        setParentTag(d->tagView->currentAlbum());
    }
}

void AddTagsLineEdit::setCurrentTag(TAlbum* const album)
{
    setCurrentTaggingAction(album ? TaggingAction(album->id()) : TaggingAction());
    setText(album ? album->title() : QString());
}

void AddTagsLineEdit::setParentTag(Album* const album)
{
    d->parentTagId = album ? album->id() : 0;
    d->completer->setContextParentTag(d->parentTagId);
}

void AddTagsLineEdit::setAllowExceedBound(bool value)
{
    Q_UNUSED(value);

    // -> the pop-up is allowed to be bigger than the line edit widget
    // Currently unimplemented, QCompleter calculates the size automatically.
    // Idea: intercept show event via event filter on completer->popup(); from there, change width.
}

/**
 * Tagging action is used by facemanagement and assignwidget
 */
void AddTagsLineEdit::slotReturnPressed()
{
    d->returnPressed = true;

    if (text().isEmpty())
    {
        //focus back to mainview

        Q_EMIT taggingActionFinished();
    }
    else
    {
        Q_EMIT taggingActionActivated(currentTaggingAction());
    }
}

void AddTagsLineEdit::slotEditingFinished()
{
    //d->currentTaggingAction = TaggingAction();
}

void AddTagsLineEdit::slotTextEdited(const QString& text)
{
    d->currentTaggingAction = TaggingAction();
    d->returnPressed        = false;

    if (text.isEmpty())
    {
        Q_EMIT taggingActionSelected(TaggingAction());
    }
    else
    {
        Q_EMIT taggingActionSelected(TaggingActionFactory::defaultTaggingAction(text, d->parentTagId));
    }

    d->completer->update(text);
}

void AddTagsLineEdit::completerActivated(const TaggingAction& action)
{
    if (!d->returnPressed)
    {
        setCurrentTaggingAction(action);

        Q_EMIT taggingActionActivated(action);
    }
}

void AddTagsLineEdit::completerHighlighted(const TaggingAction& action)
{
    setCurrentTaggingAction(action);
}

void AddTagsLineEdit::setCurrentTaggingAction(const TaggingAction& action)
{
    d->currentTaggingAction = action;

    Q_EMIT taggingActionSelected(action);
}

TaggingAction AddTagsLineEdit::currentTaggingAction() const
{
    if      (d->currentTaggingAction.isValid())
    {
        return d->currentTaggingAction;
    }
    else if (text().isEmpty())
    {
        return TaggingAction();
    }

    return TaggingActionFactory::defaultTaggingAction(text(), d->parentTagId);
}

} // namespace Digikam

#include "moc_addtagslineedit.cpp"
