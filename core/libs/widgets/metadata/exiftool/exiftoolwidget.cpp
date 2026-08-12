/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool metadata widget.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoolwidget.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QMimeData>
#include <QApplication>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QClipboard>
#include <QStyle>
#include <QPointer>
#include <QFile>
#include <QTimer>
#include <QStandardPaths>
#include <QTextStream>
#include <QActionGroup>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "exiftoollistviewgroup.h"
#include "exiftoollistviewitem.h"
#include "exiftoollistview.h"
#include "exiftoolerrorview.h"
#include "exiftoolloadingview.h"
#include "searchtextbar.h"
#include "dfiledialog.h"
#include "digikam_globals.h"

namespace Digikam
{

namespace
{

/**
 * Standard ExifTool entry list from the less important to the most important for photograph.
 */
static const char* StandardExifToolEntryList[] =
{
    "File",
    "Composite",
    "EXIF",
    "IPTC",
    "XMP",
    "Makernotes",
    "ICC Profile",
    "JFIF",
    "-1"
};

} // namespace

class Q_DECL_HIDDEN ExifToolWidget::Private
{
public:

    enum ViewMode
    {
        LoadingView = 0,
        MetadataView,
        ErrorView
    };

public:

    Private()
    {
        for (int i = 0 ; QLatin1String(StandardExifToolEntryList[i]) != QLatin1String("-1") ; ++i)
        {
            keysFilter << QLatin1String(StandardExifToolEntryList[i]);
        }
    }

    QAction*             noneAction      = nullptr;
    QAction*             photoAction     = nullptr;
    QAction*             customAction    = nullptr;
    QAction*             settingsAction  = nullptr;

    QWidget*             metadataView    = nullptr;
    ExifToolLoadingView* loadingView     = nullptr;
    ExifToolListView*    view            = nullptr;
    ExifToolErrorView*   errorView       = nullptr;
    SearchTextBar*       searchBar       = nullptr;

    QString              fileName;

    QToolButton*         filterBtn       = nullptr;
    QToolButton*         toolBtn         = nullptr;

    QStringList          tagsFilter;
    QStringList          keysFilter;

    QAction*             saveMetadata    = nullptr;
    QAction*             printMetadata   = nullptr;
    QAction*             copy2ClipBoard  = nullptr;

