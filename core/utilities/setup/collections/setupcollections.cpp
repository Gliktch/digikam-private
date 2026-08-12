/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-01
 * Description : collections setup tab
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupcollections.h"

// Qt includes

#include <QLabel>
#include <QStyle>
#include <QCheckBox>
#include <QLineEdit>
#include <QWhatsThis>
#include <QVBoxLayout>
#include <QApplication>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "applicationsettings.h"
#include "setupcollectionview.h"
#include "scancontroller.h"
#include "setup.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupCollections::Private
{
public:

    Private() = default;

public:

    const QString configGroupName               = QLatin1String("Collection Settings");
    const QString configLastAddedCollectionPath = QLatin1String("LastAddedCollectionPath");

    bool                     rootsPathChanged   = false;

    SetupCollectionTreeView* collectionView     = nullptr;
    SetupCollectionModel*    collectionModel    = nullptr;

    QCheckBox*               monitoringBox      = nullptr;
    QLineEdit*               ignoreEdit         = nullptr;
    QTabWidget*              tab                = nullptr;
    QLabel*                  ignoreLabel        = nullptr;
};

SetupCollections::SetupCollections(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    const int spacing     = layoutSpacing();


    d->tab                = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    // --------------------------------------------------------

    QWidget* const albumPanel      = new QWidget(d->tab);
    QVBoxLayout* const albumLayout = new QVBoxLayout(albumPanel);

#ifndef Q_OS_WIN

    QLabel* const albumPathLabel   = new QLabel(i18n("<p>Below are the locations of your root albums used to store "
                                                     "your images. Write access is necessary to be able to edit "
                                                     "images in these albums.</p>"
                                                     "<p>Note: Removable media (such as USB drives or DVDs) and "
                                                     "remote file systems (such as NFS, or Samba mounted with "
                                                     "cifs/smbfs) are supported.</p>"
                                                     "<p>Important: You need to mount the share natively on your "
                                                     "system before to setup a remote collection.</p>"
                                                     "<p></p>"),
                                               albumPanel);

#else

    QLabel* const albumPathLabel   = new QLabel(i18n("<p>Below are the locations of your root albums used to store "
                                                     "your images. Write access is necessary to be able "
                                                     "to edit images in these albums.</p>"
                                                     "<p></p>"),
                                                albumPanel);

#endif

    albumPathLabel->setWordWrap(true);

    d->collectionView              = new SetupCollectionTreeView(albumPanel);
    d->collectionModel             = new SetupCollectionModel(albumPanel);
    d->collectionView->setModel(d->collectionModel);

    d->monitoringBox               = new QCheckBox(i18n("Monitor the albums for external "
                                                        "changes (requires restart)"), albumPanel);

    QLabel* const monitoringNote   = new QLabel(i18n("<b>Note: Album monitoring consumes system resources and "
                                                     "can slow down digiKam. If you are using macOS and network "
                                                     "collections, you should not activate album monitoring.</b>"));
    monitoringNote->setWordWrap(true);
    monitoringNote->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    albumLayout->addWidget(albumPathLabel);
    albumLayout->addWidget(d->collectionView);
    albumLayout->addWidget(d->monitoringBox);
    albumLayout->addWidget(monitoringNote);
    albumLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    albumLayout->setSpacing(spacing);

    d->tab->insertTab(Collections, albumPanel, i18nc("@title:tab", "Root Album Folders"));

    // --------------------------------------------------------

    QWidget* const ignorePanel      = new QWidget(d->tab);
    QGridLayout* const ignoreLayout = new QGridLayout(ignorePanel);

    QLabel* const ignoreInfoLabel   = new QLabel(i18n("<p>Set the names of directories that you want to ignore "
                                                      "from your photo collections. The names are case sensitive "
                                                      "and should be separated by a semicolon.</p>"
                                                      "<p>This is for example useful when you store your photos "
                                                      "on a Synology NAS (Network Attached Storage). In every "
                                                      "directory the system creates a subdirectory @eaDir to "
                                                      "store thumbnails. To avoid digiKam inserting the original "
                                                      "photo and its corresponding thumbnail twice, @eaDir is "
                                                      "ignored by default.</p>"
                                                      "<p>Wildcard characters such as '*' or '?' are supported.</p>"
                                                      "<p>To re-include directories that are ignored by default "
                                                      "prefix it with a minus, e.g. -@eaDir.</p>"),
                                                 ignorePanel);
    ignoreInfoLabel->setWordWrap(true);

    QLabel* const logoLabel1        = new QLabel(ignorePanel);
    logoLabel1->setPixmap(QIcon::fromTheme(QLatin1String("folder")).pixmap(48));

    d->ignoreLabel                  = new QLabel(ignorePanel);
    d->ignoreLabel->setText(i18n("Additional directories to ignore "
                                 "(<a href='image'>Currently ignored directories</a>):"));

    d->ignoreEdit                   = new QLineEdit(ignorePanel);
    d->ignoreEdit->setClearButtonEnabled(true);
    d->ignoreEdit->setPlaceholderText(i18n("Enter directories that you want to "
                                           "ignore from adding to your collections."));
    ignoreInfoLabel->setBuddy(d->ignoreEdit);

    ignoreLayout->addWidget(ignoreInfoLabel, 0, 0, 1, 2);
    ignoreLayout->addWidget(logoLabel1,      1, 0, 2, 1);
    ignoreLayout->addWidget(d->ignoreLabel,  1, 1, 1, 1);
    ignoreLayout->addWidget(d->ignoreEdit,   2, 1, 1, 1);
    ignoreLayout->setColumnStretch(1, 10);
    ignoreLayout->setRowStretch(3, 10);
    ignoreLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    ignoreLayout->setSpacing(spacing);

    connect(d->ignoreLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowCurrentIgnoredDirectoriesSettings()));

    d->tab->insertTab(IgnoreDirs, ignorePanel, i18nc("@title:tab", "Ignored Directories"));

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupCollections::~SetupCollections()
{
    delete d;
}

