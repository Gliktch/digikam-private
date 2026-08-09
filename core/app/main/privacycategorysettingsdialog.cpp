/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycategorysettingsdialog.h"

// Qt includes

#include <QAbstractItemView>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QFuture>
#include <QFutureWatcher>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QSharedPointer>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QtConcurrentRun>
#include <QUuid>

// C++ includes

#include <algorithm>
#include <utility>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "privacycategorysessionowner.h"
#include "privacymanagedrootprovisioner.h"
#include "privacypassword.h"
#include "privacyrepository.h"
#include "privacyruntime.h"

namespace Digikam
{

namespace
{

QString uuidText()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void wipe(QString& value)
{
    value.detach();
    value.fill(QChar::Null);
    value.clear();
}

void wipe(QLineEdit* const edit)
{
    if (!edit)
    {
        return;
    }

    edit->setText(QString(edit->text().size(), QChar::Null));
    edit->clear();
}

QString presentationText(PrivacyPresentationMode mode)
{
    return (mode == PrivacyPresentationMode::Blur)
         ? i18nc("@item:inlistbox private category presentation", "Blurred proxy")
         : i18nc("@item:inlistbox private category presentation", "Generic privacy tile");
}

QString passwordErrorText(PrivacyPasswordError error)
{
    switch (error)
    {
        case PrivacyPasswordError::Empty:
            return i18nc("@info", "Enter a category password.");

        case PrivacyPasswordError::ContainsNul:
        case PrivacyPasswordError::ContainsCarriageReturn:
        case PrivacyPasswordError::ContainsLineFeed:
            return i18nc("@info", "The password contains an unsupported control character.");

        case PrivacyPasswordError::TooLong:
            return i18nc("@info", "The encoded password is too long.");

        case PrivacyPasswordError::None:
            break;
    }

    return i18nc("@info", "The password is invalid.");
}

QString sessionFailureText(const PrivacyCategorySessionResult& result)
{
    switch (result.status)
    {
        case PrivacyCategorySessionStatus::InvalidPassword:
        case PrivacyCategorySessionStatus::AuthenticationFailed:
            return i18nc("@info", "The category password was not accepted.");

        case PrivacyCategorySessionStatus::StoreOffline:
            return i18nc("@info", "The category store is currently offline.");

        case PrivacyCategorySessionStatus::StoreIdentityMismatch:
            return i18nc("@info", "The category store identity could not be verified.");

        case PrivacyCategorySessionStatus::CategoryNotActive:
            return i18nc("@info", "The category is not active.");

        case PrivacyCategorySessionStatus::TransactionBlocked:
            return i18nc("@info", "Another private-category operation is still active.");

        case PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired:
            return i18nc("@info",
                         "The operation was interrupted after durable work began. "
                         "Its exact state has been retained; select the category "
                         "and resume creation.");

        case PrivacyCategorySessionStatus::StoreFailure:
            return i18nc("@info", "The encrypted category store could not be prepared.");

        case PrivacyCategorySessionStatus::LockFailed:
            return i18nc("@info", "The category store could not be safely locked.");

        case PrivacyCategorySessionStatus::Conflict:
            return i18nc("@info", "The category conflicts with existing durable state.");

        case PrivacyCategorySessionStatus::StrongRecoveryRequired:
            return i18nc("@info", "Strong privacy is unavailable until recovery-key export is implemented.");

        default:
            break;
    }

    return i18nc("@info", "The private-category operation could not be completed.");
}

QString provisionFailureText(const PrivacyManagedRootProvisionResult& result)
{
    switch (result.status)
    {
        case PrivacyManagedRootProvisionStatus::PathUnavailable:
            return i18nc("@info", "The selected store folder is unavailable.");

        case PrivacyManagedRootProvisionStatus::UnsafeRoot:
            return i18nc("@info",
                         "The selected folder is not a safe managed root. It must "
                         "be owned by this user, must not be group- or world-writable, "
                         "and must not contain symbolic-link path components.");

        case PrivacyManagedRootProvisionStatus::InvalidMarker:
            return i18nc("@info",
                         "The folder contains a missing, unsafe, or conflicting "
                         "digiKam Private root marker.");

        case PrivacyManagedRootProvisionStatus::IoFailure:
            return i18nc("@info", "The managed-root marker could not be written durably.");

        default:
            break;
    }

    return i18nc("@info", "The selected folder cannot be used as a category store root.");
}

} // namespace

class PrivacyCategorySettingsDialog::Private
{
public:

