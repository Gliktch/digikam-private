/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "expoblendinglastpage.h"

// Qt includes

#include <QDir>
#include <QLabel>
#include <QPixmap>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "enfusebinary.h"
#include "expoblendingmanager.h"
#include "dlayoutbox.h"

namespace DigikamGenericExpoBlendingPlugin
{

class Q_DECL_HIDDEN ExpoBlendingLastPage::Private
{
public:

    Private() = default;

public:

    ExpoBlendingManager* mngr = nullptr;
};

ExpoBlendingLastPage::ExpoBlendingLastPage(ExpoBlendingManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, QString::fromLatin1("<b>%1</b>").arg(i18nc("@title:window", "Pre-Processing is Complete"))),
      d          (new Private)
{
    d->mngr                 = mngr;
    DVBox* const vbox       = new DVBox(this);
    QLabel* const title     = new QLabel(vbox);
    title->setOpenExternalLinks(true);
    title->setWordWrap(true);
    title->setText(QString::fromUtf8("<qt>"
                                     "<p><h1><b>%1</b></h1></p>"
                                     "<p>%2</p>"
                                     "<p>%3</p>"
                                     "<p>%4</p>"
                                     "</qt>")
                   .arg(i18nc("@info", "Bracketed Images Pre-Processing is Done"))
                   .arg(i18nc("@info", "Congratulations. Your images are ready to be fused."))
                   .arg(i18nc("@info", "To perform this operation, Enblend program will be used."))
                   .arg(i18nc("@info", "Press \"Finish\" button to fuse your items and make a pseudo HDR image.")));

    vbox->setStretchFactor(new QWidget(vbox), 10);

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-enfuse.webp")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));
}

ExpoBlendingLastPage::~ExpoBlendingLastPage()
{
    delete d;
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_expoblendinglastpage.cpp"
