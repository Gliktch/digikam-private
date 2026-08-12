/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : a dialog to delete item.
 *
 * SPDX-FileCopyrightText: 2004      by Michael Pyne <michael dot pyne at kdemail dot net>
 * SPDX-FileCopyrightText: 2006      by Ian Monroe <ian at monroe dot nu>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "deletedialog.h"

// Qt includes

#include <QStackedWidget>
#include <QPainter>
#include <QLayout>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>
#include <QKeyEvent>
#include <QHeaderView>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QIcon>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "dxmlguiwindow.h"
#include "coredburl.h"

namespace Digikam
{

class Q_DECL_HIDDEN DeleteItem::Private
{
public:

    Private() = default;

public:

    bool hasThumb = false;

    QUrl url;
};

DeleteItem::DeleteItem(QTreeWidget* const parent, const QUrl& url)
    : QTreeWidgetItem(parent),
      d              (new Private)
{
    d->url = url;

    if (d->url.scheme() == QLatin1String("digikamalbums"))
    {
        if (CoreDbUrl(d->url).isAlbumUrl())
        {
            setThumb(QIcon::fromTheme(QLatin1String("folder"))
                                      .pixmap(parent->iconSize().width()));
        }
        else
        {
            setThumb(QIcon::fromTheme(QLatin1String("tag"))
                                      .pixmap(parent->iconSize().width()));
        }
    }
    else
    {
        setThumb(QIcon::fromTheme(QLatin1String("view-preview"))
                                  .pixmap(parent->iconSize().width(),
                 QIcon::Disabled), false);
    }

    setText(1, fileUrl());
}

DeleteItem::~DeleteItem()
{
    delete d;
}

bool DeleteItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

QUrl DeleteItem::url() const
{
    return d->url;
}

QString DeleteItem::fileUrl() const
{
    if      (d->url.isLocalFile())
    {
        return (d->url.toLocalFile());
    }
    else if (d->url.scheme() == QLatin1String("digikamalbums"))
    {
        return (CoreDbUrl(d->url).fileUrl().toLocalFile());
    }

    return (d->url.toDisplayString());
}

void DeleteItem::setThumb(const QPixmap& pix, bool hasThumb)
{
    int iconSize = treeWidget()->iconSize().width();
    QPixmap pixmap(iconSize + 2, iconSize + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()  / 2) - (pix.width()  / 2),
                 (pixmap.height() / 2) - (pix.height() / 2), pix);

    QIcon icon = QIcon(pixmap);

    //  We make sure the preview icon stays the same regardless of the role

    icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::Off);
    setIcon(0, icon);

    d->hasThumb = hasThumb;
}

//----------------------------------------------------------------------------

class Q_DECL_HIDDEN DeleteItemList::Private
{

public:

    Private() = default;

    const int            iconSize        = 64;

    ThumbnailLoadThread* thumbLoadThread = nullptr;
};

DeleteItemList::DeleteItemList(QWidget* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setIconSize(QSize(d->iconSize, d->iconSize));
    setColumnCount(2);
    setHeaderLabels(QStringList() << i18n("Thumb") << i18n("Path"));
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    setToolTip(i18n("List of items that are about to be deleted."));
    setWhatsThis(i18n("This is the list of items that are about to be deleted."));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

DeleteItemList::~DeleteItemList()
{
    delete d;
}

void DeleteItemList::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        DeleteItem* const item = dynamic_cast<DeleteItem*>(*it);

        if (item && (item->fileUrl() == desc.filePath))
        {
            if (!pix.isNull())
            {
                item->setThumb(pix.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio));
            }

            return;
        }

        ++it;
    }
}

void DeleteItemList::drawRow(QPainter* p,
                             const QStyleOptionViewItem& opt,
                             const QModelIndex& index) const
{
    const DeleteItem* const item = dynamic_cast<DeleteItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        d->thumbLoadThread->find(ThumbnailIdentifier(item->fileUrl()));
    }

    QTreeWidget::drawRow(p, opt, index);
}

//----------------------------------------------------------------------------

class Q_DECL_HIDDEN DeleteWidget::Private
{
public:

    Private() = default;

    QStackedWidget*              checkBoxStack      = nullptr;

    QLabel*                      warningIcon        = nullptr;
    QLabel*                      deleteText         = nullptr;
    QLabel*                      numFiles           = nullptr;

    QCheckBox*                   shouldDelete       = nullptr;
    QCheckBox*                   doNotShowAgain     = nullptr;

    QTreeWidget*                 fileList           = nullptr;

