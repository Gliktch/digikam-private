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

#include "collectionpage.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QTemporaryFile>
#include <QMessageBox>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "dfileselector.h"
#include "digikam_debug.h"
#include "digikam_version.h"

namespace Digikam
{

class Q_DECL_HIDDEN CollectionPage::Private
{
public:

    Private() = default;

    QString        rootAlbum;

    DFileSelector* rootAlbumPathRequester = nullptr;
};

CollectionPage::CollectionPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure where you keep your images</b>")),
      d          (new Private)
{
    QWidget* const widget       = new QWidget(this);
    QVBoxLayout* const vlayout  = new QVBoxLayout(widget);

    QString picturesPath        = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    qCDebug(DIGIKAM_GENERAL_LOG) << picturesPath;

    if (picturesPath.isEmpty())
    {
        picturesPath = QDir::homePath() + i18nc("This is a path name so you should "
                                                "include the slash in the translation", "/Pictures");
    }

    QLabel* const textLabel1    = new QLabel(widget);
    textLabel1->setWordWrap(true);

    QString message             = i18n("<p>Please enter a location where you keep your images.</p> "
                                       "<p>You can choose any local folder, even one that already contains images."
                                       "<br> "
                                       "More folders can be added later under the <i>Settings</i> menu. "
                                       "</p> ");

#ifndef Q_OS_WIN

    message.append(i18n("<p><i>Note:</i> removable media (such as USB drives or DVDs) and remote file systems "
                        "(such as NFS, or Samba mounted with cifs/smbfs) are supported.</p>"));

#endif

    textLabel1->setText(message);

    d->rootAlbumPathRequester   = new DFileSelector(widget);
    d->rootAlbumPathRequester->setFileDlgMode(QFileDialog::Directory);
    d->rootAlbumPathRequester->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->rootAlbumPathRequester->setFileDlgPath(picturesPath);

    vlayout->addWidget(textLabel1);
    vlayout->addWidget(d->rootAlbumPathRequester);
    vlayout->setContentsMargins(QMargins());
    vlayout->setSpacing(layoutSpacing());


    setPageWidget(widget);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-pictures")));
}

CollectionPage::~CollectionPage()
{
    delete d;
}

QString CollectionPage::firstAlbumPath() const
{
    return d->rootAlbum;
}

void CollectionPage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("General Settings"));
    group.writeEntry("Version", digikam_version);

    config->sync();
}

bool CollectionPage::checkSettings()
{
    QString rootAlbumFolder;

    if (!checkRootAlbum(rootAlbumFolder))
    {
        return false;
    }

    d->rootAlbum = rootAlbumFolder;

    return true;
}

bool CollectionPage::checkRootAlbum(QString& rootAlbumFolder)
{
    rootAlbumFolder = d->rootAlbumPathRequester->fileDlgPath();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Root album is : " << rootAlbumFolder;

    if (rootAlbumFolder.isEmpty())
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("You must select a folder for digiKam to "
                                      "use as the root album. All of your images will go there."));
        return false;
    }

#ifndef Q_OS_WIN

    if (!QDir::isAbsolutePath(rootAlbumFolder))
    {
        rootAlbumFolder.prepend(QDir::homePath());
    }

#endif

/*
    if (QUrl::fromLocalFile(rootAlbumFolder).equals(QUrl::fromLocalFile(QDir::homePath()), QUrl::CompareWithoutFragment))
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("digiKam will not use your home folder as the "
                                      "root album. Please select another location."));
        return false;
    }
*/

    QDir targetPath(rootAlbumFolder);

    if (!targetPath.exists())
    {
        int rc = QMessageBox::question(this, i18nc("@title:window", "Create Root Album Folder?"),
                                       i18n("<p>The folder to use as the root album path does not exist:</p>"
                                                 "<p><b>%1</b></p>"
                                                 "Would you like digiKam to create it for you?",
                                                 QDir::toNativeSeparators(rootAlbumFolder)));

        if (rc == QMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkpath(rootAlbumFolder))
        {
            QMessageBox::information(this, i18nc("@title:window", "Create Root Album Folder Failed"),
                                     i18n("<p>digiKam could not create the folder to use as the root album.<br>"
                                          "Please select a different location.</p>"
                                          "<p><b>%1</b></p>", QDir::toNativeSeparators(rootAlbumFolder)));
            return false;
        }
    }

    QFileInfo path(rootAlbumFolder);

#ifdef Q_OS_WIN

    // Work around bug #189168

    QTemporaryFile temp;
    temp.setFileTemplate(path.filePath() + QLatin1String("/XXXXXX"));

    if (!temp.open())

#else

    if (!path.isWritable())

#endif
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("You do not seem to have write access for the folder "
                                      "selected to be the root album.\n"
                                      "Warning: Without write access, items cannot be edited."));
    }

    return true;
}

} // namespace Digikam

#include "moc_collectionpage.cpp"
