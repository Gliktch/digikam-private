/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the date folder view.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "albummodel.h"
#include "itemalbumfiltermodel.h"
#include "sidebarwidget.h"

namespace Digikam
{

template <class T>
class AlbumPointer;

class DateFolderViewSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    explicit DateFolderViewSideBarWidget(QWidget* const parent,
                                         DateAlbumModel* const model,
                                         ItemAlbumFilterModel* const imageFilterModel);
    ~DateFolderViewSideBarWidget()                                    override;

    void          setActive(bool active)                              override;
    void          doLoadState()                                       override;
    void          doSaveState()                                       override;
    void          applySettings()                                     override;
    void          changeAlbumFromHistory(const QList<Album*>& album)  override;
    const QIcon   getIcon()                                           override;
    const QString getCaption()                                        override;

    AlbumPointer<DAlbum> currentAlbum()                         const;

    void gotoDate(const QDate& date);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
