/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : A widget to select Physical or virtual albums with combo-box
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumselectors.h"

// Qt includes

#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummodel.h"
#include "albummanager.h"
#include "albumselectcombobox.h"
#include "albumtreeview.h"
#include "tagtreeview.h"
#include "searchutilities.h"
#include "thememanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN ModelClearButton : public AnimatedClearButton       // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

public:

    explicit ModelClearButton(AbstractCheckableAlbumModel* const model)
    {
        setClearButtonPixmap();
        stayVisibleWhenAnimatedOut(true);

        connect(this, SIGNAL(clicked()),
                model, SLOT(resetAllCheckedAlbums()));

        connect(ThemeManager::instance(), &ThemeManager::signalThemeChanged,
                this, [this]()
            {
                setClearButtonPixmap();
            }
        );
    }

    void setClearButtonPixmap()
    {
        setPixmap(QIcon::fromTheme(qApp->isLeftToRight() ? QLatin1String("edit-clear-locationbar-rtl")
                                                         : QLatin1String("edit-clear-locationbar-ltr"))
                                                               .pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
    }
};

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN AlbumSelectors::Private
{
public:

    Private() = default;

public:

    const QString configUseWholeAlbumsEntry                 = QLatin1String("UseWholeAlbumsEntry");
    const QString configUseWholeTagsEntry                   = QLatin1String("UseWholeTagsEntry");
    const QString configAlbumTypeEntry                      = QLatin1String("AlbumTypeEntry");

    QString                      configName;

    AlbumTreeViewSelectComboBox* albumSelectCB              = nullptr;
    TagTreeViewSelectComboBox*   tagSelectCB                = nullptr;
    ModelClearButton*            albumClearButton           = nullptr;
    ModelClearButton*            tagClearButton             = nullptr;

    QToolButton*                 recursiveSelectionAlbum    = nullptr;
    QToolButton*                 recursiveSelectionTags     = nullptr;

    QCheckBox*                   wholeAlbums                = nullptr;
    QCheckBox*                   wholeTags                  = nullptr;

    QTabWidget*                  tabWidget                  = nullptr;
    QWidget*                     albumWidget                = nullptr;
    QWidget*                     tagWidget                  = nullptr;

    AlbumType                    selectionMode              = All;

    bool                         allowRecursive             = false;
};

AlbumSelectors::AlbumSelectors(const QString& label,
                               const QString& configName,
                               QWidget* const parent,
                               AlbumType albumType,
                               bool allowRecursive)
    : QWidget(parent),
      d      (new Private)
{
    d->allowRecursive             = allowRecursive;
    d->configName                 = configName;
    setObjectName(d->configName);

    d->selectionMode              = albumType;

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);

    if (!label.isEmpty())
    {
        mainLayout->addWidget(new QLabel(label));
    }

    switch (d->selectionMode)
    {
        case All:
        {
            d->tabWidget = new QTabWidget(this);

            initAlbumWidget();
            d->tabWidget->insertTab(PhysAlbum, d->albumWidget, i18nc("@title", "Albums (All)"));

            initTagWidget();
            d->tabWidget->insertTab(TagsAlbum, d->tagWidget,   i18nc("@title", "Tags (0)"));

            mainLayout->addWidget(d->tabWidget);
            break;
        }

        case PhysAlbum:
        {
            initAlbumWidget();
            mainLayout->addWidget(d->albumWidget);
            break;
        }

        case TagsAlbum:
        {
            initTagWidget();
            mainLayout->addWidget(d->tagWidget);
            break;
        }
    }

    mainLayout->setContentsMargins(QMargins());
}

AlbumSelectors::~AlbumSelectors()
{
    delete d;
}

