/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view favorite item edit dialog
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotostackviewfavoriteitemdlg.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QApplication>
#include <QStyle>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QLocale>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QMimeDatabase>
#include <QComboBox>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

#ifdef HAVE_KICONTHEMES
#   include <kicondialog.h>
#endif

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "drawdecoder.h"
#include "dtextedit.h"
#include "dxmlguiwindow.h"
#include "showfotostackviewfavoritelist.h"
#include "showfotostackviewfavoriteitem.h"
#include "showfotostackviewlist.h"
#include "showfotoitemsortsettings.h"
#include "itempropertiestab.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoStackViewFavoriteItemDlg::Private
{
public:

    Private() = default;

public:

    bool                           create           = false;

    DAdjustableLabel*              hierarchyLabel   = nullptr;

    QString                        icon             = QLatin1String("folder-favorites");
    QString                        originalName;

    QPushButton*                   iconButton       = nullptr;
    QPushButton*                   resetIconButton  = nullptr;
    QPushButton*                   dateLowButton    = nullptr;
    QPushButton*                   dateAvgButton    = nullptr;
    QPushButton*                   dateHighButton   = nullptr;

    QDialogButtonBox*              buttons          = nullptr;

    DTextEdit*                     nameEdit         = nullptr;
    DTextEdit*                     descEdit         = nullptr;
    QComboBox*                     favoriteTypeBox  = nullptr;
    QDateTimeEdit*                 dateEdit         = nullptr;
    DItemsList*                    urlsEdit         = nullptr;
    QLabel*                        descLabel        = nullptr;
    QLabel*                        nbImagesLabel    = nullptr;
    QLabel*                        dateLabel        = nullptr;
    QLabel*                        iconTextLabel    = nullptr;
    QLabel*                        urlsLabel        = nullptr;
    DAdjustableLabel*              helpLabel        = nullptr;
    ShowfotoStackViewFavoriteList* list             = nullptr;
    ShowfotoStackViewFavoriteItem* pitem            = nullptr;
};

static Qt::SortOrder                        s_sortOrder = Qt::AscendingOrder;
static ShowfotoStackViewList::StackViewRole s_sortRole  = ShowfotoStackViewList::FileName;

