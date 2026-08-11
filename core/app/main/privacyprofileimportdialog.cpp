/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprofileimportdialog.h"

// Qt includes

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUuid>

#ifdef HAVE_DBUS

#   include <QDBusConnection>
#   include <QDBusConnectionInterface>
#   include <QDBusReply>

#endif

// KDE includes

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

// Local includes

#include "privacyprofileimportstager.h"
#include "privacyprofileinspector.h"
#include "privacyprofilepublication.h"

namespace Digikam
{

namespace
{

const char ImportConfigGroup[] = "Private Profile Import";
const char ShowOnStartupEntry[] = "ShowOnStartup";

QString environmentPath(const char* const name)
{
    const QString path = QFile::decodeName(qgetenv(name));
    return path.isEmpty() ? QString() : QDir::cleanPath(path);
}

void setStartupOfferEnabled(bool enabled)
{
    KConfigGroup group(KSharedConfig::openConfig(), QLatin1String(ImportConfigGroup));
    group.writeEntry(QLatin1String(ShowOnStartupEntry), enabled);
    group.sync();
}

bool startupOfferEnabled()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QLatin1String(ImportConfigGroup));
    return group.readEntry(QLatin1String(ShowOnStartupEntry), true);
}

QString exactVersionDescription(const PrivacyProfileSummary& summary)
{
    if (summary.schemaKind == PrivacyProfileSchemaKind::PrivateP1)
    {
        return i18nc("@info", "P1 (private schema 101, based on stock schema 17)");
    }

    switch (summary.schemaVersion)
    {
        case 17:
            return i18nc("@info", "Stock schema 17 (latest corresponding release: digiKam 9.1)");
        case 16:
            return i18nc("@info", "Stock schema 16 (latest corresponding release: digiKam 9.0)");
        case 15:
            return i18nc("@info", "Stock schema 15 (latest corresponding release: digiKam 7.10)");
        default:
            return i18nc("@info", "Stock schema %1 (digiKam earlier than 7.5)",
                         summary.schemaVersion);
    }
}

QString summaryDetails(const PrivacyProfileSummary& summary)
{
    QStringList lines;
    lines << i18nc("@info", "Settings file: %1",
                   summary.settingsPath.isEmpty()
                       ? i18nc("@info", "Not selected")
                       : QDir::toNativeSeparators(summary.settingsPath))
          << i18nc("@info", "Main database: %1",
                   QDir::toNativeSeparators(summary.databasePath))
          << i18nc("@info", "Thumbnail database: %1",
                   summary.thumbnailDatabasePath.isEmpty()
                       ? i18nc("@info", "Not found")
                       : QDir::toNativeSeparators(summary.thumbnailDatabasePath))
          << i18nc("@info", "Database type: %1", summary.databaseType)
          << i18nc("@info", "Version: %1", exactVersionDescription(summary))
          << i18ncp("@info", "%1 active indexed item", "%1 active indexed items",
                    summary.activeItemCount)
          << i18ncp("@info", "%1 total database item", "%1 total database items",
                    summary.totalItemCount)
          << i18nc("@info", "Database size: %1",
                   QLocale().formattedDataSize(summary.databaseBytes))
          << i18nc("@info", "Latest database modification: %1",
                   summary.latestModification.isValid()
                       ? QLocale().toString(summary.latestModification, QLocale::LongFormat)
                       : i18nc("@info", "Unknown"));

    if (summary.isPrivateProfile())
    {
        lines << i18ncp("@info", "%1 privacy category", "%1 privacy categories",
                        summary.privacyCategoryCount)
              << i18ncp("@info", "%1 protected item", "%1 protected items",
                        summary.protectedItemCount)
              << i18ncp("@info", "%1 unfinished privacy transaction",
                        "%1 unfinished privacy transactions",
                        summary.incompletePrivacyTransactionCount);
    }

    lines << QString()
          << i18ncp("@info", "%1 registered media location:",
                    "%1 registered media locations:", summary.collectionRoots.size());

    if (summary.collectionRoots.isEmpty())
    {
        lines << i18nc("@info", "None recorded");
    }
    else
    {
        for (const QString& root : summary.collectionRoots)
        {
            lines << QLatin1String("  ") + QDir::toNativeSeparators(root);
        }
    }

    lines << QString()
          << (summary.integrityOk
              ? i18nc("@info", "Database integrity check: Passed")
              : i18nc("@info", "Database integrity check: Failed"));

    return lines.join(QLatin1Char('\n'));
}

