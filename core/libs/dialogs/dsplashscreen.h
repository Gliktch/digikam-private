/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : a widget to display splash with progress bar
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPainter>
#include <QSplashScreen>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DSplashScreen : public QSplashScreen
{
    Q_OBJECT

public:

    DSplashScreen();
    ~DSplashScreen()             override;

    void setColor(const QColor& color);
    void setMessage(const QString& message);

protected:

    void drawContents(QPainter*) override;

private Q_SLOTS:

    void slotAnimate();

private:

    /// @note disabled
    explicit DSplashScreen(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
