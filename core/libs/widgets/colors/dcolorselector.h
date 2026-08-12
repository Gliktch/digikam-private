/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-20
 * Description : color selector widget
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QColor>
#include <QPushButton>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * A widget to choose a color from a palette.
 */
class DIGIKAM_EXPORT DColorSelector : public QPushButton
{
    Q_OBJECT

public:

    explicit DColorSelector(QWidget* const parent = nullptr);
    ~DColorSelector()               override;

    void setColor(const QColor& color);
    QColor color() const;

    void setAlphaChannelEnabled(bool);

Q_SIGNALS:

    void signalColorSelected(const QColor&);

private Q_SLOTS:

    void slotBtnClicked();

private:

    void paintEvent(QPaintEvent*)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
