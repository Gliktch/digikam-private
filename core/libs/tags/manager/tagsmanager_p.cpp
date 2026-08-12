/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
 *
 * SPDX-FileCopyrightText:      2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsmanager_p.h"

namespace Digikam
{

QPointer<TagsManager> TagsManager::internalPtr = QPointer<TagsManager>();

TagsManager::Private::Private(TagsManager* const parent)
    : dd(parent)
{
}

QString TagsManager::Private::joinTagNamesToList(const QStringList& stringList)
{
    const QString joinedStringList = stringList.join(QLatin1String("', '"));

    return (QLatin1Char('\'') + joinedStringList + QLatin1Char('\''));
}

void TagsManager::Private::setHelpText(QAction* const action, const QString& text)
{
    action->setStatusTip(text);
    action->setToolTip(text);

    if (action->whatsThis().isEmpty())
    {
        action->setWhatsThis(text);
    }
}

void TagsManager::Private::enableRootTagActions(bool value)
{
    for (QAction* const action : std::as_const(rootDisabledOptions))
    {
        if (value)
        {
            action->setEnabled(true);
        }
        else
        {
            action->setEnabled(false);
        }
    }
}

void TagsManager::Private::setupActions()
{
    mainToolbar = new QToolBar(dd);
    mainToolbar->setMovable(false);
    mainToolbar->setFloatable(false);
    mainToolbar->setContextMenuPolicy(Qt::PreventContextMenu);
    const int cmargin = qMin(dd->style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                             qMin(dd->style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                                  qMin(dd->style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                       dd->style()->pixelMetric(QStyle::PM_LayoutBottomMargin))));
    mainToolbar->layout()->setContentsMargins(cmargin, cmargin, cmargin, cmargin);

    QWidgetAction* const pixMapAction = new QWidgetAction(dd);
    pixMapAction->setDefaultWidget(tagPixmap);

    QWidgetAction* const searchAction = new QWidgetAction(dd);
    searchAction->setDefaultWidget(searchBar);

    mainToolbar->addAction(pixMapAction);
    mainToolbar->addAction(searchAction);

    mainToolbar->addSeparator();

    addAction                 = new QAction(QIcon::fromTheme(QLatin1String("list-add")),
                                            i18nc("@action: button", "Add Tag"), dd);
    QToolButton* const addBtn = new QToolButton(dd);
    addBtn->setDefaultAction(addAction);
    addBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    delAction                 = new QAction(QIcon::fromTheme(QLatin1String("list-remove")),
                                               i18nc("@action: button", "Remove Tag"), dd);
    QToolButton* const delBtn = new QToolButton(dd);
    delBtn->setDefaultAction(delAction);
    delBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    organizeButton            = new QPushButton(i18nc("@action: button", "Organize"), dd);
    organizeButton->setIcon(QIcon::fromTheme(QLatin1String("autocorrection")));
    organizeButton->setFlat(true);

    syncexportButton          = new QPushButton(i18nc("@action: button", "Sync &Export"), dd);
    syncexportButton->setIcon(QIcon::fromTheme(QLatin1String("network-server-database")));
    syncexportButton->setFlat(true);

    /**
     * organize group
     */
    QMenu* const organizeMenu    = new QMenu(organizeButton);
    organizeButton->setMenu(organizeMenu);

#ifdef HAVE_AKONADICONTACT

    AkonadiIface* const abc      = new AkonadiIface(organizeMenu);

    dd->connect(abc, SIGNAL(signalContactTriggered(QString)),
                tagMngrView, SLOT(slotTagNewFromABCMenu(QString)));

    // AkonadiIface instance will be deleted with organizeMenu.

#endif

    titleEdit                 = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                               i18n("Edit Tag Title"), dd);
    titleEdit->setShortcut(QKeySequence(Qt::Key_F2));