    explicit Private(PrivacyCategorySettingsDialog* const dialog)
        : q(dialog),
          sessions(PrivacyStartupRecovery::categorySessions()),
          runtime(PrivacyStartupRecovery::coordinator())
    {
    }

    void setupUi()
    {
        q->setWindowTitle(i18nc("@title:window", "Privacy Categories"));
        q->resize(880, 430);

        auto* const layout = new QVBoxLayout(q);
        auto* const introduction = new QLabel(
            i18nc("@info",
                  "Privacy categories are independently password-gated. digiKam "
                  "always starts with every category locked. Casual Privacy is "
                  "intentionally recoverable and is not secure encryption."), q);
        introduction->setWordWrap(true);
        layout->addWidget(introduction);

        table = new QTableWidget(q);
        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({
            i18nc("@title:column", "Category"),
            i18nc("@title:column", "Presentation"),
            i18nc("@title:column", "Items"),
            i18nc("@title:column", "Session"),
            i18nc("@title:column", "Store Folder")
        });
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->verticalHeader()->setVisible(false);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
        layout->addWidget(table, 1);

        auto* const actionLayout = new QHBoxLayout;
        createButton = new QPushButton(
            QIcon::fromTheme(QLatin1String("list-add")),
            i18nc("@action:button", "Create Category..."), q);
        sessionButton = new QPushButton(q);
        lockAllButton = new QPushButton(
            QIcon::fromTheme(QLatin1String("object-locked")),
            i18nc("@action:button", "Lock All"), q);
        actionLayout->addWidget(createButton);
        actionLayout->addWidget(sessionButton);
        actionLayout->addWidget(lockAllButton);
        actionLayout->addStretch(1);
        layout->addLayout(actionLayout);

        statusLabel = new QLabel(q);
        statusLabel->setWordWrap(true);
        layout->addWidget(statusLabel);

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, q);
        layout->addWidget(buttonBox);

        QObject::connect(buttonBox, &QDialogButtonBox::rejected,
                         q, &QDialog::reject);
        QObject::connect(createButton, &QPushButton::clicked,
                         q, [this]() { createCategory(); });
        QObject::connect(sessionButton, &QPushButton::clicked,
                         q, [this]() { runSelectedCategoryAction(); });
        QObject::connect(lockAllButton, &QPushButton::clicked,
                         q, [this]() { lockAll(); });
        QObject::connect(table, &QTableWidget::itemSelectionChanged,
                         q, [this]() { updateActions(); });

        reload();
    }

    const PrivacyCategory* selectedCategory() const
    {
        const int row = table->currentRow();

        if ((row < 0) || (row >= categories.size()))
        {
            return nullptr;
        }

        return &categories.at(row);
    }

