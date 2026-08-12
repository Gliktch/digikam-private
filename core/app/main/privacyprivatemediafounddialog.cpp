/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprivatemediafounddialog.h"

// Qt includes

#include <QDialogButtonBox>
#include <QCoreApplication>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardPaths>
#include <QThread>
#include <QVBoxLayout>

// C++ includes

#include <utility>

// KDE includes

#include <KLocalizedString>

// Local includes

#include "collectionlocation.h"
#include "collectionmanager.h"
#include "privacyportableimport.h"
#include "privacyportableimportcoordinator.h"
#include "privacyprocessrunner.h"

Q_DECLARE_METATYPE(Digikam::PrivacyPortableDiscoveryGroup)
Q_DECLARE_METATYPE(Digikam::PrivacyPortableImportAuthenticationResult)

namespace Digikam
{

namespace
{

PrivacyGocryptfsToolPaths importToolPaths()
{
    PrivacyGocryptfsToolPaths paths;
    paths.gocryptfs =
        QStandardPaths::findExecutable(QLatin1String("gocryptfs"));
    paths.gocryptfsXray =
        QStandardPaths::findExecutable(QLatin1String("gocryptfs-xray"));
    paths.fusermount =
        QStandardPaths::findExecutable(QLatin1String("fusermount3"));
    return paths;
}

QString importWorkspaceRoot()
{
    const QString session = QStringLiteral("import-%1")
        .arg(QCoreApplication::applicationPid());
    return QDir(QStandardPaths::writableLocation(
                    QStandardPaths::RuntimeLocation))
        .filePath(QLatin1String("digikam-private/portable-import/") +
                  session);
}

QString truncatedLocationTail(const PrivacyPortableDiscoveryGroup& group)
{
    QString location;

    if (!group.casualArchives.isEmpty())
    {
        location = group.casualArchives.constFirst().absolutePath;
    }
    else if (!group.strongStores.isEmpty())
    {
        location = group.strongStores.constFirst().rootPath;
    }

    if (location.isEmpty())
    {
        return QString();
    }

    const QStringList parts = QDir::toNativeSeparators(location)
                                  .split(QLatin1Char('/'));
    const int keep = qMin(parts.size(), 3);
    return QLatin1String(".../") +
           parts.mid(parts.size() - keep).join(QLatin1Char('/'));
}

QString backendLabel(PrivacyBackend backend)
{
    return (backend == PrivacyBackend::Strong)
         ? i18nc("@info", "Strong")
         : i18nc("@info", "Casual");
}

QList<PrivacyPortableStrongStoreCandidate> allStoreCandidates(
    const PrivacyPortableDiscoveryResult& discovery)
{
    QList<PrivacyPortableStrongStoreCandidate> stores;

    for (const PrivacyPortableDiscoveryGroup& group : discovery.groups)
    {
        stores += group.strongStores;
    }

    return stores;
}

class PrivacyPortableImportVerifyWorker : public QObject
{
    Q_OBJECT

public:

    PrivacyPortableImportVerifyWorker(
        PrivacyPortableStoreInspector* inspector,
        QList<PrivacyPortableStrongStoreCandidate> stores)
        : m_inspector(inspector),
          m_stores(std::move(stores))
    {
    }

public Q_SLOTS:

    void verify(int row, const PrivacyPortableDiscoveryGroup& group,
                const QString& passwordText)
    {
        PrivacyPortableImportAuthenticationResult result;
        const PrivacyPassword password =
            PrivacyPassword::fromUnicode(passwordText);

        if (!password.isValid())
        {
            result.status =
                PrivacyPortableImportAuthenticationStatus::InvalidPassword;
            result.detail = i18nc("@info", "The password is invalid.");
        }
        else if (group.backend == PrivacyBackend::Casual)
        {
            result = PrivacyPortableImportAuthenticator::authenticateCasual(
                group, m_stores, password, *m_inspector);
        }
        else
        {
            result = PrivacyPortableImportAuthenticator::authenticateStrong(
                group, password, *m_inspector);
        }

        Q_EMIT verified(row, result);
    }

Q_SIGNALS:

    void verified(int row,
                  PrivacyPortableImportAuthenticationResult result);

private:

    PrivacyPortableStoreInspector* m_inspector;
    QList<PrivacyPortableStrongStoreCandidate> m_stores;
};

} // namespace

class Q_DECL_HIDDEN PrivacyPrivateMediaFoundDialog::Private
{
public:

    struct Row
    {
        PrivacyPortableDiscoveryGroup group;
        QLabel* state = nullptr;
        QLineEdit* password = nullptr;
        bool unlocked = false;
        bool busy = false;
        PrivacyPortableImportAuthenticationResult auth;
    };