    DeleteDialogMode::ListMode   listMode           = DeleteDialogMode::Files;
    DeleteDialogMode::DeleteMode deleteMode         = DeleteDialogMode::UseTrash;
};

DeleteWidget::DeleteWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setObjectName(QLatin1String("DeleteDialogBase"));

    const int spacing  = layoutSpacing();

    d->checkBoxStack   = new QStackedWidget(this);
    QLabel* const logo = new QLabel(this);
    logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    d->warningIcon     = new QLabel(this);
    d->warningIcon->setWordWrap(false);

    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy.setHeightForWidth(d->warningIcon->sizePolicy().hasHeightForWidth());
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    d->warningIcon->setSizePolicy(sizePolicy);

    d->deleteText      = new QLabel(this);
    d->deleteText->setAlignment(Qt::AlignCenter);
    d->deleteText->setWordWrap(true);

    QHBoxLayout* const hbox = new QHBoxLayout();
    hbox->setSpacing(spacing);
    hbox->setContentsMargins(QMargins());
    hbox->addWidget(logo);
    hbox->addWidget(d->deleteText, 10);
    hbox->addWidget(d->warningIcon);

    d->fileList        = new DeleteItemList(this);
    d->numFiles        = new QLabel(this);
    d->numFiles->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    d->numFiles->setWordWrap(false);

    d->shouldDelete    = new QCheckBox(d->checkBoxStack);
    d->shouldDelete->setGeometry(QRect(0, 0, 542, 32));
    d->shouldDelete->setToolTip(i18n("If checked, files will be permanently removed instead of "
                                     "being placed in the Trash."));
    d->shouldDelete->setWhatsThis(i18n("<p>If this box is checked, items will be "
                                       "<b>permanently removed</b> instead of "
                                       "being placed in the Trash.</p>"
                                       "<p><em>Use this option with caution</em>: most filesystems "
                                       "are unable to "
                                       "undelete deleted items reliably.</p>"));
    d->shouldDelete->setText(i18n("&Delete items instead of moving them to the trash"));

    connect(d->shouldDelete, SIGNAL(toggled(bool)),
            this, SLOT(slotShouldDelete(bool)));

    d->doNotShowAgain       = new QCheckBox(d->checkBoxStack);
    d->doNotShowAgain->setGeometry(QRect(0, 0, 100, 30));
    d->doNotShowAgain->setText(i18n("Do not &ask again"));

    QVBoxLayout* const vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(spacing);
    vbox->addLayout(hbox);
    vbox->addWidget(d->fileList, 10);
    vbox->addWidget(d->numFiles);
    vbox->addWidget(d->checkBoxStack);

    d->checkBoxStack->addWidget(d->shouldDelete);
    d->checkBoxStack->addWidget(d->doNotShowAgain);
    d->checkBoxStack->setCurrentWidget(d->shouldDelete);

    bool deleteInstead      = !ApplicationSettings::instance()->getUseTrash();
    slotShouldDelete(deleteInstead);
    d->shouldDelete->setChecked(deleteInstead);
}

DeleteWidget::~DeleteWidget()
{
    delete d;
}

void DeleteWidget::setUrls(const QList<QUrl>& urls)
{
    d->fileList->clear();

    for (const QUrl& url : std::as_const(urls))
    {
        new DeleteItem(d->fileList, url);
    }

    updateText();
}

void DeleteWidget::slotShouldDelete(bool shouldDelete)
{
    setDeleteMode(shouldDelete ? DeleteDialogMode::DeletePermanently : DeleteDialogMode::UseTrash);
}

void DeleteWidget::setDeleteMode(DeleteDialogMode::DeleteMode deleteMode)
{
    d->deleteMode = deleteMode;
    updateText();
}

void DeleteWidget::setListMode(DeleteDialogMode::ListMode listMode)
{
    d->listMode = listMode;
    updateText();
}

