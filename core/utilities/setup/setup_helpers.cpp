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

void Setup::setTemplate(const Template& t)
{
    if (d->templatePage)
    {
        d->templatePage->setTemplate(t);
    }
}

QSize Setup::sizeHint() const
{
    // The minimum size is very small. But the default initial size is such
    // that some important tabs get a scroll bar, although the dialog could be larger
    // on a normal display (QScrollArea size hint does not take widget into account)
    // Adjust size hint here so that certain selected tabs are display full per default.

    QSize hint          = DConfigDlg::sizeHint();
    int maxHintHeight   = 0;
    int maxWidgetHeight = 0;

    for (int page = 0 ; page != SetupPageEnumLast ; ++page)
    {
        // only take tabs into account here that should better be displayed without scrolling

        if (
            (page == CollectionsPage) ||
            (page == AlbumViewPage)   ||
            (page == TemplatePage)    ||
            (page == LightTablePage)  ||
            (page == SurveyPage)  ||
            (page == EditorPage)      ||
            (page == PluginsPage)     ||
            (page == MiscellaneousPage)
           )
        {
            DConfigDlgWdgItem* const item = d->pageItem((Page)page);

            if (!item)
            {
                continue;
            }

            QWidget* const wdg            = item->widget();
            maxHintHeight                 = qMax(maxHintHeight, wdg->sizeHint().height());
            QScrollArea* const scrollArea = qobject_cast<QScrollArea*>(wdg);

            if (scrollArea)
            {
                maxWidgetHeight = qMax(maxWidgetHeight, scrollArea->widget()->sizeHint().height());
            }
        }
    }

    // The additional 20 is a hack to make it work.
    // Don't know why, the largest page would have scroll bars without this

    if (maxWidgetHeight > maxHintHeight)
    {
        hint.setHeight(hint.height() + (maxWidgetHeight - maxHintHeight) + 20);
    }

    return hint;
}

bool Setup::openSetup(Setup* const setup)
{
    QEventLoop loop;
    bool success = false;

    connect(setup, &QDialog::finished,
            setup, [&loop, &success](int result)             // clazy:exclude=lambda-in-connect
            {
                success = (result == QDialog::Accepted);
                loop.quit();
            }
    );

    setup->DConfigDlg::open();

    loop.exec();
    delete setup;

    return success;
}

void Setup::showPage(Setup::Page page)
{
    DConfigDlgWdgItem* item = nullptr;

    if (page == LastPageUsed)
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));

        item = d->pageItem((Page)group.readEntry(QLatin1String("Setup Page"), (int)CollectionsPage));
        d->collectionsPage->setActiveTab((SetupCollections::CollectionsTab)group.readEntry(QLatin1String("Collections Tab"), (int)SetupCollections::Collections));
        d->albumViewPage->setActiveTab((SetupAlbumView::AlbumTab)group.readEntry(QLatin1String("AlbumView Tab"),             (int)SetupAlbumView::IconView));
        d->tooltipPage->setActiveTab((SetupToolTip::ToolTipTab)group.readEntry(QLatin1String("ToolTip Tab"),                 (int)SetupToolTip::IconItems));
        d->metadataPage->setActiveTab((SetupMetadata::MetadataTab)group.readEntry(QLatin1String("Metadata Tab"),             (int)SetupMetadata::Behavior));
        d->metadataPage->setActiveSubTab((SetupMetadata::MetadataSubTab)group.readEntry(QLatin1String("Metadata SubTab"),    (int)SetupMetadata::ExifViewer));

#ifdef HAVE_GEOLOCATION

        d->geolocationPage->setActiveTab((SetupGeolocation::GeolocationTab)group.readEntry(QLatin1String("Geolocation Tab"), (int)SetupGeolocation::MarbleView));

#endif

        d->editorPage->setActiveTab((SetupEditor::EditorTab)group.readEntry(QLatin1String("Editor Tab"),                     (int)SetupEditor::EditorWindow));
        d->iccPage->setActiveTab((SetupICC::ICCTab)group.readEntry(QLatin1String("ICC Tab"),                                 (int)SetupICC::Behavior));
        d->cameraPage->setActiveTab((SetupCamera::CameraTab)group.readEntry(QLatin1String("Camera Tab"),                     (int)SetupCamera::Devices));
        d->pluginsPage->setActiveTab((SetupPlugins::PluginTab)group.readEntry(QLatin1String("Plugin Tab"),                   (int)SetupPlugins::Generic));
        d->miscPage->setActiveTab((SetupMisc::MiscTab)group.readEntry(QLatin1String("Misc Tab"),                             (int)SetupMisc::Behavior));
    }
    else
    {
        item = d->pageItem(page);
    }

    if (!item)
    {
        item = d->pageItem(CollectionsPage);
    }

    setCurrentPage(item);
}

Setup::Page Setup::activePageIndex() const
{
    const DConfigDlgWdgItem* const cur = currentPage();

    if (cur == d->page_collections)
    {
        return CollectionsPage;
    }

    if (cur == d->page_albumView)
    {
        return AlbumViewPage;
    }

    if (cur == d->page_tooltip)
    {
        return ToolTipPage;
    }

    if (cur == d->page_metadata)
    {
        return MetadataPage;
    }

    if (cur == d->page_template)
    {
        return TemplatePage;
    }

    if (cur == d->page_lighttable)
    {
        return LightTablePage;
    }

    if (cur == d->page_survey)
    {
        return SurveyPage;
    }

#ifdef HAVE_GEOLOCATION

    if (cur == d->page_geolocation)
    {
        return GeolocationPage;
    }

#endif

    if (cur == d->page_editor)
    {
        return EditorPage;
    }

    if (cur == d->page_icc)
    {
        return ICCPage;
    }

    if (cur == d->page_camera)
    {
        return CameraPage;
    }

    if (cur == d->page_plugins)
    {
        return PluginsPage;
    }

    if (cur == d->page_misc)
    {
        return MiscellaneousPage;
    }

    return DatabasePage;
}

void Setup::onlineVersionCheck()
{
    OnlineVersionDlg* const dlg = new OnlineVersionDlg(qApp->activeWindow(),
                                                       QLatin1String(digikam_version_short),
                                                       digiKamBuildDate(),
                                                       ApplicationSettings::instance()->getUpdateType(),
                                                       ApplicationSettings::instance()->getUpdateWithDebug());

    connect(dlg, &OnlineVersionDlg::signalSetupUpdate,
            [=]()
        {
            Setup::execSinglePage(nullptr, Setup::MiscellaneousPage);
        }
    );

    dlg->exec();
}

} // namespace Digikam
