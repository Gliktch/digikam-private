/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "vidslideintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QGroupBox>
#include <QIcon>
#include <QStandardPaths>
#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "vidslidewizard.h"
#include "vidslidesettings.h"
#include "ffmpegbinary.h"
#include "ffmpeglauncher.h"
#include "dbinarysearch.h"

namespace DigikamGenericVideoSlideShowPlugin
{

class Q_DECL_HIDDEN VidSlideIntroPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    QComboBox*        imageGetOption = nullptr;
    DHBox*            hbox           = nullptr;
    VidSlideWizard*   wizard         = nullptr;
    VidSlideSettings* settings       = nullptr;
    FFmpegBinary      ffmpegBin;
    DBinarySearch*    binSearch      = nullptr;
};

VidSlideIntroPage::VidSlideIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                        "<p><h1><b>Welcome to Video Slideshow tool</b></h1></p>"
                        "<p>This assistant will guide you to export</p>"
                        "<p>your images as a video stream.</p>"
                        "<p>You can generate quickly a "
                        "<a href='https://en.wikipedia.org/wiki/Time-lapse_photography'>Time-lapse</a> "
                        "movie from images</p>"
                        "<p>captured with a tripod mounted camera controlled with an "
                        "<a href='https://en.wikipedia.org/wiki/Intervalometer#Photography'>intervalometer</a>.</p>"
                        "<p></p><p>You can also create a video presentation with transition</p>"
                        "<p>effects and audio tracks to show on a TV screen.</p>"
                        "</qt>"));

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(VidSlideSettings::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(VidSlideSettings::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    // ---------------------

    QGroupBox* const binaryBox      = new QGroupBox(vbox);
    QGridLayout* const binaryLayout = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "FFmpeg Binary"));
    d->binSearch = new DBinarySearch(binaryBox);
    d->binSearch->addBinary(d->ffmpegBin);
    const auto paths = d->wizard->settings()->defaultFFmpegSearchPaths();

    for (const QString& path : paths)
    {
        d->binSearch->addDirectory(path);
    }

    vbox->setStretchFactor(desc,      2);
    vbox->setStretchFactor(d->hbox,   1);
    vbox->setStretchFactor(binaryBox, 3);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("view-presentation")));
}

VidSlideIntroPage::~VidSlideIntroPage()
{
    delete d;
}

void VidSlideIntroPage::initializePage()
{
    bool albumSupport = (d->settings->iface && d->settings->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(VidSlideSettings::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->wizard->settings()->selMode);
    }

    d->binSearch->allBinariesFound();
}

bool VidSlideIntroPage::validatePage()
{
    d->wizard->settings()->selMode = (VidSlideSettings::Selection)d->imageGetOption->currentIndex();

    if (d->ffmpegBin.isValid())
    {
        QString path = d->ffmpegBin.path();

        if      (path.isEmpty() || (path == FFmpegBinary::ffmpegToolBin()))
        {

#ifdef Q_OS_WIN

            QStringList possiblePaths({QLatin1String("C:/Program Files/digiKam"),
                                       QLatin1String("C:/Program Files/digiKam/bin")});

            path = QStandardPaths::findExecutable(FFmpegBinary::ffmpegToolBin(), possiblePaths);

#else

            path = QStandardPaths::findExecutable(FFmpegBinary::ffmpegToolBin());

#endif

        }
        else if (QFileInfo(path).isDir())
        {
            if (!path.endsWith(QLatin1Char('/')))
            {
                path.append(QLatin1Char('/'));
            }

            path.append(FFmpegBinary::ffmpegToolBin());
        }

        d->wizard->settings()->ffmpegPath = path;

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << d->ffmpegBin.directory() << d->ffmpegBin.path();

        FFmpegLauncher ffmpeg(this);
        ffmpeg.setSettings(d->wizard->settings());
        d->wizard->settings()->ffmpegCodecs  = ffmpeg.supportedCodecs();
        d->wizard->settings()->ffmpegFormats = ffmpeg.supportedFormats();

        return true;
    }

    return false;
}

} // namespace DigikamGenericVideoSlideShowPlugin

#include "moc_vidslideintropage.cpp"
