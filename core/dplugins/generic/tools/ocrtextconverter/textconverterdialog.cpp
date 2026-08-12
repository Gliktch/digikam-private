/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : Text converter batch dialog
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "textconverterdialog.h"

// C++ includes

#include <sstream>

// Qt includes

#include <QGridLayout>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QMessageBox>
#include <QTabWidget>
#include <QMenu>
#include <QScrollArea>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dcombobox.h"
#include "dprogresswdg.h"
#include "textconverterlist.h"
#include "textconvertersettings.h"
#include "ocroptions.h"
#include "dtextedit.h"
#include "dlayoutbox.h"
#include "textconverterthread.h"
#include "textconverteraction.h"
#include "tesseractbinary.h"
#include "dbinarysearch.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class Q_DECL_HIDDEN TextConverterDialog::Private
{
public:

    enum OcrTabs
    {
        RecognitionTab = 0,
        ReviewTab
    };

    enum ProcessActions
    {
        ProcessAll = 0,
        ProcessSelected
    };

public:

    Private() = default;

public:

    bool                              busy                  = false;

    QList<QUrl>                       fileList;

    QMap<QUrl, QString>               textEditList;

    DProgressWdg*                     progressBar           = nullptr;

    TextConverterActionThread*        thread                = nullptr;

    DInfoInterface*                   iface                 = nullptr;

    TextConverterList*                listView              = nullptr;

    TextConverterSettings*            ocrSettings           = nullptr;

    DTextEdit*                        textedit              = nullptr;

    QPushButton*                      saveTextButton        = nullptr;

    TextConverterListViewItem*        currentSelectedItem   = nullptr;

    TesseractBinary                   tesseractBin;
    DBinarySearch*                    binWidget             = nullptr;

    QTabWidget*                       tabView               = nullptr;
};

