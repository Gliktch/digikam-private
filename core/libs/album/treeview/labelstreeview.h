/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View.
 *
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>

// Local includes

#include "coredbconstants.h"
#include "album.h"
#include "statesavingobject.h"
#include "itemlisterrecord.h"

namespace Digikam
{

class LabelsTreeView : public QTreeWidget, public StateSavingObject
{
    Q_OBJECT

public:

    enum Labels
    {
        Ratings = 0,
        Picks,
        Colors
    };

public:

    explicit LabelsTreeView(QWidget* const parent = nullptr, bool setCheckable = false);
    ~LabelsTreeView() override;

    /**
     * @return true if the tree widget is checkable
     *         and false if not
     */
    bool isCheckable()                           const;

    /**
     * @return true if Loading state function is running
     */
    bool isLoadingState()                        const;

    /**
     * @return a Pixmap of a 30*30 pixels golden star used
     *         for rating and widget icon
     */
    QPixmap goldenStarPixmap(bool fillin=true)   const;

    /**
     * @brief Creates a 30*30 rectangular pixmap with
     *        specific color
     *
     * @param color wanted to be set
     * @return pixmap has a rectangle filled with the color
     */
    QPixmap colorRectPixmap(const QColor& color) const;

    /**
     * @brief Provide the current selection from the tree-view
     *        hierarchy
     *
     * @return a QHash with three keys: "Ratings", "Picks", and
     *         "Colors", every key dedicated to an int list which
     *         holds the rows selected
     */
    QHash<Labels, QList<int> > selectedLabels();

    /**
     * @brief Restores the selection state from the AlbumHistory
     *        class
     *
     * @param neededLabels is a QHash to restore the selection
     *        from it, the hash is formatted just like the hash
     *        generated from @see selectedLabels()
     */
    void restoreSelectionFromHistory(QHash<Labels, QList<int> > neededLabels);

    /**
     * @brief Emits a signal to the search handler to set the Current
     *        album from currently selected labels
     */
    void setCurrentAlbum();

protected:

    /**
     * @brief Loading and saving state function inherited from
     *        StateSavingObject
     */
    void doLoadState() override;
    void doSaveState() override;

private:

    /**
     * @brief Initializing the tree-view hierarchy
     */
    void initTreeView();
    void initRatingsTree();
    void initPicksTree();
    void initColorsTree();

    /**
     * @brief Index based access to the items
     */
    QTreeWidgetItem* getOrCreateItem(QTreeWidgetItem* const parent);

private Q_SLOTS:

    void slotSettingsChanged();

Q_SIGNALS:

    void signalSetCurrentAlbum();
    void signalColorNameChanged(int, const QString& name);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
