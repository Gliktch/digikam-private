/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatawidget.h"

// Qt includes

#include <QButtonGroup>
#include <QClipboard>
#include <QDataStream>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QMimeData>
#include <QPaintDevice>
#include <QPainter>
#include <QPointer>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QTextDocument>
#include <QToolButton>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QStandardPaths>
#include <QMenu>
#include <QApplication>
#include <QStyle>
#include <QElapsedTimer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "metadatalistview.h"
#include "metadatalistviewitem.h"
#include "mdkeylistviewitem.h"
#include "searchtextbar.h"
#include "setup.h"
#include "dfiledialog.h"
#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN MetadataWidget::Private
{

public:

    Private() = default;

public:

    QAction*               noneAction       = nullptr;
    QAction*               photoAction      = nullptr;
    QAction*               customAction     = nullptr;
    QAction*               settingsAction   = nullptr;

    QGridLayout*           mainLayout       = nullptr;

    QToolButton*           filterBtn        = nullptr;
    QToolButton*           toolBtn          = nullptr;

    QString                fileName;

    QStringList            tagsFilter;

    QAction*               saveMetadata     = nullptr;
    QAction*               printMetadata    = nullptr;
    QAction*               copy2ClipBoard   = nullptr;

    QMenu*                 optionsMenu      = nullptr;

    MetadataListView*      view             = nullptr;

    SearchTextBar*         searchBar        = nullptr;

    DMetadata*             metadata         = nullptr;
    DMetadata::MetaDataMap metaDataMap;
};

MetadataWidget::MetadataWidget(QWidget* const parent, const QString& name)
    : QWidget(parent),
      d      (new Private)
{
    setObjectName(name);

    const int spacing = layoutSpacing();

    d->mainLayout     = new QGridLayout(this);

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

    d->toolBtn = new QToolButton(this);
    d->toolBtn->setToolTip(i18nc("@info: metadata view", "Tools"));
    d->toolBtn->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    d->toolBtn->setPopupMode(QToolButton::InstantPopup);
    d->toolBtn->setWhatsThis(i18nc("@info: metadata view", "Run tool over metadata tags."));

    QMenu* const toolMenu = new QMenu(d->toolBtn);
    d->saveMetadata       = toolMenu->addAction(i18nc("@action:inmenu", "Save in file"));
    d->printMetadata      = toolMenu->addAction(i18nc("@action:inmenu", "Print"));
    d->copy2ClipBoard     = toolMenu->addAction(i18nc("@action:inmenu", "Copy to Clipboard"));
    d->toolBtn->setMenu(toolMenu);

    d->view         = new MetadataListView(this);
    QString barName = name + QLatin1String("SearchBar");
    d->searchBar    = new SearchTextBar(this, barName);

    // -----------------------------------------------------------------

    d->mainLayout->addWidget(d->filterBtn, 0, 0, 1, 1);
    d->mainLayout->addWidget(d->searchBar, 0, 1, 1, 3);
    d->mainLayout->addWidget(d->toolBtn,   0, 4, 1, 1);
    d->mainLayout->addWidget(d->view,      1, 0, 1, 5);
    d->mainLayout->setColumnStretch(2, 10);
    d->mainLayout->setRowStretch(1, 10);
    d->mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->mainLayout->setSpacing(0);
}

MetadataWidget::~MetadataWidget()
{
    delete d->metadata;
    delete d;
}

void MetadataWidget::setup()
{
    connect(d->optionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotFilterChanged(QAction*)));

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

void MetadataWidget::slotFilterChanged(QAction* action)
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

QStringList MetadataWidget::getTagsFilter() const
{
    return d->tagsFilter;
}

void MetadataWidget::setTagsFilter(const QStringList& list)
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

MetadataListView* MetadataWidget::view() const
{
    return d->view;
}

void MetadataWidget::enabledToolButtons(bool b)
{
    d->toolBtn->setEnabled(b);
}

bool MetadataWidget::setMetadata(const DMetadata& data)
{
    if (d->metadata)
    {
        delete d->metadata;
    }

    d->metadata = new DMetadata(data.data());

    // Cleanup all metadata contents.

    setMetadataMap();

    if (d->metadata->isEmpty())
    {
        setMetadataEmpty();

        return false;
    }

    // Try to decode current metadata.

    QElapsedTimer execTimer;
    execTimer.start();

    bool error = !decodeMetadata();

    qCDebug(DIGIKAM_GENERAL_LOG) << getMetadataTitle() << "decoding took"
                                 << execTimer.elapsed() << "ms (" << error << ")";

    enabledToolButtons(!error);

    // Refresh view using decoded metadata.

    buildView();

    return true;
}

