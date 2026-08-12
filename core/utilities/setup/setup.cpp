/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setup_p.h"

namespace Digikam
{

Setup::Setup(QWidget* const parent)
    : DConfigDlg(parent),
      d         (new Private)
{
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint   |
                   Qt::WindowContextHelpButtonHint);

    setWindowTitle(i18nc("@title:window", "Configure"));
    setStandardButtons(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    button(QDialogButtonBox::Ok)->setDefault(true);
    setConfigGroup(QLatin1String("Setup Dialog"));
    setFaceType(List);
    setModal(true);

    d->databasePage     = new SetupDatabase();
    d->page_database    = addPage(d->databasePage, i18nc("@title: settings section", "Database"));
    d->page_database->setHeader(i18nc("@title", "Database Settings\nCustomize database settings"));
    d->page_database->setIcon(QIcon::fromTheme(QLatin1String("network-server-database")));

    d->collectionsPage  = new SetupCollections();
    d->page_collections = addPage(d->collectionsPage, i18nc("@title: settings section", "Collections"));
    d->page_collections->setHeader(i18nc("@title", "Collections Settings\nSet root albums locations"));
    d->page_collections->setIcon(QIcon::fromTheme(QLatin1String("folder-pictures")));

    d->albumViewPage  = new SetupAlbumView();
    d->page_albumView = addPage(d->albumViewPage, i18nc("@title: settings section", "Views"));
    d->page_albumView->setHeader(i18nc("@title", "Application Views Settings\nCustomize the look of the views"));
    d->page_albumView->setIcon(QIcon::fromTheme(QLatin1String("view-list-icons")));

    d->tooltipPage  = new SetupToolTip();
    d->page_tooltip = addPage(d->tooltipPage, i18nc("@title: settings section", "Tool-Tip"));
    d->page_tooltip->setHeader(i18nc("@title", "Items Tool-Tip Settings\nCustomize information in item tool-tips"));
    d->page_tooltip->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));

    d->metadataPage  = new SetupMetadata();
    d->page_metadata = addPage(d->metadataPage, i18nc("@title: settings section", "Metadata"));
    d->page_metadata->setHeader(i18nc("@title", "Embedded Image Information Management\nSetup relations between images and metadata"));
    d->page_metadata->setIcon(QIcon::fromTheme(QLatin1String("format-text-code")));

    d->templatePage  = new SetupTemplate();
    d->page_template = addPage(d->templatePage, i18nc("@title: settings section", "Templates"));
    d->page_template->setHeader(i18nc("@title", "Metadata templates\nManage your collection of metadata templates"));
    d->page_template->setIcon(QIcon::fromTheme(QLatin1String("im-user")));

    d->editorPage  = new SetupEditor();
    d->page_editor = addPage(d->editorPage, i18nc("@title: settings section", "Image Editor"));
    d->page_editor->setHeader(i18nc("@title", "Image Editor Settings\nCustomize the image editor settings"));
    d->page_editor->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));

    d->iccPage  = new SetupICC(buttonBox());
    d->page_icc = addPage(d->iccPage, i18nc("@title: settings section", "Color Management"));
    d->page_icc->setHeader(i18nc("@title", "Settings for Color Management\nCustomize the color management settings"));
    d->page_icc->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-display-color")));

    d->lighttablePage  = new SetupLightTable();
    d->page_lighttable = addPage(d->lighttablePage, i18nc("@title: settings section", "Light Table"));
    d->page_lighttable->setHeader(i18nc("@title", "Light Table Settings\nCustomize tool used to compare items"));
    d->page_lighttable->setIcon(QIcon::fromTheme(QLatin1String("lighttable")));

    d->surveyPage      = new SetupSurvey();
    d->page_survey = addPage(d->surveyPage, i18nc("@title: settings section", "Survey"));
    d->page_survey->setHeader(i18nc("@title", "Survey Settings\nCustomize tool used to review items"));
    d->page_survey->setIcon(QIcon::fromTheme(QLatin1String("preview")));

