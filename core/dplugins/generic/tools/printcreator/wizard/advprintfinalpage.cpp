/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advprintfinalpage.h"

// Qt includes

#include <QImage>
#include <QIcon>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QDesktopServices>
#include <QPrintDialog>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advprintthread.h"
#include "advprintwizard.h"
#include "advprintcaptionpage.h"
#include "advprintphotopage.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "dimg.h"

namespace DigikamGenericPrintCreatorPlugin
{

class Q_DECL_HIDDEN AdvPrintFinalPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = wizard->iface();
        }
    }

    const float        FONT_HEIGHT_RATIO = 0.8F;

    DHistoryView*      progressView      = nullptr;
    DProgressWdg*      progressBar       = nullptr;
    AdvPrintWizard*    wizard            = nullptr;
    AdvPrintSettings*  settings          = nullptr;
    AdvPrintThread*    printThread       = nullptr;
    AdvPrintPhotoPage* photoPage         = nullptr;
    DInfoInterface*    iface             = nullptr;
    bool               complete          = false;
};

AdvPrintFinalPage::AdvPrintFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    DVBox* const vbox = new DVBox(this);
    d->progressView   = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    vbox->setStretchFactor(d->progressBar, 10);
    vbox->setSpacing(layoutSpacing());

    vbox->setContentsMargins(QMargins());

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("system-run")));
}

AdvPrintFinalPage::~AdvPrintFinalPage()
{
    if (d->printThread)
    {
        d->printThread->cancel();
    }

    delete d;
}

void AdvPrintFinalPage::setPhotoPage(AdvPrintPhotoPage* const photoPage)
{
    d->photoPage = photoPage;
}

void AdvPrintFinalPage::initializePage()
{
    d->complete = false;

    Q_EMIT completeChanged();

    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void AdvPrintFinalPage::slotProcess()
{
    if (!d->wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    if (d->settings->photos.isEmpty())
    {
        d->progressView->addEntry(i18n("No page to print..."),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    d->progressView->addEntry(i18n("Starting to pre-process files..."),
                              DHistoryView::ProgressEntry);

    d->progressView->addEntry(i18n("%1 items to process", d->settings->inputImages.count()),
                              DHistoryView::ProgressEntry);

    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->settings->photos.count());

    // set the default crop regions if not already set

    int sizeIndex              = d->photoPage->ui()->ListPhotoSizes->currentRow();
    d->settings->outputLayouts = d->settings->photosizes.at(sizeIndex);
    d->printThread             = new AdvPrintThread(this);

    connect(d->printThread, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));

    connect(d->printThread, SIGNAL(signalMessage(QString,bool)),
            this, SLOT(slotMessage(QString,bool)));

    connect(d->printThread, SIGNAL(signalComplete(bool)),
            this, SLOT(slotPrint(bool)));

    d->printThread->preparePrint(d->settings, sizeIndex);
    d->printThread->start();
}

void AdvPrintFinalPage::slotPrint(bool b)
{
    if (!b)
    {
        slotDone(b);
        return;
    }

    if (!print())
    {
        d->progressView->addEntry(i18n("Printing process aborted..."),
                                  DHistoryView::ErrorEntry);
        return;
    }

    disconnect(d->printThread, SIGNAL(signalComplete(bool)),
               this, SLOT(slotPrint(bool)));

    connect(d->printThread, SIGNAL(signalComplete(bool)),
            this, SLOT(slotDone(bool)));

    d->printThread->print(d->settings);
    d->printThread->start();
}

void AdvPrintFinalPage::cleanupPage()
{
    if (d->printThread)
    {
        d->printThread->cancel();
    }

    if (d->settings->gimpFiles.count() > 0)
    {
        removeGimpFiles();
    }
}

void AdvPrintFinalPage::slotMessage(const QString& mess, bool err)
{
    d->progressView->addEntry(mess, err ? DHistoryView::ErrorEntry
                                        : DHistoryView::ProgressEntry);
}

void AdvPrintFinalPage::slotDone(bool completed)
{
    d->progressBar->progressCompleted();
    d->complete = completed;

    if (!d->complete)
    {
        d->progressView->addEntry(i18n("Printing process is not completed"),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Printing process completed."),
                                  DHistoryView::ProgressEntry);

        if      (d->settings->printerName == d->settings->outputName(AdvPrintSettings::FILES))
        {
            if (d->settings->openInFileBrowser)
            {
                QDesktopServices::openUrl(d->settings->outputDir);
                d->progressView->addEntry(i18n("Open destination directory in file-browser."),
                                          DHistoryView::ProgressEntry);
            }
        }
        else if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::GIMP))
        {
            if (!d->settings->gimpFiles.isEmpty())
            {
                QStringList args;
                QString prog = d->settings->gimpPath;

                for (QStringList::ConstIterator it = d->settings->gimpFiles.constBegin() ;
                    it != d->settings->gimpFiles.constEnd() ; ++it)
                {
                    args << (*it);
                }

                QProcess process;
                process.setProcessEnvironment(adjustedEnvironmentForAppImage());

                if (!process.startDetached(prog, args))
                {
                    d->progressView->addEntry(i18n("There was an error to launch the external "
                                                   "Gimp program. Please make sure it is properly "
                                                   "installed."),
                                              DHistoryView::WarningEntry);
                    return;
                }
            }
        }
    }

    Q_EMIT completeChanged();
}

