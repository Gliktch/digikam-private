/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Qt Model for Tags - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "albummodeldragdrophandler.h"
#include "albummodel.h"

namespace Digikam
{

class TagDragDropHandler : public AlbumModelDragDropHandler
{
    Q_OBJECT

public:

    explicit TagDragDropHandler(TagModel* const model);

    TagModel* model()                              const override;

    bool dropEvent(QAbstractItemView* view,
                   const QDropEvent* e,
                   const QModelIndex& droppedOn)         override;

    Qt::DropAction accepts(const QDropEvent* e,
                           const QModelIndex& dropIndex) override;

    QStringList mimeTypes() const                        override;
    QMimeData* createMimeData(const QList<Album*>&)      override;

Q_SIGNALS:

    void assignTags(const QList<qlonglong>& imageIDs, const QList<int>& tagIDs);

private Q_SLOTS:

    void slotMoveMergeTags();
    void slotConfirmPerson();
    void slotAssignTagItem();

private:

    QAbstractItemView*    m_view      = nullptr;
    QObject*              m_source    = nullptr;
    TAlbum*               m_destAlbum = nullptr;

    Qt::KeyboardModifiers m_modifiers;
    QPoint                m_position;

    QList<int>            m_tagIDs;
    QList<qlonglong>      m_imageIDs;
};

} // namespace Digikam
