/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 04-10-2009
 * Description : a tool to import items from a KIO accessible location
 *
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ftimportwidget.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QPointer>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>
#include <kcombobox.h>
#include <kurlrequester.h>

// Local includes

#include "digikam_debug.h"
#include "dfiledialog.h"
#include "dexpanderbox.h"

namespace DigikamGenericFileTransferPlugin
{

class Q_DECL_HIDDEN FTImportWidget::Private
{
public:

    Private() = default;

public:

    KUrlComboRequester* sourceCombo  = nullptr;
    QPushButton*        importBtn    = nullptr;
    DItemsList*         imageList    = nullptr;
    QWidget*            uploadWidget = nullptr;
    QUrl                sourceUrl;
};

FTImportWidget::FTImportWidget(QWidget* const parent, DInfoInterface* const iface)
    : QWidget(parent),
      d      (new Private)
{
    // Setup remote source selection

    QLabel* const label       = new QLabel(this);
    d->sourceCombo            = new KUrlComboRequester(this);

    if (d->sourceCombo->button())
    {
        d->sourceCombo->button()->hide();
    }

    d->sourceCombo->comboBox()->setEditable(true);

    label->setText(i18n("Source Location: "));
    d->sourceCombo->setWhatsThis(i18n("Sets the source address to download the images to. "
                                      "This can be any address as used in your file-manager, e.g:<br>"
                                      "<i>ftp://my.server.org/sub/folder</i><br>"
                                      "<i>fish://username@my.server.org/sub/folder</i>"));

    d->importBtn = new QPushButton(i18n("Select Source Location..."), this);
    d->importBtn->setIcon(QIcon::fromTheme(QLatin1String("folder-remote")));

    // Setup image list

    d->imageList = new DItemsList(this);
    d->imageList->setObjectName(QLatin1String("FTImport ImagesList"));
    d->imageList->setAllowRAW(true);
    d->imageList->setIface(iface);
    d->imageList->listView()->setColumnEnabled(DItemsListView::Thumbnail, false);
    d->imageList->setControlButtons(DItemsList::Remove | DItemsList::MoveUp | DItemsList::MoveDown | DItemsList::Clear);
    d->imageList->listView()->setWhatsThis(i18n("This is the list of source items to import "
                                                "into the selected destination."));

    // Separator and Arrow

    DLineWidget* const vline1 = new DLineWidget(Qt::Vertical, this);
    QLabel* const arrow       = new QLabel(QChar(0x21D2));
    arrow->setAlignment(Qt::AlignCenter);
    DLineWidget* const vline2 = new DLineWidget(Qt::Vertical, this);

    // Setup Destination Selector

    QLabel* const uploadLabel = new QLabel(i18n("Select Destination Location:"), this);
    d->uploadWidget           = iface->uploadWidget(this);

    // Layout Widget

    QGridLayout* const grid = new QGridLayout(this);

    grid->addWidget(label,              0, 0, 1, 1);
    grid->addWidget(d->sourceCombo,     1, 0, 1, 1);
    grid->addWidget(d->importBtn,       2, 0, 1, 1);
    grid->addWidget(d->imageList,       3, 0, 4, 1);

    grid->addWidget(vline1,             0, 1, 4, 1);
    grid->addWidget(arrow,              4, 1, 1, 1);
    grid->addWidget(vline2,             5, 1, 2, 1);

    grid->addWidget(uploadLabel,        0, 2, 1, 1);
    grid->addWidget(d->uploadWidget,    1, 2, 6, 1);

    grid->setSpacing(layoutSpacing());
    grid->setContentsMargins(QMargins(0, 0, 0, 0));
    grid->setRowStretch(3, 10);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(2, 10);

    connect(d->importBtn, SIGNAL(clicked(bool)),
            this, SLOT(slotShowImportDialogClicked(bool)));

    connect(d->sourceCombo, SIGNAL(textChanged(QString)),
            this, SLOT(slotLabelUrlChanged()));
}

FTImportWidget::~FTImportWidget()
{
    delete d;
}

QUrl FTImportWidget::sourceUrl() const
{
    return d->sourceUrl;
}

QList<QUrl> FTImportWidget::history() const
{
    QList<QUrl> urls;

    for (int i = 0 ; i <= d->sourceCombo->comboBox()->count() ; ++i)
    {
        urls << QUrl(d->sourceCombo->comboBox()->itemText(i));
    }

    return urls;
}

void FTImportWidget::setHistory(const QList<QUrl>& urls)
{
    d->sourceCombo->comboBox()->clear();

    for (const QUrl& url : std::as_const(urls))
    {
        d->sourceCombo->comboBox()->addUrl(url);
    }
}

void FTImportWidget::setSourceUrl(const QUrl& url)
{
    d->sourceUrl = url;
    updateSourceLabel();
}

void FTImportWidget::slotShowImportDialogClicked(bool checked)
{
    Q_UNUSED(checked);

    QPointer<DFileDialog> importDlg = new DFileDialog(this, i18nc("@title:window", "Select Items to Import..."),
                                                      QString(),
                                                      i18n("All Files (*)"));
    importDlg->setAcceptMode(QFileDialog::AcceptOpen);
    importDlg->setFileMode(QFileDialog::ExistingFiles);
    importDlg->exec();

    if (importDlg->hasAcceptedUrls())
    {
        d->imageList->slotAddImages(importDlg->selectedUrls());
    }

    delete importDlg;
}

void FTImportWidget::updateSourceLabel()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Call for url "
                                     << d->sourceUrl.toDisplayString()
                                     << ", valid = "
                                     << d->sourceUrl.isValid();

    QString urlString = i18n("<i>not selected</i>");

    if (d->sourceUrl.isValid())
    {
        urlString = d->sourceUrl.toDisplayString();
        d->sourceCombo->setUrl(QUrl(urlString));
    }
}

void FTImportWidget::slotLabelUrlChanged()
{
    d->sourceUrl = d->sourceCombo->url();
}

DItemsList* FTImportWidget::imagesList() const
{
    return d->imageList;
}

QWidget* FTImportWidget::uploadWidget() const
{
    return d->uploadWidget;
}

QList<QUrl> FTImportWidget::sourceUrls() const
{
    return d->imageList->imageUrls();
}

} // namespace DigikamGenericFileTransferPlugin

#include "moc_ftimportwidget.cpp"