void MetadataWidget::setMetadataEmpty()
{
    d->view->clear();
    enabledToolButtons(false);
}

DMetadata* MetadataWidget::getMetadata() const
{
    return d->metadata;
}

bool MetadataWidget::storeMetadataToFile(const QUrl& url, const QByteArray& metaData)
{
    if (url.isEmpty())
    {
        return false;
    }

    QFile file(url.toLocalFile());

    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream stream(&file);
    stream.writeRawData(metaData.data(), metaData.size());
    file.close();

    return true;
}

void MetadataWidget::setMetadataMap(const DMetadata::MetaDataMap& data)
{
    d->metaDataMap = data;
}

const DMetadata::MetaDataMap& MetadataWidget::getMetadataMap()
{
    return d->metaDataMap;
}

void MetadataWidget::setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& tagsFilter)
{
    d->view->setIfdList(ifds, tagsFilter);
}

void MetadataWidget::setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& keysFilter,
                                const QStringList& tagsFilter)
{
    d->view->setIfdList(ifds, keysFilter, tagsFilter);
}

QString MetadataWidget::metadataToText() const
{
    QString textmetadata  = i18nc("@info: metadata clipboard", "File name: %1 (%2)", d->fileName, getMetadataTitle());
    int i                 = 0;
    QTreeWidgetItem* item = nullptr;

    do
    {
        item                            = d->view->topLevelItem(i);
        MdKeyListViewItem* const lvItem = dynamic_cast<MdKeyListViewItem*>(item);

        if (lvItem)
        {
            textmetadata.append(QLatin1String("\n\n>>> "));
            textmetadata.append(lvItem->getDecryptedKey());
            textmetadata.append(QLatin1String(" <<<\n\n"));

            int j                  = 0;
            QTreeWidgetItem* item2 = nullptr;

            do
            {
                item2 = dynamic_cast<QTreeWidgetItem*>(lvItem);
                item2 = item2 ? item2->child(j) : nullptr;

                if (item2)
                {
                    const MetadataListViewItem* const lvItem2 = dynamic_cast<MetadataListViewItem*>(item2);

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
            while (item2);
        }

        ++i;
    }
    while (item);

    return textmetadata;
}

void MetadataWidget::slotCopy2Clipboard()
{
    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(metadataToText());
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void MetadataWidget::slotPrintMetadata()
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

QUrl MetadataWidget::saveMetadataToFile(const QString& caption, const QString& fileFilter)
{
    QPointer<DFileDialog> fileSaveDialog = new DFileDialog(this, caption,
                                                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    fileSaveDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileSaveDialog->setFileMode(QFileDialog::AnyFile);
    fileSaveDialog->selectFile(d->fileName);
    fileSaveDialog->setNameFilter(fileFilter);
    fileSaveDialog->exec();

    // Check for cancel.

    if (!fileSaveDialog->hasAcceptedUrls())
    {
        delete fileSaveDialog;

        return QUrl();
    }

    QUrl url = fileSaveDialog->selectedUrls().constFirst();
    delete fileSaveDialog;

    return url;
}

void MetadataWidget::setMode(int mode)
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

int MetadataWidget::getMode() const
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

QString MetadataWidget::getCurrentItemKey() const
{
    return d->view->getCurrentItemKey();
}

void MetadataWidget::setCurrentItemByKey(const QString& itemKey)
{
    d->view->setCurrentItemByKey(itemKey);
}

bool MetadataWidget::loadFromData(const QString& fileName, const DMetadata& data)
{
    setFileName(fileName);

    return (setMetadata(data));
}

QString MetadataWidget::getTagTitle(const QString&)
{
    return QString();
}

QString MetadataWidget::getTagDescription(const QString&)
{
    return QString();
}

void MetadataWidget::setFileName(const QString& fileName)
{
    d->fileName = fileName;
}

void MetadataWidget::setUserAreaWidget(QWidget* const w)
{
    QVBoxLayout* const vLayout = new QVBoxLayout();
    vLayout->setSpacing(layoutSpacing());
    vLayout->addWidget(w);
    vLayout->addStretch();
    d->mainLayout->addLayout(vLayout, 3, 0, 1, 5);
}

void MetadataWidget::buildView()
{
    d->view->slotSearchTextChanged(d->searchBar->searchTextSettings());
}

} // namespace Digikam

#include "moc_metadatawidget.cpp"