PrivacyProfilePaths activeProfilePaths()
{
    PrivacyProfilePaths paths;
    paths.configFilePath = KSharedConfig::openConfig()->name();
    paths.configHome = QFileInfo(paths.configFilePath).absolutePath();
    paths.dataHome = environmentPath("XDG_DATA_HOME");
    paths.cacheHome = environmentPath("XDG_CACHE_HOME");
    paths.stateHome = environmentPath("XDG_STATE_HOME");
    paths.transactionHome = environmentPath("DIGIKAM_PRIVATE_TRANSACTION_HOME");
    const PrivacyProfileSummary configured =
        PrivacyProfileInspector::inspectSettingsFile(paths.configFilePath);

    if (configured.isUsable())
    {
        paths.coreDatabasePath = configured.databasePath;
        paths.thumbnailDatabasePath = configured.thumbnailDatabasePath;
    }
    else
    {
        const QString databaseHome = environmentPath("DIGIKAM_PRIVATE_DATABASE_HOME");
        paths.coreDatabasePath = QDir(databaseHome).filePath(QLatin1String("digikam4.db"));
        paths.thumbnailDatabasePath = QDir(databaseHome).filePath(
            QLatin1String("thumbnails-digikam.db"));
    }

    return paths;
}

bool anotherDigikamProcessIsVisible()
{
#ifdef HAVE_DBUS

    QDBusConnectionInterface* const interface = QDBusConnection::sessionBus().interface();

    if (!interface)
    {
        return false;
    }

    const QDBusReply<QStringList> reply = interface->registeredServiceNames();
    const QString ownService = QLatin1String("org.kde.digikam-") +
                               QString::number(QCoreApplication::applicationPid());

    if (reply.isValid())
    {
        for (const QString& service : reply.value())
        {
            if (service.startsWith(QLatin1String("org.kde.digikam-")) &&
                (service != ownService))
            {
                return true;
            }
        }
    }

#endif

    return false;
}

} // namespace

class Q_DECL_HIDDEN PrivacyProfileImportDialog::Private
{
public:

    explicit Private(PrivacyProfileImportDialog* const dialog, bool startup)
        : q(dialog), startupOffer(startup)
    {
    }

    void buildUi()
    {
        q->setWindowTitle(i18nc("@title:window", "Import a digiKam Profile"));
        q->setMinimumWidth(660);

        auto* const layout = new QVBoxLayout(q);
        auto* const introduction = new QLabel(
            i18nc("@info",
                  "Importing creates an independent digiKam Private catalogue and settings "
                  "copy. Later changes made in stock digiKam are not synchronized here, and "
                  "changes made here are not synchronized back. Both applications can still "
                  "change the same media and companion metadata files stored beside it, such "
                  "as .xmp files."), q);
        introduction->setWordWrap(true);
        layout->addWidget(introduction);

        auto* const sourceBox = new QGroupBox(i18nc("@title:group", "Detected Source"), q);
        auto* const sourceLayout = new QFormLayout(sourceBox);
        pathLabel = new QLabel(sourceBox);
        pathLabel->setWordWrap(true);
        pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse |
                                           Qt::TextSelectableByKeyboard);
        versionLabel = new QLabel(sourceBox);
        countLabel = new QLabel(sourceBox);
        modifiedLabel = new QLabel(sourceBox);
        statusLabel = new QLabel(sourceBox);
        statusLabel->setWordWrap(true);
        sourceLayout->addRow(i18nc("@label", "Database:"), pathLabel);
        sourceLayout->addRow(i18nc("@label", "Version:"), versionLabel);
        sourceLayout->addRow(i18nc("@label", "Indexed items:"), countLabel);
        sourceLayout->addRow(i18nc("@label", "Last modified:"), modifiedLabel);
        sourceLayout->addRow(i18nc("@label", "Status:"), statusLabel);
        layout->addWidget(sourceBox);