    void reload()
    {
        QString selectedUuid;

        if (const PrivacyCategory* const selected = selectedCategory())
        {
            selectedUuid = selected->uuid;
        }

        PrivacyRepositorySnapshot loaded;

        if (!PrivacyRepository().loadSnapshot(&loaded))
        {
            snapshot = PrivacyRepositorySnapshot();
            categories.clear();
            table->setRowCount(0);
            statusLabel->setText(i18nc("@info", "Private-category state could not be loaded."));
            updateActions();
            return;
        }

        snapshot = loaded;
        categories = snapshot.categories;
        std::sort(categories.begin(), categories.end(),
                  [](const PrivacyCategory& left, const PrivacyCategory& right)
                  {
                      return (QString::localeAwareCompare(left.name, right.name) < 0);
                  });

        QHash<QString, int> itemCounts;

        for (const PrivacyItem& item : std::as_const(snapshot.items))
        {
            ++itemCounts[item.categoryUuid];
        }

        QHash<QString, PrivacyStore> storesByCategory;

        for (const PrivacyStore& store : std::as_const(snapshot.stores))
        {
            storesByCategory.insert(store.categoryUuid, store);
        }

        QHash<QString, PrivacyStorageRoot> rootsByUuid;

        for (const PrivacyStorageRoot& root : std::as_const(snapshot.storageRoots))
        {
            rootsByUuid.insert(root.uuid, root);
        }

        table->setRowCount(categories.size());
        int selectedRow = -1;

        for (int row = 0 ; row < categories.size() ; ++row)
        {
            const PrivacyCategory& category = categories.at(row);
            const PrivacyStore store = storesByCategory.value(category.uuid);
            const PrivacyStorageRoot root = rootsByUuid.value(store.rootUuid);
            QString sessionText;

            if (category.lifecycleState == PrivacyCategoryLifecycleState::Creating)
            {
                sessionText = i18nc("@item:inlistbox", "Creation interrupted");
            }
            else if (category.lifecycleState != PrivacyCategoryLifecycleState::Active)
            {
                sessionText = i18nc("@item:inlistbox", "Attention required");
            }
            else
            {
                sessionText = (sessions && sessions->ownsSecret(category.uuid))
                            ? i18nc("@item:inlistbox", "Unlocked")
                            : i18nc("@item:inlistbox", "Locked");

                if (runtime && root.isValid())
                {
                    const PrivacyRootRuntimeState rootState = runtime->rootState(root.uuid);

                    if (rootState == PrivacyRootRuntimeState::Offline)
                    {
                        sessionText += i18nc("@item:inlistbox", " - store offline");
                    }
                    else if (rootState == PrivacyRootRuntimeState::IdentityMismatch)
                    {
                        sessionText += i18nc("@item:inlistbox", " - identity mismatch");
                    }
                }
            }

            auto* const nameItem = new QTableWidgetItem(category.name);
            nameItem->setData(Qt::UserRole, category.uuid);
            table->setItem(row, 0, nameItem);
            table->setItem(row, 1, new QTableWidgetItem(
                               presentationText(category.presentationMode)));
            table->setItem(row, 2, new QTableWidgetItem(
                               QString::number(itemCounts.value(category.uuid))));
            table->setItem(row, 3, new QTableWidgetItem(sessionText));
            auto* const pathItem = new QTableWidgetItem(root.configuredPath);
            pathItem->setToolTip(root.configuredPath);
            table->setItem(row, 4, pathItem);

            if (category.uuid == selectedUuid)
            {
                selectedRow = row;
            }
        }

        if ((selectedRow < 0) && !categories.isEmpty())
        {
            selectedRow = 0;
        }

        if (selectedRow >= 0)
        {
            table->selectRow(selectedRow);
        }

        if (categories.isEmpty())
        {
            statusLabel->setText(
                i18nc("@info", "No privacy categories have been created yet."));
        }

        updateActions();
    }

    bool exactCreateRequest(const PrivacyCategory& category,
                            PrivacyCategoryCreateRequest* const request) const
    {
        if (!request ||
            (category.lifecycleState != PrivacyCategoryLifecycleState::Creating))
        {
            return false;
        }

        QList<PrivacyStore> matchingStores;
        QList<PrivacyTransaction> matchingTransactions;

        for (const PrivacyStore& store : std::as_const(snapshot.stores))
        {
            if (store.categoryUuid == category.uuid)
            {
                matchingStores.append(store);
            }
        }

        for (const PrivacyTransaction& transaction : std::as_const(snapshot.transactions))
        {
            if ((transaction.categoryUuid == category.uuid) &&
                (transaction.type == PrivacyTransactionType::CreateCategory) &&
                (transaction.state == PrivacyTransactionState::Created))
            {
                matchingTransactions.append(transaction);
            }
        }

        if ((matchingStores.size() != 1) || (matchingTransactions.size() != 1))
        {
            return false;
        }

        PrivacyStorageRoot root;
        int rootMatches = 0;

        for (const PrivacyStorageRoot& candidate : std::as_const(snapshot.storageRoots))
        {
            if (candidate.uuid == matchingStores.constFirst().rootUuid)
            {
                root = candidate;
                ++rootMatches;
            }
        }

        if ((rootMatches != 1) || !root.isValid())
        {
            return false;
        }

        request->categoryUuid = category.uuid;
        request->storeUuid = matchingStores.constFirst().uuid;
        request->transactionUuid = matchingTransactions.constFirst().uuid;
        request->name = category.name;
        request->backend = category.backend;
        request->presentationMode = category.presentationMode;
        request->unlockedThumbnailMode = category.unlockedThumbnailMode;
        request->tagVisibilityMode = category.tagVisibilityMode;
        request->storageRoot = root;

        return true;
    }

