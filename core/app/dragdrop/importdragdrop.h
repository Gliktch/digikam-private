/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-09-07
 * Description : Qt Model for ImportUI - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMenu>

// Local includes

#include "abstractitemdragdrophandler.h"
#include "importimagemodel.h"
#include "album.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT ImportDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    explicit ImportDragDropHandler(ImportItemModel* const model);

    ImportItemModel* model()                              const override;

    bool           dropEvent(QAbstractItemView* view,
                             const QDropEvent* e,
                             const QModelIndex& droppedOn)      override;

    Qt::DropAction accepts(const QDropEvent* e,
                           const QModelIndex& dropIndex)        override;

    QStringList    mimeTypes() const                            override;
    QMimeData*     createMimeData(const QList<QModelIndex>&)    override;

private Q_SLOTS:

    void slotUploadCamItems();

private:

    QAbstractItemView*    m_view       = nullptr;

    Qt::KeyboardModifiers m_modifiers;
    QPoint                m_position;
};

} // namespace Digikam
