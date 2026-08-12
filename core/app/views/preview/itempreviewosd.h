/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-12-25
 * Description : On Screen Display preview overlay.
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iteminfo.h"
#include "previewosdsettings.h"

namespace Digikam
{

class ItemPreviewOsd : public QWidget
{
    Q_OBJECT

public:

    explicit ItemPreviewOsd(PreviewOsdSettings* const settings, QWidget* const parent);
    ~ItemPreviewOsd()                                override;

    void setItemInfo(const ItemInfo& inf);
    void setOsdEnabled(bool val);

private:

    void paintEvent(QPaintEvent*)                    override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
