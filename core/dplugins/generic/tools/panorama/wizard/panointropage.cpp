/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending tool
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "panointropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"
#include "dbinarysearch.h"
#include "autooptimiserbinary.h"
#include "cpcleanbinary.h"
#include "cpfindbinary.h"
#include "panomodifybinary.h"
#include "huginexecutorbinary.h"
#include "dlayoutbox.h"

namespace DigikamGenericPanoramaPlugin
{

class Q_DECL_HIDDEN PanoIntroPage::Private
{
public:

    explicit Private(PanoManager* const m)
        : mngr(m)
    {
    }

    PanoManager*    mngr                        = nullptr;
/*
    QCheckBox*      addGPlusMetadataCheckBox    = nullptr;
*/
/*
    // TODO HDR
    QCheckBox*      hdrCheckBox                 = nullptr;
*/
    QGroupBox*      formatGroupBox              = nullptr;
    QGroupBox*      settingsGroupBox            = nullptr;
    QRadioButton*   jpegRadioButton             = nullptr;
    QRadioButton*   tiffRadioButton             = nullptr;
    QRadioButton*   hdrRadioButton              = nullptr;
    DBinarySearch*  binariesWidget              = nullptr;
};

PanoIntroPage::PanoIntroPage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, QString::fromLatin1("<b>%1</b>").arg(i18nc("@title: window", "Welcome to Panorama Tool"))),
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
                                     "<p>%5 <a href='http://hugin.sourceforge.net/tutorials/overview/en.shtml'>%6</a></p>"      // krazy:exclude=insecurenet
                                     "</qt>")
                   .arg(i18nc("@info", "Welcome to Panorama Tool"))
                   .arg(i18nc("@info", "This tool stitches several images together to create a panorama, making the seam between images not visible."))
                   .arg(i18nc("@info", "This assistant will help you to configure how to import images before stitching them into a panorama."))
                   .arg(i18nc("@info", "Images must be taken from the same point of view."))
                   .arg(i18nc("@info", "For more information, please take a look at "))
                   .arg(i18nc("@info", "this page")));

    QGroupBox* const binaryBox        = new QGroupBox(vbox);
    QGridLayout* const binaryLayout   = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title: group", "Panorama Binaries"));
    d->binariesWidget = new DBinarySearch(binaryBox);
    d->binariesWidget->addBinary(d->mngr->autoOptimiserBinary());
    d->binariesWidget->addBinary(d->mngr->cpCleanBinary());
    d->binariesWidget->addBinary(d->mngr->cpFindBinary());
    d->binariesWidget->addBinary(d->mngr->panoModifyBinary());
    d->binariesWidget->addBinary(d->mngr->huginExecutorBinary());

    d->mngr->checkBinaries();

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

/*
    QVBoxLayout* const settingsVBox = new QVBoxLayout();
    d->settingsGroupBox             = new QGroupBox(i18nc("@title:group", "Panorama Settings"), this);
    d->settingsGroupBox->setLayout(settingsVBox);

    d->addGPlusMetadataCheckBox     = new QCheckBox(i18nc("@option:check", "Add Photosphere Metadata"), d->settingsGroupBox);
    d->addGPlusMetadataCheckBox->setToolTip(i18nc("@info:tooltip", "Add Exif metadata to the output panorama image for Google+ 3D viewer"));
    d->addGPlusMetadataCheckBox->setWhatsThis(i18nc("@info:whatsthis", "\"Add Photosphere Metadata\": Enabling this allows the program to add "
                                                    "metadata to the output image such that when uploaded to Google+, the "
                                                    "Google+ 3D viewer is activated and the panorama can be seen in 3D. Note "
                                                    "that this feature is most interesting for large panoramas."));
    settingsVBox->addWidget(d->addGPlusMetadataCheckBox);
    vbox->addWidget(d->settingsGroupBox);
*/
    QVBoxLayout* const formatVBox = new QVBoxLayout();
    d->formatGroupBox             = new QGroupBox(i18nc("@title: group", "File Format"), vbox);
    d->formatGroupBox->setLayout(formatVBox);
    QButtonGroup* const group     = new QButtonGroup();

    d->jpegRadioButton            = new QRadioButton(i18nc("@option: radio", "JPEG output"), d->formatGroupBox);

    // The following comment is to get the next string extracted for translation

    // xgettext: no-c-format
    d->jpegRadioButton->setToolTip(i18nc("@info: tooltip", "Selects a JPEG output with 90% compression rate "
                                         "(lossy compression, smaller size)."));
    d->jpegRadioButton->setWhatsThis(i18nc("@info:whatsthis", "\"JPEG output\": Using JPEG output, the panorama file will be smaller "
                                           "at the cost of information loss during compression. This is the easiest "
                                           "way to share the result, or print it online or in a shop."));
    formatVBox->addWidget(d->jpegRadioButton);
    group->addButton(d->jpegRadioButton);

    d->tiffRadioButton          = new QRadioButton(i18nc("@option: radio", "TIFF output"), d->formatGroupBox);
    d->tiffRadioButton->setToolTip(i18nc("@info: tooltip", "Selects a TIFF output compressed using the LZW algorithm "
                                         "(lossless compression, bigger size)."));
    d->tiffRadioButton->setWhatsThis(i18nc("@info: whatsthis", "\"TIFF output\": Using TIFF output, you get the same color depth as "
                                           "your original photos using RAW images at the cost of a bigger panorama file."));
    formatVBox->addWidget(d->tiffRadioButton);
    group->addButton(d->tiffRadioButton);

