/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : tags editor
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsedit.h"

// Qt includes

#include <QToolButton>
#include <QPushButton>
#include <QApplication>
#include <QStyle>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "addtagslineedit.h"
#include "tagcheckview.h"
#include "itemattributeswatch.h"
#include "tagmodificationhelper.h"
#include "tagsmanager.h"
#include "tagtreeview.h"
#include "searchtextbardb.h"
#include "album.h"
#include "albummodel.h"
#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "metadatahub.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagsEdit::Private
{
public:

    Private() = default;

public:

    bool                 togglingTagsSearchSettings = false;
    bool                 ignoreTagChanges           = false;
    QToolButton*         recentTagsBtn              = nullptr;
    QToolButton*         assignedTagsBtn            = nullptr;
    QPushButton*         openTagMngr                = nullptr;
    SearchTextBarDb*     tagsSearchBar              = nullptr;
    AddTagsLineEdit*     newTagEdit                 = nullptr;
    TagCheckView*        tagCheckView               = nullptr;
    TagModel*            tagModel                   = nullptr;
    DisjointMetadata*    hub                        = nullptr;
};

TagsEdit::TagsEdit(DisjointMetadata* const hub, QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->hub      = hub;

    int spacing = layoutSpacing();


    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);

    QWidget* const tagsArea     = new QWidget(viewport());
    QGridLayout* const grid3    = new QGridLayout(tagsArea);
    setWidget(tagsArea);

    d->tagModel                 = new TagModel(AbstractAlbumModel::IncludeRootAlbum, this);
    d->tagModel->setCheckable(true);
    d->tagModel->setRootCheckable(false);
    d->tagCheckView             = new TagCheckView(tagsArea, d->tagModel);
    d->tagCheckView->setCheckNewTags(true);

    d->openTagMngr              = new QPushButton(i18nc("@action", "Open Tag Manager"));

    d->newTagEdit               = new AddTagsLineEdit(tagsArea);
    d->newTagEdit->setSupportingTagModel(d->tagModel);
    d->newTagEdit->setTagTreeView(d->tagCheckView); //, "ItemDescEditTabNewTagEdit");
    //d->newTagEdit->setCaseSensitive(false);
    d->newTagEdit->setPlaceholderText(i18nc("@info", "Enter tag here."));
    d->newTagEdit->setWhatsThis(i18nc("@info", "Enter the text used to create tags here. "
                                      "'/' can be used to create a hierarchy of tags. "
                                      "',' can be used to create more than one hierarchy at the same time."));

    DHBox* const tagsSearch     = new DHBox(tagsArea);
    tagsSearch->setSpacing(spacing);

    d->tagsSearchBar            = new SearchTextBarDb(tagsSearch,
                                                      QLatin1String("ItemDescEditTabTagsSearchBar"));

    d->tagsSearchBar->setModel(d->tagCheckView->filteredModel(),
                               AbstractAlbumModel::AlbumIdRole,
                               AbstractAlbumModel::AlbumTitleRole);

    d->tagsSearchBar->setFilterModel(d->tagCheckView->albumFilterModel());

    d->assignedTagsBtn          = new QToolButton(tagsSearch);
    d->assignedTagsBtn->setToolTip(i18nc("@info", "Tags already assigned"));
    d->assignedTagsBtn->setIcon(QIcon::fromTheme(QLatin1String("tag-assigned")));
    d->assignedTagsBtn->setCheckable(true);

    d->recentTagsBtn            = new QToolButton(tagsSearch);
    QMenu* const recentTagsMenu = new QMenu(d->recentTagsBtn);
    d->recentTagsBtn->setToolTip(i18nc("@info", "Recent Tags"));
    d->recentTagsBtn->setIcon(QIcon::fromTheme(QLatin1String("tag-recents")));
    d->recentTagsBtn->setIconSize(QSize(16, 16));
    d->recentTagsBtn->setMenu(recentTagsMenu);
    d->recentTagsBtn->setPopupMode(QToolButton::InstantPopup);

    grid3->addWidget(d->openTagMngr,  0, 0, 1, 2);
    grid3->addWidget(d->newTagEdit,   1, 0, 1, 2);
    grid3->addWidget(d->tagCheckView, 2, 0, 1, 2);
    grid3->addWidget(tagsSearch,      3, 0, 1, 2);
    grid3->setRowStretch(1, 10);

    // ---

    connect(d->openTagMngr, SIGNAL(clicked()),
            this, SLOT(slotOpenTagsManager()));

    connect(d->tagCheckView->checkableAlbumModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotTagStateChanged(Album*,Qt::CheckState)));

    connect(d->tagsSearchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotTagsSearchChanged(SearchTextSettings)));

    connect(d->assignedTagsBtn, SIGNAL(toggled(bool)),
            this, SLOT(slotAssignedTagsToggled(bool)));

    connect(d->newTagEdit, SIGNAL(taggingActionActivated(TaggingAction)),
            this, SLOT(slotTaggingActionActivated(TaggingAction)));

    connect(ItemAttributesWatch::instance(), SIGNAL(signalImageTagsChanged(qlonglong)),
            this, SIGNAL(signalImageTagsChanged(qlonglong)));
}

