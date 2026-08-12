/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : a tool to create calendar.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "calwizard.h"

// Qt includes

#include <QIcon>
#include <QDate>
#include <QPrintDialog>
#include <QPrinter>
#include <QStringList>
#include <QMenu>
#include <QLocale>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "calintropage.h"
#include "calprinter.h"
#include "calsettings.h"
#include "calsystem.h"
#include "caltemplate.h"
#include "digikam_debug.h"
#include "ui_calevents.h"
#include "ui_calprogress.h"

namespace DigikamGenericCalendarPlugin
{

class Q_DECL_HIDDEN CalWizard::Private
{
public:

    Private() = default;

public:

    DInfoInterface*  iface          = nullptr;
    CalIntroPage*    introPage      = nullptr;
    CalSettings*     cSettings      = nullptr;
    CalTemplate*     wTemplate      = nullptr;
    Ui::CalEvents    calEventsUI;
    Ui::CalProgress  calProgressUI;

    QLabel*          wPrintLabel    = nullptr;
    QWidget*         wFinish        = nullptr;

    DWizardPage*     wTemplatePage  = nullptr;
    DWizardPage*     wPrintPage     = nullptr;
    DWizardPage*     wFinishPage    = nullptr;

#ifdef HAVE_KCALENDAR

    QWidget*         wEvents        = nullptr;
    DWizardPage*     wEventsPage    = nullptr;

#endif

    QPrinter*        printer        = nullptr;

    CalPrinter*      printThread    = nullptr;

    QMap<int, QUrl>  months;
};

CalWizard::CalWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("Calendar Dialog")),
      d         (new Private)
{
    setWindowTitle(i18nc("@title:window", "Create Calendar"));
    d->iface         = iface;
    d->cSettings     = CalSettings::instance(this);
    d->introPage     = new CalIntroPage(this, i18n("Welcome to Calendar Tool"));

    // ---------------------------------------------------------------

    d->wTemplate     = new CalTemplate(d->iface->currentSelectedItems(), this);
    d->wTemplatePage = new DWizardPage(this, i18n("Create Template for Calendar"));
    d->wTemplatePage->setPageWidget(d->wTemplate);
    d->wTemplatePage->setLeftBottomPix(QIcon::fromTheme(QLatin1String("resource-calendar-insert")));

    // ---------------------------------------------------------------

#ifdef HAVE_KCALENDAR

    d->wEvents     = new QWidget(this);
    d->calEventsUI.setupUi(d->wEvents);
    d->wEventsPage = new DWizardPage(this, i18n("Choose events to show on the Calendar"));
    d->wEventsPage->setPageWidget(d->wEvents);
    d->wEventsPage->setLeftBottomPix(QIcon::fromTheme(QLatin1String("text-vcalendar")));

#endif

    // ---------------------------------------------------------------

    d->wPrintLabel = new QLabel(this);
    d->wPrintLabel->setIndent(20);
    d->wPrintLabel->setWordWrap(true);
    d->wPrintPage  = new DWizardPage(this, i18n("Print Calendar"));
    d->wPrintPage->setPageWidget(d->wPrintLabel);
    d->wPrintPage->setLeftBottomPix(QIcon::fromTheme(QLatin1String("document-print")));

    // ---------------------------------------------------------------

    d->wFinish     = new QWidget(this);
    d->calProgressUI.setupUi(d->wFinish);
    d->wFinishPage = new DWizardPage(this, i18n("Printing in Progress"));
    d->wFinishPage->setPageWidget(d->wFinish);
    d->wFinishPage->setLeftBottomPix(QIcon::fromTheme(QLatin1String("system-run")));

    // ---------------------------------------------------------------

#ifdef HAVE_KCALENDAR

    d->calEventsUI.ohUrlRequester->setFileDlgFilter(i18nc("@info: open file filters", "Calendar Data File (*.ics)"));
    d->calEventsUI.ohUrlRequester->setFileDlgTitle(i18nc("@title:window", "Select Calendar Data File"));
    d->calEventsUI.ohUrlRequester->setFileDlgMode(QFileDialog::ExistingFile);

    d->calEventsUI.fhUrlRequester->setFileDlgFilter(i18nc("@info: open file filters", "Calendar Data File (*.ics)"));
    d->calEventsUI.fhUrlRequester->setFileDlgTitle(i18nc("@title:window", "Select Calendar Data File"));
    d->calEventsUI.fhUrlRequester->setFileDlgMode(QFileDialog::ExistingFile);

#endif

    // ------------------------------------------

    d->printThread = nullptr;
    d->printer     = nullptr;

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(slotPageSelected(int)));
}

CalWizard::~CalWizard()
{
    if (d->printThread)
    {
        d->printThread->cancel();
        d->printThread->wait();

        delete d->printThread;
    }

    delete d->printer;
    delete d;
}

DInfoInterface* CalWizard::iface() const
{
    return d->iface;
}

