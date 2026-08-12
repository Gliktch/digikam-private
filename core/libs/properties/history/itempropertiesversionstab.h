/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-12
 * Description : tab for displaying item versions
 *
 * SPDX-FileCopyrightText: 2010-2012 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QModelIndex>
#include <QPoint>
#include <QTabWidget>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"

class KConfigGroup;

namespace Digikam
{

class FiltersHistoryWidget;
class ItemInfo;
class ItemModel;
class VersionsWidget;

class ItemPropertiesVersionsTab : public QTabWidget
{
    Q_OBJECT

public:

    explicit ItemPropertiesVersionsTab(QWidget* const parent);
    ~ItemPropertiesVersionsTab() override;

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void clear();
    void setItem(const ItemInfo& info, const DImageHistory& history);

    VersionsWidget* versionsWidget()             const;
    FiltersHistoryWidget* filtersHistoryWidget() const;

    void addShowHideOverlay();
    void addOpenImageAction();
    void addOpenAlbumAction(const ItemModel* referenceModel);

public Q_SLOTS:

    void setEnabledHistorySteps(int count);

Q_SIGNALS:

    void imageSelected(const ItemInfo& info);
    void actionTriggered(const ItemInfo& info);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