void AlbumSelectors::initAlbumWidget()
{
    d->albumWidget             = new QWidget(this);
    d->wholeAlbums             = new QCheckBox(i18nc("@option", "All albums"), d->albumWidget);
    d->recursiveSelectionAlbum = new QToolButton(d->albumWidget);
    d->recursiveSelectionAlbum->setToolTip(i18nc("@info", "Toggle recursive album selection."));
    d->recursiveSelectionAlbum->setIcon(QIcon::fromTheme(QLatin1String("object-order-back")));
    d->recursiveSelectionAlbum->setCheckable(true);
    d->recursiveSelectionAlbum->setChecked(true);

    if (!d->allowRecursive)
    {
        d->recursiveSelectionAlbum->setVisible(false);
        d->recursiveSelectionAlbum->setChecked(false);
    }

    d->albumSelectCB    = new AlbumTreeViewSelectComboBox(d->albumWidget);
    d->albumSelectCB->setToolTip(i18nc("@info:tooltip", "Select all albums that should be processed."));
    d->albumSelectCB->setDefaultModel();
    d->albumSelectCB->setRecursive(d->allowRecursive ? d->recursiveSelectionAlbum->isChecked() : false);
    d->albumSelectCB->setNoSelectionText(i18nc("@info", "No Album Selected"));
/*
    d->albumSelectCB->setAddExcludeTristate(true);
*/
    d->albumSelectCB->addCheckUncheckContextMenuActions();

    d->albumClearButton = new ModelClearButton(d->albumSelectCB->treeView()->checkableAlbumModel());
    d->albumClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected albums"));

    QHBoxLayout* l      = new QHBoxLayout;
    l->addWidget(d->wholeAlbums);
    l->addStretch(10);
    l->addWidget(d->recursiveSelectionAlbum);

    QGridLayout* const pAlbumsGrid = new QGridLayout(d->albumWidget);
    pAlbumsGrid->addLayout(l,                   0, 0, 1, 2);
    pAlbumsGrid->addWidget(d->albumSelectCB,    1, 0, 1, 1);
    pAlbumsGrid->addWidget(d->albumClearButton, 1, 1, 1, 1);
    pAlbumsGrid->setSpacing(0);
    pAlbumsGrid->setRowStretch(2, 10);

    connect(d->wholeAlbums, SIGNAL(toggled(bool)),
            this, SLOT(slotWholeAlbums(bool)));

    connect(d->wholeAlbums, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSelectionChanged()));

    connect(d->recursiveSelectionAlbum, &QToolButton::toggled,
            d->recursiveSelectionAlbum, [this] (bool checked)
        {
            d->albumSelectCB->setRecursive(checked);
        }
    );

    connect(d->albumSelectCB->treeView()->albumModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotUpdateClearButtons()));

    d->albumSelectCB->treeView()->setObjectName(d->configName);
    d->albumSelectCB->treeView()->setEntryPrefix(QLatin1String("AlbumComboBox-"));
    d->albumSelectCB->treeView()->setRestoreCheckState(true);
}

void AlbumSelectors::initTagWidget()
{
    d->tagWidget              = new QWidget(this);
    d->wholeTags              = new QCheckBox(i18nc("@option", "All tags"), d->tagWidget);
    d->recursiveSelectionTags = new QToolButton(d->tagWidget);
    d->recursiveSelectionTags->setToolTip(i18nc("@info", "Toggle recursive tags selection."));
    d->recursiveSelectionTags->setIcon(QIcon::fromTheme(QLatin1String("object-order-back")));
    d->recursiveSelectionTags->setCheckable(true);
    d->recursiveSelectionTags->setChecked(true);

    if (!d->allowRecursive)
    {
        d->recursiveSelectionTags->setVisible(false);
        d->recursiveSelectionTags->setChecked(false);
    }

    d->tagSelectCB    = new TagTreeViewSelectComboBox(d->tagWidget);
    d->tagSelectCB->setToolTip(i18nc("@info:tooltip", "Select all tags that should be processed."));
    d->tagSelectCB->setDefaultModel();
    d->tagSelectCB->setRecursive(d->allowRecursive ? d->recursiveSelectionTags->isChecked() : false);
    d->tagSelectCB->setNoSelectionText(i18nc("@info", "No Tag Selected"));
    d->tagSelectCB->addCheckUncheckContextMenuActions();

    d->tagClearButton = new ModelClearButton(d->tagSelectCB->treeView()->checkableAlbumModel());
    d->tagClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected tags"));

    QHBoxLayout* l    = new QHBoxLayout;
    l->addWidget(d->wholeTags);
    l->addStretch(10);
    l->addWidget(d->recursiveSelectionTags);

    QGridLayout* const tAlbumsGrid = new QGridLayout(d->tagWidget);
    tAlbumsGrid->addLayout(l,                 0, 0, 1, 2);
    tAlbumsGrid->addWidget(d->tagSelectCB,    1, 0, 1, 1);
    tAlbumsGrid->addWidget(d->tagClearButton, 1, 1, 1, 1);
    tAlbumsGrid->setSpacing(0);
    tAlbumsGrid->setRowStretch(2, 10);

    connect(d->wholeTags, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSelectionChanged()));

    connect(d->wholeTags, SIGNAL(toggled(bool)),
            this, SLOT(slotWholeTags(bool)));

    connect(d->tagSelectCB->treeView()->albumModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotUpdateClearButtons()));

    connect(d->recursiveSelectionTags, &QToolButton::toggled,
            d->recursiveSelectionTags, [this] (bool checked)
        {
            d->tagSelectCB->setRecursive(checked);
        }
    );

    d->tagSelectCB->treeView()->setObjectName(d->configName);
    d->tagSelectCB->treeView()->setEntryPrefix(QLatin1String("TagComboBox-"));
    d->tagSelectCB->treeView()->setRestoreCheckState(true);
}

