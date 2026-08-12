/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "abstractcountingalbumtreeview.h"

namespace Digikam
{

class AbstractCheckableAlbumTreeView : public AbstractCountingAlbumTreeView
{
    Q_OBJECT

public:

    /**
     * @note Models of these view _can_ be checkable, they need _not_. You need to enable it on the model.
     */
    explicit AbstractCheckableAlbumTreeView(QWidget* const parent, Flags flags);
    ~AbstractCheckableAlbumTreeView()                                     override;

    /**
     * @brief Manage check state through the model directly.
     */
    AbstractCheckableAlbumModel* checkableAlbumModel()              const;
    CheckableAlbumFilterModel* checkableAlbumFilterModel()          const;

    /**
     * @brief Enable checking on middle mouse button click (default: on).
     */
    void setCheckOnMiddleClick(bool doThat);

    /**
     * @brief Tells if the check state is restored while loading / saving state.
     *
     * @return true if restoring check state is active
     */
    bool isRestoreCheckState()                                      const;

    /**
     * @brief Set whether to restore check state or not.
     *
     * @param restore if true, restore check state
     */
    void setRestoreCheckState(bool restore);

    void doLoadState()                                                    override;
    void doSaveState()                                                    override;

protected:

    void middleButtonPressed(Album* a)                                    override;
    void rowsInserted(const QModelIndex& parent, int start, int end)      override;

    void setAlbumModel(AbstractCheckableAlbumModel* const model);
    virtual void setCheckableAlbumFilterModel(CheckableAlbumFilterModel* const filterModel);

private:

    void restoreCheckStateForHierarchy(const QModelIndex& index);
    void restoreCheckState(const QModelIndex& index);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
