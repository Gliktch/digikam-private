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

void TagsManager::setupUi()
{
    setWindowTitle(i18nc("@title:window", "Tags Manager"));

    d->tagPixmap   = new QLabel();
    d->tagPixmap->setText(QLatin1String("Tag Pixmap"));
    d->tagPixmap->setMaximumWidth(40);
    d->tagPixmap->setPixmap(QIcon::fromTheme(QLatin1String("tag")).pixmap(30, 30));

    d->tagMngrView = new TagMngrTreeView(this, d->tagModel);
    d->tagMngrView->setConfigGroup(getConfigGroup());

    d->searchBar   = new SearchTextBarDb(this,
                                         QLatin1String("ItemIconViewTagSearchBar"),
                                         i18n("Search for tags..."));
    d->searchBar->setHighlightOnResult(true);
    d->searchBar->setModel(d->tagMngrView->filteredModel(),
                           AbstractAlbumModel::AlbumIdRole,
                           AbstractAlbumModel::AlbumTitleRole);
    d->searchBar->setMaximumWidth(200);
    d->searchBar->setFilterModel(d->tagMngrView->albumFilterModel());

    d->setupActions();

    // Tree Widget + Actions + Tag Properties

    d->tagPropWidget = new TagPropWidget(this);
    d->listView      = new TagList(d->tagMngrView, this);

    d->splitter      = new QSplitter(Qt::Horizontal, this);
    d->splitter->addWidget(d->listView);
    d->splitter->addWidget(d->tagMngrView);
    d->splitter->addWidget(d->tagPropWidget);

    connect(d->tagPropWidget, SIGNAL(signalTitleEditReady()),
            this, SLOT(slotTitleEditReady()));

    d->splitter->setStretchFactor(d->splitter->indexOf(d->tagMngrView), 10);

    QWidget* const centralView    = new QWidget(this);
    QHBoxLayout* const mainLayout = new QHBoxLayout(centralView);
    mainLayout->addWidget(d->splitter);
    centralView->setLayout(mainLayout);
    setCentralWidget(centralView);
}

void TagsManager::closeEvent(QCloseEvent* event)
{
    d->listView->saveSettings();
    QMainWindow::closeEvent(event);
}

void TagsManager::showEvent(QShowEvent* event)
{
    if (d->firstShowEvent)
    {
        KConfigGroup group = getConfigGroup();

        DXmlGuiWindow::setGoodDefaultWindowSize(windowHandle());
        DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
        resize(windowHandle()->size());

        d->firstShowEvent = false;
    }

    // Set main window in center of the screen

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = qApp->activeWindow())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    move(screen->geometry().center() - rect().center());

    QMainWindow::showEvent(event);
}

void TagsManager::doLoadState()
{
    KConfigGroup group = getConfigGroup();
    d->tagMngrView->doLoadState();
    group.sync();
}

void TagsManager::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    d->tagMngrView->doSaveState();
    group.sync();
}

} // namespace Digikam
