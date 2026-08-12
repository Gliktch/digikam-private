/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-22
 * Description : a widget to filter album contents by geolocation
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "geolocationfilter.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

GeolocationFilter::GeolocationFilter(QWidget* const parent)
    : QComboBox(parent)
{
    addItem(i18n("No geo filtering"),           QVariant::fromValue(ItemFilterSettings::GeolocationNoFilter));
    addItem(i18n("Images with coordinates"),    QVariant::fromValue(ItemFilterSettings::GeolocationHasCoordinates));
    addItem(i18n("Images without coordinates"), QVariant::fromValue(ItemFilterSettings::GeolocationNoCoordinates));

    setToolTip(i18n("Filter by geolocation"));
    setWhatsThis(i18n("Select how geolocation should affect the images which are shown."));

    setGeolocationFilter(ItemFilterSettings::GeolocationNoFilter);

    connect(this, SIGNAL(activated(int)),
            this, SLOT(slotFilterChanged()));
}

void GeolocationFilter::setGeolocationFilter(const ItemFilterSettings::GeolocationCondition& condition)
{
    // findData does not seem to work...
/*
    const int newIndex = findData(QVariant::fromValue<ItemFilterSettings::GeolocationCondition>(condition));
*/
    for (int i = 0 ; i < count() ; ++i)
    {
        const ItemFilterSettings::GeolocationCondition currentdata = itemData(i).value<ItemFilterSettings::GeolocationCondition>();

        if (currentdata == condition)
        {
            setCurrentIndex(i);

            Q_EMIT signalFilterChanged(condition);

            break;
        }
    }
}

ItemFilterSettings::GeolocationCondition GeolocationFilter::geolocationFilter() const
{
    return itemData(currentIndex()).value<ItemFilterSettings::GeolocationCondition>();
}

void GeolocationFilter::slotFilterChanged()
{
    Q_EMIT signalFilterChanged(geolocationFilter());
}

} // namespace Digikam

#include "moc_geolocationfilter.cpp"
