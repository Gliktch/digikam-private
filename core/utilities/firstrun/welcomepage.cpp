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

#include "welcomepage.h"

// Qt includes

#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_version.h"

namespace Digikam
{

WelcomePage::WelcomePage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("Welcome to digiKam %1",
                       QLatin1String(digikam_version_short)))
{
    DVBox* const vbox   = new DVBox(this);
    QLabel* const title = new QLabel(vbox);
    title->setWordWrap(true);
    QString text        = i18n("<qt>"
                               "<p><h1><b>Welcome to digiKam %1</b></h1></p>"
                               "<p>digiKam is an advanced digital photo management "
                               "application published as open-source.</p>"
                               "<p>This assistant will help you to configure first "
                               "run settings to be able to use digiKam quickly.</p>"
                               "</qt>",
                               QLatin1String(digikam_version_short));

#if defined Q_OS_WIN || defined Q_OS_MACOS

    // Windows settings place.

    QString newPlace = QLatin1String("~/Local Settings/");
    QString oldPlace = QLatin1String("~/AppData/Local/");

#if defined Q_OS_MACOS

    // macOS settings place.

    newPlace         = QLatin1String("~/Library/Preferences/");
    oldPlace         = QLatin1String("~/Library/Preferences/KDE/share/config/");

#endif

    text.append(i18n(
                     "<qt><br>"
                     "<p>You can ignore the following if you use digiKam "
                     "for the first time:</p>"
                     "<p><b>Transition from digiKam 4</b></p>"
                     "<p>Configuration files from digiKam 4 are not "
                     "migrated. The old database can still be used, you "
                     "just need to choose the same locations for "
                     "collections and database in the following dialogs. "
                     "It is recommended to create a backup of your database "
                     "before proceeding.</p>"
                     "<p>The new location for configuration files is "
                     "%1 (old %2). "
                     "There are unresolved problems reported when reusing old "
                     "configuration files, so it is not recommended to do it "
                     "for the moment, and therefore use at your own risk.</p>"
                     "</qt>",
                     newPlace,
                     oldPlace));

#endif

    title->setText(text);
    setPageWidget(vbox);
}

WelcomePage::~WelcomePage()
{
}

} // namespace Digikam

#include "moc_welcomepage.cpp"
