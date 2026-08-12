/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-28
 * Description : a tool to export image to a KIO accessible location
 *
 * SPDX-FileCopyrightText: 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ftexportwidget.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPointer>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kurlrequester.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dfiledialog.h"
#include "wstoolutils.h"
#include "dlayoutbox.h"

namespace DigikamGenericFileTransferPlugin
{

class Q_DECL_HIDDEN FTExportWidget::Private
{
public:

    Private() = default;

public:

    KUrlComboRequester* targetCombo         = nullptr;
    QPushButton*        targetSearchButton  = nullptr;
    QUrl                targetUrl;
};

FTExportWidget::FTExportWidget(DInfoInterface* const iface, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    Q_UNUSED(iface);

    // Setup remote target selection

    QLabel* const label       = new QLabel(this);
    d->targetCombo            = new KUrlComboRequester(this);

    if (d->targetCombo->button())
    {
        d->targetCombo->button()->hide();
    }

    d->targetCombo->comboBox()->setEditable(true);

    label->setText(i18n("Target Location: "));
    d->targetCombo->setWhatsThis(i18n("Sets the target address to upload the images to. "
                                      "This can be any address as used in your file-manager, e.g:<br>"
                                      "<i>ftp://my.server.org/sub/folder</i><br>"
                                      "<i>fish://username@my.server.org/sub/folder</i>"));

    d->targetSearchButton     = new QPushButton(i18n("Select export\nlocation..."), this);
    d->targetSearchButton->setIcon(QIcon::fromTheme(QLatin1String("folder-remote")));

    // Layout Settings Widget

    QVBoxLayout* const layout = new QVBoxLayout(this);

    layout->addWidget(label);
    layout->addWidget(d->targetCombo);
    layout->addWidget(d->targetSearchButton);
    layout->addStretch();
    layout->setSpacing(layoutSpacing());
    layout->setContentsMargins(QMargins());

    // ------------------------------------------------------------------------

    connect(d->targetSearchButton, SIGNAL(clicked(bool)),
            this, SLOT(slotShowTargetDialogClicked(bool)));

    connect(d->targetCombo, SIGNAL(textChanged(QString)),
            this, SLOT(slotTargetUrlChanged()));

    connect(d->targetCombo, SIGNAL(returnPressed(QString)),
            this, SLOT(slotTargetUrlChanged()));

    // ------------------------------------------------------------------------

    updateTargetLabel();
}

FTExportWidget::~FTExportWidget()
{
    delete d;
}

QUrl FTExportWidget::targetUrl() const
{
    return d->targetUrl;
}

QList<QUrl> FTExportWidget::history() const
{
    QList<QUrl> urls;

    for (int i = 0 ; i <= d->targetCombo->comboBox()->count() ; ++i)
    {
        urls << QUrl(d->targetCombo->comboBox()->itemText(i));
    }

    return urls;
}

void FTExportWidget::setHistory(const QList<QUrl>& urls)
{
    d->targetCombo->comboBox()->clear();

    for (const QUrl& url : std::as_const(urls))
    {
        d->targetCombo->comboBox()->addUrl(url);
    }
}

void FTExportWidget::setTargetUrl(const QUrl& url)
{
    d->targetUrl = url;
    updateTargetLabel();
}

void FTExportWidget::slotShowTargetDialogClicked(bool checked)
{
    Q_UNUSED(checked);

    QPointer<DFileDialog> targetDialog = new DFileDialog(this, i18nc("@title:window", "Select Target Location..."),
                                                         d->targetUrl.toString(), i18n("All Files (*)"));
    targetDialog->setAcceptMode(QFileDialog::AcceptSave);
    targetDialog->setFileMode(QFileDialog::Directory);
    targetDialog->setOptions(QFileDialog::ShowDirsOnly);
    targetDialog->exec();

    if (targetDialog->hasAcceptedUrls())
    {
        d->targetUrl = targetDialog->selectedUrls().constFirst();
        updateTargetLabel();

        Q_EMIT signalTargetUrlChanged(d->targetUrl);
    }

    delete targetDialog;
}

void FTExportWidget::updateTargetLabel()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Call for url "
                                     << d->targetUrl.toDisplayString()
                                     << ", valid = "
                                     << d->targetUrl.isValid();

    QString urlString = i18n("<i>not selected</i>");

    if (d->targetUrl.isValid())
    {
        urlString = d->targetUrl.toDisplayString();
        d->targetCombo->setUrl(QUrl(urlString));
    }
}

void FTExportWidget::slotTargetUrlChanged()
{
    d->targetUrl = d->targetCombo->url();

    Q_EMIT signalTargetUrlChanged(d->targetUrl);
}

} // namespace DigikamGenericFileTransferPlugin

#include "moc_ftexportwidget.cpp"
