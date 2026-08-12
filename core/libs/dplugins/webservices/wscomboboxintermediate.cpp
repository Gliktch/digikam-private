/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : A combobox which also has an intermediate state.
 *
 * SPDX-FileCopyrightText: 2009      by Pieter Edelman <pieter dot edelman at gmx dot net>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wscomboboxintermediate.h"

// Qt includes

#include <QVariant>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN WSComboBoxIntermediate::Private
{
public:

    Private() = default;

    bool    isIntermediate = false;
    QString intermediateText;
};

WSComboBoxIntermediate::WSComboBoxIntermediate(QWidget* const parent, const QString& text)
    : QComboBox(parent),
      d        (new Private)
{
    d->intermediateText = text.isNull() ? i18n("Various") : text;

    // Whenever the signal changes, there's a chance that the combobox should
    // be changed from intermediate to normal.

    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged(int)));
}

WSComboBoxIntermediate::~WSComboBoxIntermediate()
{
    delete d;
}

void WSComboBoxIntermediate::setIntermediate(bool state)
{
    if      ((state) && (!d->isIntermediate))
    {
        // If the combobox should be set to intermediate and is not yet done so,
        // append a separator and the intermediate text.

        insertSeparator(count());
        addItem(d->intermediateText, QVariant(-1));

        // Set the combobox to the intermediate index, while avoiding that it is
        // directly unset by the currentIndexChanged signal.

        blockSignals(true);
        setCurrentIndex(count() - 1);
        blockSignals(false);

        d->isIntermediate = true;
    }
    else if ((!state) && (d->isIntermediate))
    {
        // If the intermediate state should be removed, simply remove the latest
        // two items, the intermediate text and the separator.

        removeItem(count() - 1);
        removeItem(count() - 1);

        d->isIntermediate = false;
    }
}

void WSComboBoxIntermediate::slotIndexChanged(int)
{
    if (d->isIntermediate)
    {
        setIntermediate(false);
    }
}

} // namespace Digikam

#include "moc_wscomboboxintermediate.cpp"
