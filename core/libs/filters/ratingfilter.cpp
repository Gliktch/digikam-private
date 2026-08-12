/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-09
 * Description : a widget to filter album contents by rating
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ratingfilter.h"

// Qt includes

#include <QLayout>
#include <QToolButton>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dcursortracker.h"
#include "thememanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN RatingFilterWidget::Private
{
public:

    Private() = default;

public:

    bool                                dirty           = false;

    DCursorTracker*                     ratingTracker   = nullptr;

    ItemFilterSettings::RatingCondition filterCond      = ItemFilterSettings::GreaterEqualCondition;
    bool                                excludeUnrated  = false;
};

RatingFilterWidget::RatingFilterWidget(QWidget* const parent)
    : RatingWidget(parent),
      d           (new Private)
{
    d->ratingTracker = new DCursorTracker(QLatin1String(""), this);
    updateRatingTooltip();
    setMouseTracking(true);

    setWhatsThis(i18n("Select the rating value used to filter "
                      "albums' contents. Use the context pop-up menu to "
                      "set rating filter conditions."));

    // To dispatch signal from parent widget with filter condition.

    connect(this, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotRatingChanged()));
}

RatingFilterWidget::~RatingFilterWidget()
{
    delete d;
}

void RatingFilterWidget::slotRatingChanged()
{
    Q_EMIT signalRatingFilterChanged(rating(), d->filterCond, d->excludeUnrated);
}

void RatingFilterWidget::setRatingFilterCondition(ItemFilterSettings::RatingCondition cond)
{
    d->filterCond = cond;
    updateRatingTooltip();
    slotRatingChanged();
}

ItemFilterSettings::RatingCondition RatingFilterWidget::ratingFilterCondition()
{
    return d->filterCond;
}

void RatingFilterWidget::setExcludeUnratedItems(bool excluded)
{
    d->excludeUnrated = excluded;
    slotRatingChanged();
}

bool RatingFilterWidget::isUnratedItemsExcluded()
{
    return d->excludeUnrated;
}

void RatingFilterWidget::mouseMoveEvent(QMouseEvent* e)
{
    // This method have been re-implemented to display and update the famous TipTracker contents.

    if ( d->dirty )
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        int pos = e->position().toPoint().x() / regPixmapWidth() + 1;

#else

        int pos = e->x() / regPixmapWidth() + 1;

#endif

        if (rating() != pos)
        {
            setRating(pos);
        }

        updateRatingTooltip();
    }
}

void RatingFilterWidget::mousePressEvent(QMouseEvent* e)
{
    // This method must be re-implemented to handle which mouse button is pressed
    // and show the rating filter settings pop-up menu with right mouse button.
    // NOTE: Left and Middle Mouse buttons continue to change rating filter value.

    d->dirty = false;

    if ((e->button() == Qt::LeftButton) || (e->button() == Qt::MiddleButton))
    {
        d->dirty = true;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        int pos  = e->position().toPoint().x() / regPixmapWidth() + 1;

#else

        int pos  = e->x() / regPixmapWidth() + 1;

#endif

        if (rating() == pos)
        {
            setRating(rating()-1);
        }
        else
        {
            setRating(pos);
        }

        updateRatingTooltip();
        slotRatingChanged();
    }
}

void RatingFilterWidget::mouseReleaseEvent(QMouseEvent*)
{
    d->dirty = false;
}

void RatingFilterWidget::updateRatingTooltip()
{
    // Adapt tip message with rating filter condition settings.

    switch (d->filterCond)
    {
        case ItemFilterSettings::GreaterEqualCondition:
        {
            d->ratingTracker->setText(i18n("Rating greater than or equal to %1.", rating()));
            break;
        }

        case ItemFilterSettings::EqualCondition:
        {
            d->ratingTracker->setText(i18n("Rating equal to %1.", rating()));
            break;
        }

        case ItemFilterSettings::LessEqualCondition:
        {
            d->ratingTracker->setText( i18n("Rating less than or equal to %1.", rating()));
            break;
        }

        default:
        {
            break;
        }
    }
}

// -----------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN RatingFilter::Private
{
public:

    Private() = default;

    QToolButton*        optionsBtn      = nullptr;
    QToolButton*        noRatingBtn     = nullptr;

    QAction*            geCondAction    = nullptr;
    QAction*            eqCondAction    = nullptr;
    QAction*            leCondAction    = nullptr;
    QAction*            excludeUnrated  = nullptr;

    QMenu*              optionsMenu     = nullptr;

    RatingFilterWidget* ratingWidget    = nullptr;
};

