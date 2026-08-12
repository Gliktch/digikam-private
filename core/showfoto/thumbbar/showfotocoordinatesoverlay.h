/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-28
 * Description : overlay for GPS location indicator
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"
#include "showfotoitemviewdelegate.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoCoordinatesOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit ShowfotoCoordinatesOverlayWidget(QWidget* const parent = nullptr);

protected:

    void paintEvent(QPaintEvent*)                          override;
};

// ----------------------------------------------------------------------

class ShowfotoCoordinatesOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ShowfotoItemViewDelegate)

public:

    explicit ShowfotoCoordinatesOverlay(QObject* const parent);
    ShowfotoCoordinatesOverlayWidget* buttonWidget() const;

protected:

    void updatePosition();

    QWidget* createWidget()                                override;
    void setActive(bool active)                            override;
    void visualChange()                                    override;
    bool checkIndex(const QModelIndex& index)        const override;
    void slotEntered(const QModelIndex& index)             override;

protected:

    QPersistentModelIndex m_index;
};

} // namespace ShowFoto