TextConverterDialog::TextConverterDialog(QWidget* const parent, DInfoInterface* const iface)
    : DPluginDialog(parent, QLatin1String("Text Converter Dialog")),
      d            (new Private)
{
    setWindowTitle(i18nc("@title:window", "OCR Text Converter"));
    setModal(true);

    d->iface          = iface;
    const int spacing = layoutSpacing();

    m_buttons->addButton(QDialogButtonBox::Close);
    m_buttons->addButton(QDialogButtonBox::Ok);
    m_buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button", "&Start OCR"));
    m_buttons->button(QDialogButtonBox::Ok)->setDisabled(true);
    m_buttons->addButton(QDialogButtonBox::Reset);
    m_buttons->button(QDialogButtonBox::Reset)->setText(i18nc("@action: button", "&Default"));
    m_buttons->button(QDialogButtonBox::Reset)->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    m_buttons->button(QDialogButtonBox::Reset)->setToolTip(i18nc("@info:tooltip",
                                                           "Revert current settings to default values."));

    QWidget* const mainWidget = new QWidget(this);
    QVBoxLayout* const vbx    = new QVBoxLayout(this);
    vbx->addWidget(mainWidget);
    vbx->addWidget(m_buttons);
    setLayout(vbx);

    //-------------------------------------------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout(mainWidget);
    d->listView                   = new TextConverterList(mainWidget);
    d->progressBar                = new DProgressWdg(mainWidget);
    d->progressBar->reset();
    d->progressBar->setVisible(false);

    d->listView->appendControlButtonsWidget(d->progressBar);
    QBoxLayout* const blay        = d->listView->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    blay->setStretchFactor(d->progressBar, 20);

    d->tabView                    = new QTabWidget(mainWidget);

    // -- Recognition tab

    QScrollArea* const recsv      = new QScrollArea(d->tabView);
    DVBox* const recognitionTab   = new DVBox(recsv->viewport());
    QLabel* const tesseractLabel  = new QLabel(i18nc("@label", "This tool uses the %1 open-source "
                                               "engine to perform Optical Characters Recognition. "
                                               "Tesseract program and the desired languages modules must "
                                               "be installed on your system.",
                                                QString::fromUtf8("<a href='https://github.com/tesseract-ocr/tesseract'>Tesseract</a>")),
                                                recognitionTab);
    tesseractLabel->setWordWrap(true);
    tesseractLabel->setOpenExternalLinks(true);

    d->binWidget                  = new DBinarySearch(recognitionTab);
    d->binWidget->addBinary(d->tesseractBin);

#ifdef Q_OS_MACOS

    // Std Macports install

    d->binWidget->addDirectory(QLatin1String("/opt/local/bin"));

    // digiKam Bundle PKG install

    d->binWidget->addDirectory(macOSBundlePrefix() + QLatin1String("bin"));

#endif

#ifdef Q_OS_WIN

    d->binWidget->addDirectory(QLatin1String("C:/Program Files/Tesseract-OCR"));

#endif

#ifdef Q_OS_UNIX

    d->binWidget->addDirectory(QLatin1String("/usr/bin"));
    d->binWidget->addDirectory(QLatin1String("/usr/local/bin"));
    d->binWidget->addDirectory(QLatin1String("/bin"));

#endif

    d->ocrSettings                = new TextConverterSettings(recognitionTab);


    recognitionTab->setContentsMargins(spacing, spacing, spacing, spacing);
    recognitionTab->setSpacing(spacing);

    recsv->setFrameStyle(QFrame::NoFrame);
    recsv->setWidgetResizable(true);
    recsv->setWidget(recognitionTab);

    d->tabView->insertTab(Private::RecognitionTab, recsv, i18nc("@title", "Text Recognition"));

    // --- Review tab --------------------------------------------------------------------------

    DVBox* const reviewTab = new DVBox(d->tabView);
    d->textedit            = new DTextEdit(0, reviewTab);
    d->textedit->setPlaceholderText(i18nc("@info", "After processing recognition, "
                                                   "double-click on one item to "
                                                   "display recognized text here. "
                                                   "You can review words and fix "
                                                   "if necessary. Press the Save "
                                                   "button to record your changes."));
    reviewTab->setStretchFactor(d->textedit, 100);

    d->saveTextButton      = new QPushButton(reviewTab);
    d->saveTextButton->setText(i18nc("@action: button", "Save"));
    d->saveTextButton->setEnabled(false);

    reviewTab->setContentsMargins(spacing, spacing, spacing, spacing);
    reviewTab->setSpacing(spacing);

    d->tabView->insertTab(Private::ReviewTab, reviewTab, i18nc("@title", "Text Review"));

    //-------------------------------------------------------------------------------------------

    mainLayout->addWidget(d->listView, 0, 0, 1, 1);
    mainLayout->addWidget(d->tabView,  0, 1, 1, 1);
    mainLayout->setColumnStretch(0, 7);
    mainLayout->setColumnStretch(1, 3);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setContentsMargins(QMargins());

    // ---------------------------------------------------------------

    d->thread = new TextConverterActionThread(this);

    connect(d->thread, SIGNAL(signalStarting(DigikamGenericTextConverterPlugin::TextConverterActionData)),
            this, SLOT(slotTextConverterAction(DigikamGenericTextConverterPlugin::TextConverterActionData)));

    connect(d->thread, SIGNAL(signalFinished(DigikamGenericTextConverterPlugin::TextConverterActionData)),
            this, SLOT(slotTextConverterAction(DigikamGenericTextConverterPlugin::TextConverterActionData)));

    connect(d->thread, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()));

    // ---------------------------------------------------------------

    connect(m_buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotStartStop()));

    connect(m_buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(slotClose()));

    connect(m_buttons->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(slotDefault()));

    connect(d->progressBar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotStartStop()));

    connect(d->listView->listView(), &DItemsListView::itemDoubleClicked,
            this, &TextConverterDialog::slotDoubleClick);

    connect(d->listView->listView(), &DItemsListView::itemSelectionChanged,
            this, &TextConverterDialog::slotSetDisable);

    connect(d->saveTextButton, SIGNAL(clicked()),
            this, SLOT(slotUpdateText()));

    connect(d->binWidget, SIGNAL(signalBinariesFound(bool)),
            this, SLOT(slotTesseractBinaryFound(bool)));

    // ---------------------------------------------------------------

    d->listView->setIface(d->iface);
    d->listView->loadImagesFromCurrentSelection();

    d->ocrSettings->readSettings();

    // ---------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotStartFoundTesseract()));
}

