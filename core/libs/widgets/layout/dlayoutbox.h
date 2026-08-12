/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Vertical and horizontal layout widget helpers.
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QFrame>
#include <QSize>
#include <QMargins>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * An Horizontal widget to host children widgets
 */
class DIGIKAM_EXPORT DHBox : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(DHBox)

public:

    explicit DHBox(QWidget* const parent = nullptr);
    ~DHBox()                          override = default;

    void setSpacing(int space);
    void setContentsMargins(const QMargins& margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    void setStretchFactor(QWidget* const widget, int stretch);

    QSize sizeHint()            const override;
    QSize minimumSizeHint()     const override;

protected:

    DHBox(bool vertical, QWidget* const parent);

    void childEvent(QChildEvent* e)   override;
};

// ------------------------------------------------------------------------------------

/**
 * A Vertical widget to host children widgets
 */
class DIGIKAM_EXPORT DVBox : public DHBox
{
    Q_OBJECT
    Q_DISABLE_COPY(DVBox)

  public:

    explicit DVBox(QWidget* const parent = nullptr);
    ~DVBox()                          override = default;
};

} // namespace Digikam
