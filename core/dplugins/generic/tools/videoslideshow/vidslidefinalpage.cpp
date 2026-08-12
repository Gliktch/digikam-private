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

#include "vidslidefinalpage.h"

// Qt includes

#include <QIcon>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QDir>
#include <QTextBrowser>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "vidslidewizard.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "vidslidethread.h"

#ifdef HAVE_MEDIAPLAYER

#   include "vidplayerdlg.h"

#endif

namespace DigikamGenericVideoSlideShowPlugin
{

class Q_DECL_HIDDEN VidSlideFinalPage::Private
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

public:

    DHistoryView*     progressView = nullptr;
    DProgressWdg*     progressBar  = nullptr;
    bool              complete     = false;
    VidSlideThread*   encoder      = nullptr;
    VidSlideWizard*   wizard       = nullptr;
    VidSlideSettings* settings     = nullptr;
    QTextBrowser*     detailsText  = nullptr;
};

VidSlideFinalPage::VidSlideFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    setObjectName(QLatin1String("FinalPage"));

    DVBox* const vbox = new DVBox(this);
    d->progressView   = new DHistoryView(vbox);
    d->detailsText    = new QTextBrowser(vbox);
    d->detailsText->hide();
    d->progressBar    = new DProgressWdg(vbox);

    vbox->setStretchFactor(d->progressBar, 10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(layoutSpacing());

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("system-run")));
}

VidSlideFinalPage::~VidSlideFinalPage()
{
    if (d->encoder)
    {
        d->encoder->cancel();
    }

    delete d;
}

void VidSlideFinalPage::initializePage()
{
    d->complete = false;
    d->detailsText->hide();

    Q_EMIT completeChanged();

    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void VidSlideFinalPage::slotProcess()
{
    if (!d->wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    d->progressView->addEntry(i18n("Starting to generate video slideshow..."),
                              DHistoryView::ProgressEntry);

    d->progressView->addEntry(i18n("%1 input images to process", d->settings->inputImages.count()),
                                  DHistoryView::ProgressEntry);

    d->progressBar->setMinimum(0);

    // NOTE: last stage is FFmpeg encoding.
    d->progressBar->setMaximum(d->settings->inputImages.count() + 1);

    d->encoder = new VidSlideThread(this);

    connect(d->encoder, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));

    connect(d->encoder, SIGNAL(signalMessage(QString,bool)),
            this, SLOT(slotMessage(QString,bool)));

    connect(d->encoder, SIGNAL(signalComplete(bool)),
            this, SLOT(slotDone(bool)));

    d->encoder->prepareFrames(d->settings);
    d->encoder->start();
}

void VidSlideFinalPage::cleanupPage()
{
    d->detailsText->clear();
    d->detailsText->hide();

    if (d->encoder)
    {
        d->encoder->cancel();
    }
}

void VidSlideFinalPage::slotMessage(const QString& mess, bool err)
{
    d->progressView->addEntry(mess, err ? DHistoryView::ErrorEntry
                                        : DHistoryView::ProgressEntry);
}

void VidSlideFinalPage::slotDone(bool completed)
{
    d->progressBar->setValue(d->progressBar->maximum());
    d->progressBar->progressCompleted();
    d->complete = completed;

    if (!d->complete)
    {
        d->progressView->addEntry(i18n("Video Slideshow is not completed"),
                                  DHistoryView::WarningEntry);

        d->detailsText->show();
        d->detailsText->setText(d->encoder->encodingTraces());
        d->progressView->addEntry(i18n("See the video encoding trace below"),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Video Slideshow completed."),
                                  DHistoryView::ProgressEntry);

        if (d->settings->outputPlayer != VidSlideSettings::NOPLAYER)
        {
            d->progressView->addEntry(i18n("Opening video stream in player..."),
                                      DHistoryView::ProgressEntry);

#ifdef HAVE_MEDIAPLAYER

            if (d->settings->outputPlayer == VidSlideSettings::INTERNAL)
            {


                VidPlayerDlg* const player = new VidPlayerDlg(d->settings->outputFile, this);
                player->show();
                player->resize(800, 600);


            }
            else

#endif
            {
                QDesktopServices::openUrl(QUrl::fromLocalFile(d->settings->outputFile));
            }
        }
    }

    Q_EMIT completeChanged();
}

bool VidSlideFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace DigikamGenericVideoSlideShowPlugin

#include "moc_vidslidefinalpage.cpp"