void CalWizard::slotPageSelected(int curr)
{
    const DWizardPage* const current = dynamic_cast<DWizardPage*>(page(curr));

    if (current == d->wPrintPage)
    {
        d->months.clear();
        QUrl        image;
        QString     month;
        QStringList printList;
        QDate       date = CalSystem().date(d->cSettings->year(), 1, 1);

        for (int i = 1 ; i <= CalSystem().monthsInYear(date) ; ++i)
        {
            month = QLocale().standaloneMonthName(i, QLocale::LongFormat);
            image = d->cSettings->image(i);

            if (!image.isEmpty())
            {
                d->months.insert(i, image);
                printList.append(month);
            }
        }

        if (d->months.isEmpty())
        {
            d->wPrintLabel->setText(QLatin1String("<qt>") +
                                    i18n("No valid images selected for months<br>"
                                         "Click Back to select images") + QLatin1String("</qt>"));
            d->wFinishPage->setComplete(false);
        }
        else
        {
            int year = d->cSettings->year();

            QString extra;

            if ((CalSystem().month(QDate::currentDate()) >= 6    &&
                 CalSystem().year(QDate::currentDate()) == year) ||
                 CalSystem().year(QDate::currentDate()) > year)
            {
                extra = QLatin1String("<br><br><b>") +
                        i18n("Please note that you are making a "
                             "calendar for<br>the current year or a year in the "
                             "past.") + QLatin1String("</b>");
            }

            QString year_locale = QLocale().toString(date, QLatin1String("yyyy"));

            d->wPrintLabel->setText(i18n("Click Next to start Printing<br><br>"
                                         "Following months will be printed for year %1:<br>", year_locale)
                                    + printList.join(QLatin1String(" - ")) + extra);
            d->wPrintLabel->setTextFormat(Qt::RichText);

            d->wFinishPage->setComplete(true);
        }
    }

    else if (current == d->wFinishPage)
    {
        d->calProgressUI.finishLabel->clear();
        d->calProgressUI.currentProgress->reset();
        d->calProgressUI.totalProgress->reset();

        button(QWizard::BackButton)->setEnabled(false);
        button(QWizard::NextButton)->setEnabled(false);

        // Set printer settings ---------------------------------------

        if (!d->printer)
        {
            d->printer = new QPrinter(d->cSettings->resolution());
        }

        CalParams& params = d->cSettings->params;

        // Orientation

        switch (params.imgPos)
        {
            case (CalParams::Top):
            {
                d->printer->setPageOrientation(QPageLayout::Portrait);
                break;
            }

            default:
            {
                d->printer->setPageOrientation(QPageLayout::Landscape);
                break;
            }
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "printing...";

        // PageSize

        d->printer->setPageSize(QPageSize(params.pageSize));

        QPrintDialog* const printDialog = new QPrintDialog(d->printer, this);
        printDialog->setWindowTitle(i18nc("@title:window", "Print Calendar"));

        if (printDialog->exec() == QDialog::Accepted)
        {
            print();
        }
        else
        {
            d->calProgressUI.finishLabel->setText(i18n("Printing Cancelled"));
            button(QWizard::BackButton)->setEnabled(true);
        }

        delete printDialog;
    }
}

void CalWizard::print()
{
    d->calProgressUI.totalProgress->setMaximum(d->months.count());
    d->calProgressUI.totalProgress->setValue(0);
    d->calProgressUI.totalProgress->progressScheduled(i18n("Create calendar"), false, true);
    d->calProgressUI.totalProgress->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("view-calendar")).pixmap(22, 22));

    if (d->printThread)
    {
        d->printThread->cancel();
        d->printThread->wait();
        delete d->printThread;
    }

    d->cSettings->clearSpecial();

#ifdef HAVE_KCALENDAR

    d->cSettings->loadSpecial(QUrl::fromLocalFile(d->calEventsUI.ohUrlRequester->lineEdit()->text()), Qt::red);
    d->cSettings->loadSpecial(QUrl::fromLocalFile(d->calEventsUI.fhUrlRequester->lineEdit()->text()), Qt::darkGreen);

#endif

    d->printThread = new CalPrinter(d->printer, d->months, this);

    connect(d->printThread, SIGNAL(pageChanged(int)),
            this, SLOT(updatePage(int)));

    connect(d->printThread, SIGNAL(pageChanged(int)),
            d->calProgressUI.totalProgress, SLOT(setValue(int)));

    connect(d->printThread, SIGNAL(totalBlocks(int)),
            d->calProgressUI.currentProgress, SLOT(setMaximum(int)));

    connect(d->printThread, SIGNAL(blocksFinished(int)),
            d->calProgressUI.currentProgress, SLOT(setValue(int)));

    d->calProgressUI.totalProgress->setMaximum(d->months.count());
    d->printThread->start();
}

void CalWizard::updatePage(int page)
{
    const int year = d->cSettings->year();
    QDate date(year, 1, 1);

    if (page >= d->months.count())
    {
        printComplete();
        return;
    }

    const auto keys = d->months.keys();
    int month       = keys.at(page);

    d->calProgressUI.finishLabel->setText(i18n("Printing calendar page for %1 of %2",
                                          QLocale().standaloneMonthName(month, QLocale::LongFormat),
                                          QLocale().toString(date, QLatin1String("yyyy"))));
}

void CalWizard::printComplete()
{
    d->calProgressUI.totalProgress->progressCompleted();
    button(QWizard::BackButton)->setEnabled(true);
    button(QWizard::NextButton)->setEnabled(true);
    d->calProgressUI.finishLabel->setText(i18n("Printing Complete"));
}

} // Namespace Digikam

#include "moc_calwizard.cpp"