/*
    // TODO HDR
    d->hdrRadioButton           = new QRadioButton(i18nc("@option: radio", "HDR output"), d->formatGroupBox);
    d->hdrRadioButton->setToolTip(i18nc("@info: tooltip", "Selects an High Dynamic Range (HDR) image, that can be processed further "
                                        "with a dedicated software."));
    d->hdrRadioButton->setWhatsThis(i18nc("@info: whatsthis", "\"HDR output\": Output in High Dynamic Range, meaning that every piece of "
                                          "information contained in the original photos are preserved. Note that you "
                                          "need another software to process the resulting panorama, like "
                                          "<a href=\"http://qtpfsgui.sourceforge.net/\">Luminance HDR</a>"));       // krazy:exclude=insecurenet
    formatVBox->addWidget(d->hdrRadioButton);
    group->addButton(d->hdrRadioButton);
*/

    switch (d->mngr->format())
    {
        case JPEG:
        {
            d->jpegRadioButton->setChecked(true);
            break;
        }

        case TIFF:
        {
            d->tiffRadioButton->setChecked(true);
            break;
        }

        case HDR:
        {
/*
            // TODO HDR
            d->hdrRadioButton->setChecked(true);
*/
            break;
        }
    }

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-tripod.webp")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

/*
    connect(d->addGPlusMetadataCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotToggleGPano(int)));

    d->addGPlusMetadataCheckBox->setChecked(d->mngr->gPano());
*/
    slotToggleGPano(0);  // Disabled for the moment

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotChangeFileFormat(QAbstractButton*)));

    connect(d->binariesWidget, SIGNAL(signalBinariesFound(bool)),
            this, SLOT(slotBinariesChanged(bool)));
/*
    // TODO HDR
     d->hdrCheckBox->setChecked(d->mngr->hdr());
*/
}

PanoIntroPage::~PanoIntroPage()
{
    delete d;
}

bool PanoIntroPage::binariesFound()
{
    return d->binariesWidget->allBinariesFound();
}

void PanoIntroPage::slotToggleGPano(int state)
{
    d->mngr->setGPano((bool)state);
}

void PanoIntroPage::slotChangeFileFormat(QAbstractButton* button)
{
    if      (button == d->jpegRadioButton)
    {
        d->mngr->setFileFormatJPEG();
    }
    else if (button == d->tiffRadioButton)
    {
        d->mngr->setFileFormatTIFF();
    }
    else if (button == d->hdrRadioButton)
    {
        d->mngr->setFileFormatHDR();
    }
}

void PanoIntroPage::slotBinariesChanged(bool found)
{
    setComplete(found);

    Q_EMIT completeChanged();
}

/*
// TODO HDR
void PanoIntroPage::slotShowFileFormat(int state)
{
    d->mngr->setHDR(state);

    if (state)
    {
        d->formatGroupBox->setEnabled(false);
    }
    else
    {
        d->formatGroupBox->setEnabled(true);
    }
}
*/

void PanoIntroPage::initializePage()
{
    setComplete(d->binariesWidget->allBinariesFound());

    Q_EMIT completeChanged();
}

} // namespace DigikamGenericPanoramaPlugin

#include "moc_panointropage.cpp"
