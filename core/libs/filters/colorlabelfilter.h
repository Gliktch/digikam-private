/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-09
 * Description : color label filter
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

#include "colorlabelwidget.h"

namespace Digikam
{

class TAlbum;

class ColorLabelFilter : public ColorLabelWidget
{
    Q_OBJECT

public:

    explicit ColorLabelFilter(QWidget* const parent = nullptr);
    ~ColorLabelFilter() override = default;

    QList<TAlbum*> getCheckedColorLabelTags();

    void reset();

Q_SIGNALS:

    void signalColorLabelSelectionChanged(const QList<ColorLabel>&);

private Q_SLOTS:

    void slotColorLabelSelectionChanged();
};

} // namespace Digikam