    Private(PrivacyPrivateMediaFoundDialog* const dialog,
            const QString& root,
            const PrivacyPortableDiscoveryResult& discoveryResult,
            QList<PrivacyPortableStrongStoreCandidate> stores)
        : q(dialog),
          scanRoot(root),
          discovery(discoveryResult),
          inspector(processRunner, mountProbe, importToolPaths(),
                    importWorkspaceRoot()),
          worker(new PrivacyPortableImportVerifyWorker(
                     &inspector, std::move(stores))),
          coordinator(commitTarget)
    {
        qRegisterMetaType<PrivacyPortableDiscoveryGroup>();
        qRegisterMetaType<PrivacyPortableImportAuthenticationResult>();
    }

    ~Private()
    {
        if (worker)
        {
            delete worker;
            worker = nullptr;
        }
    }

    void buildUi()
    {
        q->setWindowTitle(i18nc("@title:window", "Private Media Found"));
        q->setMinimumWidth(680);
        q->resize(720, 460);
        auto* const layout = new QVBoxLayout(q);

        auto* const introduction = new QLabel(
            i18nc("@info",
                  "The folders you selected contain media protected by one or "
                  "more privacy categories. Enter the password for each "
                  "category now so digiKam Private can import it correctly. "
                  "Until a category is resolved, its private media will not be "
                  "scanned or indexed as ordinary media."), q);
        introduction->setWordWrap(true);
        layout->addWidget(introduction);

        auto* const scroll = new QScrollArea(q);
        scroll->setWidgetResizable(true);
        auto* const rowsWidget = new QWidget(scroll);
        rowsLayout = new QVBoxLayout(rowsWidget);
        rowsLayout->addStretch(1);
        scroll->setWidget(rowsWidget);
        layout->addWidget(scroll, 1);

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                         QDialogButtonBox::Cancel, q);
        importButton = buttonBox->button(QDialogButtonBox::Ok);
        importButton->setText(
            i18nc("@action:button", "Continue Import"));
        importButton->setEnabled(false);
        layout->addWidget(buttonBox);

        QObject::connect(importButton, &QPushButton::clicked,
                         q, [this]() { continueImport(); });
        QObject::connect(buttonBox, &QDialogButtonBox::rejected,
                         q, &QDialog::reject);

        addRows();
    }

    void addRows()
    {
        for (const PrivacyPortableDiscoveryGroup& group :
             discovery.groups)
        {
            Row row;
            row.group = group;
            auto* const groupBox = new QGroupBox(q);
            auto* const rowLayout = new QHBoxLayout(groupBox);
            auto* const identity = new QLabel(
                i18nc("@label", "Unknown — %1 (%2)",
                      truncatedLocationTail(group),
                      backendLabel(group.backend)), groupBox);
            identity->setWordWrap(true);
            rowLayout->addWidget(identity, 1);
            row.password = new QLineEdit(groupBox);
            row.password->setEchoMode(QLineEdit::Password);
            row.password->setPlaceholderText(
                i18nc("@info:placeholder", "Category password"));
            rowLayout->addWidget(row.password);
            row.state = new QLabel(
                i18nc("@info", "Locked"), groupBox);
            row.state->setToolTip(
                i18nc("@info:tooltip",
                      "Locked: the category password has not been verified yet."));
            rowLayout->addWidget(row.state);
            rowsLayout->insertWidget(rowsLayout->count() - 1, groupBox);

            QObject::connect(row.password, &QLineEdit::editingFinished,
                             q, [this, rowIndex = rows.size()]()
            {
                verifyRow(rowIndex);
            });
            QObject::connect(row.password, &QLineEdit::returnPressed,
                             q, [this, rowIndex = rows.size()]()
            {
                verifyRow(rowIndex, true);
            });
            rows << row;
        }
    }

    void verifyRow(int rowIndex, bool explicitEnter = false)
    {
        if ((rowIndex < 0) || (rowIndex >= rows.size()) ||
            rows.at(rowIndex).busy)
        {
            return;
        }

        Row& row = rows[rowIndex];
        const QString passwordText = row.password->text();

        if (passwordText.isEmpty())
        {
            return;
        }

        row.busy = true;
        row.password->setEnabled(false);
        row.state->setText(i18nc("@info", "Checking..."));
        Q_UNUSED(explicitEnter);
        QMetaObject::invokeMethod(
            worker, "verify", Qt::QueuedConnection,
            Q_ARG(int, rowIndex),
            Q_ARG(PrivacyPortableDiscoveryGroup, row.group),
            Q_ARG(QString, passwordText));
    }