    QMenu*               optionsMenu     = nullptr;
    QTimer*              preLoadingTimer = nullptr;   ///< To prevent flicker effect with loading view with short loading time.
};

ExifToolWidget::ExifToolWidget(QWidget* const parent)
    : QStackedWidget(parent),
      d             (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // -----------------------------------------------------------------

    d->filterBtn      = new QToolButton(this);
    d->filterBtn->setToolTip(i18nc("@info: metadata view", "Tags filter options"));
    d->filterBtn->setIcon(QIcon::fromTheme(QLatin1String("view-filter")));
    d->filterBtn->setPopupMode(QToolButton::InstantPopup);
    d->filterBtn->setWhatsThis(i18nc("@info: metadata view", "Apply tags filter over metadata."));

    d->optionsMenu                  = new QMenu(d->filterBtn);
    QActionGroup* const filterGroup = new QActionGroup(this);

    d->noneAction     = d->optionsMenu->addAction(i18nc("@action: metadata view", "No filter"));
    d->noneAction->setCheckable(true);
    filterGroup->addAction(d->noneAction);
    d->photoAction    = d->optionsMenu->addAction(i18nc("@action: metadata view", "Photograph"));
    d->photoAction->setCheckable(true);
    filterGroup->addAction(d->photoAction);
    d->customAction   = d->optionsMenu->addAction(i18nc("@action: metadata view", "Custom"));
    d->customAction->setCheckable(true);
    filterGroup->addAction(d->customAction);
    d->optionsMenu->addSeparator();
    d->settingsAction = d->optionsMenu->addAction(i18nc("@action: metadata view", "Settings"));
    d->settingsAction->setCheckable(false);

    filterGroup->setExclusive(true);
    d->filterBtn->setMenu(d->optionsMenu);

    // -----------------------------------------------------------------

    const int spacing        = layoutSpacing();

    d->metadataView          = new QWidget(this);
    QGridLayout* const grid2 = new QGridLayout(d->metadataView);

    d->toolBtn               = new QToolButton(this);
    d->toolBtn->setToolTip(i18nc("@info: metadata view", "Tools"));
    d->toolBtn->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    d->toolBtn->setPopupMode(QToolButton::InstantPopup);
    d->toolBtn->setWhatsThis(i18nc("@info: metadata view", "Run tool over metadata tags."));

    QMenu* const toolMenu    = new QMenu(d->toolBtn);
    d->saveMetadata          = toolMenu->addAction(i18nc("@action:inmenu", "Save in file"));
    d->printMetadata         = toolMenu->addAction(i18nc("@action:inmenu", "Print"));
    d->copy2ClipBoard        = toolMenu->addAction(i18nc("@action:inmenu", "Copy to Clipboard"));
    d->toolBtn->setMenu(toolMenu);

    d->preLoadingTimer       = new QTimer(this);
    d->preLoadingTimer->setInterval(2000);
    d->preLoadingTimer->setSingleShot(true);
    d->loadingView           = new ExifToolLoadingView(this);

    d->view                  = new ExifToolListView(d->metadataView);
    d->searchBar             = new SearchTextBar(d->metadataView, QLatin1String("ExifToolSearchBar"));

    grid2->addWidget(d->filterBtn, 0, 0, 1, 1);
    grid2->addWidget(d->searchBar, 0, 1, 1, 3);
    grid2->addWidget(d->toolBtn,   0, 4, 1, 1);
    grid2->addWidget(d->view,      1, 0, 1, 5);
    grid2->setColumnStretch(2, 10);
    grid2->setRowStretch(1, 10);
    grid2->setContentsMargins(spacing, spacing, spacing, spacing);
    grid2->setSpacing(0);

    // ---

    d->errorView             = new ExifToolErrorView(this);

    insertWidget(Private::LoadingView,  d->loadingView);
    insertWidget(Private::MetadataView, d->metadataView);
    insertWidget(Private::ErrorView,    d->errorView);

    setCurrentIndex(Private::MetadataView);

    setup();
}

ExifToolWidget::~ExifToolWidget()
{
    delete d;
}

void ExifToolWidget::loadFromUrl(const QUrl& url)
{
    d->fileName = url.fileName();

    d->preLoadingTimer->start();
    d->view->loadFromUrl(url);
}

void ExifToolWidget::slotLoadingResult(bool ok)
{
    d->preLoadingTimer->stop();

    if (ok)
    {
        buildView();
        SearchTextSettings settings = d->searchBar->searchTextSettings();

        if (!settings.text.isEmpty())
        {
            d->view->slotSearchTextChanged(settings);
        }

        setCurrentIndex(Private::MetadataView);
    }
    else
    {
        d->errorView->setErrorText(i18nc("@info: error message",
                                   "Cannot load data\n"
                                   "from %1\n"
                                   "with ExifTool.\n\n"
                                   "%2",
                                   d->fileName,
                                   d->view->errorString()));

        setCurrentIndex(Private::ErrorView);
    }

    d->loadingView->setBusy(false);
    d->toolBtn->setEnabled(ok);
}

void ExifToolWidget::slotPreLoadingTimerDone()
{
    setCurrentIndex(Private::LoadingView);
    d->loadingView->setBusy(true);
}

void ExifToolWidget::setup()
{
    connect(d->optionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotFilterChanged(QAction*)));

    connect(d->view, SIGNAL(signalLoadingResult(bool)),
            this, SLOT(slotLoadingResult(bool)));

    connect(d->preLoadingTimer, SIGNAL(timeout()),
            this, SLOT(slotPreLoadingTimerDone()));

    connect(d->errorView, SIGNAL(signalSetupExifTool()),
            this, SIGNAL(signalSetupExifTool()));

    connect(d->copy2ClipBoard, SIGNAL(triggered(bool)),
            this, SLOT(slotCopy2Clipboard()));

    connect(d->printMetadata, SIGNAL(triggered(bool)),
            this, SLOT(slotPrintMetadata()));

    connect(d->saveMetadata, SIGNAL(triggered(bool)),
            this, SLOT(slotSaveMetadataToFile()));

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            d->view, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->view, SIGNAL(signalTextFilterMatch(bool)),
            d->searchBar, SLOT(slotSearchResult(bool)));
}