void DeleteWidget::updateText()
{
    // set "do not ask again checkbox text

    if (d->deleteMode == DeleteDialogMode::DeletePermanently)
    {
        d->doNotShowAgain->setToolTip(i18n("If checked, this dialog will no longer be shown, and items will "
                                           "be directly and permanently deleted."));
        d->doNotShowAgain->setWhatsThis(i18n("If this box is checked, this dialog will no longer be shown, "
                                             "and items will be directly and permanently deleted."));
    }
    else
    {
        d->doNotShowAgain->setToolTip(i18n("If checked, this dialog will no longer be shown, and items will "
                                           "be directly moved to the Trash."));
        d->doNotShowAgain->setWhatsThis(i18n("If this box is checked, this dialog will no longer be shown, "
                                             "and items will be directly moved to the Trash."));
    }

    switch (d->listMode)
    {
        case DeleteDialogMode::Files:
        {
            // Delete files

            if (d->deleteMode == DeleteDialogMode::DeletePermanently)
            {
                d->deleteText->setText(i18n("These items will be <b>permanently "
                                            "deleted</b> from your hard disk."));
                d->warningIcon->setPixmap(QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(48));
            }
            else
            {
                d->deleteText->setText(i18n("These items will be moved to Trash."));
                d->warningIcon->setPixmap(QIcon::fromTheme(QLatin1String("user-trash")).pixmap(48));
                d->numFiles->setText(i18np("<b>1</b> item selected.", "<b>%1</b> items selected.",
                                           d->fileList->topLevelItemCount()));
            }

            break;
        }

        case DeleteDialogMode::Albums:
        {
            // Delete albums = folders

            if (d->deleteMode == DeleteDialogMode::DeletePermanently)
            {
                d->deleteText->setText(i18n("These albums will be <b>permanently "
                                            "deleted</b> from your hard disk."));
                d->warningIcon->setPixmap(QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(48));
            }
            else
            {
                d->deleteText->setText(i18n("These albums will be moved to Trash."));
                d->warningIcon->setPixmap(QIcon::fromTheme(QLatin1String("user-trash")).pixmap(48));
            }

            d->numFiles->setText(i18np("<b>1</b> album selected.", "<b>%1</b> albums selected.",
                                       d->fileList->topLevelItemCount()));
            break;
        }

        case DeleteDialogMode::Subalbums:
        {
            // As above, but display additional warning

            if (d->deleteMode == DeleteDialogMode::DeletePermanently)
            {
                d->deleteText->setText(i18n("<p>These albums will be <b>permanently "
                                            "deleted</b> from your hard disk.</p>"
                                            "<p>Note that <b>all subalbums</b> "
                                            "are included in this list and will "
                                            "be deleted permanently as well.</p>"));
                d->warningIcon->setPixmap(QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(48));
            }
            else
            {
                d->deleteText->setText(i18n("<p>These albums will be moved to Trash.</p>"
                                            "<p>Note that <b>all subalbums</b> "
                                            "are included in this list and will "
                                            "be moved to Trash as well.</p>"));
                d->warningIcon->setPixmap(QIcon::fromTheme(QLatin1String("user-trash")).pixmap(48));
            }

            d->numFiles->setText(i18np("<b>1</b> album selected.", "<b>%1</b> albums selected.",
                                       d->fileList->topLevelItemCount()));
            break;
        }
    }
}

//----------------------------------------------------------------------------

class Q_DECL_HIDDEN DeleteDialog::Private
{
public:

    Private() = default;

    bool              saveShouldDeleteUserPreference = true;
    bool              saveDoNotShowAgainTrash        = false;
    bool              saveDoNotShowAgainPermanent    = false;

    DeleteWidget*     page                           = nullptr;

    QDialogButtonBox* buttons                        = nullptr;
};

DeleteDialog::DeleteDialog(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setModal(true);

    d->buttons             = new QDialogButtonBox(QDialogButtonBox::Apply  |
                                                  QDialogButtonBox::Cancel |
                                                  QDialogButtonBox::Help,
                                                  this);

    d->page                = new DeleteWidget(this);
    d->page->setMinimumSize(400, 300);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
    adjustSize();

    slotShouldDelete(shouldDelete());

    connect(d->page->d->shouldDelete, SIGNAL(toggled(bool)),
            this, SLOT(slotShouldDelete(bool)));

    connect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(slotUser1Clicked()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

DeleteDialog::~DeleteDialog()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Delete Dialog"));

    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    delete d;
}

bool DeleteDialog::confirmDeleteList(const QList<QUrl>& condemnedFiles,
                                     DeleteDialogMode::ListMode listMode,
                                     DeleteDialogMode::DeleteMode deleteMode)
{
    setUrls(condemnedFiles);
    presetDeleteMode(deleteMode);
    setListMode(listMode);

    if      (deleteMode == DeleteDialogMode::NoChoiceTrash)
    {
        if (!ApplicationSettings::instance()->getShowTrashDeleteDialog())
        {
            return true;
        }
    }
    else if (deleteMode == DeleteDialogMode::NoChoiceDeletePermanently)
    {
        if (!ApplicationSettings::instance()->getShowPermanentDeleteDialog())
        {
            return true;
        }
    }

    return (exec() == QDialog::Accepted);
}

void DeleteDialog::setUrls(const QList<QUrl>& urls)
{
    d->page->setUrls(urls);
}

void DeleteDialog::slotUser1Clicked()
{
    // Save user's preference

    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (d->saveShouldDeleteUserPreference)
    {
        settings->setUseTrash(!shouldDelete());
    }

    if (d->saveDoNotShowAgainTrash)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "setShowTrashDeleteDialog"
                                     << !d->page->d->doNotShowAgain->isChecked();

        settings->setShowTrashDeleteDialog(!d->page->d->doNotShowAgain->isChecked());
    }

    if (d->saveDoNotShowAgainPermanent)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "setShowPermanentDeleteDialog"
                                     << !d->page->d->doNotShowAgain->isChecked();

        settings->setShowPermanentDeleteDialog(!d->page->d->doNotShowAgain->isChecked());
    }

    settings->saveSettings();

    QDialog::accept();
}