        auto* const chooserLayout = new QHBoxLayout;
        settingsButton = new QPushButton(
            i18nc("@action:button", "Choose digiKam Settings File..."), q);
        folderButton = new QPushButton(
            i18nc("@action:button", "Choose Database Folder..."), q);
        detailsButton = new QPushButton(i18nc("@action:button", "Details..."), q);
        chooserLayout->addWidget(settingsButton);
        chooserLayout->addWidget(folderButton);
        chooserLayout->addStretch(1);
        chooserLayout->addWidget(detailsButton);
        layout->addLayout(chooserLayout);

        showOnStartup = new QCheckBox(
            i18nc("@option:check", "Show this import offer on startup"), q);
        showOnStartup->setChecked(startupOfferEnabled());
        showOnStartup->setVisible(startupOffer);
        layout->addWidget(showOnStartup);

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                         QDialogButtonBox::Cancel, q);
        importButton = buttonBox->button(QDialogButtonBox::Ok);
        importButton->setText(i18nc("@action:button", "Import"));
        layout->addWidget(buttonBox);

        QObject::connect(settingsButton, &QPushButton::clicked, q,
                         [this]() { chooseSettingsFile(); });
        QObject::connect(folderButton, &QPushButton::clicked, q,
                         [this]() { chooseDatabaseFolder(); });
        QObject::connect(detailsButton, &QPushButton::clicked, q,
                         [this]() { showDetails(); });
        QObject::connect(importButton, &QPushButton::clicked, q,
                         [this]() { importProfile(); });
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, q,
                         &PrivacyProfileImportDialog::reject);

        loadDefaultSource();
    }

    void loadDefaultSource()
    {
        setSource(PrivacyProfileInspector::inspectSettingsFile(
            PrivacyProfileInspector::defaultStockSettingsPath()));
    }

    void setSource(const PrivacyProfileSummary& newSource)
    {
        source = newSource;
        pathLabel->setText(source.databasePath.isEmpty()
                           ? i18nc("@info", "No usable default profile was found")
                           : QDir::toNativeSeparators(source.databasePath));
        versionLabel->setText(source.versionLabel.isEmpty()
                              ? i18nc("@info", "Unknown")
                              : source.versionLabel);
        countLabel->setText(QLocale().toString(source.activeItemCount));
        modifiedLabel->setText(source.latestModification.isValid()
                               ? QLocale().toString(source.latestModification,
                                                    QLocale::ShortFormat)
                               : i18nc("@info", "Unknown"));
        statusLabel->setText(source.isUsable()
                             ? i18nc("@info", "Ready to inspect")
                             : source.error);
        detailsButton->setEnabled(source.isUsable());
        updateImportAvailability();
    }

    void updateImportAvailability()
    {
        targetPaths = activeProfilePaths();
        target = PrivacyProfileInspector::inspectCoreDatabase(
            targetPaths.coreDatabasePath, targetPaths.configFilePath,
            targetPaths.thumbnailDatabasePath);
        QString unavailable;

        if (!source.isUsable())
        {
            unavailable = source.error;
        }
        else if (!targetPaths.isValid())
        {
            unavailable = i18nc("@info", "The isolated digiKam Private profile paths are unavailable.");
        }
        else if (QFileInfo(source.databasePath).canonicalFilePath() ==
                 QFileInfo(targetPaths.coreDatabasePath).canonicalFilePath())
        {
            unavailable = i18nc("@info", "The selected source is already the active profile.");
        }
        else if (source.isPrivateProfile() &&
                 (source.incompletePrivacyTransactionCount > 0))
        {
            unavailable = i18nc("@info", "This private profile has unfinished privacy operations.");
        }
        else if (source.isPrivateProfile() && (source.protectedItemCount > 0))
        {
            unavailable = i18nc(
                "@info",
                "Protected-store validation is required before this private profile can replace the active one.");
        }
        else if (!source.isPrivateProfile() && target.isPrivateProfile() &&
                 (target.activeItemCount > 0))
        {
            unavailable = i18nc(
                "@info",
                "This populated profile requires the additive stock-catalogue importer.");
        }

        importButton->setEnabled(unavailable.isEmpty());

        if (!unavailable.isEmpty() && source.isUsable())
        {
            statusLabel->setText(unavailable);
        }
    }

    void chooseSettingsFile()
    {
        const QString path = QFileDialog::getOpenFileName(
            q, i18nc("@title:window", "Choose digiKam Settings File"),
            QFileInfo(PrivacyProfileInspector::defaultStockSettingsPath()).absolutePath(),
            i18nc("@item:inlistbox", "digiKam settings (digikamrc);;All files (*)"));

        if (path.isEmpty())
        {
            loadDefaultSource();
            return;
        }

        const PrivacyProfileSummary selected =
            PrivacyProfileInspector::inspectSettingsFile(path);

        if (!selected.isUsable())
        {
            QMessageBox::warning(q, q->windowTitle(), selected.error);
            loadDefaultSource();
            return;
        }

        setSource(selected);
    }

    void chooseDatabaseFolder()
    {
        const QString path = QFileDialog::getExistingDirectory(
            q, i18nc("@title:window", "Choose digiKam Database Folder"),
            QFileInfo(PrivacyProfileInspector::defaultStockSettingsPath()).absolutePath());

        if (path.isEmpty())
        {
            loadDefaultSource();
            return;
        }

        const PrivacyProfileSummary selected =
            PrivacyProfileInspector::inspectDatabaseFolder(path);

        if (!selected.isUsable())
        {
            QMessageBox::warning(q, q->windowTitle(), selected.error);
            loadDefaultSource();
            return;
        }

        setSource(selected);
    }

    void showDetails()
    {
        QDialog details(q);
        details.setWindowTitle(i18nc("@title:window", "digiKam Profile Details"));
        details.resize(680, 480);
        auto* const layout = new QVBoxLayout(&details);
        auto* const text = new QPlainTextEdit(&details);
        text->setReadOnly(true);
        text->setPlainText(summaryDetails(source));
        layout->addWidget(text);
        auto* const buttons = new QDialogButtonBox(QDialogButtonBox::Close, &details);
        QObject::connect(buttons, &QDialogButtonBox::rejected,
                         &details, &QDialog::reject);
        layout->addWidget(buttons);
        details.exec();
    }

    bool confirmImport()
    {
        if (anotherDigikamProcessIsVisible())
        {
            const QMessageBox::StandardButton answer = QMessageBox::warning(
                q, q->windowTitle(),
                i18nc("@info",
                      "Another digiKam window appears to be running. Close it before importing "
                      "if it may be using the source profile. You may continue because the source "
                      "will be captured with SQLite's consistent online-backup operation."),
                QMessageBox::Cancel | QMessageBox::Ignore, QMessageBox::Cancel);

            if (answer != QMessageBox::Ignore)
            {
                return false;
            }
        }

        QString text;

        if (source.isPrivateProfile())
        {
            text = i18nc(
                "@info",
                "Replace the active profile with this digiKam Private profile?\n\n"
                "Source: %1 items, %2 privacy categories, %3 protected items.\n"
                "Current: %4 items, %5 privacy categories, %6 protected items.\n\n"
                "The complete current profile will be backed up first. Media protected only by "
                "the current profile will remain on disk but become dormant until Restore "
                "Previous Profile is used.",
                source.activeItemCount, source.privacyCategoryCount,
                source.protectedItemCount, target.activeItemCount,
                target.privacyCategoryCount, target.protectedItemCount);
        }
        else
        {
            text = i18nc(
                "@info",
                "Import this stock digiKam profile into the independent digiKam Private profile?\n\n"
                "Later catalogue and settings changes are not synchronized between the two "
                "applications. Both applications may still modify the same media and companion "
                "metadata files stored beside it, such as .xmp files.");
        }

        return (QMessageBox::question(q, q->windowTitle(), text,
                                      QMessageBox::Yes | QMessageBox::Cancel,
                                      QMessageBox::Cancel) == QMessageBox::Yes);
    }

    void importProfile()
    {
        updateImportAvailability();

        if (!importButton->isEnabled() || !confirmImport())
        {
            return;
        }

        if (!QDir().mkpath(targetPaths.transactionHome))
        {
            QMessageBox::critical(q, q->windowTitle(),
                                  i18nc("@info", "The profile-import workspace could not be created."));
            return;
        }

        const QString stagingPath = QDir(targetPaths.transactionHome).filePath(
            QLatin1String("staging-") +
            QUuid::createUuid().toString(QUuid::WithoutBraces));
        QProgressDialog progress(i18nc("@info:progress", "Preparing profile import..."),
                                 i18nc("@action:button", "Cancel"), 0, 0, q);
        progress.setWindowTitle(i18nc("@title:window", "Importing digiKam Profile"));
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(0);

        const auto report = [&progress](const QString& step, int current, int total)
        {
            progress.setLabelText(step);
            progress.setRange(0, qMax(total, 0));
            progress.setValue(qBound(0, current, qMax(total, 0)));
            QCoreApplication::processEvents();
        };
        const auto canceled = [&progress]() { return progress.wasCanceled(); };
        const PrivacyProfileImportStageResult staged =
            PrivacyProfileImportStager::stage(source, stagingPath, report, canceled);

        if (!staged.success)
        {
            QDir(stagingPath).removeRecursively();

            if (!staged.canceled)
            {
                QMessageBox::critical(q, q->windowTitle(), staged.error);
            }

            return;
        }

        const PrivacyProfilePublicationResult prepared =
            PrivacyProfilePublication::prepare(staged, targetPaths,
                                               source.settingsPath, report);
        QDir(stagingPath).removeRecursively();

        if (!prepared.success)
        {
            QMessageBox::critical(q, q->windowTitle(), prepared.error);
            return;
        }

        setStartupOfferEnabled(false);
        publicationIsPrepared = true;
        QString message = i18nc(
            "@info",
            "The source was captured and the current profile was backed up. "
            "digiKam Private must now close; the import will be applied safely before "
            "the profile opens on the next launch.");

        if (!staged.warnings.isEmpty())
        {
            message += QLatin1String("\n\n") + staged.warnings.join(QLatin1Char('\n'));
        }

        QMessageBox::information(q, q->windowTitle(), message);
        q->accept();
    }

