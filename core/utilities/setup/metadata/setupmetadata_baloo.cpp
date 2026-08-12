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

void SetupMetadata::appendBalooTab()
{

#ifdef HAVE_KFILEMETADATA

    QWidget* const balooPanel      = new QWidget(d->tab);
    QVBoxLayout* const balooLayout = new QVBoxLayout(balooPanel);

    QGroupBox* const balooGroup    = new QGroupBox(i18nc("@option", "Baloo Desktop Search"), balooPanel);
    QVBoxLayout* const gLayout3    = new QVBoxLayout(balooGroup);

    d->saveToBalooBox              = new QCheckBox;
    d->saveToBalooBox->setText(i18nc("@option", "Store metadata from digiKam in Baloo"));
    d->saveToBalooBox->setWhatsThis(i18nc("@info", "Turn on this option to push rating, comments and tags "
                                                   "from digiKam into the Baloo storage"));

    d->readFromBalooBox            = new QCheckBox;
    d->readFromBalooBox->setText(i18nc("@option", "Read metadata from Baloo"));
    d->readFromBalooBox->setWhatsThis(i18nc("@info", "Turn on this option if you want to apply changes to "
                                                     "rating, comments and tags made in Baloo to digiKam's metadata storage. "
                                                     "Please note that image metadata will not be edited automatically."));

    gLayout3->addWidget(d->saveToBalooBox);
    gLayout3->addWidget(d->readFromBalooBox);

    // --------------------------------------------------------

    QFrame* const balooBox         = new QFrame(balooPanel);
    QGridLayout* const balooGrid   = new QGridLayout(balooBox);
    balooBox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QLabel* const balooLogoLabel   = new QLabel;
    balooLogoLabel->setPixmap(QIcon::fromTheme(QLatin1String("baloo")).pixmap(48));

    QLabel* const balooExplanation = new QLabel(balooBox);
    balooExplanation->setOpenExternalLinks(true);
    balooExplanation->setWordWrap(true);
    QString balootxt;

    balootxt.append(QString::fromUtf8("<p><a href='https://community.kde.org/Baloo'>Baloo</a> %1</p>")
                    .arg(i18nc("@info", "provides the basis to handle all kinds of metadata on the KDE desktop in a generic fashion. "
                               "It allows you to tag, rate and comment your files in KDE applications like Dolphin. "
                               "Please set here if you want to synchronize the metadata stored by digiKam desktop-wide with the "
                               "Baloo Desktop Search.")));

    balooExplanation->setText(balootxt);

    balooGrid->addWidget(balooLogoLabel,   0, 0, 1, 1);
    balooGrid->addWidget(balooExplanation, 0, 1, 1, 2);
    balooGrid->setColumnStretch(1, 10);
    balooGrid->setSpacing(0);

    // --------------------------------------------------------

    balooLayout->addWidget(balooGroup);
    balooLayout->addWidget(balooBox);
/*
    balooLayout->addWidget(d->resyncButton, 0, Qt::AlignRight);
*/
    balooLayout->addStretch();

    d->tab->insertTab(Baloo, balooPanel, i18nc("@title:tab", "Baloo"));

#endif // HAVE_KFILEMETADATA

}

} // namespace Digikam