TextConverterDialog::~TextConverterDialog()
{
    delete d;
}

void TextConverterDialog::slotDoubleClick(QTreeWidgetItem* element)
{
    TextConverterListViewItem* const item = dynamic_cast<TextConverterListViewItem*>(element);

    if (!item)
    {
        d->currentSelectedItem = nullptr;
        return;
    }

    d->currentSelectedItem = item;

    if (d->textEditList.contains(d->currentSelectedItem->url()))
    {
        d->tabView->setCurrentIndex(Private::ReviewTab);
        d->textedit->setText(d->textEditList[d->currentSelectedItem->url()]);
        d->saveTextButton->setEnabled(true);
    }
    else
    {
        d->textedit->clear();
    }
}

void TextConverterDialog::slotUpdateText()
{
    QString newText   = d->textedit->text();
    OcrOptions opt    = d->ocrSettings->ocrOptions();
    opt.tesseractPath = d->tesseractBin.path();
    opt.iface         = d->iface;

    if (!d->textedit->text().isEmpty()           &&
        !d->currentSelectedItem->url().isEmpty() &&
        !d->currentSelectedItem->destFileName().isEmpty())
    {
        d->textEditList[d->currentSelectedItem->url()] = newText;
        d->currentSelectedItem->setRecognizedWords(QString::fromLatin1("%1").arg(calculateNumberOfWords(newText)));

        MetaEngine::AltLangMap commentsMap;
        commentsMap.insert(QLatin1String("x-default"), newText);

        if (opt.isSaveTextFile || opt.isSaveXMP)
        {
            OcrTesseractEngine::translate(commentsMap, opt.translations);
        }

        if (opt.isSaveTextFile)
        {
            QString outFile = d->currentSelectedItem->destFileName();
            OcrTesseractEngine::saveTextFile(d->currentSelectedItem->url().toLocalFile(),
                                             outFile,
                                             commentsMap);
        }

        if (opt.isSaveXMP)
        {
            OcrTesseractEngine::saveXMP(d->currentSelectedItem->url(),
                                        commentsMap,
                                        opt.iface);
        }
    }
}

void TextConverterDialog::slotSetDisable()
{
    d->saveTextButton->setEnabled(false);
}

void TextConverterDialog::slotTextConverterAction(const DigikamGenericTextConverterPlugin::TextConverterActionData& ad)
{
    if (ad.starting)
    {
        switch (ad.action)
        {
            case PROCESS:
            {
                setBusy(true);
                d->listView->processing(ad.fileUrl);
                d->progressBar->progressStatusChanged(i18nc("@info", "Processing %1", ad.fileUrl.fileName()));
                break;
            }

            default:
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "DigikamGenericTextConverterPlugin: Unknown action";
                break;
            }
        }
    }
    else
    {
        if (ad.result != OcrTesseractEngine::PROCESS_COMPLETE)
        {
            switch (ad.action)
            {
                case PROCESS:
                {
                    processingFailed(ad.fileUrl, ad.result);
                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "DigikamGenericTextConverterPlugin: Unknown action";
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case PROCESS:
                {
                    d->textEditList[ad.fileUrl] = ad.outputText;
                    processed(ad.fileUrl, ad.destPath, ad.outputText);

                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "DigikamGenericTextConverterPlugin: Unknown action";

                    break;
                }
            }
        }
    }
}