public:

    PrivacyProfileImportDialog* q = nullptr;
    bool startupOffer = false;
    bool publicationIsPrepared = false;
    PrivacyProfileSummary source;
    PrivacyProfileSummary target;
    PrivacyProfilePaths targetPaths;
    QLabel* pathLabel = nullptr;
    QLabel* versionLabel = nullptr;
    QLabel* countLabel = nullptr;
    QLabel* modifiedLabel = nullptr;
    QLabel* statusLabel = nullptr;
    QPushButton* settingsButton = nullptr;
    QPushButton* folderButton = nullptr;
    QPushButton* detailsButton = nullptr;
    QPushButton* importButton = nullptr;
    QCheckBox* showOnStartup = nullptr;
    QDialogButtonBox* buttonBox = nullptr;
};

PrivacyProfileImportDialog::PrivacyProfileImportDialog(bool startupOffer,
                                                       QWidget* const parent)
    : QDialog(parent),
      d(new Private(this, startupOffer))
{
    d->buildUi();
}

PrivacyProfileImportDialog::~PrivacyProfileImportDialog()
{
    delete d;
}

PrivacyProfileImportOfferResult PrivacyProfileImportDialog::offerAtStartup(
    QWidget* const parent)
{
    if (!startupOfferEnabled())
    {
        return PrivacyProfileImportOfferResult::NotShown;
    }

    const PrivacyProfileSummary detected = PrivacyProfileInspector::inspectSettingsFile(
        PrivacyProfileInspector::defaultStockSettingsPath());

    if (!detected.isUsable())
    {
        return PrivacyProfileImportOfferResult::NotShown;
    }

    PrivacyProfileImportDialog dialog(true, parent);
    dialog.exec();

    return dialog.publicationPrepared()
         ? PrivacyProfileImportOfferResult::PublicationPrepared
         : PrivacyProfileImportOfferResult::Dismissed;
}