ShowfotoStackViewFavoriteItemDlg::ShowfotoStackViewFavoriteItemDlg(ShowfotoStackViewFavoriteList* const list,
                                                                   bool create)
    : QDialog(list),
      d      (new Private)
{
    setModal(true);
    setObjectName(QLatin1String("ShowfotoStackViewFavoriteItemDlg"));

    d->create               = create;
    d->list                 = list;
    d->buttons              = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);


    QWidget* const page     = new QWidget(this);
    QGridLayout* const grid = new QGridLayout(page);

    // --------------------------------------------------------

    QLabel* const nameLabel = new QLabel(page);
    nameLabel->setText(i18nc("@label: favorite item title properties", "&Name:"));

    d->nameEdit             = new DTextEdit(page);
    d->nameEdit->setLinesVisible(1);
    d->nameEdit->setPlaceholderText(i18nc("@info", "Enter favorite entry name here..."));
    d->nameEdit->setWhatsThis(i18nc("@info", "The favorite item name which must be unique and not empty"));
    nameLabel->setBuddy(d->nameEdit);

    // --------------------------------------------------------

    QLabel* const hierLabel = new QLabel(page);
    hierLabel->setText(i18nc("@label: favorite item hierarchy properties", "Hierarchy:"));

    d->hierarchyLabel       = new DAdjustableLabel(page);
    d->hierarchyLabel->setWhatsThis(i18nc("@info", "The favorite entry hierarchy which must be unique in tree-view"));

    // --------------------------------------------------------

    QLabel* const typeLabel = new QLabel(page);
    typeLabel->setText(i18nc("@label: favorite entry type properties", "Type:"));

    d->favoriteTypeBox      = new QComboBox(page);
    d->favoriteTypeBox->addItem(i18nc("@item:inlistbox", "Favorite Folder"), ShowfotoStackViewFavoriteItem::FavoriteFolder);
    d->favoriteTypeBox->addItem(i18nc("@item:inlistbox", "Favorite Item"),   ShowfotoStackViewFavoriteItem::FavoriteItem);
    d->favoriteTypeBox->setWhatsThis(i18nc("@info",
                                           "A \"Favorite Item\" is a hierarchy entry hosting advanced properties "
                                           "as date, icon, description, and images list. "
                                           "A \"Favorite Folder\" is a simple entry in the hierarchy without extra property."));
    typeLabel->setBuddy(d->favoriteTypeBox);

    // --------------------------------------------------------

    d->descLabel            = new QLabel(page);
    d->descLabel->setText(i18nc("@label: favorite item caption properties", "&Description:"));

    d->descEdit             = new DTextEdit(page);
    d->descEdit->setLinesVisible(1);
    d->descEdit->setPlaceholderText(i18nc("@info", "Enter favorite item description here..."));
    d->descLabel->setBuddy(d->descEdit);

    // --------------------------------------------------------

    d->dateLabel            = new QLabel(page);
    d->dateLabel->setText(i18nc("@label: favorite item date properties", "&Created:"));

    d->dateEdit             = new QDateTimeEdit(QDate::currentDate(), page);
    d->dateEdit->setMinimumDate(QDate(1970, 1, 1));
    d->dateEdit->setMaximumDate(QDate::currentDate().addDays(365));
    d->dateEdit->setDisplayFormat(QLatin1String("yyyy.MM.dd"));
    d->dateEdit->setCalendarPopup(true);
    d->dateEdit->setWhatsThis(i18nc("@info", "Select favorite item date. By default, day of item creation is assigned."));
    d->dateLabel->setBuddy(d->dateEdit);

    DHBox* const buttonRow  = new DHBox(page);
    d->dateLowButton        = new QPushButton(i18nc("@action: Selects the date of the oldest image", "&Oldest"),  buttonRow);
    d->dateLowButton->setWhatsThis(i18nc("@info", "Use this button to select the date of the oldest image from the list."));
    d->dateAvgButton        = new QPushButton(i18nc("@action: Calculates the average date",          "&Average"), buttonRow);
    d->dateAvgButton->setWhatsThis(i18nc("@info", "Use this button to calculate the average date of images from the list."));
    d->dateHighButton       = new QPushButton(i18nc("@action: Selects the date of the newest image", "Newest"),   buttonRow);
    d->dateHighButton->setWhatsThis(i18nc("@info", "Use this button to select the date of the newest image from the list."));

    // --------------------------------------------------------

    d->iconTextLabel        = new QLabel(page);
    d->iconTextLabel->setText(i18nc("@label", "&Icon:"));

    d->iconButton           = new QPushButton(page);
    d->iconButton->setFixedSize(40, 40);
    d->iconButton->setIcon(QIcon::fromTheme(d->icon));
    d->iconButton->setWhatsThis(i18nc("@info", "Select here the icon to use for this favorite item in tree-view."));
    d->iconTextLabel->setBuddy(d->iconButton);

    d->resetIconButton      = new QPushButton(QIcon::fromTheme(QLatin1String("view-refresh")),
                                              i18nc("@action:button", "Reset"), page);
    d->resetIconButton->setWhatsThis(i18nc("@info", "Assign favorite item icon to default in tree-view."));

#ifndef HAVE_KICONTHEMES

    d->iconTextLabel->hide();
    d->iconButton->hide();
    d->resetIconButton->hide();