void TextConverterDialog::processingFailed(const QUrl& url, int result)
{
    d->listView->processed(url, false);
    d->progressBar->setValue(d->progressBar->value()+1);

    TextConverterListViewItem* const item = dynamic_cast<TextConverterListViewItem*>(d->listView->listView()->findItem(url));

    if (!item)
    {
        return;
    }

    QString status;

    switch (result)
    {
        case OcrTesseractEngine::PROCESS_FAILED:
        {
            status = i18nc("@info", "Process failed");
            break;
        }

        case OcrTesseractEngine::PROCESS_CANCELED:
        {
            status = i18nc("@info", "Process canceled");
            break;
        }

        // TODO Tesseract ocr error

        default:
        {
            status = i18nc("@info", "Internal error");
            break;
        }
    }

    item->setStatus(status);
}

int TextConverterDialog::calculateNumberOfWords(const QString& text) const
{
    if (!text.isEmpty())
    {
        std::stringstream ss;
        ss << text.toStdString();

        int count = 0;
        std::string word;

        while (ss >> word)
        {
            if ((word.length() == 1) && std::ispunct(word[0]))
            {
                continue;
            }

            count++;
        }

        return count;
    }

    return 0;
}

void TextConverterDialog::processed(const QUrl& url,
                                    const QString& outputFile,
                                    const QString& ocrResult)
{
    TextConverterListViewItem* const item = dynamic_cast<TextConverterListViewItem*>(d->listView->listView()->findItem(url));

    if (!item)
    {
        return;
    }

    if (!outputFile.isEmpty())
    {
        item->setDestFileName(outputFile);
    }

    d->listView->processed(url, true);
    item->setStatus(i18nc("@info", "Success"));
    item->setRecognizedWords(QString::fromLatin1("%1").arg(calculateNumberOfWords(ocrResult)));
    d->progressBar->setValue(d->progressBar->value() + 1);
}

void TextConverterDialog::processAll()
{
    OcrOptions opt    = d->ocrSettings->ocrOptions();
    opt.tesseractPath = d->tesseractBin.path();
    opt.iface         = d->iface;
    d->thread->setOcrOptions(opt);
    d->thread->ocrProcessFiles(d->fileList);

    if (!d->thread->isRunning())
    {
        d->thread->start();
    }
}

void TextConverterDialog::slotStartStop()
{
    if (!d->busy)
    {
        QAction* const ac = qobject_cast<QAction*>(sender());

        if (!ac)
        {
            return;
        }

        int ptype = ac->data().toInt();

        d->fileList.clear();

        if (d->listView->listView()->topLevelItemCount() == 0)
        {
            d->textedit->clear();
        }

        QTreeWidgetItemIterator it(d->listView->listView());

        while (*it)
        {
            TextConverterListViewItem* const lvItem = dynamic_cast<TextConverterListViewItem*>(*it);

            if (lvItem)
            {
                if (
                    !lvItem->isDisabled()                                   &&
                    (lvItem->state() != TextConverterListViewItem::Success) &&
                    ((ptype == Private::ProcessAll) ? true : lvItem->isSelected())
                   )
                {
                    lvItem->setIcon(1, QIcon());
                    lvItem->setState(TextConverterListViewItem::Waiting);
                    d->fileList.append(lvItem->url());
                }
            }

            ++it;
        }

        if (d->fileList.empty())
        {
            QMessageBox::information(this, i18nc("@title:window", "Text Converter"),
                                     i18nc("@info", "The list does not contain any digital files to process. "
                                           "You need to select them."));
            setBusy(false);
            slotAborted();

            return;
        }

        d->progressBar->setMaximum(d->fileList.count());
        d->progressBar->setValue(0);
        d->progressBar->setVisible(true);
        d->progressBar->progressScheduled(i18nc("@title", "Text Converter"), true, true);
        d->progressBar->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("text-x-generic")).pixmap(22, 22));

        processAll();
    }
    else
    {
        d->fileList.clear();
        d->thread->cancel();
        setBusy(false);

        d->listView->cancelProcess();

        QTimer::singleShot(500, this, SLOT(slotAborted()));
    }
}

