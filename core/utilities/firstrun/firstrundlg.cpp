/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "firstrundlg.h"

// Qt includes

#include <QPushButton>

// Local includes

#include "digikam_globals.h"
#include "dxmlguiwindow.h"
#include "welcomepage.h"
#include "collectionpage.h"
#include "databasepage.h"
#include "rawpage.h"
#include "metadatapage.h"
#include "previewpage.h"
#include "openfilepage.h"
#include "tooltipspage.h"
#include "startscanpage.h"

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#   include "migratefromdigikam4page.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN FirstRunDlg::Private
{
public:

    Private() = default;

    WelcomePage*             welcomePage                = nullptr;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    MigrateFromDigikam4Page* migrateFromDigikam4Page    = nullptr;

#endif

    CollectionPage*          collectionPage             = nullptr;
    DatabasePage*            databasePage               = nullptr;
    RawPage*                 rawPage                    = nullptr;
    MetadataPage*            metadataPage               = nullptr;
    PreviewPage*             previewPage                = nullptr;
    OpenFilePage*            openFilePage               = nullptr;
    TooltipsPage*            tooltipsPage               = nullptr;
    StartScanPage*           startScanPage              = nullptr;
};

FirstRunDlg::FirstRunDlg(QWidget* const parent)
    : QWizard(parent),
      d      (new Private)
{
    setWizardStyle(QWizard::ModernStyle);
    setButtonLayout(QList<QWizard::WizardButton>() << QWizard::HelpButton
                                                   << QWizard::BackButton
                                                   << QWizard::CancelButton
                                                   << QWizard::NextButton
                                                   << QWizard::FinishButton);

    d->welcomePage    = new WelcomePage(this);    // First assistant page

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    if (MigrateFromDigikam4Page::checkForMigration())
    {
        d->migrateFromDigikam4Page = new MigrateFromDigikam4Page(this);
    }

#endif

    d->collectionPage = new CollectionPage(this);
    d->databasePage   = new DatabasePage(this);
    d->rawPage        = new RawPage(this);
    d->metadataPage   = new MetadataPage(this);
    d->previewPage    = new PreviewPage(this);
    d->openFilePage   = new OpenFilePage(this);
    d->tooltipsPage   = new TooltipsPage(this);

    // NOTE: Added here new assistant pages...

    d->startScanPage  = new StartScanPage(this);  // Last assistant page

    resize(600, 600);

    connect(button(QWizard::FinishButton), SIGNAL(clicked()),
            this, SLOT(slotFinishPressed()));

    connect(this, SIGNAL(helpRequested()),
            this, SLOT(slotHelp()));
}

FirstRunDlg::~FirstRunDlg()
{
    delete d;
}

void FirstRunDlg::slotHelp()
{
    openOnlineDocumentation(QLatin1String("getting_started"),
                            QLatin1String("quick_start"));
}

QString FirstRunDlg::firstAlbumPath() const
{
    return d->collectionPage->firstAlbumPath();
}

DbEngineParameters FirstRunDlg::getDbEngineParameters() const
{
    return d->databasePage->getDbEngineParameters();
}

bool FirstRunDlg::validateCurrentPage()
{
    if (currentPage() == d->collectionPage)
    {
        if (!d->collectionPage->checkSettings())
        {
            return false;
        }
        else
        {
            d->databasePage->setDatabasePath(firstAlbumPath());
        }
    }

    if (currentPage() == d->databasePage)
    {
        if (!d->databasePage->checkSettings())
        {
            return false;
        }
    }

    return true;
}

void FirstRunDlg::slotFinishPressed()
{

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    if (d->migrateFromDigikam4Page && d->migrateFromDigikam4Page->isMigrationChecked())
    {
       // The user choose to do a migration from digikam4

       d->migrateFromDigikam4Page->doMigration();
    }
    else

#endif

    {
       // Save settings to rc files.

       d->collectionPage->saveSettings();
       d->databasePage->saveSettings();
       d->rawPage->saveSettings();
       d->metadataPage->saveSettings();
       d->previewPage->saveSettings();
       d->openFilePage->saveSettings();
       d->tooltipsPage->saveSettings();
    }
}

} // namespace Digikam

#include "moc_firstrundlg.cpp"
