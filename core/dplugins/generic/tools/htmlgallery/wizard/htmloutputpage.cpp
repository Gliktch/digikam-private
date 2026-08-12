/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "htmloutputpage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dtextedit.h"
#include "htmlwizard.h"
#include "galleryinfo.h"
#include "dfileselector.h"

namespace DigikamGenericHtmlGalleryPlugin
{

class Q_DECL_HIDDEN HTMLOutputPage::Private
{
public:

    Private() = default;

public:

    DFileSelector* destUrl              = nullptr;
    QComboBox*     openInBrowser        = nullptr;
    QLabel*        titleLabel           = nullptr;
    DTextEdit*     imageSelectionTitle  = nullptr;
};

HTMLOutputPage::HTMLOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private)
{
    setObjectName(QLatin1String("OutputPage"));

    QWidget* const main      = new QWidget(this);

    // --------------------

    d->titleLabel = new QLabel(main);
    d->titleLabel->setWordWrap(false);
    d->titleLabel->setText(i18nc("@label", "Gallery Title:"));

    d->imageSelectionTitle   = new DTextEdit(main);
    d->imageSelectionTitle->setLinesVisible(1);
    d->titleLabel->setBuddy(d->imageSelectionTitle);

    // --------------------

    QLabel* const textLabel1 = new QLabel(main);
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18nc("@label", "Destination folder:"));

    d->destUrl = new DFileSelector(main);
    d->destUrl->setFileDlgTitle(i18nc("@title:window", "Destination Folder"));
    d->destUrl->setFileDlgMode(QFileDialog::Directory);
    d->destUrl->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    textLabel1->setBuddy(d->destUrl);

    // --------------------

    QLabel* const browserLabel = new QLabel(main);
    browserLabel->setWordWrap(false);
    browserLabel->setText(i18nc("@label", "Open in Browser:"));
    d->openInBrowser           = new QComboBox(main);
    d->openInBrowser->addItem(i18nc("@item: open in browser", "None"),                 GalleryConfig::NOBROWSER);
    d->openInBrowser->addItem(i18nc("@item: open in browser", "Internal"),             GalleryConfig::INTERNAL);
    d->openInBrowser->addItem(i18nc("@item: open in browser", "Default from Desktop"), GalleryConfig::DESKTOP);
    d->openInBrowser->setEditable(false);
    browserLabel->setBuddy(d->openInBrowser);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(layoutSpacing());
    grid->addWidget(d->titleLabel,          0, 0, 1, 1);
    grid->addWidget(d->imageSelectionTitle, 0, 1, 1, 1);
    grid->addWidget(textLabel1,             1, 0, 1, 1);
    grid->addWidget(d->destUrl,             1, 1, 1, 1);
    grid->addWidget(browserLabel,           2, 0, 1, 1);
    grid->addWidget(d->openInBrowser,       2, 1, 1, 1);
    grid->setRowStretch(3, 10);

    // --------------------

    setPageWidget(main);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-html")));

    connect(d->destUrl->lineEdit(), SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));

    connect(d->destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SIGNAL(completeChanged()));

    connect(d->imageSelectionTitle, SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));
}

HTMLOutputPage::~HTMLOutputPage()
{
    delete d;
}

void HTMLOutputPage::initializePage()
{
    const HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
    {
        return;
    }

    const GalleryInfo* const info  = wizard->galleryInfo();

    d->destUrl->setFileDlgPath(info->destUrl().toLocalFile());
    d->openInBrowser->setCurrentIndex(info->openInBrowser());
    d->imageSelectionTitle->setText(info->imageSelectionTitle());

    d->titleLabel->setVisible(info->m_getOption == GalleryInfo::IMAGES);
    d->imageSelectionTitle->setVisible(info->m_getOption == GalleryInfo::IMAGES);
}

bool HTMLOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
    {
        return false;
    }

    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
    {
        return false;
    }

    GalleryInfo* const info  = wizard->galleryInfo();

    if (info->m_getOption == GalleryInfo::IMAGES && d->imageSelectionTitle->text().isEmpty())
    {
        return false;
    }

    info->setDestUrl(QUrl::fromLocalFile(d->destUrl->fileDlgPath()));
    info->setOpenInBrowser(d->openInBrowser->currentIndex());
    info->setImageSelectionTitle(d->imageSelectionTitle->text());

    return true;
}

bool HTMLOutputPage::isComplete() const
{
    const HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
    {
        return false;
    }

    const GalleryInfo* const info  = wizard->galleryInfo();

    bool b                         = !d->destUrl->fileDlgPath().isEmpty();

    if (info->m_getOption == GalleryInfo::IMAGES)
    {
        b = (b && !d->imageSelectionTitle->text().isEmpty());
    }

    return b;
}

} // namespace DigikamGenericHtmlGalleryPlugin

#include "moc_htmloutputpage.cpp"