    QAction* const resetIcon     = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")),
                                               i18n("Reset Tag Icon"), dd);

    QAction* const markUnused    = new QAction(QIcon::fromTheme(QLatin1String("edit-select")),
                                               i18n("Mark Unassigned Tags"), dd);

    QAction* const invSel        = new QAction(QIcon::fromTheme(QLatin1String("tag-reset")),
                                               i18n("Invert Selection"), dd);

    QAction* const expandSel     = new QAction(QIcon::fromTheme(QLatin1String("go-down")),
                                               i18n("Expand Selected Nodes"), dd);

    QAction* const expandAll     = new QAction(QIcon::fromTheme(QLatin1String("expand-all")),
                                               i18n("Expand Tag Tree"), dd);

    QAction* const collapseAll   = new QAction(QIcon::fromTheme(QLatin1String("collapse-all")),
                                               i18n("Collapse Tag Tree"), dd);

    QAction* const delTagFromImg = new QAction(QIcon::fromTheme(QLatin1String("tag-delete")),
                                               i18n("Remove Tag from Images"), dd);

    /**
     * Tool tips
     */
    setHelpText(addAction,  i18n("Add new tag to current tag. "
                                    "Current tag is last clicked tag."));

    setHelpText(delAction,  i18n("Delete selected items. "
                                    "Also work with multiple items, "
                                    "but will not delete the root tag."));

    setHelpText(titleEdit,     i18n("Edit title from selected tag."));

    setHelpText(resetIcon,     i18n("Reset icon to selected tags. "
                                    "Works with multiple selection."));

    setHelpText(markUnused,    i18n("Mark all tags that are not assigned to images."));

    setHelpText(invSel,        i18n("Invert selection. "
                                    "Only visible items will be selected"));

    setHelpText(expandSel,     i18n("Selected items will be expanded"));

    setHelpText(expandAll,     i18n("Expand tag tree completely"));

    setHelpText(collapseAll,   i18n("Collapse tag tree completely"));

    setHelpText(delTagFromImg, i18n("Delete selected tag(s) from images. "
                                    "Works with multiple selection."));

    dd->connect(titleEdit, SIGNAL(triggered()),
                dd, SLOT(slotEditTagTitle()));

    dd->connect(resetIcon, SIGNAL(triggered()),
                dd, SLOT(slotResetTagIcon()));

    dd->connect(invSel, SIGNAL(triggered()),
                dd, SLOT(slotInvertSelection()));

    dd->connect(expandSel, SIGNAL(triggered()),
                tagMngrView, SLOT(slotExpandNode()));

    dd->connect(expandAll, SIGNAL(triggered()),
                tagMngrView, SLOT(expandAll()));

    dd->connect(collapseAll, SIGNAL(triggered()),
                tagMngrView, SLOT(slotCollapseAllNodes()));

    dd->connect(delTagFromImg, SIGNAL(triggered()),
                dd, SLOT(slotRemoveTagsFromImages()));

    dd->connect(markUnused, SIGNAL(triggered()),
                dd, SLOT(slotMarkNotAssignedTags()));

    organizeMenu->addAction(titleEdit);
    organizeMenu->addAction(resetIcon);
    organizeMenu->addAction(markUnused);
    organizeMenu->addAction(invSel);
    organizeMenu->addAction(expandSel);
    organizeMenu->addAction(expandAll);
    organizeMenu->addAction(collapseAll);
    organizeMenu->addAction(delTagFromImg);

    /**
     * Sync & Export Group
     */

    QMenu* const syncexportMenu = new QMenu(syncexportButton);
    syncexportButton->setMenu(syncexportMenu);

    QAction* const wrDbImg      = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")),
                                              i18n("Write Tags from Database to Image"), dd);

    QAction* const readTags     = new QAction(QIcon::fromTheme(QLatin1String("tag-new")),
                                              i18n("Read Tags from Image"), dd);

    QAction* const wipeAll      = new QAction(QIcon::fromTheme(QLatin1String("draw-eraser")),
                                              i18n("Wipe all tags from Database only"), dd);

    QAction* const saveTags     = new QAction(QIcon::fromTheme(QLatin1String("document-export")),
                                              i18n("Export tags to a file from selected tag"), dd);

    QAction* const loadTags     = new QAction(QIcon::fromTheme(QLatin1String("document-import")),
                                              i18n("Import tags from a file to selected tag"), dd);

    setHelpText(wrDbImg,  i18n("Write Tags Metadata to Image."));

    setHelpText(readTags, i18n("Read tags from Images into Database. "
                               "Existing tags will not be affected"));

    setHelpText(wipeAll,  i18n("Delete all tags from database only. Will not sync with files. "
                               "Proceed with caution."));

    setHelpText(saveTags, i18n("Export all tags to a file to keep a backup."));

    setHelpText(loadTags, i18n("Import all tags from a file to restore a backup."));

    dd->connect(wrDbImg, SIGNAL(triggered()),
                dd, SLOT(slotWriteToImage()));

    dd->connect(readTags, SIGNAL(triggered()),
                dd, SLOT(slotReadFromImage()));

    dd->connect(wipeAll, SIGNAL(triggered()),
                dd, SLOT(slotWipeAll()));

    dd->connect(saveTags, SIGNAL(triggered()),
                dd, SLOT(slotSaveTags()));

    dd->connect(loadTags, SIGNAL(triggered()),
                dd, SLOT(slotLoadTags()));

    syncexportMenu->addAction(wrDbImg);
    syncexportMenu->addAction(readTags);
    syncexportMenu->addAction(wipeAll);
    syncexportMenu->addAction(saveTags);
    syncexportMenu->addAction(loadTags);

    mainToolbar->addWidget(addBtn);
    mainToolbar->addWidget(delBtn);
    mainToolbar->addWidget(organizeButton);
    mainToolbar->addWidget(syncexportButton);

    QPushButton* const helpButton = new QPushButton(QIcon::fromTheme(QLatin1String("help-browser")), i18n("Help"));
    helpButton->setToolTip(i18nc("@info", "Online help about tags management"));
    helpButton->setFlat(true);

    dd->connect(helpButton, &QPushButton::clicked,
                dd, []()
        {
            openOnlineDocumentation(QLatin1String("left_sidebar"),
                                    QLatin1String("tags_view"),
                                    QLatin1String("tags-manager"));
        }
    );

    mainToolbar->addWidget(helpButton);

    QWidget* const spacer = new QWidget(dd);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainToolbar->addWidget(spacer);

    QToolButton* const laBtn = new QToolButton(dd);
    laBtn->setDefaultAction(new DLogoAction(dd));
    laBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolbar->addWidget(laBtn);
    DLogoAction::applyStyleForToolBarButton(laBtn);

    dd->addToolBar(mainToolbar);

    rootDisabledOptions.append(delAction);
    rootDisabledOptions.append(titleEdit);
    rootDisabledOptions.append(resetIcon);
    rootDisabledOptions.append(delTagFromImg);
}

} // namespace Digikam