void AlbumSelectors::slotWholeAlbums(bool b)
{
    if ((d->selectionMode == PhysAlbum) || (d->selectionMode == All))
    {
        d->albumSelectCB->setEnabled(!b);
        d->albumClearButton->setEnabled(!b);
        d->albumSelectCB->setAllSelectedText(b);
    }

    updateTabText();
}

void AlbumSelectors::slotWholeTags(bool b)
{
    if ((d->selectionMode == TagsAlbum) || (d->selectionMode == All))
    {
        d->tagSelectCB->setEnabled(!b);
        d->tagClearButton->setEnabled(!b);
        d->tagSelectCB->setAllSelectedText(b);
    }

    updateTabText();
}

void AlbumSelectors::slotUpdateClearButtons()
{
    bool selectionChanged = false;

    if ((d->selectionMode == PhysAlbum) || (d->selectionMode == All))
    {
        d->albumClearButton->animateVisible(!d->albumSelectCB->model()->checkedAlbums().isEmpty());
        selectionChanged = true;
    }

    if ((d->selectionMode == TagsAlbum) || (d->selectionMode == All))
    {
        d->tagClearButton->animateVisible(!d->tagSelectCB->model()->checkedAlbums().isEmpty());
        selectionChanged = true;
    }

    if (selectionChanged)
    {
        Q_EMIT signalSelectionChanged();
    }

    updateTabText();
}

bool AlbumSelectors::wholeAlbumsChecked() const
{
    return (d->wholeAlbums && d->wholeAlbums->isChecked());
}

AlbumList AlbumSelectors::selectedAlbums() const
{
    AlbumList albums;

    if      (wholeAlbumsChecked())
    {
        albums << AlbumManager::instance()->allPAlbums();
    }
    else if (d->albumSelectCB)
    {
        albums << d->albumSelectCB->model()->checkedAlbums();
    }

    return albums;
}

QList<int> AlbumSelectors::selectedAlbumIds() const
{
    QList<int> albumIds;
    AlbumList  albums = selectedAlbums();

    for (const Album* const album : std::as_const(albums))
    {
        albumIds << album->id();
    }

    return albumIds;
}

bool AlbumSelectors::wholeTagsChecked() const
{
    return d->wholeTags && d->wholeTags->isChecked();
}

AlbumList AlbumSelectors::selectedTags() const
{
    AlbumList albums;

    if      (wholeTagsChecked())
    {
        albums << AlbumManager::instance()->allTAlbums();
    }
    else if (d->tagSelectCB)
    {
        albums << d->tagSelectCB->model()->checkedAlbums();
    }

    return albums;
}

QList<int> AlbumSelectors::selectedTagIds() const
{
    QList<int> tagIds;
    AlbumList  tags = selectedTags();

    for (const Album* const tag : std::as_const(tags))
    {
        tagIds << tag->id();
    }

    return tagIds;
}

AlbumList AlbumSelectors::selectedAlbumsAndTags() const
{
    AlbumList albums;
    albums << selectedAlbums();
    albums << selectedTags();

    return albums;
}

