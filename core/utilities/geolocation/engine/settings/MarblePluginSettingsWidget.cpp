/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-12-21
 * Description : Marble Settings View.
 *
 * SPDX-FileCopyrightText: 2007      by Torsten Rahn <rahn at kde dot org>
 * SPDX-FileCopyrightText: 2008      by Jens-Michael Hoffmann <jensmh at gmx dot de>
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "MarblePluginSettingsWidget.h"

// Qt includes

#include <QIcon>
#include <QList>
#include <QPointer>
#include <QMessageBox>
#include <QGroupBox>
#include <QListView>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QToolButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "DialogConfigurationInterface.h"
#include "ParseRunnerPlugin.h"
#include "PluginItemDelegate.h"
#include "RenderPluginModel.h"

#include "geopluginaboutdlg.h"

using namespace Digikam;

namespace Marble
{

class Q_DECL_HIDDEN MarblePluginSettingsWidget::Private
{
public:

    Private() = default;

public:

    QListView*                           renderListView = nullptr;
    QTreeWidget*                         runnerListView = nullptr;
    PluginItemDelegate*                  itemDelegate   = nullptr;
    QPointer<RenderPluginModel>          renderPluginModel;
    QList<const ParseRunnerPlugin*>      runnerPluginList;
    QMap<QToolButton*, QTreeWidgetItem*> runnerBtnMap;
};

// ---

MarblePluginSettingsWidget::MarblePluginSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    QGroupBox* const grpRender    = new QGroupBox(i18n("Render Tools"), this);
    QVBoxLayout* const vlayRender = new QVBoxLayout(grpRender);
    d->renderListView             = new QListView(grpRender);
    d->itemDelegate               = new PluginItemDelegate(d->renderListView, this);
    d->renderListView->setItemDelegate(d->itemDelegate);
    vlayRender->addWidget(d->renderListView);

    QGroupBox* const grpRunner    = new QGroupBox(i18n("Runner Tools"), this);
    QVBoxLayout* const vlayRunner = new QVBoxLayout(grpRunner);
    d->runnerListView             = new QTreeWidget(grpRunner);
    d->runnerListView->setColumnCount(2);
    d->runnerListView->setRootIsDecorated(false);
    d->runnerListView->setUniformRowHeights(false);
    d->runnerListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->runnerListView->setAllColumnsShowFocus(true);
    d->runnerListView->setSelectionMode(QAbstractItemView::NoSelection);
    d->runnerListView->setFocusPolicy(Qt::NoFocus);
    d->runnerListView->setSortingEnabled(true);
    d->runnerListView->sortByColumn(0, Qt::AscendingOrder);
    d->runnerListView->setIconSize(QSize(16, 16));
    d->runnerListView->setHeaderHidden(true);
    d->runnerListView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    d->runnerListView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    d->runnerListView->header()->setSortIndicatorShown(false);

    vlayRunner->addWidget(d->runnerListView);

    QVBoxLayout* const vlay = new QVBoxLayout(this);
    vlay->addWidget(grpRender);
    vlay->addWidget(grpRunner);

    connect(d->itemDelegate, SIGNAL(aboutPluginClicked(QModelIndex)),
            this, SLOT(slotAboutRenderPlugin(QModelIndex)));

    connect(d->itemDelegate, SIGNAL(configPluginClicked(QModelIndex)),
            this, SLOT(slotRenderPluginConfigDialog(QModelIndex)));
}

MarblePluginSettingsWidget::~MarblePluginSettingsWidget()
{
    delete d;
}

void MarblePluginSettingsWidget::setAboutIcon(const QIcon& icon)
{
    d->itemDelegate->setAboutIcon(icon);
}

void MarblePluginSettingsWidget::setConfigIcon(const QIcon& icon)
{
    d->itemDelegate->setConfigIcon(icon);
}

void MarblePluginSettingsWidget::setModel(RenderPluginModel* const pluginModel)
{
    if (!d->renderPluginModel.isNull())
    {
        disconnect(d->renderPluginModel.data(), nullptr, this, nullptr);
    }

    d->renderPluginModel = pluginModel;
    d->renderListView->setModel(d->renderPluginModel);

    if (!d->renderPluginModel.isNull())
    {
        connect(d->renderPluginModel.data(), SIGNAL(itemChanged(QStandardItem*)),
                this, SIGNAL(pluginListViewClicked()));
    }
}

void MarblePluginSettingsWidget::setRunnerPlugins(const QList<const ParseRunnerPlugin*>& list)
{
    d->runnerPluginList = list;
    qDebug() << "runner plugins:" << list;

    for (const ParseRunnerPlugin* plug : std::as_const(d->runnerPluginList))
    {
        QTreeWidgetItem* const item = new QTreeWidgetItem(d->runnerListView);
        item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        item->setText(0, plug->name());
        item->setIcon(0, QIcon::fromTheme(QLatin1String("plugins")));
        item->setToolTip(0, plug->description());

        QWidget* const bbox     = new QWidget(d->runnerListView);
        QHBoxLayout* const hlay = new QHBoxLayout(bbox);
        QToolButton* const btn  = new QToolButton(bbox);
        hlay->addStretch();
        hlay->addWidget(btn);
        hlay->setContentsMargins(QMargins(0, 0, 0 ,0));
        btn->setIcon(QIcon::fromTheme(QLatin1String("help-about")));
        btn->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
        d->runnerListView->setItemWidget(item, 1, bbox);
        d->runnerBtnMap.insert(btn, item);

        connect(btn, SIGNAL(released()),
               this, SLOT(slotAboutRunnerPlugin()));
    }
}

void MarblePluginSettingsWidget::slotAboutRenderPlugin(const QModelIndex& index)
{
    if (d->renderPluginModel.isNull())
    {
        return;
    }

    QPointer<GeoPluginAboutDlg> dlg = new GeoPluginAboutDlg(d->renderPluginModel->pluginIface(index), this);
    dlg->exec();
    delete dlg;
}

void MarblePluginSettingsWidget::slotAboutRunnerPlugin()
{
    QToolButton* const btn = dynamic_cast<QToolButton*>(sender());

    if (!btn)
    {
        return;
    }

    QTreeWidgetItem* const item = d->runnerBtnMap.value(btn);

    if (!item)
    {
        return;
    }

    for (const ParseRunnerPlugin* plug : std::as_const(d->runnerPluginList))
    {
        if (plug->name() == item->text(0))
        {
            QPointer<GeoPluginAboutDlg> dlg = new GeoPluginAboutDlg(qobject_cast<PluginInterface*>(const_cast<ParseRunnerPlugin*>(plug)), this);
            dlg->exec();
            delete dlg;

            break;
        }
    }
}

void MarblePluginSettingsWidget::slotRenderPluginConfigDialog(const QModelIndex& index)
{
    if (d->renderPluginModel.isNull())
    {
        return;
    }

    DialogConfigurationInterface* const configInterface = d->renderPluginModel->pluginDialogConfigurationInterface(index);
    QDialog* const configDialog                         = configInterface ? configInterface->configDialog() : nullptr;

    if (configDialog)
    {
        configDialog->exec();
    }
}

} // namespace Marble

#include "moc_MarblePluginSettingsWidget.cpp"
