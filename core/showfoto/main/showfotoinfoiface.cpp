/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to item information for Showfoto
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotoinfoiface.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "showfotosetup.h"
#include "digikam_debug.h"

namespace ShowFoto
{

ShowfotoInfoIface::ShowfotoInfoIface(QObject* const parent,
                                     const QList<QUrl>& lst,
                                     const QUrl& currentActive)
    : DMetaInfoIface(parent, lst, currentActive)
{
}

void ShowfotoInfoIface::openSetupPage(SetupPage page)
{
    switch (page)
    {
        case ExifToolPage:
        {
            if (ShowfotoSetup::execExifTool(nullptr))
            {
                Q_EMIT signalSetupChanged();
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

} // namespace ShowFoto

#include "moc_showfotoinfoiface.cpp"
