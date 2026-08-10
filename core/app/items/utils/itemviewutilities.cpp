/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-04
 * Description : Various operation on items
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemviewutilities.h"

// C++ includes

#include <algorithm>
#include <utility>

// Qt includes

#include <QApplication>
#include <QCheckBox>
#include <QFutureWatcher>
#include <QInputDialog>
#include <QList>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QProgressDialog>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QUrl>
#include <QtConcurrentRun>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "albumselectdialog.h"
#include "applicationsettings.h"
#include "deletedialog.h"
#include "dfiledialog.h"
#include "dio.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "surveywindow.h"
#include "loadingcacheinterface.h"
#include "queuemgrwindow.h"
#include "timelapsefilenamematch.h"
#include "thumbnailloadthread.h"
#include "fileactionmngr.h"
#include "dfileoperations.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "privacythreadimagestillitemtransactionowner.h"

namespace Digikam
{

namespace
{

constexpr qlonglong LargePrivateOperationBytes =
    500LL * 1024LL * 1024LL;

void wipeSecret(QString& value)
{
    value.detach();
    value.fill(QChar::Null);
    value.clear();
}

bool acknowledgeExternalApplicationRisk(QWidget* const parent)
{
    KConfigGroup group(KSharedConfig::openConfig(),
                       QStringLiteral("Privacy"));

    if (group.readEntry("ExternalApplicationRiskAcknowledged", false))
    {
        return true;
    }

    QMessageBox warning(
        QMessageBox::Warning,
        i18nc("@title:window", "Private External Access"),
        i18nc("@info",
              "The external application will receive a writable private copy. "
              "digiKam will preserve edits and new sidecars for explicit "
              "reconciliation, while an unchanged copy can be removed "
              "silently when privacy is locked or digiKam exits.\n\n"
              "The external application may create recent-file records, "
              "thumbnails, swap files or caches outside digiKam's control."),
        QMessageBox::Ok | QMessageBox::Cancel, parent);
    auto* const remember = new QCheckBox(
        i18nc("@option:check", "Do not show this warning again"), &warning);
    warning.setCheckBox(remember);

    if (warning.exec() != QMessageBox::Ok)
    {
        return false;
    }

    if (remember->isChecked())
    {
        group.writeEntry("ExternalApplicationRiskAcknowledged", true);
        group.sync();
    }

    return true;
}

bool acknowledgeLargeExternalCheckout(qlonglong bytes, QWidget* const parent)
{
    if (bytes < LargePrivateOperationBytes)
    {
        return true;
    }

    const QString size = QString::number(
        static_cast<double>(bytes) / (1024.0 * 1024.0), 'f', 1);
    return (QMessageBox::warning(
                parent,
                i18nc("@title:window", "Large Private Checkout"),
                i18nc("@info",
                      "This checkout will decrypt about %1 MiB. It may take "
                      "several minutes; keep the collection and category-store "
                      "storage connected until preparation finishes. Continue?",
                      size),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Cancel) == QMessageBox::Yes);
}

void finishExternalCheckout(
    const QSharedPointer<PrivacyThreadImageIOStillItemTransactionOwner>& owner,
    const QString& transactionUuid, QWidget* const parent,
    PrivacyExternalCheckoutDecision decision =
        static_cast<PrivacyExternalCheckoutDecision>(0))
{
    if (owner.isNull() || transactionUuid.isEmpty())
    {
        return;
    }

    auto* const progress = new QProgressDialog(
        i18nc("@info:progress", "Checking private external changes..."),
        QString(), 0, 0, parent);
    progress->setWindowTitle(
        i18nc("@title:window", "Private External Access"));
    progress->setCancelButton(nullptr);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->show();
    const QPointer<QProgressDialog> guardedProgress(progress);
    const QPointer<QWidget> guardedParent(parent);
    auto* const watcher = new QFutureWatcher<PrivacyExternalCheckoutResult>(
        parent ? static_cast<QObject*>(parent) : static_cast<QObject*>(qApp));
    QObject::connect(
        watcher, &QFutureWatcher<PrivacyExternalCheckoutResult>::finished,
        watcher,
        [watcher, owner, transactionUuid, decision, guardedProgress,
         guardedParent]()
        {
            const PrivacyExternalCheckoutResult result = watcher->result();
            watcher->deleteLater();

            if (guardedProgress)
            {
                guardedProgress->deleteLater();
            }

            if (result.status ==
                PrivacyExternalCheckoutStatus::CompletedUnchanged)
            {
                QMessageBox::information(
                    guardedParent,
                    i18nc("@title:window", "Private External Access Finished"),
                    i18nc("@info", "The private checkout has been closed."));
                return;
            }

            if ((decision ==
                 PrivacyExternalCheckoutDecision::PreserveForLater) &&
                (result.status == PrivacyExternalCheckoutStatus::ChangesPending))
            {
                QMessageBox::information(
                    guardedParent,
                    i18nc("@title:window", "Private Changes Preserved"),
                    i18nc("@info",
                          "The changed files remain encrypted in the category "
                          "store for later reconciliation."));
                return;
            }

            if (result.status == PrivacyExternalCheckoutStatus::ChangesPending)
            {
                QMessageBox choice(
                    QMessageBox::Warning,
                    i18nc("@title:window", "Private External Changes"),
                    i18nc("@info",
                          "The external copy changed or contains new files. "
                          "Preserve the complete result for later, or discard "
                          "it after confirmation?"),
                    QMessageBox::NoButton, guardedParent);
                QPushButton* const preserve = choice.addButton(
                    i18nc("@action:button", "Preserve for Later"),
                    QMessageBox::AcceptRole);
                QPushButton* const discard = choice.addButton(
                    i18nc("@action:button", "Discard Changes"),
                    QMessageBox::DestructiveRole);
                choice.addButton(QMessageBox::Cancel);
                choice.exec();

                if (choice.clickedButton() == preserve)
                {
                    finishExternalCheckout(
                        owner, transactionUuid, guardedParent,
                        PrivacyExternalCheckoutDecision::PreserveForLater);
                }
                else if ((choice.clickedButton() == discard) &&
                         (QMessageBox::warning(
                              guardedParent,
                              i18nc("@title:window", "Discard Private Changes"),
                              i18nc("@info",
                                    "Permanently discard every changed and new "
                                    "file in this checkout?"),
                              QMessageBox::Discard | QMessageBox::Cancel,
                              QMessageBox::Cancel) == QMessageBox::Discard))
                {
                    finishExternalCheckout(
                        owner, transactionUuid, guardedParent,
                        PrivacyExternalCheckoutDecision::ConfirmedDiscard);
                }

                return;
            }

            QMessageBox message(
                QMessageBox::Warning,
                i18nc("@title:window", "Private External Access Pending"),
                (result.status ==
                 PrivacyExternalCheckoutStatus::AuthenticationRequired)
                    ? i18nc("@info",
                            "Unlock the category before finishing this private "
                            "checkout. Its files remain encrypted at rest.")
                    : i18nc("@info",
                            "The private checkout could not be finished. Its "
                            "files have been left in place for recovery."),
                QMessageBox::Ok, guardedParent);
            message.setDetailedText(result.detail);
            message.exec();
        });
    watcher->setFuture(QtConcurrent::run(
        [owner, transactionUuid, decision]()
        {
            return owner->finishExternalCheckout(transactionUuid, decision);
        }));
}

void promptForExternalCheckoutFinish(
    const QSharedPointer<PrivacyThreadImageIOStillItemTransactionOwner>& owner,
    const QString& transactionUuid, QWidget* const parent)
{
    QMessageBox prompt(
        QMessageBox::Information,
        i18nc("@title:window", "Private External Access"),
        i18nc("@info",
              "Use the external application normally. Return here and choose "
              "Finish when it has finished reading or saving the file. Closing "
              "this prompt leaves the encrypted checkout available for later "
              "recovery."),
        QMessageBox::NoButton, parent);
    QPushButton* const finish = prompt.addButton(
        i18nc("@action:button", "Finish External Access"),
        QMessageBox::AcceptRole);
    prompt.addButton(i18nc("@action:button", "Leave Open"),
                     QMessageBox::RejectRole);
    prompt.exec();

    if (prompt.clickedButton() == finish)
    {
        finishExternalCheckout(owner, transactionUuid, parent);
    }
}

} // namespace

ItemViewUtilities::ItemViewUtilities(QWidget* const parentWidget)
    : QObject (parentWidget),
      m_widget(parentWidget)
{
    connect(this, SIGNAL(signalImagesDeleted(QList<qlonglong>)),
            AlbumManager::instance(), SLOT(slotImagesDeleted(QList<qlonglong>)));
}

void ItemViewUtilities::setAsAlbumThumbnail(Album* album,
                                            const ItemInfo& itemInfo)
{
    if (!album)
    {
        return;
    }

    if      (album->type() == Album::PHYSICAL)
    {
        PAlbum* const palbum = static_cast<PAlbum*>(album);

        QString err;
        AlbumManager::instance()->updatePAlbumIcon(palbum, itemInfo.id(), err);
    }
    else if (album->type() == Album::TAG)
    {
        TAlbum* const talbum = static_cast<TAlbum*>(album);

        QString err;
        AlbumManager::instance()->updateTAlbumIcon(talbum, QString(), itemInfo.id(), err);
    }
}

void ItemViewUtilities::rename(const QUrl& imageUrl,
                               const QString& newName,
                               bool overwrite)
{
    if (imageUrl.isEmpty() || !imageUrl.isLocalFile() || newName.isEmpty())
    {
        return;
    }

    DIO::rename(imageUrl, newName, overwrite);
}

bool ItemViewUtilities::deleteImages(const QList<ItemInfo>& infos,
                                     const DeleteMode deleteMode)
{
    if (infos.isEmpty())
    {
        return false;
    }

    QList<ItemInfo> deleteInfos = infos;

    QList<QUrl> urlList;
    QList<qlonglong> imageIds;

    // Buffer the urls for deletion and imageids for notification of the AlbumManager

    for (const ItemInfo& info : std::as_const(deleteInfos))
    {
        urlList  << info.fileUrl();
        imageIds << info.id();
    }

    DeleteDialog dialog(m_widget);

    DeleteDialogMode::DeleteMode deleteDialogMode = DeleteDialogMode::NoChoiceTrash;

    if (deleteMode == ItemViewUtilities::DeletePermanently)
    {
        deleteDialogMode = DeleteDialogMode::NoChoiceDeletePermanently;
    }

    if (!dialog.confirmDeleteList(urlList, DeleteDialogMode::Files, deleteDialogMode))
    {
        return false;
    }

    const bool useTrash = !dialog.shouldDelete();

    DIO::del(deleteInfos, useTrash);

    // Signal the Albummanager about the ids of the deleted images.

    Q_EMIT signalImagesDeleted(imageIds);

    return true;
}

void ItemViewUtilities::deleteImagesDirectly(const QList<ItemInfo>& infos,
                                             const DeleteMode deleteMode)
{
    // This method deletes the selected items directly, without confirmation.
    // It is not used in the default setup.

    if (infos.isEmpty())
    {
        return;
    }

    QList<qlonglong> imageIds;

    for (const ItemInfo& info : std::as_const(infos))
    {
        imageIds << info.id();
    }

    const bool useTrash = (deleteMode == ItemViewUtilities::DeleteUseTrash);

    DIO::del(infos, useTrash);

    // Signal the Albummanager about the ids of the deleted images.

    Q_EMIT signalImagesDeleted(imageIds);
}

void ItemViewUtilities::notifyFileContentChanged(const QList<QUrl>& urls)
{
    for (const QUrl& url : std::as_const(urls))
    {
        QString path = url.toLocalFile();
        ThumbnailLoadThread::deleteThumbnail(path);

        // clean LoadingCache as well - be pragmatic, do it here.

        LoadingCacheInterface::fileChanged(path);
    }
}

void ItemViewUtilities::copyItemsToExternalFolder(const QList<ItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Copy To Folder Settings"));
    QString startingPath      = group.readEntry(QLatin1String("Last Copy To Folder Path"), QString());

    if (startingPath.isEmpty())
    {
        startingPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }

    QUrl url = DFileDialog::getExistingDirectoryUrl(m_widget, i18nc("@title:window", "Select Target Folder"),
                                                    QUrl::fromLocalFile(startingPath));

    if (url.isEmpty() || !url.isLocalFile())
    {
        return;
    }

    group.writeEntry(QLatin1String("Last Copy To Folder Path"), url.toLocalFile());

    DIO::copy(infos, url);
}

void ItemViewUtilities::createNewAlbumForInfos(const QList<ItemInfo>& infos,
                                               Album* currentAlbum)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (currentAlbum && (currentAlbum->type() != Album::PHYSICAL))
    {
        currentAlbum = nullptr;
    }

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "move the selected images into.</p>"));

    Album* select = AlbumManager::instance()->findAlbum(m_lastAlbumId);

    if (select && (select->type() != Album::PHYSICAL))
    {
        select = currentAlbum;
    }

    Album* const album = AlbumSelectDialog::selectAlbum(m_widget, dynamic_cast<PAlbum*>(select), header);

    if (!album)
    {
        return;
    }

    PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

    if (!palbum)
    {
        return;
    }

    m_lastAlbumId = palbum->globalID();

    DIO::move(infos, palbum);
}