#endif

    // --------------------------------------------------------

    d->urlsLabel            = new QLabel(page);
    d->urlsLabel->setText(i18nc("@label: favorites item file properties", "&Items:"));

    d->nbImagesLabel        = new QLabel(page);
    QFont fnt;
    fnt.setItalic(true);
    d->nbImagesLabel->setFont(fnt);

    d->urlsEdit             = new DItemsList(page);
    d->urlsEdit->setIsLessThanHandler(itemIsLessThanHandler);
    d->urlsEdit->setIconSize(d->list->iconSize().width());
    d->urlsEdit->setAllowRAW(true);
    d->urlsEdit->setAllowDuplicate(false);
    d->urlsEdit->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    d->urlsEdit->setControlButtons(DItemsList::Add | DItemsList::Remove | DItemsList::Clear);
    d->urlsEdit->listView()->setColumn(DItemsListView::Filename, i18nc("@title:column file name",      "Name"), true);
    d->urlsEdit->listView()->setColumn(DItemsListView::User1,    i18nc("@title:column file size",      "Size"), true);
    d->urlsEdit->listView()->setColumn(DItemsListView::User2,    i18nc("@title:column file type-mime", "Type"), true);
    d->urlsEdit->listView()->setColumn(DItemsListView::User3,    i18nc("@title:column file date",      "Date"), true);
    d->urlsEdit->listView()->setColumn(DItemsListView::User4,    i18nc("@title:column file path",      "Path"), true);

    d->urlsEdit->setWhatsThis(i18nc("@info", "This is the list of files hosted by this favorite item. "
                                             "The current selected file from this list will be automatically "
                                             "shown in editor when favorite is open. If none is selected, "
                                             "first one from the list will be displayed."));
    d->urlsLabel->setBuddy(d->urlsEdit);

    // --------------------------------------------------------

    d->helpLabel = new DAdjustableLabel(page);
    QPalette pal = d->helpLabel->palette();
    pal.setColor(QPalette::WindowText, Qt::red);
    d->helpLabel->setPalette(pal);

    // --------------------------------------------------------

    grid->addWidget(nameLabel,          0,  0, 1, 1);
    grid->addWidget(d->nameEdit,        0,  1, 1, 4);
    grid->addWidget(hierLabel,          1,  0, 1, 1);
    grid->addWidget(d->hierarchyLabel,  1,  1, 1, 4);
    grid->addWidget(typeLabel,          2,  0, 1, 1);
    grid->addWidget(d->favoriteTypeBox, 2,  1, 1, 4);
    grid->addWidget(d->descLabel,       3,  0, 1, 1);
    grid->addWidget(d->descEdit,        3,  1, 1, 4);
    grid->addWidget(d->dateLabel,       4,  0, 1, 1);
    grid->addWidget(d->dateEdit,        4,  1, 1, 3);
    grid->addWidget(buttonRow,          4,  3, 1, 1);
    grid->addWidget(d->iconTextLabel,   5,  0, 1, 1);
    grid->addWidget(d->iconButton,      5,  1, 1, 1);
    grid->addWidget(d->resetIconButton, 5,  2, 1, 1);
    grid->addWidget(d->urlsEdit,        6,  1, 4, 4);
    grid->addWidget(d->urlsLabel,       7,  0, 1, 1);
    grid->addWidget(d->nbImagesLabel,   8,  0, 1, 1);
    grid->addWidget(d->helpLabel,       10, 0, 1, 5);
    grid->setRowStretch(9, 10);
    grid->setColumnStretch(4, 10);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // --------------------------------------------------------

    connect(d->nameEdit, SIGNAL(textChanged()),
            this, SLOT(slotModified()));

    connect(d->descEdit, SIGNAL(textChanged()),
            this, SLOT(slotModified()));

    connect(d->favoriteTypeBox, SIGNAL(activated(int)),
            this, SLOT(slotTypeActivated()));

    connect(d->urlsEdit, SIGNAL(signalImageListChanged()),
            this, SLOT(slotModified()));

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));

    connect(d->urlsEdit, SIGNAL(signalImageListChanged()),
            this, SLOT(slotUpdateMetadata()));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    connect(d->dateLowButton, SIGNAL(clicked()),
            this, SLOT(slotDateLowButtonClicked()));

    connect(d->dateAvgButton, SIGNAL(clicked()),
            this, SLOT(slotDateAverageButtonClicked()));

    connect(d->dateHighButton, SIGNAL(clicked()),
            this, SLOT(slotDateHighButtonClicked()));
}

ShowfotoStackViewFavoriteItemDlg::~ShowfotoStackViewFavoriteItemDlg()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(objectName());

    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    delete d;
}

bool ShowfotoStackViewFavoriteItemDlg::canAccept() const
{
    if (favoriteType() == ShowfotoStackViewFavoriteItem::FavoriteItem)
    {
        bool b1 = name().isEmpty();
        bool b2 = urls().isEmpty();
        bool b3 = false;     // If dialog in edit mode, the original name can be accepted.

        if (d->create || (name() != d->originalName))
        {
            b3 = d->list->findFavoriteByHierarchy(ShowfotoStackViewFavoriteItem::hierarchyFromParent(name(), d->pitem));
        }

        bool b4 = (!b1 && !b2 && !b3);

        if (b4)
        {
            d->helpLabel->setAdjustedText(QString());
        }
        else
        {
            if      (b1)
            {
                d->helpLabel->setAdjustedText(i18nc("@label", "Note: favorite item name cannot be empty!"));
            }
            else if (b2)
            {
                d->helpLabel->setAdjustedText(i18nc("@label", "Note: favorite item files list cannot be empty!"));
            }
            else if (b3)
            {
                d->helpLabel->setAdjustedText(i18nc("@label", "Note: favorite item name already exists in hierarchy!"));
            }
        }

        return (
                !b1 &&
                !b2 &&
                !b3
               );
    }
    else
    {
        bool b1 = name().isEmpty();
        bool b3 = false;     // If dialog in edit mode, the original name can be accepted.

        if (d->create || (name() != d->originalName))
        {
            b3 = d->list->findFavoriteByHierarchy(ShowfotoStackViewFavoriteItem::hierarchyFromParent(name(), d->pitem));
        }

        bool b4 = (!b1 && !b3);

        if (b4)
        {
            d->helpLabel->setAdjustedText(QString());
        }
        else
        {
            if      (b1)
            {
                d->helpLabel->setAdjustedText(i18nc("@label", "Note: favorite folder name cannot be empty!"));
            }
            else if (b3)
            {
                d->helpLabel->setAdjustedText(i18nc("@label", "Note: favorite folder name already exists in hierarchy!"));
            }
        }

        return (
                !b1 &&
                !b3
               );
    }
}

