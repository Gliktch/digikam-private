/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "htmlintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "htmlwizard.h"
#include "galleryinfo.h"
#include "dlayoutbox.h"

namespace DigikamGenericHtmlGalleryPlugin
{

class Q_DECL_HIDDEN HTMLIntroPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<HTMLWizard*>(dialog);

        if (wizard)
        {
            info  = wizard->galleryInfo();
            iface = info->m_iface;
        }
    }

    QComboBox*       imageGetOption = nullptr;
    DHBox*           hbox           = nullptr;
    HTMLWizard*      wizard         = nullptr;
    GalleryInfo*     info           = nullptr;
    DInfoInterface*  iface          = nullptr;
};

HTMLIntroPage::HTMLIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                        "<p><h1><b>Welcome to HTML Gallery tool</b></h1></p>"
                        "<p>This assistant will guide you to export quickly</p><p></p>"
                        "<p>your images as a small static HTML photo gallery.</p>"
                        "<p>This tool is fully compliant with "
                        "<a href='https://en.wikipedia.org/wiki/HTML'>HTML and CSS standards</a></p>"
                        "<p>and the output can be customized with a nice theme.</p>"
                        "</qt>"));

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(GalleryInfo::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(GalleryInfo::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("text-html")));
}

HTMLIntroPage::~HTMLIntroPage()
{
    delete d;
}

void HTMLIntroPage::initializePage()
{
    bool albumSupport = (d->iface && d->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(GalleryInfo::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->info->m_getOption);
    }
}

bool HTMLIntroPage::validatePage()
{
    d->info->m_getOption = (GalleryInfo::ImageGetOption)d->imageGetOption->currentIndex();

    return true;
}

} // namespace DigikamGenericHtmlGalleryPlugin

#include "moc_htmlintropage.cpp"