void ItemViewUtilities::insertToLightTableAuto(const QList<ItemInfo>& all,
                                               const QList<ItemInfo>& selected,
                                               const ItemInfo& current)
{
    ItemInfoList list   = ItemInfoList(selected);
    ItemInfo singleInfo = current;

    if (list.isEmpty() || ((list.size() == 1) && LightTableWindow::lightTableWindow()->isEmpty()))
    {
        list = ItemInfoList(all);
    }

    if (singleInfo.isNull() && !list.isEmpty())
    {
        singleInfo = list.first();
    }

    insertToLightTable(list, current, (list.size() <= 1));
}

void ItemViewUtilities::insertToLightTable(const QList<ItemInfo>& list,
                                           const ItemInfo& current,
                                           bool addTo)
{
    LightTableWindow* const ltview = LightTableWindow::lightTableWindow();

    // If addTo is false, the light table will be emptied before adding
    // the images.

    ltview->loadItemInfos(ItemInfoList(list), current, addTo);
    ltview->setLeftRightItems(ItemInfoList(list), addTo);

    if (ltview->isHidden())
    {
        ltview->show();
    }

    ltview->unminimizeAndActivateWindow();
}

void ItemViewUtilities::insertToQueueManager(const QList<ItemInfo>& list, const ItemInfo& current, bool newQueue)
{
    Q_UNUSED(current);

    QueueMgrWindow* const bqmview = QueueMgrWindow::queueManagerWindow();

    if (bqmview->isHidden())
    {
        bqmview->show();
    }

    bqmview->unminimizeAndActivateWindow();

    if (newQueue)
    {
        bqmview->loadItemInfosToNewQueue(ItemInfoList(list));
    }
    else
    {
        bqmview->loadItemInfosToCurrentQueue(ItemInfoList(list));
    }
}

