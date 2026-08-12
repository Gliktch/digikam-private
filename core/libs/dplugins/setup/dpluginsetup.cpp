/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Config panel for generic dplugins.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginsetup.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dpluginconfview.h"
#include "dpluginaboutdlg.h"
#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginSetup::Private
{
public:

    Private() = default;

    QLabel*          pluginsNumber              = nullptr;
    QLabel*          pluginsNumberActivated     = nullptr;

    QPushButton*     checkAllBtn                = nullptr;
    QPushButton*     clearBtn                   = nullptr;

    QGridLayout*     grid                       = nullptr;
    QWidget*         hbox                       = nullptr;

    SearchTextBar*   pluginFilter               = nullptr;
    DPluginConfView* pluginsList                = nullptr;
};

DPluginSetup::DPluginSetup(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing         = layoutSpacing();


    d->grid                   = new QGridLayout(this);
    d->pluginFilter           = new SearchTextBar(this, QLatin1String("PluginsSearchBar"));
    d->pluginsNumber          = new QLabel(this);
    d->pluginsNumberActivated = new QLabel(this);
    d->checkAllBtn            = new QPushButton(i18nc("@action: search bar", "Check All"), this);
    d->clearBtn               = new QPushButton(i18nc("@action: search bar", "Clear"),     this);

    d->grid->addWidget(d->pluginFilter,           0, 0, 1, 1);
    d->grid->addWidget(d->pluginsNumber,          0, 1, 1, 1);
    d->grid->addWidget(d->pluginsNumberActivated, 0, 2, 1, 1);
    d->grid->addWidget(d->checkAllBtn,            0, 4, 1, 1);
    d->grid->addWidget(d->clearBtn,               0, 5, 1, 1);
    d->grid->setColumnStretch(3, 10);
    d->grid->setRowStretch(1, 10);
    d->grid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->checkAllBtn, SIGNAL(clicked()),
            this, SLOT(slotCheckAll()));

    connect(d->clearBtn, SIGNAL(clicked()),
            this, SLOT(slotClearList()));

    connect(d->pluginFilter, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));
}

DPluginSetup::~DPluginSetup()
{
    delete d;
}

void DPluginSetup::setPluginConfView(DPluginConfView* const view)
{
    d->pluginsList = view;
    d->grid->addWidget(d->pluginsList, 1, 0, 1, 6);

    connect(d->pluginsList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemClicked()));

    connect(d->pluginsList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotAboutPlugin(QTreeWidgetItem*,int)));

    connect(d->pluginsList, SIGNAL(signalSearchResult(int)),
            this, SLOT(slotSearchResult(int)));

    updateInfo();
}

void DPluginSetup::applySettings()
{
    d->pluginsList->apply();
}

void DPluginSetup::slotAboutPlugin(QTreeWidgetItem* item, int)
{
    if (!item)
    {
        return;
    }

    QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(d->pluginsList->plugin(item));
    dialogExec(dlg);

    delete dlg;
}

void DPluginSetup::slotSearchTextChanged(const SearchTextSettings& settings)
{
    d->pluginsList->setFilter(settings.text, settings.caseSensitive);
}

void DPluginSetup::updateInfo()
{
    bool b = (d->pluginsList->itemsWithVisiblyProperty() != 0);
    d->pluginsNumberActivated->setVisible(b);
    d->checkAllBtn->setVisible(b);
    d->clearBtn->setVisible(b);

    if (d->pluginFilter->text().isEmpty())
    {
        // List is not filtered

        int cnt = d->pluginsList->count();

        if (cnt > 0)
        {
            d->pluginsNumber->setText(i18ncp("@info", "1 plugin installed", "%1 plugins installed", cnt));
        }
        else
        {
            d->pluginsNumber->setText(i18nc("@info", "No plugin installed"));
        }

        int act = d->pluginsList->activated();

        if (act > 0)
        {
            d->pluginsNumberActivated->setText(i18ncp("@info: %1: number of plugins activated", "(%1 activated)", "(%1 activated)", act));
        }
        else
        {
            d->pluginsNumberActivated->setText(QString());
        }
    }
    else
    {
        // List filtering is active

        int cnt = d->pluginsList->itemsVisible();

        if (cnt > 0)
        {
            d->pluginsNumber->setText(i18ncp("@info", "1 plugin found", "%1 plugins found", cnt));
        }
        else
        {
            d->pluginsNumber->setText(i18nc("@info", "No plugin found"));
        }

        d->pluginsNumberActivated->setText(QString());
    }
}

void DPluginSetup::slotCheckAll()
{
    d->pluginsList->selectAll();
    updateInfo();
}

void DPluginSetup::slotClearList()
{
    d->pluginsList->clearAll();
    updateInfo();
}

void DPluginSetup::slotItemClicked()
{
    updateInfo();
}

void DPluginSetup::slotSetFilter(const QString& filter, Qt::CaseSensitivity cs)
{
    d->pluginsList->setFilter(filter, cs);
    updateInfo();
}

void DPluginSetup::slotSearchResult(int found)
{
    d->pluginFilter->slotSearchResult(found ? true : false);
}

} // namespace Digikam

#include "moc_dpluginsetup.cpp"
