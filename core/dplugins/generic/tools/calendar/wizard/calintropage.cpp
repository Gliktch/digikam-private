/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to create calendar.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "calintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "calwizard.h"

namespace DigikamGenericCalendarPlugin
{

CalIntroPage::CalIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title)
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);

    QString str = QLatin1String("<qt>");
    str.append(i18n("<p><h1><b>Welcome to Calendar Tool</b></h1></p>"
                    "<p>This assistant will guide you to create "
                    "and print a calendar with a selection of images taken "
                    "from your collection.</p>"));

#ifdef HAVE_KCALENDAR

    str.append(i18n("<p>This tool will also allow to set specific dates "
                    "on your calendar using external data event files as "
                    "<a href='https://en.wikipedia.org/wiki/VCalendar'>vCalendar</a>, and "
                    "<a href='https://en.wikipedia.org/wiki/Icalendar'>iCalendar</a> formats.</p>"));

#endif

    str.append(QLatin1String("</qt>"));

    desc->setText(str);
    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("office-calendar")));
}

} // namespace DigikamGenericCalendarPlugin

#include "moc_calintropage.cpp"