void ShowfotoStackViewFavoriteItemDlg::slotModified()
{
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(canAccept());
    d->hierarchyLabel->setAdjustedText(ShowfotoStackViewFavoriteItem::hierarchyFromParent(name(), d->pitem));
    int numberOfImages = d->urlsEdit->imageUrls().count();
    d->nbImagesLabel->setText(i18ncp("@info", "%1 file", "%1 files", numberOfImages));
    d->dateLowButton->setEnabled(numberOfImages > 0);
    d->dateAvgButton->setEnabled(numberOfImages > 0);
    d->dateHighButton->setEnabled(numberOfImages > 0);
}

QString ShowfotoStackViewFavoriteItemDlg::name() const
{
    return d->nameEdit->text();
}

int ShowfotoStackViewFavoriteItemDlg::favoriteType() const
{
    return d->favoriteTypeBox->currentIndex();
}

QString ShowfotoStackViewFavoriteItemDlg::description() const
{
    return d->descEdit->text();
}

QDate ShowfotoStackViewFavoriteItemDlg::date() const
{
    return d->dateEdit->date();
}

QString ShowfotoStackViewFavoriteItemDlg::icon() const
{
    return d->icon;
}

QList<QUrl> ShowfotoStackViewFavoriteItemDlg::urls() const
{
    return d->urlsEdit->imageUrls();
}

QUrl ShowfotoStackViewFavoriteItemDlg::currentUrl() const
{
    return d->urlsEdit->getCurrentUrl();
}

void ShowfotoStackViewFavoriteItemDlg::setName(const QString& name)
{
    d->nameEdit->setText(name);
    d->originalName = name;
}

void ShowfotoStackViewFavoriteItemDlg::setFavoriteType(int favoriteType)
{
    d->favoriteTypeBox->setCurrentIndex(favoriteType);
    slotTypeActivated();
}

void ShowfotoStackViewFavoriteItemDlg::setDescription(const QString& desc)
{
    d->descEdit->setText(desc);
}

void ShowfotoStackViewFavoriteItemDlg::setDate(const QDate& date)
{
    d->dateEdit->setDate(date);
}

void ShowfotoStackViewFavoriteItemDlg::setIcon(const QString& icon)
{
    d->icon = icon;
}

void ShowfotoStackViewFavoriteItemDlg::setUrls(const QList<QUrl>& urls)
{
    d->urlsEdit->slotAddImages(urls);
}

void ShowfotoStackViewFavoriteItemDlg::setCurrentUrl(const QUrl& url)
{
    d->urlsEdit->setCurrentUrl(url);
}

void ShowfotoStackViewFavoriteItemDlg::setIconSize(int size)
{
    d->urlsEdit->setIconSize(size);
}

void ShowfotoStackViewFavoriteItemDlg::setSortOrder(int order)
{
    s_sortOrder = (Qt::SortOrder)order;
    d->urlsEdit->listView()->sortItems((int)s_sortRole + (int)DItemsListView::Filename, s_sortOrder);
}

void ShowfotoStackViewFavoriteItemDlg::setSortRole(int role)
{
    s_sortRole = (ShowfotoStackViewList::StackViewRole)role;
    d->urlsEdit->listView()->sortItems((int)s_sortRole + (int)DItemsListView::Filename, s_sortOrder);
}

void ShowfotoStackViewFavoriteItemDlg::setParentItem(ShowfotoStackViewFavoriteItem* const pitem)
{
    d->pitem = pitem;
}

void ShowfotoStackViewFavoriteItemDlg::slotIconResetClicked()
{
    d->icon = QLatin1String("folder-favorites");
    d->iconButton->setIcon(QIcon::fromTheme(d->icon));
}