void TextConverterDialog::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    // Stop current conversion if necessary

    if (d->busy)
    {
        slotStartStop();
    }

    d->ocrSettings->saveSettings();
    d->listView->listView()->clear();
    e->accept();
}

void TextConverterDialog::slotClose()
{
    // Stop current conversion if necessary

    if (d->busy)
    {
        slotStartStop();
    }

    d->ocrSettings->saveSettings();
    d->listView->listView()->clear();
    d->fileList.clear();
    accept();
}

void TextConverterDialog::slotDefault()
{
    d->ocrSettings->setDefaultSettings();
}

void TextConverterDialog::addItems(const QList<QUrl>& itemList)
{
    d->listView->slotAddImages(itemList);
}

void TextConverterDialog::slotThreadFinished()
{
    setBusy(false);
    slotAborted();
}

void TextConverterDialog::setBusy(bool busy)
{
    d->busy = busy;

    if (d->busy)
    {
        m_buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action", "&Abort"));
        m_buttons->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@info", "Abort OCR processing of Raw files."));
        unplugProcessMenu();
    }
    else
    {
        m_buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action", "&Start OCR"));
        m_buttons->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@info", "Start OCR using the current settings."));
        plugProcessMenu();
    }

    d->ocrSettings->setEnabled(!d->busy);
    d->listView->listView()->viewport()->setEnabled(!d->busy);
    d->busy ? setCursor(Qt::WaitCursor) : unsetCursor();
}

void TextConverterDialog::slotAborted()
{
    d->progressBar->setValue(0);
    d->progressBar->setVisible(false);
    d->progressBar->progressCompleted();
}

void TextConverterDialog::slotStartFoundTesseract()
{
    bool b = d->binWidget->allBinariesFound();
    slotTesseractBinaryFound(b);
}

void TextConverterDialog::slotTesseractBinaryFound(bool found)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Tesseract binary found:" << found;

    QStringList langs = d->tesseractBin.tesseractLanguages();
    d->ocrSettings->populateLanguagesMode(langs);

    bool b = found && !langs.isEmpty();

    setBusy(false);

    // Disable Start button if Tesseract is not found or if no language plugin installed.

    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(b);

    if (b)
    {
        m_buttons->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@info", "Start OCR using the current settings."));
    }
    else
    {
        m_buttons->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@info", "Tesseract program or language modules\n"
                                                                  "are not installed on your system."));
    }
}

void TextConverterDialog::plugProcessMenu()
{
    if (!m_buttons->button(QDialogButtonBox::Ok)->menu())
    {
        QMenu* const processMenu = new QMenu(this);
        m_buttons->button(QDialogButtonBox::Ok)->setMenu(processMenu);

        connect(processMenu, SIGNAL(aboutToShow()),
                this, SLOT(slotProcessMenu()));
    }
}

void TextConverterDialog::unplugProcessMenu()
{
    m_buttons->button(QDialogButtonBox::Ok)->setMenu(nullptr);
}

void TextConverterDialog::slotProcessMenu()
{
    if (!d->busy)
    {
        QMenu* const processMenu = qobject_cast<QMenu*>(sender());

        if (processMenu)
        {
            processMenu->clear();

            QAction* ac = nullptr;
            ac          = processMenu->addAction(i18nc("@action", "Process All Items"),      this, SLOT(slotStartStop()));
            ac->setData(Private::ProcessAll);
            ac          = processMenu->addAction(i18nc("@action", "Process Selected Items"), this, SLOT(slotStartStop()));
            ac->setData(Private::ProcessSelected);
        }
    }
}

} // namespace DigikamGenericTextConverterPlugin

#include "moc_textconverterdialog.cpp"