    void updateActions()
    {
        const PrivacyCategory* const category = selectedCategory();
        createButton->setEnabled(!busy && sessions);
        lockAllButton->setEnabled(!busy && sessions && !categories.isEmpty());
        sessionButton->setEnabled(false);
        sessionButton->setIcon(QIcon());

        if (busy || !sessions || !category)
        {
            sessionButton->setText(i18nc("@action:button", "Unlock"));
            return;
        }

        if (category->lifecycleState == PrivacyCategoryLifecycleState::Creating)
        {
            PrivacyCategoryCreateRequest request;
            sessionButton->setText(i18nc("@action:button", "Resume Creation..."));
            sessionButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
            sessionButton->setEnabled(exactCreateRequest(*category, &request));
            return;
        }

        if (category->lifecycleState != PrivacyCategoryLifecycleState::Active)
        {
            sessionButton->setText(i18nc("@action:button", "Unavailable"));
            return;
        }

        const bool unlocked = sessions->ownsSecret(category->uuid);
        sessionButton->setText(unlocked
            ? i18nc("@action:button", "Lock Now")
            : i18nc("@action:button", "Unlock..."));
        sessionButton->setIcon(QIcon::fromTheme(
            unlocked ? QLatin1String("object-locked")
                     : QLatin1String("object-unlocked")));
        sessionButton->setEnabled(true);
    }

    void setBusy(bool value)
    {
        busy = value;
        table->setEnabled(!busy);
        buttonBox->button(QDialogButtonBox::Close)->setEnabled(!busy);
        updateActions();
    }

    void watchOperation(const QFuture<PrivacyCategorySessionResult>& future,
                        const QString& title,
                        const QString& progressText,
                        const QString& successText)
    {
        setBusy(true);
        auto* const progress = new QProgressDialog(progressText, QString(), 0, 0, q);
        progress->setWindowTitle(title);
        progress->setCancelButton(nullptr);
        progress->setWindowModality(Qt::WindowModal);
        progress->setMinimumDuration(0);
        progress->show();

        auto* const watcher = new QFutureWatcher<PrivacyCategorySessionResult>(q);
        QObject::connect(watcher, &QFutureWatcher<PrivacyCategorySessionResult>::finished,
                         q, [this, watcher, progress, title, successText]()
        {
            const PrivacyCategorySessionResult result = watcher->result();
            watcher->deleteLater();
            progress->deleteLater();
            setBusy(false);
            reload();

            if (result.succeeded())
            {
                statusLabel->setText(successText);
            }
            else
            {
                QMessageBox::warning(q, title, sessionFailureText(result));
            }
        });
        watcher->setFuture(future);
    }

