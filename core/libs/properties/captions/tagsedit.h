/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : tags editor
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QScrollArea>

// Local includes

#include "digikam_export.h"
#include "disjointmetadata.h"

namespace Digikam
{

class Album;
class TAlbum;
class SearchTextSettings;
class TaggingAction;
class AddTagsLineEdit;
class TagCheckView;

class DIGIKAM_GUI_EXPORT TagsEdit : public QScrollArea
{
    Q_OBJECT

public:

    explicit TagsEdit(DisjointMetadata* const hub, QWidget* const parent);
    ~TagsEdit()                                      override;

    void updateRecentTags();

    AddTagsLineEdit* newTagEdit() const;
    TagCheckView* tagsCheckView() const;
    void activateAssignedTagsButton();
    void updateTagsView();

Q_SIGNALS:

    void signalChanged();
    void signalImageTagsChanged(qlonglong imageId);

public Q_SLOTS:

    void slotUnifyPartiallyTags();

private Q_SLOTS:

    void slotOpenTagsManager();
    void slotTagStateChanged(Album* album, Qt::CheckState checkState);
    void slotTagsSearchChanged(const SearchTextSettings& settings);
    void slotAssignedTagsToggled(bool t);
    void slotTaggingActionActivated(const TaggingAction& action);
    void slotRecentTagsMenuActivated(int id);

private:

    void setTagState(TAlbum* const tag, DisjointMetadataDataFields::Status status);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
