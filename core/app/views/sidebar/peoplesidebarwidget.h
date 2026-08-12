/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for People
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "albummodel.h"
#include "searchmodificationhelper.h"
#include "sidebarwidget.h"
#include "facescansettings.h"

namespace Digikam
{

class PeopleSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    explicit PeopleSideBarWidget(QWidget* const parent,
                                 TagModel* const tagModel,
                                 SearchModificationHelper* const searchModificationHelper);
    ~PeopleSideBarWidget()                                           override;

    void          setActive(bool active)                             override;
    void          doLoadState()                                      override;
    void          doSaveState()                                      override;
    void          applySettings()                                    override;
    void          changeAlbumFromHistory(const QList<Album*>& album) override;
    const QIcon   getIcon()                                          override;
    const QString getCaption()                                       override;

    static void   doFaceScan(const FaceScanSettings& faceScanSettings);

private Q_SLOTS:

    void slotInit();
    void slotScanForFaces();
    void slotScanComplete();

Q_SIGNALS:

    void signalFindDuplicates(const QList<TAlbum*>& albums);

private:

    class Private;
    static Private* d;
};

} // namespace Digikam
