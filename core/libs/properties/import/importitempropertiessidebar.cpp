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

#include "importitempropertiessidebar.h"

// Qt includes

#include <QSplitter>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_config.h"
#include "dmetadata.h"
#include "camiteminfo.h"
#include "importitempropertiestab.h"
#include "itempropertiesmetadatatab.h"

#ifdef HAVE_GEOLOCATION
#   include "itempropertiesgpstab.h"
#endif // HAVE_GEOLOCATION

namespace Digikam
{

class Q_DECL_HIDDEN ImportItemPropertiesSideBarImport::Private
{
public:

    Private() = default;

    bool                       dirtyMetadataTab     = false;
    bool                       dirtyCameraItemTab   = false;
    bool                       dirtyGpsTab          = false;

    DMetadata*                 metaData             = nullptr;

    CamItemInfo                itemInfo;

#ifdef HAVE_GEOLOCATION

    ItemPropertiesGPSTab*      gpsTab               = nullptr;

#endif // HAVE_GEOLOCATION

    ItemPropertiesMetadataTab* metadataTab          = nullptr;
    ImportItemPropertiesTab*   cameraItemTab        = nullptr;
};

ImportItemPropertiesSideBarImport::ImportItemPropertiesSideBarImport(QWidget* const parent,
                                                                     SidebarSplitter* const splitter,
                                                                     Qt::Edge side,
                                                                     bool mimimizedDefault)
    : Sidebar(parent, splitter, side, mimimizedDefault),
      d      (new Private)
{
    d->cameraItemTab = new ImportItemPropertiesTab(parent);
    d->metadataTab   = new ItemPropertiesMetadataTab(parent);

    appendTab(d->cameraItemTab, QIcon::fromTheme(QLatin1String("configure")),             i18n("Properties"));
    appendTab(d->metadataTab,   QIcon::fromTheme(QLatin1String("format-text-code")),              i18n("Metadata")); // krazy:exclude=iconnames

#ifdef HAVE_GEOLOCATION

    d->gpsTab        = new ItemPropertiesGPSTab(parent);
    appendTab(d->gpsTab,        QIcon::fromTheme(QLatin1String("globe")), i18n("Geolocation"));

#endif // HAVE_GEOLOCATION

    // --- NOTE: use dynamic binding as slotChangedTab() is a virtual method which can be re-implemented in derived classes.

    connect(this, &ImportItemPropertiesSideBarImport::signalChangedTab,
            this, &ImportItemPropertiesSideBarImport::slotChangedTab);
}

ImportItemPropertiesSideBarImport::~ImportItemPropertiesSideBarImport()
{
    delete d;
}

void ImportItemPropertiesSideBarImport::applySettings()
{
    /// @todo Still needed?

    /// @todo Are load/saveState called by the creator?
}

QUrl ImportItemPropertiesSideBarImport::url() const
{
    return d->itemInfo.url();
}

void ImportItemPropertiesSideBarImport::itemChanged(const CamItemInfo& itemInfo, const DMetadata& meta)
{
    if (itemInfo.isNull())
    {
        return;
    }

    if (d->metaData)
    {
        delete d->metaData;
    }

    d->metaData           = new DMetadata(meta.data());
    d->itemInfo           = itemInfo;
    d->dirtyMetadataTab   = false;
    d->dirtyCameraItemTab = false;
    d->dirtyGpsTab        = false;

    slotChangedTab(getActiveTab());
}

void ImportItemPropertiesSideBarImport::slotNoCurrentItem()
{
    d->itemInfo           = CamItemInfo();

    if (d->metaData)
    {
        delete d->metaData;
        d->metaData = nullptr;
    }

    d->dirtyMetadataTab   = false;
    d->dirtyCameraItemTab = false;
    d->dirtyGpsTab        = false;

    d->cameraItemTab->setCurrentItem();
    d->metadataTab->setCurrentUrl();

#ifdef HAVE_GEOLOCATION

    d->gpsTab->setCurrentUrl();

#endif // HAVE_GEOLOCATION

}

void ImportItemPropertiesSideBarImport::slotChangedTab(QWidget* tab)
{
    if (d->itemInfo.isNull())
    {
        return;
    }

    setCursor(Qt::WaitCursor);

    if      ((tab == d->cameraItemTab) && !d->dirtyCameraItemTab)
    {
        d->cameraItemTab->setCurrentItem(d->itemInfo, d->metaData);

        d->dirtyCameraItemTab = true;
    }
    else if ((tab == d->metadataTab) && !d->dirtyMetadataTab)
    {
        d->metadataTab->setCurrentData(d->metaData, d->itemInfo.url());
        d->dirtyMetadataTab = true;
    }

#ifdef HAVE_GEOLOCATION

    else if ((tab == d->gpsTab) && !d->dirtyGpsTab)
    {
        d->gpsTab->setMetadata(d->metaData, d->itemInfo.url());
        d->dirtyGpsTab = true;
    }

    d->gpsTab->setActive(tab == d->gpsTab);

#endif // HAVE_GEOLOCATION

    unsetCursor();
}

void ImportItemPropertiesSideBarImport::doLoadState()
{
    /// @todo This code is taken from ItemPropertiesSideBar::doLoadState()
    ///       Ideally ImportItemPropertiesSideBarImport should be a subclass of
    ///       ItemPropertiesSideBar

    Sidebar::doLoadState();

    KConfigGroup group                  = getConfigGroup();

    KConfigGroup groupCameraItemTab     = KConfigGroup(&group, entryName(QLatin1String("Camera Item Properties Tab")));
    d->cameraItemTab->readSettings(groupCameraItemTab);

#ifdef HAVE_GEOLOCATION

    KConfigGroup groupGPSTab            = KConfigGroup(&group, entryName(QLatin1String("GPS Properties Tab")));
    d->gpsTab->readSettings(groupGPSTab);

#endif // HAVE_GEOLOCATION

    const KConfigGroup groupMetadataTab = KConfigGroup(&group, entryName(QLatin1String("Metadata Properties Tab")));
    d->metadataTab->readSettings(groupMetadataTab);
}

void ImportItemPropertiesSideBarImport::doSaveState()
{
    /// @todo This code is taken from ItemPropertiesSideBar::doSaveState()
    ///       Ideally ImportItemPropertiesSideBarImport should be a subclass of
    ///       ItemPropertiesSideBar

    Sidebar::doSaveState();

    KConfigGroup group              = getConfigGroup();

    KConfigGroup groupCameraItemTab = KConfigGroup(&group, entryName(QLatin1String("Camera Item Properties Tab")));
    d->cameraItemTab->writeSettings(groupCameraItemTab);

#ifdef HAVE_GEOLOCATION

    KConfigGroup groupGPSTab        = KConfigGroup(&group, entryName(QLatin1String("GPS Properties Tab")));
    d->gpsTab->writeSettings(groupGPSTab);

#endif // HAVE_GEOLOCATION

    KConfigGroup groupMetadataTab   = KConfigGroup(&group, entryName(QLatin1String("Metadata Properties Tab")));
    d->metadataTab->writeSettings(groupMetadataTab);
}

} // namespace Digikam

#include "moc_importitempropertiessidebar.cpp"
