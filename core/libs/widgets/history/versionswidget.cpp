/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-03
 * Description : widget displaying all image versions in a list
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "versionswidget.h"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QToolButton>
#include <QTreeView>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "dimagehistory.h"
#include "itemhistorygraphmodel.h"
#include "iteminfo.h"
#include "itemlistmodel.h"
#include "thumbnailloadthread.h"
#include "versionsdelegate.h"
#include "versionmanagersettings.h"
#include "versionsoverlays.h"
#include "versionstreeview.h"

namespace Digikam
{

class Q_DECL_HIDDEN VersionsWidget::Private
{
public:

    Private() = default;

    VersionsTreeView*        view               = nullptr;
    ItemHistoryGraphModel*   model              = nullptr;
    VersionsDelegate*        delegate           = nullptr;

    ShowHideVersionsOverlay* showHideOverlay    = nullptr;

    QButtonGroup*            viewButtonGroup    = nullptr;
    QToolButton*             listModeButton     = nullptr;
    QToolButton*             treeModeButton     = nullptr;
    QToolButton*             combinedModeButton = nullptr;

    const QString configCurrentMode             = QLatin1String("Version Properties View Mode");
};

VersionsWidget::VersionsWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    QGridLayout* const layout = new QGridLayout;

    d->viewButtonGroup        = new QButtonGroup(this);
    d->listModeButton         = new QToolButton;
    d->listModeButton->setIcon(QIcon::fromTheme(QLatin1String("view-list-icons")));
    d->listModeButton->setCheckable(true);
    d->listModeButton->setToolTip(i18n("Show available versions in a list"));
    d->viewButtonGroup->addButton(d->listModeButton, ItemHistoryGraphModel::ImagesListMode);

    d->treeModeButton         = new QToolButton;
    d->treeModeButton->setIcon(QIcon::fromTheme(QLatin1String("view-list-tree")));
    d->treeModeButton->setCheckable(true);
    d->treeModeButton->setToolTip(i18n("Show available versions as a tree"));
    d->viewButtonGroup->addButton(d->treeModeButton, ItemHistoryGraphModel::ImagesTreeMode);

    d->combinedModeButton     = new QToolButton;
    d->combinedModeButton->setIcon(QIcon::fromTheme(QLatin1String("view-list-details")));
    d->combinedModeButton->setCheckable(true);
    d->combinedModeButton->setToolTip(i18n("Show available versions and the applied filters in a combined list"));
    d->viewButtonGroup->addButton(d->combinedModeButton, ItemHistoryGraphModel::CombinedTreeMode);

    QHBoxLayout* const buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(d->listModeButton);
    buttonLayout->addWidget(d->treeModeButton);
    buttonLayout->addWidget(d->combinedModeButton);

    d->model                = new ItemHistoryGraphModel(this);
    d->model->imageModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());

    d->view                 = new VersionsTreeView;
    d->view->setModel(d->model);
    d->view->setWordWrap(true);
    d->view->setRootIsDecorated(false);
    d->view->setHeaderHidden(true);
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    //d->view->setFrameShape(QFrame::NoFrame);
    d->view->setFrameShadow(QFrame::Plain);

    layout->addLayout(buttonLayout, 0, 1);
    layout->addWidget(d->view,      1, 0, 1, 2);
    layout->setColumnStretch(0, 1);
    layout->setRowStretch(1, 1);
    setLayout(layout);

    connect(d->view->delegate(), SIGNAL(animationStateChanged()),
            d->view->viewport(), SLOT(update()));

    connect(d->view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotViewCurrentChanged(QModelIndex,QModelIndex)));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->viewButtonGroup, SIGNAL(idClicked(int)),
            this, SLOT(slotViewModeChanged(int)));

#else

    connect(d->viewButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotViewModeChanged(int)));

#endif

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

VersionsWidget::~VersionsWidget()
{
    delete d;
}

void VersionsWidget::readSettings(const KConfigGroup& group)
{
    int mode = group.readEntry(d->configCurrentMode, (int)ItemHistoryGraphModel::CombinedTreeMode);

    switch (mode)
    {
        case ItemHistoryGraphModel::ImagesListMode:
        {
            d->listModeButton->setChecked(true);
            break;
        }

        case ItemHistoryGraphModel::ImagesTreeMode:
        {
            d->treeModeButton->setChecked(true);
            break;
        }

        case ItemHistoryGraphModel::CombinedTreeMode:
        default:
        {
            d->combinedModeButton->setChecked(true);
            break;
        }
    }

    slotViewModeChanged(mode);
}

VersionsTreeView* VersionsWidget::view() const
{
    return d->view;
}

VersionsDelegate* VersionsWidget::delegate() const
{
    return d->delegate;
}

ActionVersionsOverlay* VersionsWidget::addActionOverlay(const QIcon& icon, const QString& text, const QString& tip)
{
    ActionVersionsOverlay* const overlay = new ActionVersionsOverlay(this, icon, text, tip);
    d->view->addOverlay(overlay);

    return overlay;
}

ShowHideVersionsOverlay* VersionsWidget::addShowHideOverlay()
{
    d->showHideOverlay = new ShowHideVersionsOverlay(this);
    d->showHideOverlay->setSettings(ApplicationSettings::instance()->getVersionManagerSettings());
    d->view->addOverlay(d->showHideOverlay);

    return d->showHideOverlay;
}

void VersionsWidget::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configCurrentMode, d->viewButtonGroup->checkedId());
}

void VersionsWidget::setCurrentItem(const ItemInfo& info)
{
    d->model->setHistory(info);
    applyViewMode();
}

void VersionsWidget::slotViewCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    ItemInfo info = d->model->imageInfo(current);

    if (!info.isNull())
    {
        Q_EMIT imageSelected(info);
    }

    switch (d->model->mode())
    {
        case ItemHistoryGraphModel::ImagesListMode:
        case ItemHistoryGraphModel::ImagesTreeMode:
        {
            break;
        }

        case ItemHistoryGraphModel::CombinedTreeMode:
        {
            // toplevel image index

            if (!info.isNull() && !current.parent().isValid())
            {
                d->view->expand(current);
            }

            if (previous.isValid() && d->model->isImage(previous) && !previous.parent().isValid())
            {
                d->view->collapse(previous);
            }

            break;
        }
    }
}

void VersionsWidget::slotViewModeChanged(int mode)
{
    d->model->setMode((ItemHistoryGraphModel::Mode)mode);
    applyViewMode();
}

void VersionsWidget::applyViewMode()
{
    switch (d->model->mode())
    {
        case ItemHistoryGraphModel::ImagesListMode:
        {
            break;
        }

        case ItemHistoryGraphModel::ImagesTreeMode:
        {
            d->view->expandAll();
            break;
        }

        case ItemHistoryGraphModel::CombinedTreeMode:
        {
            d->view->collapseAll();
            break;
        }
    }

    QModelIndex subjectIndex = d->model->indexForInfo(d->model->subject());
    d->view->scrollTo(subjectIndex, QAbstractItemView::PositionAtCenter);
    d->view->setCurrentIndex(subjectIndex);
}

void VersionsWidget::slotSetupChanged()
{
    d->view->setToolTipEnabled(ApplicationSettings::instance()->showToolTipsIsValid());

    if (d->showHideOverlay)
    {
        d->showHideOverlay->setSettings(ApplicationSettings::instance()->getVersionManagerSettings());
    }
}

} // namespace Digikam

#include "moc_versionswidget.cpp"