bool DeleteDialog::shouldDelete() const
{
    return d->page->d->shouldDelete->isChecked();
}

void DeleteDialog::slotShouldDelete(bool shouldDelete)
{
    // This is called once from constructor, and then when the user changed the checkbox state.
    // In that case, save the user's preference.

    d->saveShouldDeleteUserPreference = true;

    d->buttons->button(QDialogButtonBox::Apply)->setText(shouldDelete ? i18n("&Delete")
                                                                      : i18n("&Move to Trash"));
    d->buttons->button(QDialogButtonBox::Apply)->setIcon(shouldDelete ? QIcon::fromTheme(QLatin1String("edit-delete"))
                                                                      : QIcon::fromTheme(QLatin1String("albumfolder-user-trash")));  // In red - bug 513774

    d->buttons->button(QDialogButtonBox::Apply)->setDefault(!shouldDelete);
    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(shouldDelete);
}

void DeleteDialog::presetDeleteMode(DeleteDialogMode::DeleteMode mode)
{
    switch (mode)
    {
        case DeleteDialogMode::NoChoiceTrash:
        {
            // access the widget directly, signals will be fired to DeleteDialog and DeleteWidget

            d->page->d->shouldDelete->setChecked(false);
            d->page->d->checkBoxStack->setCurrentWidget(d->page->d->doNotShowAgain);
            d->saveDoNotShowAgainTrash = true;
            break;
        }

        case DeleteDialogMode::NoChoiceDeletePermanently:
        {
            d->page->d->shouldDelete->setChecked(true);
            d->page->d->checkBoxStack->setCurrentWidget(d->page->d->doNotShowAgain);
            d->saveDoNotShowAgainPermanent = true;
/*
            d->page->d->checkBoxStack->hide();
*/
            break;
        }

        case DeleteDialogMode::UserPreference:
        {
            break;
        }

        case DeleteDialogMode::UseTrash:
        case DeleteDialogMode::DeletePermanently:
        {
            // toggles signals which do the rest

            d->page->d->shouldDelete->setChecked(mode == DeleteDialogMode::DeletePermanently);

            // the preference set by this preset method will be ignored
            // for the next DeleteDialog instance and not stored as user preference.
            // Only if the user once changes this value, it will be taken as user preference.

            d->saveShouldDeleteUserPreference = false;
            break;
        }
    }
}

void DeleteDialog::setListMode(DeleteDialogMode::ListMode mode)
{
    d->page->setListMode(mode);

    switch (mode)
    {
        case DeleteDialogMode::Files:
        {
            setWindowTitle(i18nc("@title:window", "About to Delete Selected Items"));
            break;
        }

        case DeleteDialogMode::Albums:
        case DeleteDialogMode::Subalbums:
        {
            setWindowTitle(i18nc("@title:window", "About to Delete Selected Albums"));
            break;
        }
    }
}

void DeleteDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->modifiers() == 0)
    {
        if ((e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return))
        {
            if      (d->buttons->button(QDialogButtonBox::Apply)->hasFocus())
            {
                e->accept();
                d->buttons->button(QDialogButtonBox::Apply)->animateClick();
                return;
            }
            else if (d->buttons->button(QDialogButtonBox::Cancel)->hasFocus())
            {
                e->accept();
                d->buttons->button(QDialogButtonBox::Cancel)->animateClick();
                return;
            }
        }
    }

    QDialog::keyPressEvent(e);
}

void DeleteDialog::showEvent(QShowEvent* e)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Delete Dialog"));

    DXmlGuiWindow::setGoodDefaultWindowSize(windowHandle());
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    QDialog::showEvent(e);
}

void DeleteDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("main_window"),
                            QLatin1String("image_view"),
                            QLatin1String("deleting-photograph"));
}

} // namespace Digikam

#include "moc_deletedialog.cpp"
