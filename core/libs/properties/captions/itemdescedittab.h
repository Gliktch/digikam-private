/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>
#include <QPixmap>
#include <QEvent>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"
#include "iteminfolist.h"
#include "albummanager.h"
#include "searchtextbar.h"
#include "disjointmetadata.h"

class KConfigGroup;

namespace Digikam
{

class ItemInfo;
class TaggingAction;
class DisjointMetadata;
class AddTagsLineEdit;

class ItemDescEditTab : public DVBox
{
    Q_OBJECT

public:

    enum DescEditTab
    {
        DESCRIPTIONS = 0,
        TAGS,
        INFOS
    };

public:

    explicit ItemDescEditTab(QWidget* const parent);
    ~ItemDescEditTab()                      override;

    void setItem(const ItemInfo& info = ItemInfo());
    void setItems(const ItemInfoList& infos);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    bool isModified() const;

    void setCurrentTab(int);
    int  currentTab() const;

Q_SIGNALS:

    void signalProgressMessageChanged(const QString& actionDescription);
    void signalProgressValueChanged(float percent);
    void signalRightSideBarBusy(bool busy);
    void signalProgressFinished();

    void signalPrevItem();
    void signalNextItem();

    void signalAskToApplyChanges(const QList<ItemInfo>& infos, DisjointMetadata* hub);

protected:

    bool eventFilter(QObject* o, QEvent* e) override;

private Q_SLOTS:

    void slotApplyAllChanges();
    void slotApplyChangesToAllVersions();
    void slotRevertAllChanges();
    void slotChangingItems();
    void slotModified();
    void slotReloadForMetadataChange();

    void slotImagesChanged(int albumId);

    void slotMoreMenu();
    void slotReadFromFileMetadataToDatabase();
    void slotWriteToFileMetadataFromDatabase();

    void slotAskToApplyChanges(const QList<ItemInfo>& infos, DisjointMetadata* hub);

    ///@{
    /// Description view methods (itemdescedittab_descview.cpp)

public:

    void assignColorLabel(int colorId);
    void assignPickLabel(int pickId);
    void assignRating(int rating);
    void setFocusToTitlesEdit();
    void setFocusToCommentsEdit();

    void replaceColorLabel(int colorId);
    void replacePickLabel(int pickId);
    void replaceRating(int rating);

private:

    void initDescriptionView();
    void updateComments();
    void updatePickLabel();
    void updateColorLabel();
    void updateRating();
    void updateDate();

    void resetTitleEditPlaceholderText();
    void resetCaptionEditPlaceholderText();

private Q_SLOTS:

    void slotCommentChanged();
    void slotTitleChanged();
    void slotDateTimeChanged(const QDateTime& dateTime);
    void slotPickLabelChanged(int pickId);
    void slotColorLabelChanged(int colorId);
    void slotRatingChanged(int rating);

    void slotImageRatingChanged(qlonglong imageId);
    void slotImageDateChanged(qlonglong imageId);
    void slotImageCaptionChanged(qlonglong imageId);

    ///@}

    ///@{
    /// Tags view methods (itemdescedittab_tagsview.cpp)

public:

    void populateTags();
    void setFocusToTagsView();
    void setFocusToNewTagEdit();
    void activateAssignedTagsButton();

    AddTagsLineEdit* getNewTagEdit() const;

Q_SIGNALS:

    void signalTagFilterMatch(bool);

private:

    void initTagsView();
    void setTagState(TAlbum* const tag, DisjointMetadataDataFields::Status status);
    void updateTagsView();
    void updateRecentTags();

private Q_SLOTS:

    void slotTagsSearchChanged(const SearchTextSettings& settings);
    void slotTagStateChanged(Album* album, Qt::CheckState checkState);
    void slotTaggingActionActivated(const TaggingAction&);
    void slotImageTagsChanged(qlonglong imageId);
    void slotOpenTagsManager();

    void slotRecentTagsMenuActivated(int);
    void slotAssignedTagsToggled(bool);
    void slotUnifyPartiallyTags();

    ///@}

    ///@{
    /// Information view methods (itemdescedittab_infoview.cpp)

private:

    void initInformationView();
    void updateTemplate();

private Q_SLOTS:

    void slotTemplateSelected();

    ///@}

    ///@{
    /// Private container (itemdescedittab_p.cpp)

private:

    class Private;
    Private* const d = nullptr;

    friend class Private;

    ///@}
};

} // namespace Digikam
