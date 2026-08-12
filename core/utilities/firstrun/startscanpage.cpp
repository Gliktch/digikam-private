/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "startscanpage.h"

// Qt includes

#include <QLabel>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"

namespace Digikam
{

StartScanPage::StartScanPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("Scan Your Collection"))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const text = new QLabel(vbox);
    text->setWordWrap(true);
    text->setOpenExternalLinks(true);
    text->setText(i18n("<qt>"
                       "<p>Congratulations. Your minimal setup is done.</p>"
                       "<p>Remember, you can customize more settings using the digiKam configuration panel. "
                       "To learn more about the digiKam world, we recommend reading the <b>digiKam handbook</b> "
                       "using the <b>Help/Handbook</b> menu entry (you need to install the separate digiKam documentation package.) "
                       "You can also read the manual online from the "
                       "<a href='https://docs.digikam.org/en/index.html'>dedicated website</a>.</p>"
                       "<p>Press <b>Finish</b> to close this assistant. digiKam will scan your "
                       "collection to register all albums in the database. Items including in "
                       "albums are collected after startup stage in background.</p>"
                       "<p><i>Note:</i> depending on your collection size, this operation can take a while. "
                       "If you cancel the scanning operation, it will start again in the next digiKam session.</p>"
                       "<p>When scanning has completed, at next start-up, digiKam will only check your collection "
                       "to identify new albums - which is much faster. You can force digiKam to scan for new items "
                       "manually using the <b>Tools->Maintenance->Scan for new items</b> menu entry.</p>"
                       "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("edit-find")));
}

StartScanPage::~StartScanPage()
{
}

} // namespace Digikam

#include "moc_startscanpage.cpp"