RatingFilter::RatingFilter(QWidget* const parent)
    : DHBox(parent),
      d    (new Private)
{
    d->ratingWidget = new RatingFilterWidget(this);

    d->noRatingBtn  = new QToolButton(this);
    d->noRatingBtn->setCheckable(true);
    d->noRatingBtn->setToolTip( i18n("Items Without Rating"));
    d->noRatingBtn->setIcon(QIcon::fromTheme(QLatin1String("rating-unrated")));

    d->optionsBtn   = new QToolButton(this);
    d->optionsBtn->setToolTip( i18n("Rating Filter Options"));
    d->optionsBtn->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->optionsBtn->setPopupMode(QToolButton::InstantPopup);

    d->optionsMenu  = new QMenu(d->optionsBtn);
    d->geCondAction = d->optionsMenu->addAction(i18n("Greater Than or Equals Condition"));
    d->geCondAction->setCheckable(true);
    d->eqCondAction = d->optionsMenu->addAction(i18n("Equals Condition"));
    d->eqCondAction->setCheckable(true);
    d->leCondAction = d->optionsMenu->addAction(i18n("Less Than or Equals Condition"));
    d->leCondAction->setCheckable(true);
    d->optionsMenu->addSeparator();
    d->excludeUnrated = d->optionsMenu->addAction(i18n("Exclude Items Without Rating"));
    d->excludeUnrated->setCheckable(true);
    d->optionsBtn->setMenu(d->optionsMenu);

    layout()->setAlignment(d->ratingWidget, Qt::AlignVCenter | Qt::AlignRight);

    setContentsMargins(QMargins());
    setSpacing(0);

    connect(d->optionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOptionsTriggered(QAction*)));

    connect(d->optionsMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotOptionsMenu()));

    connect(d->noRatingBtn, SIGNAL(toggled(bool)),
            this, SLOT(slotNoRatingToggled(bool)));

    connect(d->ratingWidget, SIGNAL(signalRatingFilterChanged(int,ItemFilterSettings::RatingCondition,bool)),
            this, SLOT(slotRatingFilterChanged(int,ItemFilterSettings::RatingCondition,bool)));

    connect(d->ratingWidget, SIGNAL(signalRatingFilterChanged(int,ItemFilterSettings::RatingCondition,bool)),
            this, SIGNAL(signalRatingFilterChanged(int,ItemFilterSettings::RatingCondition,bool)));
}

RatingFilter::~RatingFilter()
{
    delete d;
}

void RatingFilter::setRatingFilterCondition(ItemFilterSettings::RatingCondition cond)
{
    d->ratingWidget->setRatingFilterCondition(cond);
}

ItemFilterSettings::RatingCondition RatingFilter::ratingFilterCondition()
{
    return d->ratingWidget->ratingFilterCondition();
}

void RatingFilter::setExcludeUnratedItems(bool excluded)
{
    d->ratingWidget->setExcludeUnratedItems(excluded);
    d->excludeUnrated->setChecked(excluded);
}

bool RatingFilter::isUnratedItemsExcluded()
{
    return d->ratingWidget->isUnratedItemsExcluded();
}

void RatingFilter::slotOptionsMenu()
{
    d->geCondAction->setChecked(false);
    d->eqCondAction->setChecked(false);
    d->leCondAction->setChecked(false);

    switch (ratingFilterCondition())
    {
        case ItemFilterSettings::GreaterEqualCondition:
        {
            d->geCondAction->setChecked(true);
            break;
        }

        case ItemFilterSettings::EqualCondition:
        {
            d->eqCondAction->setChecked(true);
            break;
        }

        case ItemFilterSettings::LessEqualCondition:
        {
            d->leCondAction->setChecked(true);
            break;
        }
    }
}

void RatingFilter::slotOptionsTriggered(QAction* action)
{
    if (action)
    {
        if      (action == d->geCondAction)
        {
            setRatingFilterCondition(ItemFilterSettings::GreaterEqualCondition);
        }
        else if (action == d->eqCondAction)
        {
            setRatingFilterCondition(ItemFilterSettings::EqualCondition);
        }
        else if (action == d->leCondAction)
        {
            setRatingFilterCondition(ItemFilterSettings::LessEqualCondition);
        }
        else if (action == d->excludeUnrated)
        {
            setExcludeUnratedItems(d->excludeUnrated->isChecked());
        }
    }
}

void RatingFilter::slotNoRatingToggled(bool checked)
{
    if (checked)
    {
        d->ratingWidget->blockSignals(true);
        d->ratingWidget->setRating(0);
        d->ratingWidget->blockSignals(false);

        Q_EMIT signalRatingFilterChanged(0, ItemFilterSettings::LessEqualCondition, false);
    }
    else
    {
        Q_EMIT signalRatingFilterChanged(0, ratingFilterCondition(), isUnratedItemsExcluded());
    }
}

void RatingFilter::slotRatingFilterChanged(int, ItemFilterSettings::RatingCondition, bool)
{
    d->noRatingBtn->blockSignals(true);
    d->noRatingBtn->setChecked(false);
    d->noRatingBtn->blockSignals(false);
}

void RatingFilter::setRating(int val)
{
    d->ratingWidget->setRating(val);
}

int RatingFilter::rating() const
{
    return d->ratingWidget->rating();
}

void RatingFilter::reset()
{
    setRating(0);
    setExcludeUnratedItems(false);
    setRatingFilterCondition(ItemFilterSettings::GreaterEqualCondition);
}

} // namespace Digikam

#include "moc_ratingfilter.cpp"
