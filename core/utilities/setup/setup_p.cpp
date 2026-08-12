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

DConfigDlgWdgItem* Setup::Private::pageItem(Setup::Page page) const
{
    switch (page)
    {
        case Setup::DatabasePage:
        {
            return page_database;
        }

        case Setup::CollectionsPage:
        {
            return page_collections;
        }

        case Setup::AlbumViewPage:
        {
            return page_albumView;
        }

        case Setup::ToolTipPage:
        {
            return page_tooltip;
        }

        case Setup::MetadataPage:
        {
            return page_metadata;
        }

        case Setup::TemplatePage:
        {
            return page_template;
        }

        case Setup::LightTablePage:
        {
            return page_lighttable;
        }

        case Setup::SurveyPage:
        {
            return page_survey;
        }

#ifdef HAVE_GEOLOCATION

        case Setup::GeolocationPage:
        {
            return page_geolocation;
        }

#endif

        case Setup::EditorPage:
        {
            return page_editor;
        }

        case Setup::ICCPage:
        {
            return page_icc;
        }

        case Setup::CameraPage:
        {
            return page_camera;
        }

        case Setup::PluginsPage:
        {
            return page_plugins;
        }

        case Setup::MiscellaneousPage:
        {
            return page_misc;
        }

        default:
        {
            return nullptr;
        }
    }
}

} // namespace Digikam
