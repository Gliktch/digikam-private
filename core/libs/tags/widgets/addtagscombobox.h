/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-15
 * Description : Special combo box for adding or creating tags
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QEvent>

// Local includes

#include "albumselectcombobox.h"
#include "taggingaction.h"

namespace Digikam
{

class AddTagsCompletionBox;
class AddTagsLineEdit;
class Album;
class CheckableAlbumFilterModel;
class TAlbum;
class TagModel;
class TagPropertiesFilterModel;
class TagTreeView;

class AddTagsComboBox : public TagTreeViewSelectComboBox
{
    Q_OBJECT

public:

    explicit AddTagsComboBox(QWidget* const parent = nullptr);
    ~AddTagsComboBox()                                                          override;

    /**
     * @brief You must call this after construction.
     * If filtered/filterModel is 0, a default one is constructed
     */
    void setAlbumModels(TagModel* const model,
                        TagPropertiesFilterModel* const filteredModel = nullptr,
                        CheckableAlbumFilterModel* const filterModel = nullptr) override;

    /**
     * @return the currently set tagging action.
     * This is the last action emitted by either taggingActionActivated()
     * or taggingActionSelected()
     */
    TaggingAction currentTaggingAction();

    /**
     * @brief Sets the currently selected tag
     */
    void setCurrentTag(TAlbum* const album);

    void setPlaceholderText(const QString& message);

    QString text()                                              const;
    void setText(const QString& text);

    AddTagsLineEdit* lineEdit()                                 const;

public Q_SLOTS:

    /**
     * @brief Set a parent tag for suggesting a parent tag for a new tag, and a default action.
     */
    void setParentTag(TAlbum* const album);

Q_SIGNALS:

    /**
     * @brief Emitted when the user activates an action (typically, by pressing return)
     */
    void taggingActionActivated(const TaggingAction& action);

    /**
     * @brief Emitted when an action is selected, but not explicitly activated.
     * (typically by selecting an item in the tree view
     */
    void taggingActionSelected(const TaggingAction& action);

protected Q_SLOTS:

    void slotViewIndexActivated(const QModelIndex&);
    void slotLineEditActionActivated(const TaggingAction& action);
    void slotLineEditActionSelected(const TaggingAction& action);

protected:

    bool eventFilter(QObject* object, QEvent* event)                            override;

private:

    /// @note disabled
    void setEditable(bool editable);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