bool PrivacyProfileImportDialog::restorePreviousProfile(QWidget* const parent)
{
    const PrivacyProfilePaths paths = activeProfilePaths();

    if (!paths.isValid())
    {
        QMessageBox::critical(parent, i18nc("@title:window", "Restore Previous Profile"),
                              i18nc("@info", "The isolated profile paths are unavailable."));
        return false;
    }

    const QList<PrivacyProfileBackup> backups =
        PrivacyProfilePublication::restorableBackups(paths.transactionHome);

    if (backups.isEmpty())
    {
        QMessageBox::information(parent, i18nc("@title:window", "Restore Previous Profile"),
                                 i18nc("@info", "No verified previous profile backup is available."));
        return false;
    }

    QStringList labels;

    for (const PrivacyProfileBackup& backup : backups)
    {
        labels << i18ncp("@item:inlistbox", "%2 — %1 indexed item",
                        "%2 — %1 indexed items", backup.summary.activeItemCount,
                        QLocale().toString(backup.createdAt.toLocalTime(),
                                           QLocale::ShortFormat));
    }

    bool accepted = false;
    const QString selected = QInputDialog::getItem(
        parent, i18nc("@title:window", "Restore Previous Profile"),
        i18nc("@label", "Profile backup:"), labels, 0, false, &accepted);

    if (!accepted)
    {
        return false;
    }

    const int index = labels.indexOf(selected);

    if ((index < 0) ||
        (QMessageBox::warning(
             parent, i18nc("@title:window", "Restore Previous Profile"),
             i18nc("@info",
                   "Restore the selected profile backup? The active profile will be backed up "
                   "first, then digiKam Private will close and restore the selected profile on "
                   "its next launch. Media managed only by the active profile will remain on "
                   "disk but become dormant."),
             QMessageBox::Yes | QMessageBox::Cancel,
             QMessageBox::Cancel) != QMessageBox::Yes))
    {
        return false;
    }

    QProgressDialog progress(i18nc("@info:progress", "Preparing profile restoration..."),
                             QString(), 0, 0, parent);
    progress.setWindowTitle(i18nc("@title:window", "Restore Previous Profile"));
    progress.setWindowModality(Qt::WindowModal);
    progress.setCancelButton(nullptr);
    progress.setMinimumDuration(0);
    const auto report = [&progress](const QString& step, int current, int total)
    {
        progress.setLabelText(step);
        progress.setRange(0, qMax(total, 0));
        progress.setValue(qBound(0, current, qMax(total, 0)));
        QCoreApplication::processEvents();
    };
    const PrivacyProfilePublicationResult prepared =
        PrivacyProfilePublication::prepareRestore(backups.at(index), paths, report);

    if (!prepared.success)
    {
        QMessageBox::critical(parent, i18nc("@title:window", "Restore Previous Profile"),
                              prepared.error);
        return false;
    }

    QMessageBox::information(
        parent, i18nc("@title:window", "Restore Previous Profile"),
        i18nc("@info", "The active profile was backed up and restoration is ready. "
                        "digiKam Private must now close; the selected profile will be restored "
                        "before it opens on the next launch."));
    return true;
}

bool PrivacyProfileImportDialog::publicationPrepared() const
{
    return d->publicationIsPrepared;
}

void PrivacyProfileImportDialog::reject()
{
    if (d->startupOffer)
    {
        setStartupOfferEnabled(d->showOnStartup->isChecked());
    }

    QDialog::reject();
}

} // namespace Digikam