void AlbumSelectors::setAlbumSelected(Album* const album, SelectionType type)
{
    if (d->albumWidget && album)
    {
        if (type == SingleSelection)
        {
            d->albumSelectCB->model()->resetCheckedAlbums();
        }

        d->albumSelectCB->model()->setChecked(album, true);
        d->wholeAlbums->setChecked(false);
    }
}

void AlbumSelectors::setTagSelected(Album* const album, SelectionType type)
{
    if (d->tagWidget && album)
    {
        if (type == SingleSelection)
        {
            d->tagSelectCB->model()->resetCheckedAlbums();
        }

        d->tagSelectCB->model()->setChecked(album, true);
        d->wholeTags->setChecked(false);
    }
}

void AlbumSelectors::setTypeSelection(int albumType)
{
    if (d->selectionMode == All)
    {
        d->tabWidget->setCurrentIndex(albumType);
    }
}

int AlbumSelectors::typeSelection() const
{
    if (d->selectionMode == All)
    {
        return d->tabWidget->currentIndex();
    }
    else
    {
        return d->selectionMode;
    }
}

void AlbumSelectors::resetPAlbumSelection()
{
    d->albumSelectCB->model()->resetAllCheckedAlbums();
    d->wholeAlbums->setChecked(false);
    slotWholeAlbums(wholeAlbumsChecked());
}

void AlbumSelectors::resetTAlbumSelection()
{
    d->tagSelectCB->model()->resetAllCheckedAlbums();
    d->wholeTags->setChecked(false);
    slotWholeTags(wholeTagsChecked());
}

void AlbumSelectors::resetSelection()
{
    if (d->albumWidget)
    {
        resetPAlbumSelection();
    }

    if (d->tagWidget)
    {
        resetTAlbumSelection();
    }
}

void AlbumSelectors::loadState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configName);

    if (d->albumWidget)
    {
        d->wholeAlbums->setChecked(group.readEntry(d->configUseWholeAlbumsEntry, true));
        d->albumSelectCB->treeView()->loadState();
        d->albumClearButton->animateVisible(!d->albumSelectCB->model()->checkedAlbums().isEmpty());

        slotWholeAlbums(wholeAlbumsChecked());
        d->albumSelectCB->updateText();
    }

    if (d->tagWidget)
    {
        d->wholeTags->setChecked(group.readEntry(d->configUseWholeTagsEntry, false));
        d->tagSelectCB->treeView()->loadState();
        d->tagClearButton->animateVisible(!d->tagSelectCB->model()->checkedAlbums().isEmpty());

        slotWholeTags(wholeTagsChecked());
        d->tagSelectCB->updateText();
    }

    if (d->selectionMode == All)
    {
        d->tabWidget->setCurrentIndex(group.readEntry(d->configAlbumTypeEntry, (int)PhysAlbum));
    }
}

void AlbumSelectors::saveState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configName);

    if (d->albumWidget)
    {
        group.writeEntry(d->configUseWholeAlbumsEntry, wholeAlbumsChecked());
        d->albumSelectCB->treeView()->saveState();
    }

    if (d->tagWidget)
    {
        group.writeEntry(d->configUseWholeTagsEntry, wholeTagsChecked());
        d->tagSelectCB->treeView()->saveState();
    }

    if (d->selectionMode == All)
    {
        group.writeEntry(d->configAlbumTypeEntry, typeSelection());
    }
}

void AlbumSelectors::updateTabText()
{
    if (d->selectionMode == All)
    {
        d->tabWidget->tabBar()->setTabText(PhysAlbum,
                                           wholeAlbumsChecked() ? i18nc("@title", "Albums (All)")
                                                                : i18nc("@title", "Albums (%1)",
                                                 selectedAlbums().count()));
        d->tabWidget->tabBar()->setTabText(TagsAlbum,
                                           wholeTagsChecked() ? i18nc("@title", "Tags (All)")
                                                              : i18nc("@title", "Tags (%1)",
                                                 selectedTags().count()));
    }
}

} // namespace Digikam

#include "albumselectors.moc"

#include "moc_albumselectors.cpp"