    void createCategory()
    {
        if (!sessions || busy)
        {
            return;
        }

        QDialog dialog(q);
        dialog.setWindowTitle(i18nc("@title:window", "Create Privacy Category"));
        auto* const layout = new QVBoxLayout(&dialog);
        auto* const explanation = new QLabel(
            i18nc("@info",
                  "Casual Privacy prevents ordinary viewing but deliberately uses "
                  "a recoverable legacy ZIP format for protected originals. Choose "
                  "an existing persistent folder for this category's encrypted "
                  "credential and derivative store. Removable or network storage "
                  "may leave the category unavailable while disconnected."), &dialog);
        explanation->setWordWrap(true);
        layout->addWidget(explanation);

        auto* const form = new QFormLayout;
        auto* const nameEdit = new QLineEdit(&dialog);
        nameEdit->setMaxLength(128);
        nameEdit->setPlaceholderText(i18nc("@info:placeholder", "Private"));
        auto* const rootEdit = new QLineEdit(&dialog);
        auto* const browseButton = new QPushButton(
            i18nc("@action:button", "Browse..."), &dialog);
        auto* const rootRow = new QWidget(&dialog);
        auto* const rootLayout = new QHBoxLayout(rootRow);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->addWidget(rootEdit, 1);
        rootLayout->addWidget(browseButton);
        auto* const presentation = new QComboBox(&dialog);
        presentation->addItem(presentationText(PrivacyPresentationMode::Generic),
                              static_cast<int>(PrivacyPresentationMode::Generic));
        presentation->addItem(presentationText(PrivacyPresentationMode::Blur),
                              static_cast<int>(PrivacyPresentationMode::Blur));
        auto* const passwordEdit = new QLineEdit(&dialog);
        auto* const confirmationEdit = new QLineEdit(&dialog);
        passwordEdit->setEchoMode(QLineEdit::Password);
        confirmationEdit->setEchoMode(QLineEdit::Password);
        form->addRow(i18nc("@label", "Name:"), nameEdit);
        form->addRow(i18nc("@label", "Store folder:"), rootRow);
        form->addRow(i18nc("@label", "Locked presentation:"), presentation);
        form->addRow(i18nc("@label", "Password:"), passwordEdit);
        form->addRow(i18nc("@label", "Confirm password:"), confirmationEdit);
        layout->addLayout(form);

        auto* const buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        buttons->button(QDialogButtonBox::Ok)->setText(
            i18nc("@action:button", "Create"));
        layout->addWidget(buttons);

        QObject::connect(browseButton, &QPushButton::clicked, &dialog,
                         [rootEdit, &dialog]()
        {
            const QString current = rootEdit->text().trimmed();
            const QString initial = !current.isEmpty() && QDir(current).exists()
                                  ? current : QDir::rootPath();
            const QString selected = QFileDialog::getExistingDirectory(
                &dialog, i18nc("@title:window", "Choose Privacy Store Folder"),
                initial, QFileDialog::ShowDirsOnly);

            if (!selected.isEmpty())
            {
                rootEdit->setText(selected);
            }
        });
        QObject::connect(buttons, &QDialogButtonBox::rejected,
                         &dialog, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog,
                         [this, &dialog, nameEdit, rootEdit,
                          passwordEdit, confirmationEdit]()
        {
            const QString name = nameEdit->text().trimmed();

            if (name.isEmpty() || rootEdit->text().trimmed().isEmpty())
            {
                QMessageBox::warning(&dialog,
                    i18nc("@title:window", "Incomplete Category"),
                    i18nc("@info", "Enter a category name and choose a store folder."));
                return;
            }

            for (const PrivacyCategory& category : std::as_const(categories))
            {
                if (category.name.compare(name, Qt::CaseInsensitive) == 0)
                {
                    QMessageBox::warning(&dialog,
                        i18nc("@title:window", "Category Already Exists"),
                        i18nc("@info", "Choose a different category name."));
                    return;
                }
            }

            PrivacyPasswordError passwordError = PrivacyPasswordError::None;
            PrivacyPassword probe = PrivacyPassword::fromUnicode(
                passwordEdit->text(), &passwordError);

            if (!probe.isValid())
            {
                QMessageBox::warning(&dialog,
                    i18nc("@title:window", "Invalid Password"),
                    passwordErrorText(passwordError));
                return;
            }

            QString normalizedPassword = passwordEdit->text().normalized(
                QString::NormalizationForm_C);
            QString normalizedConfirmation = confirmationEdit->text().normalized(
                QString::NormalizationForm_C);
            const bool matches = (normalizedPassword == normalizedConfirmation);
            wipe(normalizedPassword);
            wipe(normalizedConfirmation);

            if (!matches)
            {
                QMessageBox::warning(&dialog,
                    i18nc("@title:window", "Passwords Do Not Match"),
                    i18nc("@info", "Enter the same category password twice."));
                return;
            }

            dialog.accept();
        });

        if (dialog.exec() != QDialog::Accepted)
        {
            wipe(passwordEdit);
            wipe(confirmationEdit);
            return;
        }

        const QString categoryName = nameEdit->text().trimmed();
        const QString rootPath = rootEdit->text();
        const PrivacyPresentationMode presentationMode =
            static_cast<PrivacyPresentationMode>(presentation->currentData().toInt());
        QString password = passwordEdit->text();
        wipe(passwordEdit);
        wipe(confirmationEdit);

        const PrivacyManagedRootProvisionResult provisioned =
            PrivacyManagedRootProvisioner::provision(rootPath);

        if (!provisioned.succeeded())
        {
            wipe(password);
            QString message = provisionFailureText(provisioned);

            if (!provisioned.detail.isEmpty())
            {
                message += QLatin1String("\n\n") + provisioned.detail;
            }

            QMessageBox::warning(q,
                i18nc("@title:window", "Store Folder Rejected"), message);
            return;
        }

        PrivacyCategoryCreateRequest request;
        request.categoryUuid = uuidText();
        request.storeUuid = uuidText();
        request.transactionUuid = uuidText();
        request.name = categoryName;
        request.backend = PrivacyBackend::Casual;
        request.presentationMode = presentationMode;
        request.unlockedThumbnailMode = PrivacyUnlockedThumbnailMode::FocusedClear;
        request.tagVisibilityMode = PrivacyTagVisibilityMode::UnlockedOnly;
        request.storageRoot = provisioned.root;

        const QSharedPointer<QString> secret =
            QSharedPointer<QString>::create(std::move(password));
        const QSharedPointer<PrivacyCategorySessionOwner> owner = sessions;
        const QFuture<PrivacyCategorySessionResult> future = QtConcurrent::run(
            [owner, request, secret]()
            {
                PrivacyCategorySessionResult result;

                try
                {
                    result = owner->createCategory(request, *secret);
                }
                catch (...)
                {
                    result.status = PrivacyCategorySessionStatus::TransactionBlocked;
                }

                wipe(*secret);
                return result;
            });
        watchOperation(future,
                       i18nc("@title:window", "Creating Privacy Category"),
                       i18nc("@info:progress", "Creating the encrypted category store..."),
                       i18nc("@info", "The privacy category was created and unlocked for this session."));
    }

