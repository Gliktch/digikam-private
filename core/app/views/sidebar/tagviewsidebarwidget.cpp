/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the tag view.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagviewsidebarwidget.h"

// Qt includes

#include <QButtonGroup>
#include <QRadioButton>
#include <QApplication>
#include <QStyle>
#include <QPushButton>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "applicationsettings.h"
#include "searchtextbardb.h"
#include "tagfolderview.h"
#include "tagsmanager.h"
#include "coredb.h"
#include "coredbsearchxml.h"
#include "autotagsscanwidget.h"
#include "autotagsengine.h"
#include "dexpanderbox.h"
#include "systemsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagViewSideBarWidget::Private
{
public:

    enum TagsSource
    {
        NoTags = 0,
        ExistingTags
    };

public:

    Private() = default;

public:

    QPushButton*            helpButton             = nullptr;
    QPushButton*            openTagMngr            = nullptr;
    SearchTextBarDb*        tagSearchBar           = nullptr;
    TagFolderView*          tagFolderView          = nullptr;
    QButtonGroup*           btnGroup               = nullptr;
    QRadioButton*           noTagsBtn              = nullptr;
    QRadioButton*           tagsBtn                = nullptr;
    TagViewSideBarWidget*   parentInstance         = nullptr;

    AutotagsScanWidget*     settingsWdg            = nullptr;
    QPushButton*            rescanButton           = nullptr;

    DLabelExpander*         autotagsExpander       = nullptr;

    bool                    noTagsWasChecked       = false;
    bool                    ExistingTagsWasChecked = false;

    QString                 noTagsSearchXml;

    const QString           configTagsSourceEntry  = QLatin1String("TagsSource");
};

TagViewSideBarWidget::TagViewSideBarWidget(QWidget* const parent, TagModel* const model)
    : SidebarWidget(parent),
      d            (new Private)
{
    d->parentInstance = this;

    setObjectName(QLatin1String("TagView Sidebar"));
    setProperty("Shortcut", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F2));

    const int spacing         = layoutSpacing();

    QVBoxLayout* const layout = new QVBoxLayout(this);

    d->openTagMngr = new QPushButton( i18n("Open Tag Manager"));
    d->noTagsBtn   = new QRadioButton(i18n("No Tags"), this);
    d->tagsBtn     = new QRadioButton(i18n("Existing Tags"), this);
    d->btnGroup    = new QButtonGroup(this);
    d->btnGroup->addButton(d->noTagsBtn);
    d->btnGroup->addButton(d->tagsBtn);
    d->btnGroup->setId(d->noTagsBtn, 0);
    d->btnGroup->setId(d->tagsBtn, 1);
    d->btnGroup->setExclusive(true);

    d->tagFolderView = new TagFolderView(this, model);
    d->tagFolderView->setConfigGroup(getConfigGroup());
    d->tagFolderView->setAlbumManagerCurrentAlbum(true);
/*
    d->tagFolderView->filteredModel()->doNotListTagsWithProperty(TagPropertyName::person());
    d->tagFolderView->filteredModel()->setFilterBehavior(AlbumFilterModel::StrictFiltering);
*/
    d->tagSearchBar  = new SearchTextBarDb(this, QLatin1String("ItemIconViewTagSearchBar"));
    d->tagSearchBar->setHighlightOnResult(true);
    d->tagSearchBar->setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
