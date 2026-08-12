/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : A widget to select Physical or virtual albums with combo-box
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

namespace Digikam
{

class Album;
typedef QList<Album*> AlbumList;

class AlbumSelectors : public QWidget
{
    Q_OBJECT

public:

    enum AlbumType
    {
        PhysAlbum = 0,
        TagsAlbum,
        All
    };

    enum SelectionType
    {
        SingleSelection = 0,
        MultipleSelection
    };

public:

    /**
     * Default Constructor. 'label' is front text of label which title widget. 'configName' is name used to store
     * Albums configuration in settings file. 'parent' is parent widget.
     */
    explicit AlbumSelectors(const QString& label,
                            const QString& configName,
                            QWidget* const parent = nullptr,
                            AlbumType albumType = All,
                            bool allowRecursive = false);
    ~AlbumSelectors()                         override;

    /**
     * Return list of selected physical albums
     */
    AlbumList selectedAlbums()          const;

    /**
     * Return list of selected physical album ids
     */
    QList<int> selectedAlbumIds()       const;

    /**
     * Return list of selected tag albums
     */
    AlbumList selectedTags()            const;

    /**
     * Return list of selected tag album ids
     */
    QList<int> selectedTagIds()         const;

    /**
     * Return list of selected physical and tag albums.
     */
    AlbumList selectedAlbumsAndTags()   const;

    /**
     * Reset all Physical Albums selection.
     */
    void resetPAlbumSelection();

    /**
     * Reset all Tag Albums selection.
     */
    void resetTAlbumSelection();

    /**
     * Reset all Physical and Tag Albums selection.
     */
    void resetSelection();

    /** Select Physical Album from list. If singleSelection is true, only this one is
     *  selected from tree-view and all others are deselected.
     */
    void setAlbumSelected(Album* const album, SelectionType type);

    /** Select Tag Album from list. If singleSelection is true, only this one is
     *  selected from tree-view and all others are deselected.
     */
    void setTagSelected(Album* const album, SelectionType type);

    /**
     * Sets the search type selection with the AlbumType.
     */
    void setTypeSelection(int albumType);

    /**
     * Returns the selected album type.
     */
    int typeSelection()                 const;

    /**
     * Return true if whole Albums collection option is checked.
     */
    bool wholeAlbumsChecked()           const;

    /**
     * Return true if whole Tags collection option is checked.
     */
    bool wholeTagsChecked()             const;

public Q_SLOTS:

    /**
     * Called in constructor. Restore previous settings saved in configuration file.
     */
    void loadState();

    /**
     * Save settings in configuration file. Must be called explicitly by host implementation.
     */
    void saveState();

Q_SIGNALS:

    void signalSelectionChanged();

private Q_SLOTS:

    void slotUpdateClearButtons();
    void slotWholeAlbums(bool);
    void slotWholeTags(bool);

private:

    void initAlbumWidget();
    void initTagWidget();
    void updateTabText();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