    void runSelectedCategoryAction()
    {
        const PrivacyCategory* const selected = selectedCategory();

        if (!sessions || busy || !selected)
        {
            return;
        }

        const PrivacyCategory category = *selected;

        if (category.lifecycleState == PrivacyCategoryLifecycleState::Creating)
        {
            PrivacyCategoryCreateRequest request;

            if (!exactCreateRequest(category, &request))
            {
                return;
            }

            bool accepted = false;
            QString password = QInputDialog::getText(
                q, i18nc("@title:window", "Resume Category Creation"),
                i18nc("@label", "Password for %1:", category.name),
                QLineEdit::Password, QString(), &accepted);

            if (!accepted)
            {
                wipe(password);
                return;
            }

            const QSharedPointer<QString> secret =
                QSharedPointer<QString>::create(std::move(password));
            const QSharedPointer<PrivacyCategorySessionOwner> owner = sessions;
            const QFuture<PrivacyCategorySessionResult> future = QtConcurrent::run(
                [owner, request, secret]()
                {
                    PrivacyCategorySessionResult result;

                    try
                    {
                        result = owner->createCategory(request, *secret);
                    }
                    catch (...)
                    {
                        result.status = PrivacyCategorySessionStatus::TransactionBlocked;
                    }

                    wipe(*secret);
                    return result;
                });
            watchOperation(future,
                           i18nc("@title:window", "Resuming Category Creation"),
                           i18nc("@info:progress", "Verifying and resuming the category store..."),
                           i18nc("@info", "Category creation completed and the category is unlocked."));
            return;
        }

        if (category.lifecycleState != PrivacyCategoryLifecycleState::Active)
        {
            return;
        }

        const QSharedPointer<PrivacyCategorySessionOwner> owner = sessions;

        if (sessions->ownsSecret(category.uuid))
        {
            const QFuture<PrivacyCategorySessionResult> future = QtConcurrent::run(
                [owner, category]()
                {
                    try
                    {
                        return owner->lockCategory(category.uuid);
                    }
                    catch (...)
                    {
                        return PrivacyCategorySessionResult {
                            PrivacyCategorySessionStatus::TransactionBlocked
                        };
                    }
                });
            watchOperation(future,
                           i18nc("@title:window", "Locking Privacy Category"),
                           i18nc("@info:progress", "Locking the category store..."),
                           i18nc("@info", "The category is now locked."));
            return;
        }

        bool accepted = false;
        QString password = QInputDialog::getText(
            q, i18nc("@title:window", "Unlock Privacy Category"),
            i18nc("@label", "Password for %1:", category.name),
            QLineEdit::Password, QString(), &accepted);

        if (!accepted)
        {
            wipe(password);
            return;
        }

        const QSharedPointer<QString> secret =
            QSharedPointer<QString>::create(std::move(password));
        const QFuture<PrivacyCategorySessionResult> future = QtConcurrent::run(
            [owner, category, secret]()
            {
                PrivacyCategorySessionResult result;

                try
                {
                    result = owner->unlockCategory(category.uuid, *secret);
                }
                catch (...)
                {
                    result.status = PrivacyCategorySessionStatus::TransactionBlocked;
                }

                wipe(*secret);
                return result;
            });
        watchOperation(future,
                       i18nc("@title:window", "Unlocking Privacy Category"),
                       i18nc("@info:progress", "Verifying and unlocking the category..."),
                       i18nc("@info", "The category is unlocked for this digiKam session."));
    }

