/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-12
 * Description : tab for displaying item versions
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempropertiesversionstab.h"

// Qt includes

#include <QListView>
#include <QGridLayout>
#include <QLabel>
#include <QIcon>
#include <QString>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "itemversionsmodel.h"
#include "dmetadata.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "versionswidget.h"
#include "filtershistorywidget.h"
#include "applicationsettings.h"
#include "versionsoverlays.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPropertiesVersionsTab::Private
{
public:

    Private() = default;

    VersionsWidget*       versionsWidget        = nullptr;
    FiltersHistoryWidget* filtersHistoryWidget  = nullptr;
    DImageHistory         history;
    ItemInfo              info;

    const QString configActiveTab               = QLatin1String("Version Properties Tab");
};

ItemPropertiesVersionsTab::ItemPropertiesVersionsTab(QWidget* const parent)
    : QTabWidget(parent),
      d         (new Private)
{
    d->versionsWidget       = new VersionsWidget(this);
    insertTab(0, d->versionsWidget, i18nc("@title", "Versions"));

    d->filtersHistoryWidget = new FiltersHistoryWidget(this);
    insertTab(1, d->filtersHistoryWidget, i18nc("@title", "Used Filters"));

    connect(d->versionsWidget, SIGNAL(imageSelected(ItemInfo)),
            this, SIGNAL(imageSelected(ItemInfo)));
}

ItemPropertiesVersionsTab::~ItemPropertiesVersionsTab()
{
    delete d;
}

void ItemPropertiesVersionsTab::readSettings(KConfigGroup& group)
{
    QString tab = group.readEntry(d->configActiveTab, "versions");

    if (tab == QLatin1String("versions"))
    {
        setCurrentWidget(d->versionsWidget);
    }
    else
    {
        setCurrentWidget(d->filtersHistoryWidget);
    }

    d->versionsWidget->readSettings(group);
}

void ItemPropertiesVersionsTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configActiveTab, (currentWidget() == d->versionsWidget ? "versions" : "filters"));

    d->versionsWidget->writeSettings(group);
}

VersionsWidget* ItemPropertiesVersionsTab::versionsWidget() const
{
    return d->versionsWidget;
}

FiltersHistoryWidget* ItemPropertiesVersionsTab::filtersHistoryWidget() const
{
    return d->filtersHistoryWidget;
}

void ItemPropertiesVersionsTab::clear()
{
    d->filtersHistoryWidget->clearData();
    d->versionsWidget->setCurrentItem(ItemInfo());
}

void ItemPropertiesVersionsTab::setItem(const ItemInfo& info, const DImageHistory& history)
{
    clear();

    if (info.isNull())
    {
        return;
    }

    d->history = history;

    if (d->history.isNull())
    {
        d->history = info.imageHistory();
    }

    d->info = info;

    d->versionsWidget->setCurrentItem(info);
    d->filtersHistoryWidget->setHistory(d->history);
}

void ItemPropertiesVersionsTab::addShowHideOverlay()
{
    d->versionsWidget->addShowHideOverlay();
}

void ItemPropertiesVersionsTab::addOpenImageAction()
{
    ActionVersionsOverlay* const overlay = d->versionsWidget->addActionOverlay(QIcon::fromTheme(QLatin1String("document-open")),
                                                                               i18nc("@action: open image", "Open"),
                                                                               i18nc("@info:tooltip",  "Open file"));

    connect(overlay, SIGNAL(activated(ItemInfo)),
            this, SIGNAL(actionTriggered(ItemInfo)));
}

void ItemPropertiesVersionsTab::addOpenAlbumAction(const ItemModel* referenceModel)
{
    ActionVersionsOverlay* const overlay = d->versionsWidget->addActionOverlay(QIcon::fromTheme(QLatin1String("folder-pictures")),
                                                                               i18nc("@action: go to album", "Go To Albums"),
                                                                               i18nc("@info:tooltip", "Go to the album of this image"));
    overlay->setReferenceModel(referenceModel);

    connect(overlay, SIGNAL(activated(ItemInfo)),
            this, SIGNAL(actionTriggered(ItemInfo)));
}

void ItemPropertiesVersionsTab::setEnabledHistorySteps(int count)
{
    d->filtersHistoryWidget->setEnabledEntries(count);
}

} // namespace Digikam

#include "moc_itempropertiesversionstab.cpp"
