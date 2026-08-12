/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "expoblendingintropage.h"

// Qt includes

#include <QTimer>
#include <QLabel>
#include <QPixmap>
#include <QGroupBox>
#include <QStandardPaths>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_config.h"
#include "dbinarysearch.h"
#include "alignbinary.h"
#include "enfusebinary.h"
#include "dlayoutbox.h"

namespace DigikamGenericExpoBlendingPlugin
{

class Q_DECL_HIDDEN ExpoBlendingIntroPage::Private
{
public:

    Private(ExpoBlendingManager* const m)
        : mngr(m)
    {
    }

    ExpoBlendingManager* mngr           = nullptr;
    DBinarySearch*       binariesWidget = nullptr;
};

ExpoBlendingIntroPage::ExpoBlendingIntroPage(ExpoBlendingManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "Welcome to Stacked Images Tool")),
      d          (new Private(mngr))
{
    DVBox* const vbox   = new DVBox(this);
    QLabel* const title = new QLabel(vbox);
    title->setWordWrap(true);
    title->setOpenExternalLinks(true);
    title->setText(QString::fromUtf8("<qt>"
                                     "<p><h1><b>%1</b></h1></p>"
                                     "<p>%2</p>"
                                     "<p>%3</p>"
                                     "<p>%4</p>"
                                     "<p>%5</p>"
                                     "<p>%6 <a href='https://en.wikipedia.org/wiki/Bracketing'>%7</a></p>"
                                     "</qt>")
                   .arg(i18nc("@info", "Welcome to Stacked Images Tool"))
                   .arg(i18nc("@info", "This tool fuses bracketed images with different exposure to make pseudo HDR Image"))
                   .arg(i18nc("@info", "It can also be used to merge focus bracketed stack to get a single image with increased depth of field."))
                   .arg(i18nc("@info", "This assistant will help you to configure how to import images before merging them to a single one."))
                   .arg(i18nc("@info", "Bracketed images must be taken with the same camera, in the same conditions, and if possible using a tripod."))
                   .arg(i18nc("@info", "For more information, please take a look at"))
                   .arg(i18nc("@info", "this page")));

    QGroupBox* const binaryBox      = new QGroupBox(vbox);
    QGridLayout* const binaryLayout = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title: group", "Exposure Blending Binaries"));
    d->binariesWidget = new DBinarySearch(binaryBox);
    d->binariesWidget->addBinary(d->mngr->alignBinary());
    d->binariesWidget->addBinary(d->mngr->enfuseBinary());

#ifdef Q_OS_MACOS

    // Hugin bundle PKG install

    d->binariesWidget->addDirectory(QLatin1String("/Applications/Hugin/HuginTools"));
    d->binariesWidget->addDirectory(QLatin1String("/Applications/Hugin/Hugin.app/Contents/MacOS"));
    d->binariesWidget->addDirectory(QLatin1String("/Applications/Hugin/tools_mac"));

    // Std Macports install

    d->binariesWidget->addDirectory(QLatin1String("/opt/local/bin"));

    // digiKam Bundle PKG install

    d->binariesWidget->addDirectory(macOSBundlePrefix() + QLatin1String("bin"));

#endif

#ifdef Q_OS_WIN

    d->binariesWidget->addDirectory(QLatin1String("C:/Program Files/Hugin/bin"));
    d->binariesWidget->addDirectory(QLatin1String("C:/Program Files (x86)/Hugin/bin"));
    d->binariesWidget->addDirectory(QLatin1String("C:/Program Files/GnuWin32/bin"));
    d->binariesWidget->addDirectory(QLatin1String("C:/Program Files (x86)/GnuWin32/bin"));

#endif

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-stack.webp")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->binariesWidget, SIGNAL(signalBinariesFound(bool)),
            this, SIGNAL(signalExpoBlendingIntroPageIsValid(bool)));

    QTimer::singleShot(1000, this, SLOT(slotExpoBlendingIntroPageIsValid()));
}

ExpoBlendingIntroPage::~ExpoBlendingIntroPage()
{
    delete d;
}

bool ExpoBlendingIntroPage::binariesFound()
{
    return d->binariesWidget->allBinariesFound();
}

void ExpoBlendingIntroPage::slotExpoBlendingIntroPageIsValid()
{
    Q_EMIT signalExpoBlendingIntroPageIsValid(binariesFound());
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_expoblendingintropage.cpp"