void ShowfotoStackViewFavoriteItemDlg::slotIconChanged()
{

#ifdef HAVE_KICONTHEMES

    QPointer<KIconDialog> dlg = new KIconDialog(this);
    dlg->setup(KIconLoader::NoGroup, KIconLoader::Application, false, 20, false, false, false);
    QString ico               = dlg->openDialog();
    delete dlg;

    if (ico.isEmpty() || (ico == d->icon))
    {
        return;
    }

    d->icon                   = ico;
    d->iconButton->setIcon(QIcon::fromTheme(d->icon));

#endif

}

QList<QDate> ShowfotoStackViewFavoriteItemDlg::getItemDates() const
{
    QList<QDate> dates;
    QDateTime dt;
    QTreeWidgetItemIterator it(d->urlsEdit->listView());

    while (*it)
    {
        dt = (*it)->data(DItemsListView::User3, Qt::UserRole).toDateTime();

        if (dt.isValid())
        {
            dates << dt.date();
        }

        ++it;
    }

    return dates;
}

void ShowfotoStackViewFavoriteItemDlg::slotUpdateMetadata()
{
    QTreeWidgetItemIterator it(d->urlsEdit->listView());

    while (*it)
    {
        DItemsListViewItem* const lvItem = dynamic_cast<DItemsListViewItem*>(*it);

        if (lvItem)
        {
            QFileInfo inf(lvItem->url().toLocalFile());
            ShowfotoItemInfo iteminf = ShowfotoItemInfo::itemInfoFromFile(inf);

            QString str              = ItemPropertiesTab::humanReadableBytesCount(iteminf.size);
            lvItem->setText(DItemsListView::User1, str);
            lvItem->setData(DItemsListView::User1, Qt::UserRole, iteminf.size);

            QFileInfo fileInfo(iteminf.name);
            QString rawFilesExt      = DRawDecoder::rawFiles();
            QString ext              = fileInfo.suffix().toUpper();

            if (!ext.isEmpty() && rawFilesExt.toUpper().contains(ext))
            {
                lvItem->setText(DItemsListView::User2, i18nc("@info: item properties", "RAW Image"));
            }
            else
            {
                lvItem->setText(DItemsListView::User2, QMimeDatabase().mimeTypeForFile(fileInfo).comment());
            }

            QDateTime dt             = (iteminf.ctime.isValid() ? iteminf.ctime : iteminf.dtime);
            str                      = QLocale().toString(dt, QLocale::ShortFormat);
            lvItem->setText(DItemsListView::User3, str);
            lvItem->setData(DItemsListView::User3, Qt::UserRole, dt);

            lvItem->setText(DItemsListView::User4, inf.absolutePath());
        }

        ++it;
    }
}

bool ShowfotoStackViewFavoriteItemDlg::favoriteItemDialog(ShowfotoStackViewFavoriteList* const list,
                                                          QString& name,
                                                          int& favoriteType,
                                                          QString& desc,
                                                          QDate& date,
                                                          QString& icon,
                                                          QList<QUrl>& urls,
                                                          QUrl& current,
                                                          int iconSize,
                                                          int sortOrder,
                                                          int sortRole,
                                                          ShowfotoStackViewFavoriteItem* const pitem,
                                                          bool create)
{
    QPointer<ShowfotoStackViewFavoriteItemDlg> dlg = new ShowfotoStackViewFavoriteItemDlg(list, create);
    dlg->setParentItem(pitem);
    dlg->setName(name);
    dlg->setDescription(desc);
    dlg->setDate(date);
    dlg->setIcon(icon);
    dlg->setUrls(urls);
    dlg->setCurrentUrl(current);
    dlg->setIconSize(iconSize);
    dlg->setSortOrder(sortOrder);
    dlg->setSortRole(sortRole);
    dlg->setFavoriteType(favoriteType);

    bool valRet = dlg->exec();

    if (valRet == QDialog::Accepted)
    {
        name         = dlg->name();
        desc         = dlg->description();
        date         = dlg->date();
        icon         = dlg->icon();
        urls         = dlg->urls();
        current      = dlg->currentUrl();
        favoriteType = dlg->favoriteType();
    }

    delete dlg;

    return valRet;
}