void ItemViewUtilities::insertSilentToQueueManager(const QList<ItemInfo>& list,
                                                   const ItemInfo& /*current*/,
                                                   int queueid)
{
    QueueMgrWindow* const bqmview = QueueMgrWindow::queueManagerWindow();
    bqmview->loadItemInfos(ItemInfoList(list), queueid);
}

void ItemViewUtilities::openInfos(const ItemInfo& info,
                                  const QList<ItemInfo>& allInfosToOpen,
                                  Album* currentAlbum)
{
    if (info.isNull())
    {
        return;
    }

    QFileInfo fi(info.filePath());
    QString imagefilter = ApplicationSettings::instance()->getImageFileFilter();
    imagefilter        += ApplicationSettings::instance()->getRawFileFilter();

    // If the current item is not an image file.

    if (!imagefilter.contains(fi.suffix().toLower()))
    {
        // Openonly the first one from the list.

        openInfosWithDefaultApplication(QList<ItemInfo>() << info);

        return;
    }

    // Run digiKam ImageEditor with all image from current Album.

    ImageWindow* const imview = ImageWindow::imageWindow();

    imview->disconnect(this);

    connect(imview, SIGNAL(signalURLChanged(QUrl)),
            this, SIGNAL(editorCurrentUrlChanged(QUrl)));

    imview->loadItemInfos(ItemInfoList(allInfosToOpen), info,
                          currentAlbum ? i18n("Album \"%1\"", currentAlbum->title())
                                       : QString());

    if (imview->isHidden())
    {
        imview->show();
    }

    imview->unminimizeAndActivateWindow();
}

void ItemViewUtilities::openInfosWithDefaultApplication(const QList<ItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    const QSharedPointer<PrivacyThreadImageIOStillItemTransactionOwner> owner =
        PrivacyThreadImageIOStillItemTransactionOwner::current();
    QList<ItemInfo> protectedInfos;

    if (owner)
    {
        for (const ItemInfo& info : std::as_const(infos))
        {
            if (!owner->actionContextForImage(info.id())
                     .protectedCategory.uuid.isEmpty())
            {
                protectedInfos << info;
            }
        }
    }

    if (!protectedInfos.isEmpty())
    {
        if ((infos.size() != 1) || (protectedInfos.size() != 1))
        {
            const QMessageBox::StandardButton choice = QMessageBox::question(
                m_widget,
                i18nc("@title:window", "Open Protected Selection"),
                i18nc("@info",
                      "Private writable checkout currently opens one protected "
                      "item at a time. Open this selection using its public "
                      "placeholders instead?"),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Cancel);

            if (choice != QMessageBox::Yes)
            {
                return;
            }
        }
        else
        {
            const ItemInfo info = protectedInfos.constFirst();
            const PrivacyStillItemActionContext context =
                owner->actionContextForImage(info.id());
            QString password;

            if (!acknowledgeLargeExternalCheckout(
                    context.materializationSize, m_widget))
            {
                return;
            }

            if (!owner->categoryIsUnlocked(context.protectedCategory.uuid))
            {
                QMessageBox choice(
                    QMessageBox::Question,
                    i18nc("@title:window", "Open Protected Item"),
                    i18nc("@info",
                          "Unlock %1 and open a writable private checkout, or "
                          "open the public placeholder?",
                          context.protectedCategory.name),
                    QMessageBox::NoButton, m_widget);
                QPushButton* const unlock = choice.addButton(
                    i18nc("@action:button", "Unlock and Open"),
                    QMessageBox::AcceptRole);
                QPushButton* const placeholder = choice.addButton(
                    i18nc("@action:button", "Open Placeholder"),
                    QMessageBox::ActionRole);
                choice.addButton(QMessageBox::Cancel);
                choice.exec();

                if (choice.clickedButton() == placeholder)
                {
                    DFileOperations::openFilesWithDefaultApplication(
                        QList<QUrl>() << info.fileUrl());
                    return;
                }

                if (choice.clickedButton() != unlock)
                {
                    return;
                }

                if (!acknowledgeExternalApplicationRisk(m_widget))
                {
                    return;
                }

                bool accepted = false;
                password = QInputDialog::getText(
                    m_widget,
                    i18nc("@title:window", "Unlock Privacy Category"),
                    i18nc("@label", "Password for %1:",
                          context.protectedCategory.name),
                    QLineEdit::Password, QString(), &accepted);

                if (!accepted)
                {
                    wipeSecret(password);
                    return;
                }
            }
            else if (!acknowledgeExternalApplicationRisk(m_widget))
            {
                return;
            }

            auto* const progress = new QProgressDialog(
                i18nc("@info:progress",
                      "Preparing writable private checkout..."),
                QString(), 0, 0, m_widget);
            progress->setWindowTitle(
                i18nc("@title:window", "Private External Access"));
            progress->setCancelButton(nullptr);
            progress->setWindowModality(Qt::WindowModal);
            progress->setMinimumDuration(0);
            progress->show();
            const QPointer<QProgressDialog> guardedProgress(progress);
            const QPointer<QWidget> guardedParent(m_widget);
            const QSharedPointer<QString> secret =
                QSharedPointer<QString>::create(std::move(password));
            auto* const watcher =
                new QFutureWatcher<PrivacyExternalCheckoutResult>(
                    m_widget ? static_cast<QObject*>(m_widget)
                             : static_cast<QObject*>(qApp));
            QObject::connect(
                watcher,
                &QFutureWatcher<PrivacyExternalCheckoutResult>::finished,
                watcher,
                [watcher, owner, guardedProgress, guardedParent]()
                {
                    const PrivacyExternalCheckoutResult result =
                        watcher->result();
                    watcher->deleteLater();

                    if (guardedProgress)
                    {
                        guardedProgress->deleteLater();
                    }

                    if (result.status ==
                        PrivacyExternalCheckoutStatus::ChangesPending)
                    {
                        finishExternalCheckout(
                            owner, result.transactionUuid, guardedParent);
                        return;
                    }

                    if ((result.status ==
                         PrivacyExternalCheckoutStatus::RecoveryRequired) &&
                        !result.transactionUuid.isEmpty() &&
                        (QMessageBox::question(
                             guardedParent,
                             i18nc("@title:window",
                                   "Private Checkout Already Open"),
                             i18nc("@info",
                                   "A writable checkout for this item already "
                                   "exists. Check and finish it now?"),
                             QMessageBox::Yes | QMessageBox::Cancel,
                             QMessageBox::Yes) == QMessageBox::Yes))
                    {
                        finishExternalCheckout(
                            owner, result.transactionUuid, guardedParent);
                        return;
                    }

                    if (!result.succeeded())
                    {
                        QMessageBox message(
                            QMessageBox::Warning,
                            i18nc("@title:window",
                                  "Private External Access Failed"),
                            i18nc("@info",
                                  "The protected item could not be prepared for "
                                  "external access."),
                            QMessageBox::Ok, guardedParent);
                        message.setDetailedText(result.detail);
                        message.exec();
                        return;
                    }

                    QUrl primary;

                    for (const PrivacyExternalCheckoutAsset& asset :
                         result.assets)
                    {
                        if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                            (asset.ordinal == 0))
                        {
                            primary = asset.checkoutUrl;
                            break;
                        }
                    }

                    if (primary.isEmpty())
                    {
                        (void)owner->finishExternalCheckout(
                            result.transactionUuid);
                        QMessageBox::warning(
                            guardedParent,
                            i18nc("@title:window",
                                  "Private External Access Failed"),
                            i18nc("@info",
                                  "The prepared checkout has no primary media "
                                  "item and was not opened."));
                        return;
                    }

                    DFileOperations::openFilesWithDefaultApplication(
                        QList<QUrl>() << primary);
                    promptForExternalCheckoutFinish(
                        owner, result.transactionUuid, guardedParent);
                });
            watcher->setFuture(QtConcurrent::run(
                [owner, info, secret]()
                {
                    PrivacyExternalCheckoutResult result =
                        owner->prepareExternalOpen(info, *secret);
                    wipeSecret(*secret);
                    return result;
                }));
            return;
        }
    }

    QList<QUrl> urls;

    for (const ItemInfo& inf : std::as_const(infos))
    {
        urls << inf.fileUrl();
    }

    DFileOperations::openFilesWithDefaultApplication(urls);
}

// ---

namespace
{

bool lessThanByTimeForItemInfo(const ItemInfo& a, const ItemInfo& b)
{
    return (a.dateTime() < b.dateTime());
}

bool lowerThanByNameForItemInfo(const ItemInfo& a, const ItemInfo& b)
{
    return (a.name() < b.name());
}

bool lowerThanBySizeForItemInfo(const ItemInfo& a, const ItemInfo& b)
{
    return (a.fileSize() < b.fileSize());
}

} // namespace

// ---

void ItemViewUtilities::createGroupByTimeFromInfoList(const ItemInfoList& itemInfoList)
{
    QList<ItemInfo> groupingList = itemInfoList;

    // sort by time

    std::stable_sort(groupingList.begin(), groupingList.end(), lessThanByTimeForItemInfo);

    QList<ItemInfo>::iterator it, it2;

    for (it = groupingList.begin() ; it != groupingList.end() ; )
    {
        const ItemInfo& leader = *it;
        QList<ItemInfo> group;
        QDateTime time         = it->dateTime();

        if (time.isValid())
        {
            for (it2 = it + 1 ; it2 != groupingList.end() ; ++it2)
            {
                if (qAbs(time.secsTo(it2->dateTime())) < 2)
                {
                    group << *it2;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            ++it;
            continue;
        }

        // increment to next item not put in the group

        it = it2;

        if (!group.isEmpty())
        {
            FileActionMngr::instance()->addToGroup(leader, group);
        }
    }
}

void ItemViewUtilities::createGroupByFilenameFromInfoList(const ItemInfoList& itemInfoList)
{
    QList<ItemInfo> groupingList = itemInfoList;

    // sort by Name

    std::stable_sort(groupingList.begin(), groupingList.end(), lowerThanByNameForItemInfo);

    QList<ItemInfo>::iterator it, it2;

    for (it = groupingList.begin() ; it != groupingList.end() ; )
    {
        QList<ItemInfo> group;
        QString fname = it->name().left(it->name().indexOf(QLatin1Char('.')));

        // don't know the leader yet so put first element also in group

        group << *it;

        for (it2 = it + 1 ; it2 != groupingList.end() ; ++it2)
        {
            QString fname2 = it2->name().left(it2->name().indexOf(QLatin1Char('.')));

            if (fname == fname2)
            {
                group << *it2;
            }
            else
            {
                break;
            }
        }

        // increment to next item not put in the group

        it = it2;

        if (group.size() > 1)
        {
            // sort by filesize and take smallest as leader

            std::stable_sort(group.begin(), group.end(), lowerThanBySizeForItemInfo);
            int rawCount = 0;

            for (int i = 0 ; i < group.size() ; ++i)
            {
                if (group.at(i).format().startsWith(QLatin1String("RAW")))
                {
                    ++rawCount;

                    if (i < group.size() - 1)
                    {
                        continue;
                    }
                }

                const ItemInfo& leader = group.takeAt((rawCount == group.size()) ? 0 : i);
                FileActionMngr::instance()->addToGroup(leader, group);
                break;
            }
        }
    }
}

// ---

namespace
{

bool imageMatchesTimelapseGroup(const ItemInfoList& group, const ItemInfo& itemInfo)
{
    if (group.size() < 2)
    {
        return true;
    }

    auto const timeBetweenPhotos      = qAbs(group.first().dateTime()
                                                          .secsTo(group.last()
                                                          .dateTime())) / (group.size()-1);

    auto const predictedNextTimestamp = group.last().dateTime()
                                                    .addSecs(timeBetweenPhotos);

    return (qAbs(itemInfo.dateTime().secsTo(predictedNextTimestamp)) <= 1);
}

void sortTimelapseGroupingListBySequence(ItemInfoList& groupingList)
{
    QList<TimelapseFilenameMatch> nameSortedMatches;
    nameSortedMatches.reserve(groupingList.size());

    for (const ItemInfo& itemInfo : std::as_const(groupingList))
    {
        nameSortedMatches.append(TimelapseFilenameMatch(itemInfo.name()));
    }

    const QList<qsizetype> sequenceOrder = timelapseFilenameSequenceOrder(nameSortedMatches);
    ItemInfoList sortedList;
    sortedList.reserve(groupingList.size());

    for (const qsizetype index : sequenceOrder)
    {
        sortedList.append(groupingList.at(index));
    }

    groupingList = sortedList;
}

} // namespace

// ---

void ItemViewUtilities::createGroupByTimelapseFromInfoList(const ItemInfoList& itemInfoList)
{
    if (itemInfoList.size() < 3)
    {
        return;
    }

    ItemInfoList groupingList = itemInfoList;

    std::stable_sort(groupingList.begin(), groupingList.end(), lowerThanByNameForItemInfo);
    sortTimelapseGroupingListBySequence(groupingList);

    TimelapseFilenameMatch previousNumberMatch;
    ItemInfoList group;

    for (const auto& itemInfo : std::as_const(groupingList))
    {
        TimelapseFilenameMatch numberMatch(itemInfo.name());

        // if this is an end of currently processed group

        if (!previousNumberMatch.directlyPreceeds(numberMatch) || !imageMatchesTimelapseGroup(group, itemInfo))
        {
            if (group.size() > 2)
            {
                FileActionMngr::instance()->addToGroup(group.takeFirst(), group);
            }

            group.clear();
        }

        group.append(itemInfo);
        previousNumberMatch = std::move(numberMatch);
    }

    if (group.size() > 2)
    {
        FileActionMngr::instance()->addToGroup(group.takeFirst(), group);
    }
}

} // namespace Digikam

#include "moc_itemviewutilities.cpp"
