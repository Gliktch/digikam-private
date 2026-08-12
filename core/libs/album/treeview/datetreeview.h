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

#include "abstractcheckablealbumtreeview.h"

namespace Digikam
{

class DateTreeView : public AbstractCountingAlbumTreeView
{
    Q_OBJECT

public:

    explicit DateTreeView(QWidget* const parent = nullptr, Flags flags = DefaultFlags);

    DateAlbumModel* dalbumModel()                   const;
    DAlbum* currentAlbum()                          const;
    DAlbum* albumForIndex(const QModelIndex& index) const;

    void setDAlbumModel(DateAlbumModel* const model);
    void setAlbumFilterModel(AlbumFilterModel* const filterModel) override;

public Q_SLOTS:

    void setCurrentAlbum(int dateId, bool selectInAlbumManager = true);
};

} // namespace Digikam