/*
    d->tagSearchBar->setModel(d->tagFolderView->filteredModel(),
                              AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
*/
    d->tagSearchBar->setFilterModel(d->tagFolderView->albumFilterModel());

    d->autotagsExpander            = new DLabelExpander(this);
    d->autotagsExpander->setText(i18n("Auto-tag Scan"));
    d->autotagsExpander->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    d->autotagsExpander->setObjectName(QLatin1String("AutotagScanWidgetExpanded"));

    QWidget* const autotagsWdg     = new QWidget(d->autotagsExpander);
    QVBoxLayout* const autotagsLay = new QVBoxLayout(autotagsWdg);

    d->settingsWdg    = new AutotagsScanWidget(AutotagsScanWidget::SettingsDisplayMode::Normal, autotagsWdg);

    DHBox* const hbox = new DHBox(d->autotagsExpander);
    d->rescanButton   = new QPushButton(hbox);
    d->rescanButton->setText(i18n("Auto-tag Scan"));
    d->rescanButton->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    d->rescanButton->setWhatsThis(i18nc("@info", "Use this button to scan the selected albums for objects to auto-tag"));

    d->helpButton     = new QPushButton(hbox);
    d->helpButton->setToolTip(i18nc("@info", "Help"));
    d->helpButton->setIcon(QIcon::fromTheme(QLatin1String("help-browser")));
    hbox->setStretchFactor(d->rescanButton, 10);

    QFontMetrics fmt  = d->rescanButton->fontMetrics();
    d->helpButton->setIconSize(QSize(fmt.height(), fmt.height()));

    connect(d->helpButton, &QPushButton::clicked,
            this, []()
        {
            openOnlineDocumentation(QLatin1String("left_sidebar"),
                                    QLatin1String("tags_view"));
        }
    );

    autotagsLay->addWidget(d->settingsWdg);
    autotagsLay->addWidget(hbox);
    autotagsLay->setContentsMargins(0, 0, 0, 0);

    d->autotagsExpander->setLineVisible(true);
    d->autotagsExpander->setWidget(autotagsWdg);
    d->autotagsExpander->setExpandByDefault(true);
    d->autotagsExpander->layout()->setContentsMargins(0, 0, 0, spacing);

    layout->addWidget(d->openTagMngr);
    layout->addWidget(d->noTagsBtn);
    layout->addWidget(d->tagsBtn);
    layout->addWidget(d->tagFolderView, 10);
    layout->addWidget(d->tagSearchBar);
    layout->addWidget(d->autotagsExpander);
    layout->setContentsMargins(0, spacing, spacing, 0);

    connect(d->openTagMngr, SIGNAL(clicked()),
            this,SLOT(slotOpenTagManager()));

    connect(d->tagFolderView, SIGNAL(signalFindDuplicates(QList<TAlbum*>)),
            this, SIGNAL(signalFindDuplicates(QList<TAlbum*>)));

    connect(d->rescanButton, SIGNAL(pressed()),
            this, SLOT(slotScanForAutotags()));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->btnGroup, SIGNAL(idClicked(int)),
            this, SLOT(slotToggleTagsSelection(int)));

#else

    connect(d->btnGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotToggleTagsSelection(int)));

#endif

    // hide autotags expander and rescan button if autotags are disabled

    if (!SystemSettings(qApp->applicationName()).enableAutoTags)
    {
        d->autotagsExpander->setVisible(false);
        d->rescanButton->setVisible(false);        
    }
}

TagViewSideBarWidget::~TagViewSideBarWidget()
{
    delete d;
}

void TagViewSideBarWidget::setActive(bool active)
{
    if (active)
    {
        if (d->noTagsBtn->isChecked())
        {
            setNoTagsAlbum();
        }
        else
        {
            AlbumManager::instance()->setCurrentAlbums(d->tagFolderView->selectedTags());
        }
    }
}

void TagViewSideBarWidget::doLoadState()
{
    KConfigGroup group        = getConfigGroup();
    bool noTagsBtnWasChecked  = group.readEntry(d->configTagsSourceEntry, false);

    d->autotagsExpander->setExpanded(group.readEntry(d->autotagsExpander->objectName(),
                                                     d->autotagsExpander->isExpandByDefault()));
    d->noTagsBtn->setChecked(noTagsBtnWasChecked);
    d->tagsBtn->setChecked(!noTagsBtnWasChecked);
    d->noTagsWasChecked       = noTagsBtnWasChecked;
    d->ExistingTagsWasChecked = !noTagsBtnWasChecked;

    d->tagFolderView->loadState();
    d->tagFolderView->setDisabled(noTagsBtnWasChecked);

    d->settingsWdg->loadState();
}

void TagViewSideBarWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(d->configTagsSourceEntry,
                     d->noTagsBtn->isChecked());
    group.writeEntry(d->autotagsExpander->objectName(),
                     d->autotagsExpander->isExpanded());

    d->tagFolderView->saveState();

    d->settingsWdg->saveState();

    group.sync();
}

void TagViewSideBarWidget::applySettings()
{
    const ApplicationSettings* const settings = ApplicationSettings::instance();
    d->tagFolderView->setExpandNewCurrentItem(settings->getExpandNewCurrentItem());
}