void SetupCollections::setActiveTab(CollectionsTab tab)
{
    d->tab->setCurrentIndex(tab);
}

SetupCollections::CollectionsTab SetupCollections::activeTab() const
{
    return (CollectionsTab)d->tab->currentIndex();
}

void SetupCollections::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configLastAddedCollectionPath,
                     d->collectionModel->lastAddedCollectionPath);

    d->collectionModel->apply();

    ApplicationSettings* const settings = ApplicationSettings::instance();
    settings->setAlbumMonitoring(d->monitoringBox->isChecked());

    QString ignoreDirectory;
    CoreDbAccess().db()->getUserIgnoreDirectoryFilterSettings(&ignoreDirectory);

    if (d->ignoreEdit->text() != ignoreDirectory)
    {
        CoreDbAccess().db()->setUserIgnoreDirectoryFilterSettings(
            Setup::cleanUserFilterString(d->ignoreEdit->text(), true, true));

        ScanController::instance()->completeCollectionScanInBackground(false, false);
    }
}

void SetupCollections::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->collectionModel->lastAddedCollectionPath =
        group.readEntry(d->configLastAddedCollectionPath, QString());

    d->collectionModel->loadCollections();

    const ApplicationSettings* const settings = ApplicationSettings::instance();
    d->monitoringBox->setChecked(settings->getAlbumMonitoring());

    QString ignoreDirectory;
    CoreDbAccess().db()->getUserIgnoreDirectoryFilterSettings(&ignoreDirectory);
    d->ignoreEdit->setText(ignoreDirectory);
}

void SetupCollections::slotShowCurrentIgnoredDirectoriesSettings() const
{
    QStringList ignoreDirectoryList;
    CoreDbAccess().db()->getIgnoreDirectoryFilterSettings(&ignoreDirectoryList);

    QString text = i18n("<p>Directories starting with a dot are ignored by "
                        "default.<br> <code>%1</code></p>",
                        ignoreDirectoryList.join(QLatin1Char(';')));

    QWhatsThis::showText(d->ignoreLabel->mapToGlobal(QPoint(0, 0)), text, d->ignoreLabel);
}

} // namespace Digikam

#include "moc_setupcollections.cpp"
