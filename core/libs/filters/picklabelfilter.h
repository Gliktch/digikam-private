/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-16
 * Description : pick label filter
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QWidget>

// Local includes

#include "picklabelwidget.h"

namespace Digikam
{

class TAlbum;

class PickLabelFilter : public PickLabelWidget
{
    Q_OBJECT

public:

    explicit PickLabelFilter(QWidget* const parent = nullptr);
    ~PickLabelFilter() override = default;

    QList<TAlbum*> getCheckedPickLabelTags();

    void reset();

Q_SIGNALS:

    void signalPickLabelSelectionChanged(const QList<PickLabel>&);

private Q_SLOTS:

    void slotPickLabelSelectionChanged();
};

} // namespace Digikam