TagsEdit::~TagsEdit()
{
    delete d;
}

void TagsEdit::slotOpenTagsManager()
{
    TagsManager* const tagMngr = TagsManager::instance();
    tagMngr->show();
    tagMngr->activateWindow();
    tagMngr->raise();
}

void TagsEdit::slotTagStateChanged(Album* album, Qt::CheckState checkState)
{
    const TAlbum* const tag = dynamic_cast<TAlbum*>(album);

    if (!tag || d->ignoreTagChanges)
    {
        return;
    }

    switch (checkState)
    {
        case Qt::Checked:
        {
            d->hub->setTag(tag->id());

            break;
        }

        default:
        {
            d->hub->setTag(tag->id(), DisjointMetadataDataFields::MetadataInvalid);

            break;
        }
    }

    Q_EMIT signalChanged();
}

void TagsEdit::slotTagsSearchChanged(const SearchTextSettings& settings)
{
    Q_UNUSED(settings);

    // if we filter, we should reset the assignedTagsBtn again.

    if (d->assignedTagsBtn->isChecked() && !d->togglingTagsSearchSettings)
    {
        d->togglingTagsSearchSettings = true;
        d->assignedTagsBtn->setChecked(false);
        d->togglingTagsSearchSettings = false;
    }
}

void TagsEdit::slotAssignedTagsToggled(bool t)
{
    d->tagCheckView->checkableAlbumFilterModel()->setFilterChecked(t);
    d->tagCheckView->checkableAlbumFilterModel()->setFilterPartiallyChecked(t);
    d->tagCheckView->checkableAlbumFilterModel()->setFilterBehavior(t ? AlbumFilterModel::StrictFiltering
                                                                      : AlbumFilterModel::FullFiltering);

    if (t)
    {
        // if we filter by assigned, we should initially clear the normal search.

        if (!d->togglingTagsSearchSettings)
        {
            d->togglingTagsSearchSettings = true;
            d->tagsSearchBar->clear();
            d->togglingTagsSearchSettings = false;
        }

        // Only after above change, do this.

        d->tagCheckView->expandMatches(d->tagCheckView->rootIndex());
   }
}

void TagsEdit::slotTaggingActionActivated(const TaggingAction& action)
{
    TAlbum* assigned = nullptr;

    if      (action.shallAssignTag())
    {
        assigned = AlbumManager::instance()->findTAlbum(action.tagId());

        if (assigned)
        {
            d->tagModel->setChecked(assigned, true);
            d->tagCheckView->checkableAlbumFilterModel()->updateFilter();
        }
    }
    else if (action.shallCreateNewTag())
    {
        TAlbum* const parent = AlbumManager::instance()->findTAlbum(action.parentTagId());

        // tag is assigned automatically

        assigned = d->tagCheckView->tagModificationHelper()->slotTagNew(parent, action.newTagName());
    }

    if (assigned)
    {
        d->tagCheckView->scrollTo(d->tagCheckView->albumFilterModel()->indexForAlbum(assigned));
        QTimer::singleShot(0, d->newTagEdit, SLOT(clear()));
    }
}