bool AdvPrintFinalPage::isComplete() const
{
    return d->complete;
}

void AdvPrintFinalPage::removeGimpFiles()
{
    for (QStringList::ConstIterator it = d->settings->gimpFiles.constBegin() ;
         it != d->settings->gimpFiles.constEnd() ; ++it)
    {
        if (QFile::exists(*it))
        {
            if (QFile::remove(*it) == false)
            {
                QMessageBox::information(this,
                                         QString(),
                                         i18n("Could not remove the GIMP's temporary files."));
                break;
            }
        }
    }
}

bool AdvPrintFinalPage::checkTempPath(const QString& tempPath) const
{
    // does the temp path exist?

    if (!QDir(tempPath).exists())
    {
        if (!QDir().mkpath(tempPath))
        {
            d->progressView->addEntry(i18n("Unable to create a temporary folder. "
                                           "Please make sure you have proper permissions "
                                           "to this folder and try again."),
                                      DHistoryView::WarningEntry);

            return false;
        }
    }

    return true;
}

bool AdvPrintFinalPage::print()
{
    // Real printer to use.

    if (
        (d->settings->printerName != d->settings->outputName(AdvPrintSettings::FILES)) &&
        (d->settings->printerName != d->settings->outputName(AdvPrintSettings::GIMP))
       )
    {
        // tell him again!

        d->photoPage->printer()->setFullPage(true);

        qreal left, top, right, bottom;
        auto margins = d->photoPage->printer()->pageLayout().margins(QPageLayout::Millimeter);
        left         = margins.left();
        top          = margins.top();
        right        = margins.right();
        bottom       = margins.bottom();

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Margins before print dialog: left "
                                             << left
                                             << " right "
                                             << right
                                             << " top "
                                             << top
                                             << " bottom "
                                             << bottom;

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "(1) paper page "
                                             << d->photoPage->printer()->pageLayout().pageSize().id()
                                             << " size "
                                             << d->photoPage->printer()->pageLayout().pageSize().size(QPageSize::Millimeter);

        auto pageSize = d->photoPage->printer()->pageLayout().pageSize().id();
        QPrintDialog* const dialog    = new QPrintDialog(d->photoPage->printer(), this);
        dialog->setWindowTitle(i18nc("@title:window", "Print Creator"));

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "(2) paper page "
                                             << dialog->printer()->pageLayout().pageSize().id()
                                             << " size "
                                             << dialog->printer()->pageLayout().pageSize().size(QPageSize::Millimeter);

        if (dialog->exec() != QDialog::Accepted)
        {
            return false;
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "(3) paper page "
                                             << dialog->printer()->pageLayout().pageSize().id()
                                             << " size "
                                             << dialog->printer()->pageLayout().pageSize().size(QPageSize::Millimeter);

        // Why paperSize changes if printer properties is not pressed?

        if (pageSize != d->photoPage->printer()->pageLayout().pageSize().id())
        {
            d->photoPage->printer()->setPageSize(QPageSize(pageSize));
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "(4) paper page "
                                             << dialog->printer()->pageLayout().pageSize().id()
                                             << " size "
                                             << dialog->printer()->pageLayout().pageSize().size(QPageSize::Millimeter);

        margins = dialog->printer()->pageLayout().margins(QPageLayout::Millimeter);
        left    = margins.left();
        top     = margins.top();
        right   = margins.right();
        bottom  = margins.bottom();

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Dialog exit, new margins: left "
                                             << left
                                             << " right "
                                             << right
                                             << " top "
                                             << top
                                             << " bottom "
                                             << bottom;

        d->settings->outputPrinter = d->photoPage->printer();

        return true;
    }
    else if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::GIMP))
    {
        d->settings->imageFormat = AdvPrintSettings::JPEG;

        if (!checkTempPath(d->settings->tempPath))
        {
            return false;
        }

        if (d->settings->gimpFiles.count() > 0)
        {
            removeGimpFiles();
        }

        d->settings->outputPath = d->settings->tempPath;

        return true;
    }
    else if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::FILES))
    {
        d->settings->outputPath = d->settings->outputDir.toLocalFile();

        return true;
    }

    return false;
}

} // namespace DigikamGenericPrintCreatorPlugin

#include "moc_advprintfinalpage.cpp"
