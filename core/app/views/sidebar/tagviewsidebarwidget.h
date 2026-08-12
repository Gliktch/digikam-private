/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the tag view.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_config.h"
#include "albummodel.h"
#include "sidebarwidget.h"
#include "autotagsscansettings.h"

namespace Digikam
{

template <class T>
class AlbumPointer;

class TagViewSideBarWidget : public SidebarWidget
{

    Q_OBJECT

public:

    explicit TagViewSideBarWidget(QWidget* const parent, TagModel* const model);
    ~TagViewSideBarWidget()                                           override;

    void          setActive(bool active)                              override;
    void          doLoadState()                                       override;
    void          doSaveState()                                       override;
    void          applySettings()                                     override;
    void          changeAlbumFromHistory(const QList<Album*>& album)  override;
    const QIcon   getIcon()                                           override;
    const QString getCaption()                                        override;

    AlbumPointer<TAlbum> currentAlbum()                         const;

    void          doAutotagsScan(const AutotagsScanSettings& autotagsScanSettings);

private:

    void setNoTagsAlbum();

public Q_SLOTS:

    void setCurrentAlbum(TAlbum* album);
    void slotOpenTagManager();
    void slotToggleTagsSelection(int radioClicked);
    void slotScanForAutotags();
    void slotScanComplete();

Q_SIGNALS:

    void signalFindDuplicates(const QList<TAlbum*>& albums);

public:

    // Declared as public due to use by Private
    class Private;

private:

    Private* const d = nullptr;
};

} // namespace Digikam
