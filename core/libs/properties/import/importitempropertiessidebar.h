/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : item properties side bar used by import tool.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QWidget>

// Local includes

#include "sidebar.h"
#include "digikam_export.h"

namespace Digikam
{

class SidebarSplitter;
class CamItemInfo;
class DMetadata;

class DIGIKAM_GUI_EXPORT ImportItemPropertiesSideBarImport : public Sidebar
{
    Q_OBJECT

public:

    explicit ImportItemPropertiesSideBarImport(QWidget* const parent,
                                          SidebarSplitter* const splitter,
                                          Qt::Edge side = Qt::LeftEdge,
                                          bool mimimizedDefault = false);
    ~ImportItemPropertiesSideBarImport() override;

    void applySettings();

    void itemChanged(const CamItemInfo& itemInfo, const DMetadata& meta);

    QUrl url() const;

public Q_SLOTS:

    virtual void slotNoCurrentItem();

Q_SIGNALS:

    void signalFirstItem();
    void signalPrevItem();
    void signalNextItem();
    void signalLastItem();

protected:

    /**
     * load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState() override;

    /**
     * save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState() override;

private Q_SLOTS:

    virtual void slotChangedTab(QWidget* tab);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