void TagViewSideBarWidget::changeAlbumFromHistory(const QList<Album*>& album)
{
    if (album.first()->type() == Album::TAG)
    {
        d->tagsBtn->setChecked(true);
        d->tagFolderView->setEnabled(true);
        d->ExistingTagsWasChecked = true;
        d->noTagsWasChecked       = false;
        d->tagFolderView->setCurrentAlbums(album);
    }
    else
    {
        d->noTagsBtn->setChecked(true);
        d->tagFolderView->setDisabled(true);
        d->noTagsWasChecked       = true;
        d->ExistingTagsWasChecked = false;
    }
}

AlbumPointer<TAlbum> TagViewSideBarWidget::currentAlbum() const
{
    return AlbumPointer<TAlbum> (d->tagFolderView->currentAlbum());
}

void TagViewSideBarWidget::setNoTagsAlbum()
{
    if (d->noTagsSearchXml.isEmpty())
    {
        SearchXmlWriter writer;
        writer.setFieldOperator((SearchXml::standardFieldOperator()));
        writer.writeGroup();
        writer.writeField(QLatin1String("nottagged"), SearchXml::Equal);
        writer.finishField();
        writer.finishGroup();
        writer.finish();
        d->noTagsSearchXml = writer.xml();
    }

    QString title = SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch);
    SAlbum* album = AlbumManager::instance()->findSAlbum(title);                // cppcheck-suppress constVariablePointer

    int id;

    if (album)
    {
        id = album->id();
        CoreDbAccess().db()->updateSearch(id, DatabaseSearch::AdvancedSearch,
                                          SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), d->noTagsSearchXml);
    }
    else
    {
        id = CoreDbAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                            SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), d->noTagsSearchXml);
    }

    album = new SAlbum(i18n("No Tags Album"), id);

    if (album)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << album);
    }
}

void TagViewSideBarWidget::slotScanForAutotags()
{
    AutotagsScanSettings autotagsScanSettings = d->settingsWdg->settings();
    doAutotagsScan(autotagsScanSettings);
}

void TagViewSideBarWidget::slotScanComplete()
{
    d->settingsWdg->setEnabled(true);
    d->rescanButton->setEnabled(true);
}

const QIcon TagViewSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("tag"));
}

const QString TagViewSideBarWidget::getCaption()
{
    return i18n("Tags");
}

void TagViewSideBarWidget::setCurrentAlbum(TAlbum* album)
{
    d->tagFolderView->setCurrentAlbums(QList<Album*>() << album);
}

void TagViewSideBarWidget::slotOpenTagManager()
{
    TagsManager* const tagMngr = TagsManager::instance();
    tagMngr->show();
    tagMngr->activateWindow();
    tagMngr->raise();
}

void TagViewSideBarWidget::slotToggleTagsSelection(int radioClicked)
{
    switch (Private::TagsSource(radioClicked))
    {
        case Private::NoTags:
        {
            if (!d->noTagsWasChecked)
            {
                setNoTagsAlbum();
                d->tagFolderView->setDisabled(true);
                d->noTagsWasChecked       = d->noTagsBtn->isChecked();
                d->ExistingTagsWasChecked = d->tagsBtn->isChecked();
            }

            break;
        }
        case Private::ExistingTags:
        {
            if (!d->ExistingTagsWasChecked)
            {
                d->tagFolderView->setEnabled(true);
                setActive(true);
                d->noTagsWasChecked       = d->noTagsBtn->isChecked();
                d->ExistingTagsWasChecked = d->tagsBtn->isChecked();
            }

            break;
        }
    }
}

void TagViewSideBarWidget::doAutotagsScan(const AutotagsScanSettings& autotagsScanSettings)
{
    AutotagsEngine* const autotagsDetector = new AutotagsEngine(autotagsScanSettings);
    autotagsDetector->start();

    connect(autotagsDetector, SIGNAL(signalComplete()),
            d->parentInstance, SLOT(slotScanComplete()));

    connect(autotagsDetector, SIGNAL(signalCanceled()),
            d->parentInstance, SLOT(slotScanComplete()));

    connect(autotagsDetector, SIGNAL(signalScanNotification(QString,int)),
            d->parentInstance, SIGNAL(signalNotificationError(QString,int)));

    d->settingsWdg->setEnabled(false);
    d->rescanButton->setEnabled(false);
}

} // namespace Digikam

#include "moc_tagviewsidebarwidget.cpp"