    void lockAll()
    {
        if (!sessions || busy)
        {
            return;
        }

        setBusy(true);
        auto* const progress = new QProgressDialog(
            i18nc("@info:progress", "Locking all privacy categories..."),
            QString(), 0, 0, q);
        progress->setWindowTitle(i18nc("@title:window", "Locking Privacy Categories"));
        progress->setCancelButton(nullptr);
        progress->setWindowModality(Qt::WindowModal);
        progress->setMinimumDuration(0);
        progress->show();

        const QSharedPointer<PrivacyCategorySessionOwner> owner = sessions;
        auto* const watcher =
            new QFutureWatcher<QList<PrivacyCategorySessionResult> >(q);
        QObject::connect(
            watcher,
            &QFutureWatcher<QList<PrivacyCategorySessionResult> >::finished,
            q, [this, watcher, progress]()
        {
            const QList<PrivacyCategorySessionResult> results = watcher->result();
            watcher->deleteLater();
            progress->deleteLater();
            setBusy(false);
            reload();
            const bool failed = std::any_of(
                results.cbegin(), results.cend(),
                [](const PrivacyCategorySessionResult& result)
                {
                    return !result.succeeded();
                });

            if (failed)
            {
                QMessageBox::warning(
                    q, i18nc("@title:window", "Lock All Incomplete"),
                    i18nc("@info", "One or more privacy categories could not be safely locked."));
            }
            else
            {
                statusLabel->setText(i18nc("@info", "All privacy categories are locked."));
            }
        });
        watcher->setFuture(QtConcurrent::run([owner]()
        {
            try
            {
                return owner->lockAllCategories();
            }
            catch (...)
            {
                return QList<PrivacyCategorySessionResult> {
                    PrivacyCategorySessionResult {
                        PrivacyCategorySessionStatus::TransactionBlocked
                    }
                };
            }
        }));
    }

public:

    PrivacyCategorySettingsDialog* q = nullptr;
    QSharedPointer<PrivacyCategorySessionOwner> sessions;
    QSharedPointer<PrivacyRuntimeCoordinator> runtime;
    PrivacyRepositorySnapshot snapshot;
    QList<PrivacyCategory> categories;
    QTableWidget* table = nullptr;
    QPushButton* createButton = nullptr;
    QPushButton* sessionButton = nullptr;
    QPushButton* lockAllButton = nullptr;
    QLabel* statusLabel = nullptr;
    QDialogButtonBox* buttonBox = nullptr;
    bool busy = false;
};

PrivacyCategorySettingsDialog::PrivacyCategorySettingsDialog(QWidget* const parent)
    : QDialog(parent),
      d(new Private(this))
{
    d->setupUi();
}

PrivacyCategorySettingsDialog::~PrivacyCategorySettingsDialog() = default;

void PrivacyCategorySettingsDialog::reject()
{
    if (!d->busy)
    {
        QDialog::reject();
    }
}

} // namespace Digikam
