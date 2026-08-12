/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-19
 * Description : slide properties widget
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QString>
#include <QColor>
#include <QUrl>

// Local includes

#include "slideshowsettings.h"

namespace DigikamGenericSlideShowPlugin
{

class SlideProperties : public QWidget
{
    Q_OBJECT

public:

    explicit SlideProperties(SlideShowSettings* const settings, QWidget* const parent);
    ~SlideProperties()              override;

    void setCurrentUrl(const QUrl& url);

private:

    void paintEvent(QPaintEvent*)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericSlideShowPlugin
