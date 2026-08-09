/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface - View methods.
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemiconview_p.h"

// C++ includes

#include <utility>

// Qt includes

#include <QCoreApplication>
#include <QDir>
#include <QFutureWatcher>
#include <QHash>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QProgressDialog>
#include <QStringList>
#include <QWaitCondition>
#include <QtConcurrent>

// Local includes

#include "privacythreadimagestillitemtransactionowner.h"

namespace Digikam
{

namespace
{

class PrivacyProtectAcknowledgementState
{
public:

    QMutex mutex;
    QWaitCondition changed;
    bool completed = false;
    bool abandoned = false;
    bool accepted = false;
    QMetaObject::Connection shutdownConnection;
};

QString privacyActionFailureText(
    const PrivacyStillItemTransactionResult& result,
    const QString& fallback)
{
    switch (result.status)
    {
        case PrivacyStillItemTransactionStatus::AuthenticationRequired:
        {
            return i18nc("@info", "The category password was not accepted.");
        }

        case PrivacyStillItemTransactionStatus::CategoryUnavailable:
        {
            return result.detail.isEmpty() ? fallback : result.detail;
        }

        case PrivacyStillItemTransactionStatus::AssociatedAssetSetUnsupported:
        {
            return i18nc("@info",
                         "This item has associated files that are not supported yet.");
        }

        case PrivacyStillItemTransactionStatus::PreflightRejected:
        {
            return i18nc("@info",
                         "The related-file safety check did not pass.");
        }

        case PrivacyStillItemTransactionStatus::SourceChanged:
        {
            return i18nc("@info",
                         "The item or its related files changed during the safety check.");
        }

        case PrivacyStillItemTransactionStatus::RootUnavailable:
        {
            return i18nc("@info",
                         "The collection storage is no longer safely available.");
        }

        case PrivacyStillItemTransactionStatus::RecoveryRequired:
        case PrivacyStillItemTransactionStatus::CleanupPending:
        {
            return i18nc("@info",
                         "The action could not finish safely. Restart digiKam to reconcile it.");
        }

        default:
        {
            return fallback;
        }
    }
}

} // namespace

void ItemIconView::setHostWindowActions(const HostActionsMap& actions)
{
    d->stackedView->imagePreviewView()->setHostWindowActions(actions);

#ifdef HAVE_MEDIAPLAYER

    d->stackedView->mediaPlayerView()->setHostWindowActions(actions);

#endif // HAVE_MEDIAPLAYER

}

void ItemIconView::connectIconViewFilter(FilterStatusBar* const filterbar)
{
    ItemAlbumFilterModel* const model = d->iconView->itemAlbumFilterModel();

    connect(model, SIGNAL(filterMatches(bool)),
            filterbar, SLOT(slotFilterMatches(bool)));

    connect(model, SIGNAL(filterSettingsChanged(ItemFilterSettings)),
            filterbar, SLOT(slotFilterSettingsChanged(ItemFilterSettings)));

    connect(filterbar, SIGNAL(signalResetFilters()),
            d->filterWidget, SLOT(slotResetFilters()));

    connect(filterbar, SIGNAL(signalPopupFiltersView()),
            this, SLOT(slotPopupFiltersView()));
}

void ItemIconView::slotEscapePreview()
{
    if (
        (viewMode() == StackedView::IconViewMode)  ||
        (viewMode() == StackedView::MapWidgetMode) ||
        (viewMode() == StackedView::TableViewMode) ||
        (viewMode() == StackedView::TrashViewMode) ||
        (viewMode() == StackedView::WelcomePageMode)
       )
    {
        return;
    }

    // pass a null image info, because we want to fall back to the old view mode

    slotTogglePreviewMode(ItemInfo());
}

void ItemIconView::slotMapWidgetView()
{
    d->stackedView->setViewMode(StackedView::MapWidgetMode, true);
}

void ItemIconView::slotTableView()
{
    d->stackedView->setViewMode(StackedView::TableViewMode, true);
}

void ItemIconView::slotIconView()
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        Q_EMIT signalThumbSizeChanged(d->thumbSize);
    }

    // and switch to icon view

    d->stackedView->setViewMode(StackedView::IconViewMode, true);

    // make sure the next/previous buttons are updated

    slotImageSelected();
}

void ItemIconView::slotImagePreview()
{
    slotTogglePreviewMode(currentInfo());
}

/**
 * @brief This method toggles between AlbumView/MapWidgetView and ImagePreview modes, depending on the context.
 */
void ItemIconView::slotTogglePreviewMode(const ItemInfo& info)
{
    if (
        (viewMode() == StackedView::IconViewMode)  ||
        (viewMode() == StackedView::TableViewMode) ||
        (viewMode() == StackedView::MapWidgetMode)
       )
    {
        d->lastViewMode = viewMode();

        if (!info.isNull())
        {
            if (info.isLocationAvailable())
            {
                if (viewMode() == StackedView::IconViewMode)
                {
                    d->stackedView->setPreviewItem(info,
                                                   d->iconView->previousInfo(info),
                                                   d->iconView->nextInfo(info));
                }
                else
                {
                    d->stackedView->setPreviewItem(info,
                                                   ItemInfo(),
                                                   ItemInfo());
                }
            }
            else
            {
                QModelIndex index = d->iconView->indexForInfo(info);
                d->iconView->showIndexNotification(index,
                                                   i18nc("@info: item icon view",
                                                         "The storage location of this image\n"
                                                         "is currently not available"));
            }
        }
    }
    else
    {
        // go back to the last AlbumViewMode

        d->stackedView->setViewMode(d->lastViewMode);
    }

    // make sure the next/previous buttons are updated

    slotImageSelected();
}

void ItemIconView::slotViewModeChanged()
{
    toggleZoomActions();

    switch (viewMode())
    {
        case StackedView::IconViewMode:
        {
            Q_EMIT signalSwitchedToIconView();
            Q_EMIT signalThumbSizeChanged(d->thumbSize);

            break;
        }

        case StackedView::PreviewImageMode:
        {
            Q_EMIT signalSwitchedToPreview();

            slotZoomFactorChanged(d->stackedView->zoomFactor());

            break;
        }

        case StackedView::WelcomePageMode:
        {
            Q_EMIT signalSwitchedToIconView();

            break;
        }

        case StackedView::MediaPlayerMode:
        {
            Q_EMIT signalSwitchedToPreview();

            break;
        }

        case StackedView::MapWidgetMode:
        {
            Q_EMIT signalSwitchedToMapView();

            // TODO: connect map view's zoom buttons to main status bar zoom buttons

            break;
        }

        case StackedView::TableViewMode:
        {
            Q_EMIT signalSwitchedToTableView();
            Q_EMIT signalThumbSizeChanged(d->thumbSize);

            break;
        }

        case StackedView::TrashViewMode:
        {
            d->msgNotifyTimer->stop();
            d->errorWidget->animatedHide();

            Q_EMIT signalSwitchedToTrashView();

            break;
        }
    }
}

void ItemIconView::toggleShowBar(bool b)
{
    d->stackedView->thumbBarDock()->showThumbBar(b);

    // See bug #319876 : force to reload current view mode to set thumbbar visibility properly.

    d->stackedView->setViewMode(viewMode());
}

StackedView::StackedViewMode ItemIconView::viewMode() const
{
    return d->stackedView->viewMode();
}

void ItemIconView::slotSetupMetadataFilters(int tab)
{
    Setup::execMetadataFilters(this, tab);
}

void ItemIconView::slotSetupExifTool()
{
    Setup::execExifTool(this);
}

void ItemIconView::toggleFullScreen(bool set)
{
    d->stackedView->imagePreviewView()->toggleFullScreen(set);
}

void ItemIconView::setToolsIconView(DCategorizedView* const view)
{
    d->rightSideBar->appendTab(view,
                               QIcon::fromTheme(QLatin1String("document-edit")),
                               i18nc("@title: item icon view", "Tools"));
}

void ItemIconView::refreshView()
{
    d->rightSideBar->refreshTagsView();
}

void ItemIconView::slotShowContextMenu(QContextMenuEvent* event,
                                       const QList<QAction*>& extraGroupingActions)
{
    const Album* const album = currentAlbum();

    if (
        !album          ||
        album->isRoot() ||
        (
         (album->type() != Album::PHYSICAL) &&
         (album->type() != Album::TAG)
        )
       )
    {
        return;
    }

    QMenu menu(this);
    ContextMenuHelper cmHelper(&menu);

    cmHelper.addAction(QLatin1String("full_screen"));
    cmHelper.addAction(QLatin1String("options_show_menubar"));
    cmHelper.addSeparator();
    cmHelper.addStandardActionPaste(this, SLOT(slotImagePaste()));

    if (!extraGroupingActions.isEmpty())
    {
        cmHelper.addSeparator();
        cmHelper.addGroupMenu(QList<qlonglong>(), extraGroupingActions);
    }

    cmHelper.exec(event->globalPos());
}

void ItemIconView::slotShowContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info,
                                             const QList<QAction*>& extraGroupingActions,
                                             ItemFilterModel* imageFilterModel)
{
    QList<qlonglong> selectedImageIds = selectedInfoList(true, true).toImageIdList();

    // --------------------------------------------------------

    QMenu menu(this);
    ContextMenuHelper cmHelper(&menu);
    cmHelper.setItemFilterModel(imageFilterModel);

    cmHelper.addAction(QLatin1String("full_screen"));
    cmHelper.addAction(QLatin1String("options_show_menubar"));
    cmHelper.addSeparator();

    // --------------------------------------------------------

    QAction* const viewAction = new QAction(i18nc("@action: View the selected image", "Preview"), this);
    viewAction->setIcon(QIcon::fromTheme(QLatin1String("view-preview")));
    viewAction->setEnabled(selectedImageIds.count() == 1);
    cmHelper.addAction(viewAction);

    cmHelper.addOpenAndNavigateActions(selectedImageIds);
    cmHelper.addSeparator();

    // --------------------------------------------------------

    QMenu* const privacyMenu = new QMenu(
        i18nc("@action: private-media workflow", "Privacy"));
    privacyMenu->setIcon(QIcon::fromTheme(QLatin1String("object-locked")));
    QAction* privacyUnprotectAction = nullptr;
    QAction* privacyResumeAction = nullptr;
    bool privacyResumeRequiresFreshAuthentication = false;
    PrivacyStillItemActionContext privacyActionContext;
    QHash<const QAction*, PrivacyCategory> privacyProtectActions;
    const QSharedPointer<PrivacyThreadImageIOStillItemTransactionOwner> privacyOwner =
        PrivacyThreadImageIOStillItemTransactionOwner::current();
    const bool oneMedia = privacyOwner &&
                          (selectedImageIds.size() == 1) &&
                          (selectedImageIds.constFirst() == info.id()) &&
                          ((info.category() == DatabaseItem::Image) ||
                           (info.category() == DatabaseItem::Video));

    if (!oneMedia)
    {
        QAction* const unavailable = privacyMenu->addAction(
            i18nc("@action: private-media workflow", "Select One Photo or Video"));
        unavailable->setEnabled(false);
    }
    else
    {
        privacyActionContext = privacyOwner->actionContextForImage(info.id());

        if (privacyActionContext.availability ==
            PrivacyStillItemActionAvailability::Unprotectable)
        {
            privacyUnprotectAction = privacyMenu->addAction(
                QIcon::fromTheme(QLatin1String("object-unlocked")),
                i18nc("@action: restore a protected item", "Unprotect from %1...",
                      privacyActionContext.protectedCategory.name));
        }
        else if ((privacyActionContext.availability ==
                  PrivacyStillItemActionAvailability::ResumeProtectable) ||
                 (privacyActionContext.availability ==
                  PrivacyStillItemActionAvailability::ResumeUnprotectable))
        {
            privacyResumeRequiresFreshAuthentication =
                (privacyActionContext.availability ==
                 PrivacyStillItemActionAvailability::ResumeUnprotectable);
            privacyResumeAction = privacyMenu->addAction(
                QIcon::fromTheme(
                    privacyResumeRequiresFreshAuthentication
                        ? QLatin1String("object-unlocked")
                        : QLatin1String("object-locked")),
                privacyResumeRequiresFreshAuthentication
                    ? i18nc("@action: resume restoring a protected item",
                            "Resume Unprotecting from %1...",
                            privacyActionContext.protectedCategory.name)
                    : i18nc("@action: resume protecting an item",
                            "Resume Protecting in %1...",
                            privacyActionContext.protectedCategory.name));
        }
        else if (privacyActionContext.availability ==
                 PrivacyStillItemActionAvailability::ProtectedUnavailable)
        {
            QString reason = i18nc("@item:inmenu", "Recovery Required");

            if (privacyActionContext.publicRootState ==
                PrivacyRootRuntimeState::Offline)
            {
                reason = i18nc("@item:inmenu", "Storage Offline");
            }
            else if (privacyActionContext.publicRootState ==
                     PrivacyRootRuntimeState::IdentityMismatch)
            {
                reason = i18nc("@item:inmenu", "Storage Identity Mismatch");
            }

            QAction* const unavailable = privacyMenu->addAction(
                i18nc("@action: private-media workflow",
                      "Protected in %1 - %2",
                      privacyActionContext.protectedCategory.name, reason));
            unavailable->setEnabled(false);
        }
        else if (privacyActionContext.availability ==
                 PrivacyStillItemActionAvailability::Protectable)
        {
            if (!info.isLocationAvailable())
            {
                QAction* const unavailable = privacyMenu->addAction(
                    i18nc("@action: private-media workflow", "Item Unavailable"));
                unavailable->setEnabled(false);
            }
            else
            {
                for (const PrivacyCategory& category :
                     privacyActionContext.protectCategories)
                {
                    QAction* const action = privacyMenu->addAction(
                        QIcon::fromTheme(QLatin1String("object-locked")),
                        i18nc("@action: protect an item in a category", "Protect in %1...",
                              category.name));
                    privacyProtectActions.insert(action, category);
                }

                if (privacyActionContext.protectCategories.isEmpty())
                {
                    QAction* const unavailable = privacyMenu->addAction(
                        i18nc("@action: private-media workflow",
                              "No Active Casual Privacy Categories"));
                    unavailable->setEnabled(false);
                }
            }
        }
        else
        {
            QAction* const unavailable = privacyMenu->addAction(
                i18nc("@action: private-media workflow", "Privacy State Unavailable"));
            unavailable->setEnabled(false);
        }
    }

    cmHelper.addSubMenu(privacyMenu);
    cmHelper.addSeparator();

    // --------------------------------------------------------

    QMenu* const fmenu = new QMenu(i18nc("@action: face workflow", "Faces"));
    fmenu->setIcon(QIcon::fromTheme(QLatin1String("edit-image-face-show")));
    fmenu->addAction(DigikamApp::instance()->actionCollection()->action(QLatin1String("image_scan_for_faces")));
    fmenu->addAction(DigikamApp::instance()->actionCollection()->action(QLatin1String("image_recognize_faces")));
    fmenu->addAction(DigikamApp::instance()->actionCollection()->action(QLatin1String("image_remove_all_faces")));
    cmHelper.addSubMenu(fmenu);
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addAction(QLatin1String("image_find_similar"));
    cmHelper.addStandardActionLightTable();
    cmHelper.addQueueManagerMenu();
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addAction(QLatin1String("image_rotate"));
    cmHelper.addAction(QLatin1String("cut_album_selection"));
    cmHelper.addAction(QLatin1String("copy_album_selection"));
    cmHelper.addAction(QLatin1String("paste_album_selection"));
    cmHelper.addAction(QLatin1String("image_rename"));
    cmHelper.addStandardActionItemDelete(this, SLOT(slotImageDelete()), selectedImageIds.count());
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addIQSAction(this, SLOT(slotImageQualitySorter()));
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addStandardActionThumbnail(selectedImageIds, currentAlbum());
    cmHelper.addAssignTagsMenu(selectedImageIds);
    cmHelper.addRemoveTagsMenu(selectedImageIds);
    cmHelper.addRemoveAllTags(selectedImageIds);
    cmHelper.addLabelsAction();

    if (d->leftSideBar->getActiveTab() != d->peopleSideBar)
    {
        cmHelper.addSeparator();

        cmHelper.addGroupMenu(selectedImageIds, extraGroupingActions);
    }

    // special action handling --------------------------------

    connect(&cmHelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmHelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmHelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmHelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmHelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmHelper, SIGNAL(signalPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    connect(&cmHelper, SIGNAL(signalGotoTag(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    connect(&cmHelper, SIGNAL(signalGotoTag(int)),
            d->albumHistory, SLOT(slotClearSelectTAlbum(int)));

    connect(&cmHelper, SIGNAL(signalGotoAlbum(ItemInfo)),
            this, SLOT(slotGotoAlbumAndItem(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalGotoAlbum(ItemInfo)),
            d->albumHistory, SLOT(slotClearSelectPAlbum(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalGotoDate(ItemInfo)),
            this, SLOT(slotGotoDateAndItem(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalSetThumbnail(ItemInfo)),
            this, SLOT(slotSetAsAlbumThumbnail(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(slotImageAddToExistingQueue(int)));

    connect(&cmHelper, SIGNAL(signalCreateGroup()),
            this, SLOT(slotCreateGroupFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(slotCreateGroupByTimeFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByFilename()),
            this, SLOT(slotCreateGroupByFilenameFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByTimelapse()),
            this, SLOT(slotCreateGroupByTimelapseFromSelection()));

    connect(&cmHelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(slotRemoveSelectedFromGroup()));

    connect(&cmHelper, SIGNAL(signalUngroup()),
            this, SLOT(slotUngroupSelected()));

    // --------------------------------------------------------

    const QAction* const choice = cmHelper.exec(event->globalPos());
    const auto watchPrivacyAction =
        [this](const QFuture<PrivacyStillItemTransactionResult>& future,
               QProgressDialog* const progress,
               const QString& failureTitle,
               const QString& failureText)
        {
            auto* const watcher =
                new QFutureWatcher<PrivacyStillItemTransactionResult>(this);

            connect(watcher,
                    &QFutureWatcher<PrivacyStillItemTransactionResult>::finished,
                    this,
                    [this, watcher, progress, failureTitle, failureText]()
                    {
                        const PrivacyStillItemTransactionResult result =
                            watcher->result();
                        progress->deleteLater();
                        watcher->deleteLater();

                        if (!result.succeeded() &&
                            (result.status !=
                             PrivacyStillItemTransactionStatus::AcknowledgementRequired))
                        {
                            QMessageBox message(
                                QMessageBox::Warning, failureTitle,
                                privacyActionFailureText(result, failureText),
                                QMessageBox::Ok, this);

                            if (!result.detail.isEmpty())
                            {
                                message.setDetailedText(result.detail);
                            }

                            message.exec();
                        }
                        else if (result.succeeded())
                        {
                            d->iconView->viewport()->update();
                            d->tableView->update();
                            slotNotificationError(
                                (result.status ==
                                 PrivacyStillItemTransactionStatus::Protected)
                                    ? i18nc("@info", "The item is now protected.")
                                    : i18nc("@info", "The item is now unprotected."),
                                DNotificationWidget::Information);
                        }
                    });
            watcher->setFuture(future);
        };

    if (choice && (choice == viewAction))
    {
        slotTogglePreviewMode(info);
    }
    else if (choice && privacyOwner && (choice == privacyResumeAction))
    {
        QString password;
        bool accepted = true;
        const bool passwordRequired =
            privacyResumeRequiresFreshAuthentication ||
            !privacyOwner->categoryIsUnlocked(
                privacyActionContext.protectedCategory.uuid);

        if (passwordRequired)
        {
            password = QInputDialog::getText(
                this,
                i18nc("@title:window", "Resume Private Item Recovery"),
                i18nc("@label", "Password for %1:",
                      privacyActionContext.protectedCategory.name),
                QLineEdit::Password, QString(), &accepted);
        }

        if (accepted)
        {
            auto* const progress = new QProgressDialog(
                privacyResumeRequiresFreshAuthentication
                    ? i18nc("@info:progress",
                            "Resuming restoration of the protected original...")
                    : i18nc("@info:progress",
                            "Resuming protection of the selected item..."),
                QString(), 0, 0, this);
            progress->setWindowTitle(
                i18nc("@title:window", "Recovering Private Item"));
            progress->setCancelButton(nullptr);
            progress->setWindowModality(Qt::WindowModal);
            progress->setMinimumDuration(0);
            progress->show();

            const QString transactionUuid =
                privacyActionContext.recoveryTransactionUuid;
            const QSharedPointer<QString> passwordSecret =
                QSharedPointer<QString>::create(std::move(password));
            QFuture<PrivacyStillItemTransactionResult> future =
                QtConcurrent::run(
                    [privacyOwner, imageId = info.id(), transactionUuid,
                     passwordSecret]()
                    {
                        PrivacyStillItemTransactionResult result;

                        try
                        {
                            result = privacyOwner->resume(
                                imageId, transactionUuid, *passwordSecret);
                        }
                        catch (...)
                        {
                            result.status =
                                PrivacyStillItemTransactionStatus::RecoveryRequired;
                            result.detail.clear();
                        }

                        passwordSecret->fill(QChar());
                        return result;
                    });
            watchPrivacyAction(
                future, progress,
                i18nc("@title:window", "Private Recovery Failed"),
                i18nc("@info", "The private item could not be recovered."));
        }

        password.fill(QChar());
    }
    else if (choice && privacyOwner && (choice == privacyUnprotectAction))
    {
        const bool confirmed = (QMessageBox::warning(
            this,
            i18nc("@title:window", "Remove Privacy Protection"),
            i18nc("@info",
                  "This will restore the original file to its public collection "
                  "path and remove its privacy protection. Continue?"),
            QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel) == QMessageBox::Yes);

        if (!confirmed)
        {
            return;
        }

        bool accepted = false;
        QString password = QInputDialog::getText(
            this,
            i18nc("@title:window", "Unprotect Private Item"),
            i18nc("@label", "Password for the private category:"),
            QLineEdit::Password, QString(), &accepted);

        if (accepted)
        {
            auto* const progress = new QProgressDialog(
                i18nc("@info:progress", "Restoring the protected original..."),
                QString(), 0, 0, this);
            progress->setWindowTitle(i18nc("@title:window", "Unprotecting Item"));
            progress->setCancelButton(nullptr);
            progress->setWindowModality(Qt::WindowModal);
            progress->setMinimumDuration(0);
            progress->show();

            const QSharedPointer<QString> passwordSecret =
                QSharedPointer<QString>::create(std::move(password));
            QFuture<PrivacyStillItemTransactionResult> future = QtConcurrent::run(
                [privacyOwner, info, passwordSecret]()
                {
                    PrivacyStillItemTransactionResult result;

                    try
                    {
                        result = privacyOwner->unprotect(info, *passwordSecret);
                    }
                    catch (...)
                    {
                        result.status =
                            PrivacyStillItemTransactionStatus::RecoveryRequired;
                        result.detail.clear();
                    }

                    passwordSecret->fill(QChar());
                    return result;
                });
            watchPrivacyAction(
                future, progress,
                i18nc("@title:window", "Unprotect Failed"),
                i18nc("@info", "The item could not be unprotected."));
        }

        password.fill(QChar());
    }
    else if (choice && privacyOwner && privacyProtectActions.contains(choice))
    {
        const PrivacyCategory category = privacyProtectActions.value(
            choice);
        const bool confirmed = (QMessageBox::warning(
            this,
            i18nc("@title:window", "Protect Private Item"),
            i18nc("@info",
                  "digiKam will replace the public file with a privacy-safe "
                  "display proxy and invalidate its clear cached previews. "
                  "Rebuilding privacy-safe views may take a moment. Continue?"),
            QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel) == QMessageBox::Yes);

        if (!confirmed)
        {
            return;
        }

        QString password;
        bool accepted = true;

        if (!privacyOwner->categoryIsUnlocked(category.uuid))
        {
            password = QInputDialog::getText(
                this,
                i18nc("@title:window", "Unlock Private Category"),
                i18nc("@label", "Password for %1:", category.name),
                QLineEdit::Password, QString(), &accepted);
        }

        if (accepted)
        {
            auto* const progress = new QProgressDialog(
                i18nc("@info:progress", "Protecting the selected item..."),
                QString(), 0, 0, this);
            progress->setWindowTitle(i18nc("@title:window", "Protecting Item"));
            progress->setCancelButton(nullptr);
            progress->setWindowModality(Qt::WindowModal);
            progress->setMinimumDuration(0);
            progress->show();

            const QPointer<ItemIconView> view(this);
            const QPointer<QProgressDialog> progressGuard(progress);
            const QSharedPointer<QString> passwordSecret =
                QSharedPointer<QString>::create(std::move(password));
            QFuture<PrivacyStillItemTransactionResult> future = QtConcurrent::run(
                [privacyOwner, info, category, view, progressGuard,
                 passwordSecret]()
                {
                    PrivacyStillItemTransactionResult result;

                    try
                    {
                        result = privacyOwner->protect(
                                info, category.uuid, *passwordSecret,
                                [view, progressGuard]
                                (const PrivacyProtectPreflightResult& preflight)
                            {
                                if (!view)
                                {
                                    return false;
                                }

                                const QSharedPointer<
                                    PrivacyProtectAcknowledgementState> state =
                                        QSharedPointer<
                                            PrivacyProtectAcknowledgementState>::create();
                                QCoreApplication* const application =
                                    QCoreApplication::instance();

                                if (!application)
                                {
                                    return false;
                                }

                                state->shutdownConnection = QObject::connect(
                                    application, &QCoreApplication::aboutToQuit,
                                    application,
                                    [state]()
                                    {
                                        QMutexLocker locker(&state->mutex);
                                        state->abandoned = true;
                                        state->completed = true;
                                        state->changed.wakeAll();
                                    },
                                    Qt::DirectConnection);
                                const bool invoked = QMetaObject::invokeMethod(
                                    view.data(),
                                    [state, preflight, view, progressGuard]()
                                    {
                                        {
                                            QMutexLocker locker(&state->mutex);

                                            if (state->abandoned)
                                            {
                                                state->completed = true;
                                                state->changed.wakeAll();
                                                return;
                                            }
                                        }

                                        bool accepted = false;

                                        if (view)
                                        {
                                            if (progressGuard)
                                            {
                                                progressGuard->hide();
                                            }

                                            const int warningCount =
                                                preflight.bridge.items.isEmpty()
                                                    ? 0
                                                    : preflight.bridge.items.constFirst()
                                                          .inventory.exposureWarnings.size();
                                            const int assetCount =
                                                preflight.bridge.items.isEmpty()
                                                    ? 0
                                                    : preflight.bridge.items.constFirst()
                                                          .inventory.requiredAssets.size();
                                            QString prompt;

                                            if (assetCount > 1)
                                            {
                                                prompt = i18ncp(
                                                    "@info",
                                                    "One file will be protected as one set.",
                                                    "%1 files will be protected together as one set.",
                                                    assetCount);
                                            }

                                            if (warningCount > 0)
                                            {
                                                const QString warning = i18ncp(
                                                    "@info",
                                                    "One related copy may remain publicly accessible.",
                                                    "%1 related copies may remain publicly accessible.",
                                                    warningCount);

                                                if (prompt.isEmpty())
                                                {
                                                    prompt = warning;
                                                }
                                                else
                                                {
                                                    prompt += QLatin1String("\n\n");
                                                    prompt += warning;
                                                }
                                            }

                                            prompt += QLatin1String("\n\n");
                                            prompt += i18nc(
                                                "@info",
                                                "Protect this item?");

                                            QMessageBox message(
                                                QMessageBox::Warning,
                                                (assetCount > 1)
                                                    ? i18nc("@title:window",
                                                           "Confirm Protected File Set")
                                                    : i18nc("@title:window",
                                                           "Related Copies Detected"),
                                                prompt,
                                                QMessageBox::Yes | QMessageBox::Cancel,
                                                view.data());
                                            message.setDefaultButton(
                                                QMessageBox::Cancel);
                                            QStringList protectedPaths;
                                            QStringList aliasPaths;

                                            if (!preflight.bridge.items.isEmpty())
                                            {
                                                const auto& assets =
                                                    preflight.bridge.items.constFirst()
                                                        .inventory.requiredAssets;
                                                const auto& warnings =
                                                    preflight.bridge.items.constFirst()
                                                        .inventory.exposureWarnings;

                                                for (const auto& asset : assets)
                                                {
                                                    protectedPaths << QDir(
                                                        asset.location.root.absolutePath)
                                                        .filePath(
                                                            asset.location.relativePath);
                                                }

                                                for (const auto& warning : warnings)
                                                {
                                                    aliasPaths << QDir(
                                                        warning.alias.root.absolutePath)
                                                        .filePath(
                                                            warning.alias.relativePath);
                                                }
                                            }

                                            protectedPaths.sort(Qt::CaseSensitive);
                                            protectedPaths.removeDuplicates();
                                            aliasPaths.sort(Qt::CaseSensitive);
                                            aliasPaths.removeDuplicates();
                                            QStringList details;

                                            if (!protectedPaths.isEmpty())
                                            {
                                                details << i18nc(
                                                    "@info",
                                                    "Files to protect:\n%1",
                                                    protectedPaths.join(
                                                        QLatin1Char('\n')));
                                            }

                                            if (!aliasPaths.isEmpty())
                                            {
                                                details << i18nc(
                                                    "@info",
                                                    "Related copies that will remain public:\n%1",
                                                    aliasPaths.join(
                                                        QLatin1Char('\n')));
                                            }

                                            if (!details.isEmpty())
                                            {
                                                message.setInformativeText(
                                                    details.join(
                                                        QLatin1String("\n\n")));
                                            }

                                            accepted = (message.exec() ==
                                                        QMessageBox::Yes);

                                            if (progressGuard)
                                            {
                                                progressGuard->show();
                                            }
                                        }

                                        QMutexLocker locker(&state->mutex);

                                        if (!state->abandoned)
                                        {
                                            state->accepted = accepted;
                                        }

                                        state->completed = true;
                                        state->changed.wakeAll();
                                    },
                                    Qt::QueuedConnection);

                                if (!invoked)
                                {
                                    QObject::disconnect(
                                        state->shutdownConnection);
                                    return false;
                                }

                                QMutexLocker locker(&state->mutex);

                                while (!state->completed)
                                {
                                    if (QCoreApplication::closingDown() || !view)
                                    {
                                        state->abandoned = true;
                                        locker.unlock();
                                        QObject::disconnect(
                                            state->shutdownConnection);
                                        return false;
                                    }

                                    state->changed.wait(&state->mutex, 100);
                                }

                                const bool accepted = state->accepted;
                                locker.unlock();
                                QObject::disconnect(state->shutdownConnection);
                                return accepted;
                            });
                    }
                    catch (...)
                    {
                        result.status =
                            PrivacyStillItemTransactionStatus::RecoveryRequired;
                        result.detail.clear();
                    }

                    passwordSecret->fill(QChar());
                    return result;
                });
            watchPrivacyAction(
                future, progress,
                i18nc("@title:window", "Protect Failed"),
                i18nc("@info", "The item could not be protected."));
        }

        password.fill(QChar());
    }
}

void ItemIconView::slotShowGroupContextMenu(QContextMenuEvent* event,
                                            const QList<ItemInfo>& selectedInfos,
                                            ItemFilterModel* imageFilterModel)
{
    QList<qlonglong> selectedImageIDs;

    for (const ItemInfo& info : std::as_const(selectedInfos))
    {
        selectedImageIDs << info.id();
    }

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setItemFilterModel(imageFilterModel);
    cmhelper.addGroupActions(selectedImageIDs);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(slotCreateGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(slotCreateGroupByTimeFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByFilename()),
            this, SLOT(slotCreateGroupByFilenameFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTimelapse()),
            this, SLOT(slotCreateGroupByTimelapseFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(slotUngroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(slotRemoveSelectedFromGroup()));

    cmhelper.exec(event->globalPos());
}

} // namespace Digikam
