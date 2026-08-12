/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * SPDX-FileCopyrightText: 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "jalbumoutputpage.h"

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
#include "jalbumwizard.h"
#include "jalbumsettings.h"
#include "dfileselector.h"
#include "dtextedit.h"

namespace DigikamGenericJAlbumPlugin
{

class Q_DECL_HIDDEN JAlbumOutputPage::Private
{
public:

    Private() = default;

public:

    DFileSelector*  destUrl             = nullptr;
    QLabel*         titleLabel          = nullptr;
    DPlainTextEdit* imageSelectionTitle = nullptr;
};

JAlbumOutputPage::JAlbumOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private)
{
    setObjectName(QLatin1String("OutputPage"));

    QWidget* const main      = new QWidget(this);

    // --------------------

    d->titleLabel = new QLabel(main);
    d->titleLabel->setWordWrap(false);
    d->titleLabel->setText(i18n("Project Title:"));

    d->imageSelectionTitle   = new DPlainTextEdit(main);
    d->imageSelectionTitle->setLinesVisible(1);
    d->imageSelectionTitle->setPlaceholderText(i18n("Set here the project title"));
    d->titleLabel->setBuddy(d->imageSelectionTitle);

    // --------------------

    QLabel* const textLabel1 = new QLabel(main);
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18n("Projects folder:"));

    d->destUrl = new DFileSelector(main);
    d->destUrl->setFileDlgTitle(i18nc("@title:window", "Projects Folder"));
    d->destUrl->setFileDlgMode(QFileDialog::Directory);
    d->destUrl->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    textLabel1->setBuddy(d->destUrl);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(layoutSpacing());
    grid->addWidget(d->titleLabel,          0, 0, 1, 1);
    grid->addWidget(d->imageSelectionTitle, 0, 1, 1, 1);
    grid->addWidget(textLabel1,             1, 0, 1, 1);
    grid->addWidget(d->destUrl,             1, 1, 1, 1);
    grid->setRowStretch(2, 10);

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

JAlbumOutputPage::~JAlbumOutputPage()
{
    delete d;
}

void JAlbumOutputPage::initializePage()
{
    JAlbumWizard* const wizard = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
    {
        return;
    }

    JAlbumSettings* const info  = wizard->settings();

    d->destUrl->setFileDlgPath(info->m_destPath);
    d->imageSelectionTitle->setText(info->m_imageSelectionTitle);
}

bool JAlbumOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
    {
        return false;
    }

    if (d->imageSelectionTitle->text().isEmpty())
    {
        return false;
    }

    JAlbumWizard* const wizard  = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
    {
        return false;
    }

    JAlbumSettings* const settings  = wizard->settings();
    settings->m_destPath            = d->destUrl->fileDlgPath();
    settings->m_imageSelectionTitle = d->imageSelectionTitle->text();

    return true;
}

bool JAlbumOutputPage::isComplete() const
{
    const JAlbumWizard* const wizard = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
    {
        return false;
    }

    bool b = !d->destUrl->fileDlgPath().isEmpty();
    b      = b && !d->imageSelectionTitle->text().isEmpty();

    return b;
}

} // namespace DigikamGenericJAlbumPlugin

#include "moc_jalbumoutputpage.cpp"
