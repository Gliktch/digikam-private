/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to indicate icon-view filters status
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filterstatusbar.h"

// Qt includes

#include <QToolButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN FilterStatusBar::Private
{
public:

    enum FilterStatus
    {
        None = 0,
        Match,
        NotMatch
    };

public:

    Private() = default;

public:

    int               status        = None;

    DAdjustableLabel* info          = nullptr;
    QToolButton*      resetBtn      = nullptr;
    QToolButton*      settingsBtn   = nullptr;

    ItemFilterSettings settings;
};

FilterStatusBar::FilterStatusBar(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    QHBoxLayout* const vlay = new QHBoxLayout(this);

    d->info                 = new DAdjustableLabel(this);
    d->info->setContextMenuPolicy(Qt::NoContextMenu);
    d->info->setAutoFillBackground(true);
    d->info->setFocusPolicy(Qt::NoFocus);
    d->info->setWhatsThis(i18n("Background color indicates the global image filter status, "
                               "encompassing all filter settings from the right sidebar.\n\n"
                               "NO COLOR: no filter is active, all items are visible.\n"
                               "RED: filtering is on, but no items match.\n"
                               "GREEN: filter(s) match(es) at least one item.\n\n"
                               "Move mouse cursor over this text to see more details about active filters.\n"
                               "Press the Reset button from the right side to clear all filter settings.\n"
                               "Press the Settings button from the right side to open the filters panel."));

    d->resetBtn    = new QToolButton(this);
    d->resetBtn->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->resetBtn->setToolTip(i18n("Reset all active filters"));
    d->resetBtn->setFocusPolicy(Qt::NoFocus);
    d->resetBtn->setAutoRaise(true);

    d->settingsBtn = new QToolButton(this);
    d->settingsBtn->setIcon(QIcon::fromTheme(QLatin1String("view-filter")));
    d->settingsBtn->setToolTip(i18n("Open filter settings panel"));
    d->settingsBtn->setFocusPolicy(Qt::NoFocus);
    d->settingsBtn->setAutoRaise(true);

    vlay->addWidget(d->info);
    vlay->addWidget(d->resetBtn);
    vlay->addWidget(d->settingsBtn);
    vlay->setSpacing(layoutSpacing());

    vlay->setContentsMargins(QMargins());
    vlay->setStretchFactor(d->info, 10);

    connect(d->resetBtn, SIGNAL(released()),
            this, SIGNAL(signalResetFilters()));

    connect(d->settingsBtn, SIGNAL(released()),
            this, SIGNAL(signalPopupFiltersView()));
}

FilterStatusBar::~FilterStatusBar()
{
    delete d;
}

void FilterStatusBar::slotFilterMatches(bool match)
{
    QStringList filtersList;
    QString     message;

    if (d->settings.isFilteringByText())
    {
        filtersList.append(i18n("<br><nobr><i>Text</i></nobr>"));
    }

    if (d->settings.isFilteringByTypeMime())
    {
        filtersList.append(i18n("<br><nobr><i>Mime Type</i></nobr>"));
    }

    if (d->settings.isFilteringByGeolocation())
    {
        filtersList.append(i18n("<br><nobr><i>Geolocation</i></nobr>"));
    }

    if (d->settings.isFilteringByRating())
    {
        filtersList.append(i18n("<br><nobr><i>Rating</i></nobr>"));
    }

    if (d->settings.isFilteringByPickLabels())
    {
        filtersList.append(i18n("<br><nobr><i>Pick Labels</i></nobr>"));
    }

    if (d->settings.isFilteringByColorLabels())
    {
        filtersList.append(i18n("<br><nobr><i>Color Labels</i></nobr>"));
    }

    if (d->settings.isFilteringByTags())
    {
        filtersList.append(i18n("<br><nobr><i>Tags</i></nobr>"));
    }

    if (filtersList.count() > 1)
    {
        message = i18n("<nobr><b>Active filters:</b></nobr>");
    }
    else
    {
        message = i18n("<nobr><b>Active filter:</b></nobr>");
    }

    message.append(filtersList.join(QString()));

    if (filtersList.isEmpty())
    {
        d->info->setAdjustedText(i18n("No active filter"));
        d->info->setToolTip(QString());
        d->resetBtn->setEnabled(false);
        d->status = Private::None;
    }
    else
    {
        if (filtersList.count() == 1)
        {
            d->info->setAdjustedText(i18n("One active filter"));
        }
        else
        {
            d->info->setAdjustedText(i18np("1 active filter", "%1 active filters", filtersList.count()));
        }

        d->info->setToolTip(message);
        d->resetBtn->setEnabled(true);
        d->status = match ? Private::Match : Private::NotMatch;
    }

    QPalette pal = palette();

    switch (d->status)
    {
        case Private::NotMatch:
        {
            pal.setColor(backgroundRole(), QColor(220, 140, 140));
            pal.setColor(foregroundRole(), Qt::black);
            break;
        }

        case Private::Match:
        {
            pal.setColor(backgroundRole(), QColor(140, 220, 140));
            pal.setColor(foregroundRole(), Qt::black);
            break;
        }

        default: // Private::None
        {
            break;
        }
    }

    d->info->setPalette(pal);

    update();
}

void FilterStatusBar::slotFilterSettingsChanged(const ItemFilterSettings& settings)
{
    d->settings = settings;
}

} // namespace Digikam

#include "moc_filterstatusbar.cpp"
