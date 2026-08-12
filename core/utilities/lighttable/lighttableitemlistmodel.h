/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : model for the light table thumbs bar.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "iteminfo.h"
#include "itemlistmodel.h"

namespace Digikam
{

class LightTableItemListModel : public ItemListModel
{
    Q_OBJECT

public:

    explicit LightTableItemListModel(QWidget* const parent);
    ~LightTableItemListModel()                                                                        override;

    void clearLightTableState();

    void setExclusiveLightTableState(bool exclusive);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                         const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole)         override;

    void prepareImageInfosAboutToBeRemoved(int begin, int end)                                        override;

    void imageInfosCleared()                                                                          override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
