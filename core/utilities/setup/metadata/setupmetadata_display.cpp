/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * SPDX-FileCopyrightText: 2003-2004 by Ralf Holzer <ralf at well dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmetadata_p.h"

namespace Digikam
{

void SetupMetadata::appendDisplayTab()
{
    QWidget* const displayPanel      = new QWidget;
    QGridLayout* const displayLayout = new QGridLayout;

    QLabel* const displayLabel       = new QLabel(i18nc("@label", "Select Metadata Fields to be Displayed "
                                                                  "when using Custom Filters"));

    QLabel* const displayIcon        = new QLabel;
    displayIcon->setPixmap(QIcon::fromTheme(QLatin1String("view-list-tree")).pixmap(32));

    d->displaySubTab                 = new QTabWidget;
    d->tagsCfgPanel                  = new MetadataPanel(d->displaySubTab);

    displayLayout->addWidget(displayIcon,      0, 0);
    displayLayout->addWidget(displayLabel,     0, 1);
    displayLayout->addWidget(d->displaySubTab, 1, 0, 1, 3);
    displayLayout->setColumnStretch(2, 1);

    displayPanel->setLayout(displayLayout);
    d->tab->insertTab(Display, displayPanel, i18nc("@title:tab", "Views"));
}

} // namespace Digikam
