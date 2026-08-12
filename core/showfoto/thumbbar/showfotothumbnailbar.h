/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 02-08-2013
 * Description : Thumbnail bar for Showfoto
 *
 * SPDX-FileCopyrightText: 2013      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local Includes

#include "showfotocategorizedview.h"

namespace ShowFoto
{

class ShowfotoItemViewToolTip;

class ShowfotoThumbnailBar : public ShowfotoCategorizedView
{
    // cppcheck-suppress duplInheritedMember
    Q_OBJECT

public:

    explicit ShowfotoThumbnailBar(QWidget* const parent = nullptr);
    ~ShowfotoThumbnailBar()                                   override;

    /**
     * This installs a duplicate filter model, if the ShwofotoItemModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ShowfotoItemModel* model, ShowfotoSortFilterModel* filterModel);

    QModelIndex nextIndex(const QModelIndex& index)     const;
    QModelIndex previousIndex(const QModelIndex& index) const;
    QModelIndex firstIndex()                            const;
    QModelIndex lastIndex()                             const;

    int thumbnailIndexForUrl(const QUrl& url)           const;

    /**
     * Sets the policy always for the one scroll bar which is relevant, depending on orientation.
     */
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    ShowfotoItemInfo findItemByUrl(const QUrl& url);

    void installOverlays();

public Q_SLOTS:

    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected:

    bool event(QEvent*)                                       override;

private:

    /// @note disabled
    ShowfotoThumbnailBar(const ShowfotoThumbnailBar&)            = delete;
    ShowfotoThumbnailBar& operator=(const ShowfotoThumbnailBar&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
