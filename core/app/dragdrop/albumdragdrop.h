/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class AlbumDragDropHandler : public AlbumModelDragDropHandler
{
    Q_OBJECT

public:

    explicit AlbumDragDropHandler(AlbumModel* const model);

    AlbumModel* model()                            const override;

    bool dropEvent(QAbstractItemView* view,
                   const QDropEvent* e,
                   const QModelIndex& droppedOn)         override;

    Qt::DropAction accepts(const QDropEvent* e,
                           const QModelIndex& dropIndex) override;

    QStringList mimeTypes() const override;
    QMimeData* createMimeData(const QList<Album*>&)      override;

private Q_SLOTS:

    void slotMoveCopyAlbum();
    void slotMoveCopyItems();
    void slotCopyFromCamera();
    void slotMoveCopyExtern();

private:

    QAbstractItemView*    m_view         = nullptr;
    QObject*              m_source       = nullptr;
    PAlbum*               m_destAlbum    = nullptr;
    PAlbum*               m_droppedAlbum = nullptr;

    Qt::KeyboardModifiers m_modifiers;
    QPoint                m_position;

    QList<QUrl>           m_srcURLs;
    QList<qlonglong>      m_imageIDs;
};

} // namespace Digikam