void TagsEdit::updateRecentTags()
{
    QMenu* const menu = dynamic_cast<QMenu*>(d->recentTagsBtn->menu());

    if (!menu)
    {
        return;
    }

    menu->clear();

    AlbumList recentTags = AlbumManager::instance()->getRecentlyAssignedTags();

    if (recentTags.isEmpty())
    {
        QAction* const noTagsAction = menu->addAction(i18nc("@action", "No Recently Assigned Tags"));
        noTagsAction->setEnabled(false);
    }
    else
    {
        for (AlbumList::const_iterator it = recentTags.constBegin() ;
             it != recentTags.constEnd() ; ++it)
        {
            TAlbum* const album = static_cast<TAlbum*>(*it);

            if (album)
            {
                AlbumThumbnailLoader* const loader = AlbumThumbnailLoader::instance();
                QPixmap                     icon;

                if (!loader->getTagThumbnail(album, icon))
                {
                    if (icon.isNull())
                    {
                        icon = loader->getStandardTagIcon(album, AlbumThumbnailLoader::SmallerSize);
                    }
                }

                const TAlbum* const parent = dynamic_cast<TAlbum*> (album->parent());

                if (parent)
                {
                    QString text          = album->title()      +
                                            QLatin1String(" (") +
                                            parent->prettyUrl() +
                                            QLatin1Char(')');
                    QAction* const action = menu->addAction(icon, text);
                    int id                = album->id();

                    connect(action, &QAction::triggered,
                            this, [this, id]()
                            {
                                slotRecentTagsMenuActivated(id);
                            }
                    );
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Tag" << album
                                                 << "do not have a valid parent";
                }
            }
        }
    }
}

void TagsEdit::slotRecentTagsMenuActivated(int id)
{
    AlbumManager* const albumMan = AlbumManager::instance();

    if (id > 0)
    {
        TAlbum* const album = albumMan->findTAlbum(id);

        if (album)
        {
            d->tagModel->setChecked(album, true);
            d->tagCheckView->checkableAlbumFilterModel()->updateFilter();
        }
    }
}

AddTagsLineEdit* TagsEdit::newTagEdit() const
{
    return d->newTagEdit;
}

TagCheckView* TagsEdit::tagsCheckView() const
{
    return d->tagCheckView;
}

void TagsEdit::activateAssignedTagsButton()
{
    d->assignedTagsBtn->click();
}

void TagsEdit::slotUnifyPartiallyTags()
{
    const auto albums = d->tagModel->partiallyCheckedAlbums();

    for (Album* const album : albums)
    {
        d->tagModel->setChecked(album, true);
    }
}

void TagsEdit::setTagState(TAlbum* const tag, DisjointMetadataDataFields::Status status)
{
    if (!tag)
    {
        return;
    }

    switch (status)
    {
        case DisjointMetadataDataFields::MetadataDisjoint:
        {
            d->tagModel->setCheckState(tag, Qt::PartiallyChecked);

            break;
        }

        case DisjointMetadataDataFields::MetadataAvailable:
        {
            d->tagModel->setChecked(tag, true);

            break;
        }

        case DisjointMetadataDataFields::MetadataInvalid:
        {
            d->tagModel->setChecked(tag, false);

            break;
        }

        default:
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Untreated tag status enum value " << status;
            d->tagModel->setCheckState(tag, Qt::PartiallyChecked);

            break;
        }
    }
}

void TagsEdit::updateTagsView()
{
    // Avoid that the automatic tag toggling handles these calls and
    // modification is indicated to this widget

    TagCheckView::ToggleAutoTags toggle = d->tagCheckView->getToggleAutoTags();
    d->tagCheckView->setToggleAutoTags(TagCheckView::NoToggleAuto);
    d->ignoreTagChanges                 = true;

    // first reset the tags completely

    d->tagModel->resetAllCheckedAlbums();

    // Then update checked state for all tags of the currently selected images

    const QMap<int, DisjointMetadataDataFields::Status> hubMap = d->hub->tags();

    for (QMap<int, DisjointMetadataDataFields::Status>::const_iterator it = hubMap.begin() ;
         it != hubMap.end() ; ++it)
    {
        TAlbum* const tag = AlbumManager::instance()->findTAlbum(it.key());
        setTagState(tag, it.value());
    }

    d->ignoreTagChanges = false;
    d->tagCheckView->setToggleAutoTags(toggle);

    // The condition is a temporary fix not to destroy name filtering on image change.
    // See comments in these methods.

    if (d->assignedTagsBtn->isChecked())
    {
        slotAssignedTagsToggled(d->assignedTagsBtn->isChecked());
    }
}

} // namespace Digikam

#include "moc_tagsedit.cpp"