    void onVerified(int rowIndex,
                    const PrivacyPortableImportAuthenticationResult& result)
    {
        if ((rowIndex < 0) || (rowIndex >= rows.size()))
        {
            return;
        }

        Row& row = rows[rowIndex];
        row.busy = false;
        row.password->setEnabled(true);

        if (result.succeeded())
        {
            row.unlocked = true;
            row.auth = result;
            row.password->clear();
            row.state->setText(i18nc("@info", "Unlocked"));
            row.state->setToolTip(
                i18nc("@info:tooltip",
                      "Unlocked: the category password was verified."));
        }
        else
        {
            row.password->clear();
            row.state->setText(i18nc("@info", "Locked"));
            row.state->setToolTip(
                result.detail.isEmpty()
                    ? i18nc("@info:tooltip",
                            "Locked: verification failed.")
                    : result.detail);
        }

        bool anyUnlocked = false;

        for (const Row& candidate : std::as_const(rows))
        {
            if (candidate.unlocked)
            {
                anyUnlocked = true;
                break;
            }
        }

        importButton->setEnabled(anyUnlocked);
    }

    void continueImport()
    {
        int imported = 0;
        int failed = 0;

        for (const Row& row : std::as_const(rows))
        {
            if (!row.unlocked)
            {
                continue;
            }

            const PrivacyPortableImportGroupResult result =
                coordinator.commit(row.auth.candidate, albumRootIds,
                                   i18nc("@info", "Imported private media"));

            if (result.committed)
            {
                ++imported;
            }
            else
            {
                ++failed;
            }
        }

        QString message = i18ncp(
            "@info", "%1 privacy category imported",
            "%1 privacy categories imported", imported);

        if (failed > 0)
        {
            message += QLatin1Char('\n') +
                       i18ncp("@info",
                              "%1 category could not be imported",
                              "%1 categories could not be imported",
                              failed);
        }

        QMessageBox::information(q, q->windowTitle(), message);
        q->accept();
    }

    PrivacyPrivateMediaFoundDialog* q = nullptr;
    QString scanRoot;
    PrivacyPortableDiscoveryResult discovery;
    QHash<QString, int> albumRootIds;
    QList<Row> rows;
    QVBoxLayout* rowsLayout = nullptr;
    QDialogButtonBox* buttonBox = nullptr;
    QPushButton* importButton = nullptr;

    QProcessPrivacyProcessRunner processRunner;
    ProcMountInfoPrivacyMountStateProbe mountProbe;
    PrivacyGocryptfsPortableStoreInspector inspector;
    PrivacyPortableImportVerifyWorker* worker = nullptr;
    PrivacyCoreDbPortableImportCommitTarget commitTarget;
    PrivacyPortableImportCoordinator coordinator;
    QThread workerThread;
};

PrivacyPrivateMediaFoundDialog::PrivacyPrivateMediaFoundDialog(
    const QString& scanRoot,
    const PrivacyPortableDiscoveryResult& discovery,
    QWidget* const parent)
    : QDialog(parent),
      d(new Private(this, scanRoot, discovery, allStoreCandidates(discovery)))
{
    if (CollectionManager::instance())
    {
        const CollectionLocation location =
            CollectionManager::instance()->locationForPath(scanRoot);

        if (location.status() != CollectionLocation::LocationNull)
        {
            d->albumRootIds.insert(scanRoot, location.id());
        }

        for (const PrivacyPortableDiscoveryGroup& group :
             d->discovery.groups)
        {
            for (const PrivacyPortableCasualArchiveCandidate& archive :
                 group.casualArchives)
            {
                if (!d->albumRootIds.contains(archive.rootPath))
                {
                    const CollectionLocation archiveLocation =
                        CollectionManager::instance()->locationForPath(
                            archive.rootPath);

                    if (archiveLocation.status() !=
                        CollectionLocation::LocationNull)
                    {
                        d->albumRootIds.insert(
                            archive.rootPath, archiveLocation.id());
                    }
                }
            }
        }
    }

    d->worker->moveToThread(&d->workerThread);
    QObject::connect(
        d->worker, &PrivacyPortableImportVerifyWorker::verified,
        this, [this](int row,
                     const PrivacyPortableImportAuthenticationResult& result)
        {
            d->onVerified(row, result);
        });
    d->workerThread.start();
    d->buildUi();
}

PrivacyPrivateMediaFoundDialog::~PrivacyPrivateMediaFoundDialog()
{
    d->workerThread.quit();
    d->workerThread.wait(5000);
    delete d;
}

bool PrivacyPrivateMediaFoundDialog::offer(const QString& scanRoot,
                                           QWidget* const parent)
{
    const PrivacyPortableDiscoveryResult discovery =
        PrivacyPortableDiscovery::scan({ scanRoot });

    if (discovery.groups.isEmpty())
    {
        QMessageBox::information(
            parent, i18nc("@title:window", "Private Media Found"),
            i18nc("@info",
                  "No private media were found in the selected folder."));
        return false;
    }

    PrivacyPrivateMediaFoundDialog dialog(scanRoot, discovery, parent);
    dialog.exec();
    return (dialog.result() == QDialog::Accepted);
}

} // namespace Digikam

#include "privacyprivatemediafounddialog.moc"