#ifdef HAVE_GEOLOCATION

    d->geolocationPage  = new SetupGeolocation();
    d->page_geolocation = addPage(d->geolocationPage, i18nc("@title: settings section", "Geolocation"));
    d->page_geolocation->setHeader(i18nc("@title", "Geolocation Settings\nCustomize view to geolocalize items"));
    d->page_geolocation->setIcon(QIcon::fromTheme(QLatin1String("map-globe")));

#endif

    d->cameraPage  = new SetupCamera();
    d->page_camera = addPage(d->cameraPage, i18nc("@title: settings section", "Cameras"));
    d->page_camera->setHeader(i18nc("@title", "Camera Settings\nManage your camera devices"));
    d->page_camera->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));

    connect(d->cameraPage, SIGNAL(signalUseFileMetadataChanged(bool)),
            d->tooltipPage, SLOT(slotUseFileMetadataChanged(bool)));

    d->pluginsPage  = new SetupPlugins();
    d->page_plugins = addPage(d->pluginsPage, i18nc("@title: settings section", "Plugins"));
    d->page_plugins->setHeader(i18nc("@title", "Plug-in Settings\nSet which plugins will be accessible from application"));
    d->page_plugins->setIcon(QIcon::fromTheme(QLatin1String("preferences-plugin")));

    d->miscPage  = new SetupMisc(this);
    d->page_misc = addPage(d->miscPage, i18nc("@title: settings section", "Miscellaneous"));
    d->page_misc->setHeader(i18nc("@title", "Miscellaneous Settings\nCustomize behavior of the other parts of digiKam"));
    d->page_misc->setIcon(QIcon::fromTheme(QLatin1String("preferences-other")));

    for (int i = 0 ; i != SetupPageEnumLast ; ++i)
    {
        DConfigDlgWdgItem* const item = d->pageItem((Page)i);

        if (!item)
        {
            continue;
        }

        QWidget* const wgt            = item->widget();
        QScrollArea* const scrollArea = qobject_cast<QScrollArea*>(wgt);

        if (scrollArea)
        {
            scrollArea->setFrameShape(QFrame::NoFrame);
        }
    }

    connect(buttonBox(), SIGNAL(helpRequested()),
            this, SLOT(slotHelp()));

    connect(buttonBox()->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &Setup::slotOkClicked);

    connect(buttonBox()->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &Setup::slotCancelClicked);
}

Setup::~Setup()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));
    group.writeEntry(QLatin1String("Setup Page"),      (int)activePageIndex());
    group.writeEntry(QLatin1String("Collections Tab"), (int)d->collectionsPage->activeTab());
    group.writeEntry(QLatin1String("AlbumView Tab"),   (int)d->albumViewPage->activeTab());
    group.writeEntry(QLatin1String("ToolTip Tab"),     (int)d->tooltipPage->activeTab());
    group.writeEntry(QLatin1String("Metadata Tab"),    (int)d->metadataPage->activeTab());
    group.writeEntry(QLatin1String("Metadata SubTab"), (int)d->metadataPage->activeSubTab());
    group.writeEntry(QLatin1String("Editor Tab"),      (int)d->editorPage->activeTab());

#ifdef HAVE_GEOLOCATION

    group.writeEntry(QLatin1String("Geolocation Tab"), (int)d->geolocationPage->activeTab());

#endif

    group.writeEntry(QLatin1String("ICC Tab"),         (int)d->iccPage->activeTab());
    group.writeEntry(QLatin1String("Camera Tab"),      (int)d->cameraPage->activeTab());
    group.writeEntry(QLatin1String("Plugin Tab"),      (int)d->pluginsPage->activeTab());
    group.writeEntry(QLatin1String("Misc Tab"),        (int)d->miscPage->activeTab());
    config->sync();

    delete d;
}

} // namespace Digikam

#include "moc_setup.cpp"
