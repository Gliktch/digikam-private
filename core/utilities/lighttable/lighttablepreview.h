/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : digiKam light table preview item.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QMimeData>

// Local includes

#include "iteminfo.h"
#include "itempreviewview.h"

namespace Digikam
{

class LightTablePreview : public ItemPreviewView
{
    Q_OBJECT

public:

    explicit LightTablePreview(QWidget* const parent = nullptr);
    ~LightTablePreview()                              override = default;

    void setDragAndDropEnabled(bool b);
    void showDragAndDropMessage();

Q_SIGNALS:

    void signalDroppedItems(const ItemInfoList&);

private:

    void dragMoveEvent(QDragMoveEvent*)               override;
    void dragEnterEvent(QDragEnterEvent*)             override;
    void dropEvent(QDropEvent*)                       override;
    bool dragEventWrapper(const QMimeData*)     const;
};

} // namespace Digikam
