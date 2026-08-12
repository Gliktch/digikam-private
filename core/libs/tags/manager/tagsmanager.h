/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
 *
 * SPDX-FileCopyrightText:      2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPointer>
#include <QMainWindow>

// Local includes

#include "statesavingobject.h"

namespace Digikam
{

class TagModel;
class TAlbum;

class TagsManager : public QMainWindow,
                    public StateSavingObject
{
    Q_OBJECT

public:

    TagsManager();
    ~TagsManager()                      override;

public:

    static QPointer<TagsManager> internalPtr;
    static TagsManager* instance();
    static bool isCreated();

public:

    /**
     * @brief setup all gui elements for Tags Manager.
     */
    void setupUi();

protected:

    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event)   override;

    void doLoadState()                  override;
    void doSaveState()                  override;

Q_SIGNALS:

    void signalSelectionChanged(TAlbum* album);

private Q_SLOTS:

    // --- Action Slots.

    /**
     * @brief update tag properties in tagPropWidget when different item is selected.
     */
    void slotSelectionChanged();

    /**
     * @brief connected to invSel action and will invert selection of current items.
     */
    void slotInvertSelection();

    /**
     * @brief add new tag when addAction(+) is triggered.
     */
    void slotAddAction();

    /**
     * @brief delete tag/tags when delAction is triggered.
     */
    void slotDeleteAction();

    // --- Properties slots.

    /**
     * @brief connected to resetTagIcon action and will reset icon to all selected tags.
     */
    void slotResetTagIcon();

    /**
     * @brief view Tag Properties and set focus to title edit.
     */
     void slotEditTagTitle();

    /**
     * @brief title edit from Tag Properties was return button pressed.
     */
     void slotTitleEditReady();

    /**
     * @brief mark all tags that are not assigned to images.
     */
    void slotMarkNotAssignedTags();

    // --- Input / Output slots.

    /**
     * @brief connected to wrDbImg action and will write all metadata from database to images.
     */
    void slotWriteToImage();

    /**
     * @brief coonected to readTags action and will reread all images metadata into database.
     */
    void slotReadFromImage();

    /**
     * @brief will remove selected tags from all images that have them.
     */
    void slotRemoveTagsFromImages();

    /**
     * @brief connected to wipeAll action and will wipe all tag related data from database
     *        and reread from image's metadata.
     */
    void slotWipeAll();

    /**
     * @brief connected to saveTags action and will export all tags to a file.
     */
    void slotSaveTags();

    /**
     * @brief connected to loadTags action and will import all tags from a file.
     */
    void slotLoadTags();

private:

    /// @note disabled
    explicit TagsManager(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