void ShowfotoStackViewFavoriteItemDlg::slotTypeActivated()
{
    bool b = (favoriteType() == ShowfotoStackViewFavoriteItem::FavoriteItem);

    d->descLabel->setEnabled(b);
    d->descEdit->setEnabled(b);
    d->dateLabel->setEnabled(b);
    d->dateEdit->setEnabled(b);
    d->iconTextLabel->setEnabled(b);
    d->iconButton->setEnabled(b);
    d->resetIconButton->setEnabled(b);
    d->urlsEdit->setEnabled(b);
    d->urlsLabel->setEnabled(b);
    d->nbImagesLabel->setEnabled(b);
    d->dateLowButton->setEnabled(b);
    d->dateAvgButton->setEnabled(b);
    d->dateHighButton->setEnabled(b);

    if (b)
    {
        if (d->create)
        {
            setWindowTitle(i18nc("@title:window", "New Favorite Item"));
        }
        else
        {
            setWindowTitle(i18nc("@title:window", "Edit Favorite Item"));
        }
    }
    else
    {
        if (d->create)
        {
            setWindowTitle(i18nc("@title:window", "New Favorite Folder"));
        }
        else
        {
            setWindowTitle(i18nc("@title:window", "Edit Favorite Folder"));
        }
    }

    slotModified();
}

void ShowfotoStackViewFavoriteItemDlg::slotDateLowButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QDate lowDate = getItemDates().constFirst();

    if (lowDate.isValid())
    {
        d->dateEdit->setDate(lowDate);
    }

    setCursor(Qt::ArrowCursor);
}

void ShowfotoStackViewFavoriteItemDlg::slotDateHighButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QDate highDate = getItemDates().constLast();

    if (highDate.isValid())
    {
        d->dateEdit->setDate(highDate);
    }

    setCursor(Qt::ArrowCursor);
}

void ShowfotoStackViewFavoriteItemDlg::slotDateAverageButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QList<QDate> dates = getItemDates();
    qint64 julianDays  = 0;

    for (const QDate& dt : std::as_const(dates))
    {
        julianDays += dt.toJulianDay();
    }

    QDate avDate       = QDate::fromJulianDay(julianDays / dates.size());

    setCursor(Qt::ArrowCursor);

    if (avDate.isValid())
    {
        d->dateEdit->setDate(avDate);
    }
    else
    {
        QMessageBox::critical(this, i18nc("@title:window, favorite item properties", "Could not Calculate Average"),
                                    i18nc("@info: favorite item properties",  "Could not calculate date average for this favorite item."));
    }
}

bool ShowfotoStackViewFavoriteItemDlg::itemIsLessThanHandler(const QTreeWidgetItem* current, const QTreeWidgetItem& other)
{
    int result = 0;

    switch ((int)s_sortRole + (int)DItemsListView::Filename)
    {
        case DItemsListView::User1:     // Size
        {
            result = (ShowfotoItemSortSettings::compareByOrder(current->data(DItemsListView::User1, Qt::UserRole).toInt(),
                                                               other.data(DItemsListView::User1, Qt::UserRole).toInt(),
                                                               s_sortOrder));
            break;
        }

        case DItemsListView::User2:     // Mime-type
        {
            result = (ShowfotoItemSortSettings::naturalCompare(current->text(DItemsListView::User2),
                                                               other.text(DItemsListView::User2),
                                                               s_sortOrder,
                                                               Qt::CaseSensitive));
            break;
        }

        case DItemsListView::User3:     // Date
        {
            result = (ShowfotoItemSortSettings::compareByOrder(current->data(DItemsListView::User3, Qt::UserRole).toDateTime(),
                                                               other.data(DItemsListView::User3, Qt::UserRole).toDateTime(),
                                                               s_sortOrder));
            break;
        }

        default:                        // Name
        {
            result = (ShowfotoItemSortSettings::naturalCompare(current->text(DItemsListView::Filename),
                                                               other.text(DItemsListView::Filename),
                                                               s_sortOrder,
                                                               Qt::CaseSensitive));
            break;
        }
    }

    return (result < 0);
}

void ShowfotoStackViewFavoriteItemDlg::slotHelp()
{
    Digikam::openOnlineDocumentation(QLatin1String("showfoto_editor"),
                                     QLatin1String("showfoto_leftsidebar"),
                                     QLatin1String("showfoto-stacktab"));
}

void ShowfotoStackViewFavoriteItemDlg::showEvent(QShowEvent* e)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(objectName());

    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    d->nameEdit->setFocus();

    QDialog::showEvent(e);
}

} // namespace ShowFoto

#include "moc_showfotostackviewfavoriteitemdlg.cpp"