QString ExifToolWidget::metadataToText() const
{
    QString textmetadata  = i18nc("@info: metadata to text", "File name: %1 (%2)", d->fileName, QLatin1String("ExifTool"));
    int i                 = 0;
    QTreeWidgetItem* item = nullptr;

    do
    {
        item                                = d->view->topLevelItem(i);
        ExifToolListViewGroup* const lvItem = dynamic_cast<ExifToolListViewGroup*>(item);

        if (lvItem)
        {
            textmetadata.append(QLatin1String("\n\n>>> "));
            textmetadata.append(lvItem->text(0));
            textmetadata.append(QLatin1String(" <<<\n\n"));

            int j                  = 0;
            QTreeWidgetItem* child = nullptr;

            do
            {
                child = lvItem->child(j);

                if (child)
                {
                    const ExifToolListViewItem* const lvItem2 = dynamic_cast<ExifToolListViewItem*>(child);

                    if (lvItem2)
                    {
                        textmetadata.append(lvItem2->getTitle());
                        textmetadata.append(QLatin1String(" : "));
                        textmetadata.append(lvItem2->getFullValue());
                        textmetadata.append(QLatin1Char('\n'));
                    }
                }

                ++j;
            }
            while (child);
        }

        ++i;
    }
    while (item);

    return textmetadata;
}

void ExifToolWidget::slotFilterChanged(QAction* action)
{
    if      (action == d->settingsAction)
    {
        Q_EMIT signalSetupMetadataFilters();
    }
    else if (
             (action == d->noneAction)  ||
             (action == d->photoAction) ||
             (action == d->customAction)
            )
    {
        buildView();
    }
}

void ExifToolWidget::slotCopy2Clipboard()
{
    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(metadataToText());
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void ExifToolWidget::slotPrintMetadata()
{
    QPrinter printer;
    printer.setFullPage(true);

    QPointer<QPrintDialog> dialog = new QPrintDialog(&printer, qApp->activeWindow());

    if (dialog->exec())
    {
        QTextDocument doc;
        doc.setPlainText(metadataToText());
        QFont font(QApplication::font());
        font.setPointSize(10);                // we define 10pt to be a nice base size for printing.
        doc.setDefaultFont(font);
        doc.print(&printer);
    }

    delete dialog;
}

void ExifToolWidget::slotSaveMetadataToFile()
{
    QPointer<DFileDialog> fileSaveDialog = new DFileDialog(this, i18nc("@title:window", "Save ExifTool Information"),
                                                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    fileSaveDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileSaveDialog->setFileMode(QFileDialog::AnyFile);
    fileSaveDialog->selectFile(QString::fromUtf8("%1.txt").arg(d->fileName));
    fileSaveDialog->setNameFilter(QLatin1String("*.txt"));
    fileSaveDialog->exec();

    // Check for cancel.

    if (!fileSaveDialog->hasAcceptedUrls())
    {
        delete fileSaveDialog;
        return;
    }

    QUrl url = fileSaveDialog->selectedUrls().constFirst();
    delete fileSaveDialog;

    QFile file(url.toLocalFile());

    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    QTextStream stream(&file);
    stream << metadataToText();
    file.close();
}

QString ExifToolWidget::getCurrentItemKey() const
{
    return d->view->getCurrentItemKey();
}

void ExifToolWidget::setCurrentItemByKey(const QString& itemKey)
{
    d->view->setCurrentItemByKey(itemKey);
}

QStringList ExifToolWidget::getTagsFilter() const
{
    return d->tagsFilter;
}

void ExifToolWidget::setTagsFilter(const QStringList& list)
{
    d->tagsFilter = list;

    if (d->tagsFilter.isEmpty())
    {
        d->customAction->setEnabled(false);

        if (getMode() == CUSTOM)
        {
            d->noneAction->setChecked(true);
        }
    }
    else
    {
        d->customAction->setEnabled(true);
    }

    buildView();
}

void ExifToolWidget::setMode(int mode)
{
    if      (mode == NONE)
    {
        d->noneAction->setChecked(true);
    }
    else if (mode == PHOTO)
    {
        d->photoAction->setChecked(true);
    }
    else
    {
        d->customAction->setChecked(true);
    }

    buildView();
}

int ExifToolWidget::getMode() const
{
    if      (d->noneAction->isChecked())
    {
        return NONE;
    }
    else if (d->photoAction->isChecked())
    {
        return PHOTO;
    }

    return CUSTOM;
}

void ExifToolWidget::buildView()
{
    switch (getMode())
    {
        case CUSTOM:
        {
            d->view->setGroupList(getTagsFilter());
            break;
        }

        case PHOTO:
        {
            d->view->setGroupList(QStringList() << QLatin1String("FULL"), d->keysFilter);
            break;
        }

        default: // NONE
        {
            d->view->setGroupList(QStringList());
            break;
        }
    }

    d->view->slotSearchTextChanged(d->searchBar->searchTextSettings());
}

} // namespace Digikam

#include "moc_exiftoolwidget.cpp"
